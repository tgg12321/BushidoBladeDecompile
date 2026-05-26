# Matching Techniques

This document distills the project's accumulated knowledge about how to coax GCC 2.7.2 (the PsyQ-era cross-compiler) into producing byte-identical assembly for a given target. Most of this material is symptom-indexed: you see something in your build's diff, you look it up, you apply a recipe.

Companion docs:

- [`ARCHITECTURE.md`](ARCHITECTURE.md) for the build pipeline (what regfix/asmfix mean and where they sit).
- [`TOOLS.md`](TOOLS.md) for the `dc.sh` subcommand catalog.
- [`CONTRIBUTING.md`](../CONTRIBUTING.md) for the work-queue lifecycle and PR conventions.

If you've read those and need *more* depth (full named-recipe definitions, regfix syntax minutiae, hundreds of distilled gotchas), the project's internal feedback files in `memory/` (read-only — gitignored as a Claude Code agent artifact) contain another ~2,000 lines of refinement.

## The matching mindset

Matching decomp is not "translate the asm into clean C." It's: **write C that GCC 2.7.2, when run through this project's build pipeline, will compile into byte-identical assembly.**

Practical consequences:

- You're matching the *output*, not writing readable code. Sometimes the matching C is ugly. Sometimes it's downright wrong-looking (negative-index loops, intermediate variables with no semantic purpose, struct locals that exist only to defeat dead-store elimination).
- GCC 2.7.2 has known optimization passes (CSE, LICM, strength reduction, peephole, dead-code elimination, scheduling) that can transform your C into something quite different. Most matching effort is about controlling or defeating those passes.
- When source-level techniques exhaust, the project's `regfix.txt` post-pass lets you patch the resulting assembly. This is normal, not a sign of failure.

The goal of matching is to get to **score = 0 + SHA1 OK** — not score = 0 in isolation. The permuter scores against a synthetic compilation of a standalone `base.c`; the real build sometimes diverges (different externs, different layout). The build SHA1 is authoritative.

## Penalty → technique routing

After your first build attempt, run `bash tools/dc.sh debug <func_dir>` or `bash tools/dc.sh diff-align <func>` and read the penalty list. That tells you what kind of diff you have, which routes the right next step. Use this table:

| Penalty profile | Likely cause | First-line technique |
|---|---|---|
| `Ins: 0, Del: 0, Reord: 0`, only `Reg ≥ 1` | Pure register naming | regfix `swap` rules. Don't run the permuter — there's no structure to find. Don't run `gen-regfix` — it floods on register cycles. Read the diff manually, write swaps. |
| `Ins: 0, Del: 0, Reord ≥ 1`, `Reg` low | Scheduling / instruction reorder | C-level: `do { } while(0);` barriers, intermediate variables, decl reordering. Permuter explores these. Or regfix `reorder` / `fill_delay` / `drain_delay`. |
| `Ins ≥ 1` (target has more instructions than mine) | Target reloads things you've cached in a local | Don't cache inner derefs; let GCC reload naturally. Cache outer pointers; read inner each access. |
| `Del ≥ 1` (mine has more) | Mine reloads things target caches | Cache outer pointers in locals; reduce repeated derefs. |
| `Stack Differences` dominates | Frame-size mismatch | Usually a dead-code-elimination artifact — target reserved an `$s`-reg slot for code GCC DCE'd. 4-5 regfix rules bridge simple cases. `dc.sh frame-shift --apply` for cascade cases. |
| `Reord` high + `Reg` high | Compound — register cycle + scheduling | Permuter (5-10 min). Then regfix for the residual. |
| `Insertions` of `lui+ori` magic constant in preheader | LICM hoisted a constant out of a loop | regfix unhoist recipe (~7 rules). Don't waste tokens on C-level fixes — they all fail. |
| `Insertions` of `move v0,a0; beqz a0` early-exit | Early-exit alias optimization | Auto-detected by `gen-regfix`. Apply the 8-rule recipe. |
| Score plateau 3000+ with 30+ inst label-bounded region | Scheduler divergence at scale | `dc.sh asmfix-slice` (widest viable slice within the function body). Last resort. |
| Single-insn byte diff in MIDDLE of a sibling function (you just matched something else) | Label drift | `dc.sh fix-label-drift`. If linker passes but sibling differs by 1 byte in a branch immediate, `dc.sh fix-label-drift --also-verify`. |
| Function symbol moved (`verify` says MATCH 0 diffs but SHA1 fails) | `regfix reorder @ 0` or `delete @ 0` | Function-label preamble (`func_NAME:`, `.ent`, `.frame`) attaches to first instruction; reorder moves them. Fix by using two substs that swap the *content* of idx 0 and idx N. |

If none of those fit, your diff is structurally complex enough to need the full toolbox in escalation order — see [`CONTRIBUTING.md`](../CONTRIBUTING.md) "Escalation ladder."

## C-side techniques

### Register allocation (declaration order)

GCC 2.7.2 assigns `$s0`–`$s7` to local variables in **declaration order**. To match a target that uses `$s2` for variable X and `$s4` for variable Y, declare X before Y. This is the cheapest technique — usually try this before `register T x asm("$sN")` pins.

### Type widening for signed loads

`s32 x = arr[i]` (where `arr` is `s16*`) emits `lh` (sign-extending load). `s16 x = arr[i]` may emit `lhu + sll + sra` (unsigned load + manual sign-extension). To control the opcode, control the destination type.

**Cross-file pitfall:** if a global is declared `extern s16` in one file and `extern u16` in another, the two compilations emit `lh` vs `lhu` — and one of them will be wrong. Always audit sibling `.c` files for the canonical `extern` type before adding your own.

### Intermediate variables for subtraction order

`A - B - C` can be folded by GCC into `A - (B + C)` or any other commutative arrangement. To pin a specific order:

```c
s32 tmp = A - B;
result = tmp - C;
```

Same for chained additions, subtractions, and shifts.

### Variable declaration position = load timing

Declaring a local triggers GCC to schedule its initialization load (if any) at the declaration point. If target loads early and mine loads late: declare early. If target loads late and mine loads early: declare late, or split the expression:

```c
s32 sum = compute();   // does some work; loads can fit in pipeline stalls during this
load = arr[i];          // loaded here, after sum's computation
result = sum >> N;
```

### Struct locals to defeat DCE

When GCC dead-code-eliminates stores to a "useless" stack local, wrap related locals in a struct and access via `&s.field`. The address operator forces GCC to treat the whole struct as aliasing all fields, defeating the elimination.

```c
struct { s32 a, b, c; } locals;
locals.a = compute_a();
locals.b = locals.a + 1;
*ptr = locals.b;
// Even if locals.a is "unused" semantically, the &locals.* aliases keep it.
```

Same for forcing contiguous stack layout when target expects a specific offset relationship.

### Trim oversized arrays

`s16 buf[8]` reserves 16 bytes on the stack. If you only write `buf[0]` through `buf[3]`, declare `s16 buf[4]` to match target's frame size.

### Anti-CSE for global+offset

GCC will commonly Common-Subexpression-Eliminate `&global + 4` and `&global + 8` to share a base address. If target re-derives each:

```c
*(s32*)((u32)&global + 4) = x;
*(s32*)((u32)&global + 8) = y;
```

The `(u32)` cast and explicit pointer arithmetic prevents CSE.

### `>> N` vs `/ 2^N`

Check the target asm for a `bgez + addiu` rounding pattern around the operation:

| Target has rounding pattern | Use |
|---|---|
| Yes (`bgez + addiu (1 << N) - 1`) | `/ (1 << N)` — division emits the round-toward-zero pattern |
| No | `>> N` — shift is round-toward-negative |

These have different behavior on negative inputs; matching the operator is also matching the semantics.

### Sign-extension of `lhu` reads

```c
u16 raw = D_X;
s32 signed_val = (s32)(s16)raw;   // forces sll+sra (or direct lh)
```

The `(s32)(s16)` cast cascade is necessary because `(s32)raw` of a `u16` is zero-extension.

### `(s16)v` on PsyQ little-endian emits `lh +2`

GCC 2.7.2 treats `(s16)v` (truncate to low 16 of a 32-bit value) via memory-aligned big-endian semantics. On LE PS1, this emits `lh +2`, reading the HIGH halfword.

If you want the **low** 16 bits: `v & 0xFFFF` (emits `andi`) or `v >> 16` (after shifting them into position; emits `srl`).

If you want the **high** 16 bits: `(s16)v` works (emits `lh +2`).

This trips up everyone matching their first `s16` truncation. When in doubt, write the bit operation explicitly.

### Signed vs unsigned compare

```c
s32 x; if (x < 3) ...    // slti
u32 x; if (x < 3) ...    // sltiu
```

Same numeric range, different opcode bytes. Match target.

### `(u8)` vs `(s8)` constant signedness

`(s8)0xC3` → `addiu reg, $zero, -61` → `0xFFC3` immediate.
`(u8)0xC3` → `addiu reg, $zero, 195` → `0x00C3` immediate.

Same `sb` instruction output, different `addiu` encoding. Match target's signedness when materializing a small constant.

### Cast to flip `%` overflow check

```c
s32 v = a % b;      // emits `break 0x6` block + INT_MIN overflow check
u32 v = (u32)a % b; // emits clean `divu` — no overflow check
```

GCC's signed `%` includes a 6-instruction `break` block to handle the `INT_MIN % -1` corner case. If target doesn't have it, cast to `u32` to flip to `divu`.

### Loop shape

```c
do { ... } while (cond);          // post-test, branch-backward, matches body/bnez/delay
while (cond) { ... }              // pre-test, adds a top branch
s32 i = N; do { ... } while (--i >= 0);  // count-down, 1-2 insns tighter (free zero compare)
```

PsyQ-era code uses count-down `do { } while (--i >= 0)` very often. When in doubt, try it first.

### Branch dispatch (`if/goto` vs `if/body`)

```c
if (cond) goto label;          // emits beq + jump-into-body
if (cond) { body; }            // emits bne + jump-around-body
```

These are NOT equivalent in bytes. Match the branch sense to the target.

### Anti-cross-jump (unique tail constants)

GCC merges identical switch case tails. If target has two distinct tails with identical bodies, force them apart by giving each a unique nearby constant:

```c
case 0xC: sel = 0xC; do_thing(); break;
case 0xD: sel = 0xD; do_thing(); break;
```

Then regfix-subst `sel = 0xC` away if you need the tail to be semantically clean.

### Strength-reduction defeat

When target has explicit byte-indexed access (`sll i,3; addu base,i; lw 0xC(v0)`) but GCC strength-reduces (pre-adds `0xC` into base), write explicit byte arithmetic:

```c
s32 *p = (s32 *)((u8 *)base + i*8 + 0xC);
v = *p;
```

The `(u8 *)` cast forces byte-level arithmetic that GCC won't strength-reduce.

### Forcing re-reads of volatile pointer reads across branches

GCC dedups volatile pointer reads. Force re-reads via intermediate locals:

```c
s32 new_var = arg3;
... call ...
val = base[new_var];   // mine and target both re-read arg3 via new_var
```

### Scheduling barriers (`do { } while (0)`)

```c
stmt_A;
do {} while (0);    // sequence point; GCC can't reorder across it
stmt_B;
```

Useful when GCC schedules `stmt_B`'s instructions before `stmt_A` finishes, but target keeps them separate.

### Memory barrier for jal over-scheduling

```c
do_work_with_globals();
__asm__ volatile ("" ::: "memory");
jal_call();
```

The `"memory"` clobber prevents GCC from moving global stores into the `jal` delay slot when target has a `nop`.

### Pre-existing-pointer reassignment (avoid extra callee-save)

When target has `move $v1, $s1; move $s1, $v0` after a `jal` (reusing $s1 for the new return because the old value's last use is the next store), don't introduce a new local — **reassign the existing pointer variable to the function's return**:

```c
*old_ptr = compute(...);          // mine: stores to old_ptr's $s1 then overwrites
old_ptr = compute_returning_ptr(...);  // target: scheduler can land the store of old_ptr's last-use value into the jal's delay slot
```

Reference: `func_800770B8` went from 145 diffs to 98 with this restructure.

### Share LUI across distant references via early `u8 *base`

When you need `&g_X` early and `&g_Y` later, and `&g_Y = &g_X + N`:

```c
u8 *base = (u8 *)&g_X;       // at function entry
... use *base ...
... ((Type *)(base + N))->field = ...   // later
```

Single `lui` materialization shared across both. Reference: `func_8007B9B0` went from 1265 to 40 diffs.

## Register-asm pinning

`register T x asm("$sN") = K;` is a hint to GCC's allocator. It works best for:

- **Callee-saved registers** (`$s0`–`$s7`) where target's choice is deterministic.
- Constants that target loads into a specific register early.
- Function-return values via `register T v0 asm("$2")`.

**Don't pin leaf temp regs (`$t0`–`$t9`).** Pinning causes hoisted sign-extensions, destructive in-place modification, and changed delay-slot fills. For temp diffs, use regfix instead.

**Try without pinning first.** Sometimes removing an existing `register asm` pin actually HELPS — explicit pins can hurt prologue scheduling.

### Two coordinated hoist-control patterns

When target loads constant K into `$s2` once and reuses, but mine inlines K:

```c
// Force K into $s2:
register s32 cK asm("$s2") = K;
// Then rewrite use sites to use cK instead of K:
foo(cK);
bar(cK);
```

Or, to **defeat LICM hoisting** when GCC tries to hoist K out of a loop:

```c
register s32 cK asm("$s2");
__asm__ __volatile__("addiu %0,$0,K" : "=r"(cK));
// Rewrite use sites...
```

The `__volatile__` is load-bearing — GCC can't constant-fold cK back to K and re-hoist.

### Inline-asm constraint pitfalls (GCC 2.7.2)

| Issue | Detail |
|---|---|
| `+r` is silently dropped | Use `"=r"(out) : "0"(in)` instead. |
| `#APP`/`#NO_APP` transparent to GAS delay-slot filling | GAS in `.set reorder` fills delay slots from the next physical instruction. The inline-asm barrier exists at cc1/RTL level only. |
| `.word`-encoded inline asm ignores `"=r"`/`"r"()` register bindings | Use mnemonic form (`mfc2 %0, $25`) when you need binding. Bare `.word` only when there are no operands to bind. |
| `register T x asm("$N")` pins only stick if the var is USED inline-asm-adjacent | Declaring then not feeding to an asm block doesn't pin. |
| Decimal offsets only | maspsx chokes on hex (`16` not `0x10`). |
| Multi-instruction `asm volatile` body to lock order | Put 2+ insns in a single body. Input deps `"r"(...)` force the asm after the listed sources are computed. |

## regfix at the assembly stream

When source-level techniques are exhausted (or just too painful), `regfix.txt` lets you rewrite the maspsx-emitted assembly per-function. Full syntax reference is in [`ARCHITECTURE.md`](ARCHITECTURE.md) "The post-pass tools"; the brief version:

```
func_name: <op> <args> [@ <idx-or-range>]
```

| Op | Use case |
|---|---|
| `$X <-> $Y [@ N-M]` | Bidirectional register swap (whole function or range). |
| `subst "pattern" "replacement" @ idx` | Regex replace one instruction. |
| `fill_delay @ jal_idx <- source_idx` | Move source into nop at jal_idx+1. |
| `drain_delay @ jal_idx` | Move delay-slot insn before its jal. |
| `delete @ idx` | Remove instruction. |
| `insert "text" @ idx` | Insert BEFORE idx (post-delete numbering). |
| `insert_after "text" @ idx` | Insert AFTER idx (post-insert numbering). |
| `insert_label "label:" @ idx` | Insert label without an instruction. |
| `reorder N1,N2,... @ start-end` | Rearrange (post-insert numbering). |

Phase order: swaps → substs → fill_delay → drain_delay → deletes → inserts → insert_afters → reorders.

### Idx counting (critical)

- 0-based **per function**, counting only TEXT instructions.
- **NOT** including directives, labels, comments, or `.word` lines.
- **Pseudo-instructions** (`la`, `lb sym`, `sw reg,sym`) count as 1 in TEXT but expand to 2+ binary.
- `dc.sh dump-text <func>` is authoritative.

### Common subst gotchas

- Pattern `$N` must be escaped: `\$N`. Bare `$` is regex end-of-string.
- Replacement uses plain `$N` (re.sub with count=1 passes backslashes through literally).
- Parens must be escaped in patterns: `\(` `\)`.
- For GCC auto-labels: `\.L\d+` matches any. Hardcoding `.L988` will break when sibling functions shift the counter.
- **Same instruction with duplicate register refs needs duplicate rules.** `nor $8,$0,$8 → nor $2,$0,$2` requires the subst written TWICE at the same idx.

### Always use `dc.sh add-regfix` for new rules

```bash
bash tools/dc.sh add-regfix subst <func> "pattern" "replacement" @ 47
```

Pre-validates against `dump_text_indices` before append, and auto-rolls back on live-build validation failure. Don't write rules with raw text editing — the indices are easy to get wrong.

### Run `dc.sh regfix-suggest` before writing by hand

```bash
bash tools/dc.sh regfix-suggest <func>
```

Diffs target.s vs the live build pipeline and emits ready-to-apply rules. Knows the maspsx idx conventions. Hints for gp-rel pseudos and label drift. The output frequently closes the diff in one go.

If `regfix-suggest` wants to emit more than ~40 rules, you have a structural mismatch — reach for the permuter or a C-level restructure, not more regfix rules.

## Named recipes (reference knowledge)

These are the common patterns the project hit repeatedly, kept here as reference.

> **Note (2026-05-26):** the machine-readable recipe library (`tools/recipes/*.json`) and its
> `capture_recipe.py`/`recipes.py` tooling were archived to
> `archive/dcsh_workflow_2026-05-26/recipes/` when the `dc.sh` workflow was retired — the
> engine does not consume recipes, and most encoded a regfix `rule_skeleton`, which is
> cheat-invisible substrate under the engine (stripped before scoring). The `dc.sh recipes` /
> `apply-recipe` commands no longer exist. What survives is the *knowledge* below, plus the
> live path-scoped technique docs in [`../.claude/rules/`](../.claude/rules). Read the patterns
> below to recognize a symptom; apply them by writing the C / regfix yourself.

### LICM unhoist

**Symptom:** ~500 score plateau, diff dominated by 2 inserts + 1 delete around a magic-constant load. Target keeps `lui+ori` inline (with `ori` filling lw delay slot); mine hoisted the constant into a temp register in the loop preheader.

**Recipe:** ~7 regfix rules — delete preheader `lui`/`ori`, insert inline `lui` before the `lw`, `insert_after` the `ori` as the `lw` delay-slot fill, cascade-rename the freed register via swap.

Archived JSON: `archive/dcsh_workflow_2026-05-26/recipes/licm_unhoist.json`.

**C-level techniques DO NOT work for LICM** — volatile, memory barriers, structural variants, full t-reg clobbering all confirmed failures. Go straight to regfix.

### Call-loop family (text1b.c cluster)

**Pattern:** count-prefixed list walked + per-item dispatcher (`func_800482C8`/`func_800484A0`).

**Recipe:**
1. C: `count = *(p++); do { ... } while (count-- != 0);`
2. Frame size subst (+32 bytes for `$s0` staging)
3. Stack offset substs for prologue/epilogue
4. `subst "addu $16,$4,$5" "addu $16,$16,$5"` (s0 already = arg0)
5. Saved-base reg variants: `insert_after "addu $sN,$16,$zero" @ K`
6. `delete @ M` redundant `addu $sN,$4,$zero`
7. `insert_after "nop" @ K` after `lw $2,0($16)` (maspsx skips load-delay nop when rt/rs both $2)
8. `reorder` to swap inserts and `sw $sN` to front

Six known siblings matched: 47EE8, 47FBC, 480C0, 481E8, 483DC, 644FC.

### Early-exit alias breaker

**Symptom:** GCC emits `subu sp; beq $4,$0,.Lend; addu $2,$4,$0` (3-insn early-exit reusing `$a0` as `$v0`). Target has `bnez $4,.Lcompute; subu sp; j .Lend; addu $2,$0,$0` (4-insn explicit zero return).

**Recipe:** 8 regfix rules — 3 substs to restructure prelude, 1 subst to fix arg derefs back to `$a0`, `insert "j .L<End>" @ 2`, `insert_label ".LcXXX:" @ 3`.

**Auto-detected by `gen-regfix`.** C-level breaks all fail (`register volatile s32 ret asm("$2") = 0` etc. all get re-aliased).

### Varargs prologue

Small wrappers like `debug_printf(s32 fmt, s32 a, ...)` use post-subu spills (target) vs pre-subu spills (GCC).

**Recipe (7 rules):** subst delay slot, delete 3 leading pre-subu spills, `insert_after` 3 post-subu spills (descending in file order — same-idx insert_after is reverse-stable). Auto-detected by `gen-regfix`.

Larger varargs (e.g., `DispSleepMenuTex`) match GCC's natural pre-subu — no regfix needed.

### Nested-bool memcard family

~6 functions in the `60412...` cluster — nested bool flag check + 3-call memcard setup.

**Recipe:**
- `extern u8 D_800F11XX[]` (ARRAY form — single-byte form causes `.NON_MATCHING` BSS layout shift in splat)
- `register s32 t asm("$2"), mask asm("$3")`
- `s32 *v1 = (s32 *)&D_800F116C;` cached pointer in callee-saved register
- goto-style block layout with block_4 BEFORE check_one_zero in source order

### Delay-slot fill via `insert_after`

**Symptom:** mine binary is exactly 4 bytes (1 instruction) shorter than target. `dc.sh diff-align` flags `[delay-fill] mine has nop in delay slot at idx N`.

**Recipe (1 rule):**
```
func_X: insert_after "<target's insn at that position>" @ <maspsx_idx_of_branch>
```

### Label-shift recipe (branch-offset off by 1)

**Symptom:** single `idx N differs` with both branches same opcode/regs but immediates differ by 1 (e.g., mine `0x10400009` vs target `0x1040000A`). `diff-align` flags `[label-shift]`.

**Cause:** auto-generated `.L<N>` attached to wrong instruction position.

**Recipe (2 rules):**
```
func_X: insert_label ".L_postNNN:" @ <pos_after_redundant_insn>
func_X: subst "beq\s+\$N,\$0,\.L\d+" "beq	$N,$0,.L_postNNN" @ <pre-regfix_branch_idx>
```

### Hoist-removal recipe (`la $X,sym` hoisted out, target inlines)

**Symptom:** mine has `la $X, sym` near function entry + body uses `addu $Y, $base, $X`; target has `lui $Y, %hi(sym); addiu $Y, $Y, %lo(sym); addu $Y, $base, $Y` triples at each access site.

**Recipe (3+ rules per access site):**
```
func_X: delete @ <la_idx>
func_X: insert "addiu $Y,$Y,%lo(sym)" @ <addu_idx>
func_X: insert "lui $Y,%hi(sym)" @ <addu_idx>
func_X: subst "addu\s+\$Y,\$base,\$X" "addu	$Y,$base,$Y" @ <addu_idx>
```

Listed in reverse order (addiu first, lui second) so final after sort is `lui, addiu, addu`.

### CU split for jtbl interposition

Switch function whose target jtbl lives at an address inside a sandwich of asm-stub rodata. Three techniques (full details in the project history; choose by constraint):

1. **Monolithic split** — split `800.rodata.s` once; C absorbs middle.
2. **Sandwich split** — split asm rodata around a single jtbl, insert C object's .rodata between two unrelated jtbls.
3. **C-file split** — split the C source into two objects; new file goes in `RODATA_ALIGN2_FILES`.

**CRITICAL:** GCC emits `.align 3` (8-byte) for switch tables. After a split, section-relative offsets shift, causing `.align 3` to insert 4 bytes of unwanted padding. Always add to `RODATA_ALIGN2_FILES` in the Makefile.

### SW $RA in branch delay slot

GCC always saves `$ra` in prologue; target sometimes has `sw ra` in the delay slot of the first conditional branch. Add the function name to `tools/delay_slot_ra_funcs.txt`; `prologue_fix.py` swaps it.

### GTE 3x3 matrix-transform wrapper

For the `calc_fc_frame_8007EC5C` / `func_8007E4DC` / `func_8007EB4C` family (67-instruction "leaf" GTE wrapper).

**Three coordinated tricks (all needed):**

1. **Pin the `0xFFFF0000` mask to `$at` via inline asm:**
   ```c
   __asm__ volatile (
       ".set noat\n\t"
       "lui $1, 0xFFFF\n\t"
       "and %0, %0, $1\n\t"
       ".set at"
       : "=r"(t1) : "0"(t1)
   );
   ```
2. **Pin return value with `move %0, %1` at the END** (use `move`, NOT `addu` — audit flags `addu`).
3. **Memory barriers between paired pack-store blocks** (defeats interleaving of next pack block's andi/sll/or into prior store's nop slot).

Does NOT apply to `bgez+negu+b` sign-extraction patterns (e.g., `func_8007EA0C`) — those use unusual delay-slot precomputes pure C can't naturally emit.

### Lookup-store-via-delay-slot

**Symptom:** target uses 2 callee-saves; mine uses 3 because a local is live across a `jal`.

**Cause:** target's compiler scheduled the store of the live value into the `jal`'s delay slot, avoiding the spill.

**Recipe:** Restructure C so the store statement appears BEFORE the call statement, and the post-call code reads the value back from memory (NOT from a local):

```c
*(s16 *)(arg0 + 0x352) = lookup_expression;     // store BEFORE jal
src = *(s32 *)((((s32) *(s16 *)(arg0 + 0x352)) << 2)
               + jal_call(...));                // reload from memory, nested call
```

GCC schedules `sh $v0, 0x352(...)` into the `jal`'s delay slot. Net: −1 callee-save, matches target's frame shape. Reference: `calc_fc_frame_800203B4` (commit a502eb4).

### Inline-asm `move` to force redundant-aliasing

**Symptom:** matrix-load block uses direct sp-relative addressing; target uses pointer-via-`$t4` indirection.

**Cause:** GCC collapses `register asm("$12") = mat;` into a single `addiu $t4, $sp, K`. The pin is only honored when the value is materialized as a register elsewhere.

**Recipe:** Force the move via single-instruction inline asm:

```c
{
    register s32 *mp asm("$12");
    register s32 t5 asm("$13");
    s32 *p = mat;
    __asm__ volatile ("move %0, %1" : "=r"(mp) : "r"(p));
    t5 = *mp;
    /* ... ctc2 ops using t5 ... */
}
```

Step 2 emits `addu $t4, $v0, $zero` (the move-as-addu form target uses). Subsequent `*mp` accesses base off `$t4` instead of `$sp`.

## Dead ends — don't bother trying these

| Pattern | Why it doesn't work |
|---|---|
| C-level LICM suppression (volatile, memory barriers, structural variants, register asm hints, full t-reg clobbering) | All confirmed failures. Use the regfix unhoist recipe. |
| Permuter on trivial loops | `do { *p=0; n--; p++; } while (n>=0)` has only one C structure. If 2 declaration reorderings don't fix registers, escalate to regfix immediately. |
| Permuter on register-only diffs | Permuter randomizes structure; pure register problems aren't structural. Read the penalty list — if Ins/Del/Reorder=0 and only Reg≥1, write swap rules. |
| Register asm on leaf temp regs | Causes hoisted sign-extensions, destructive in-place modification, changed delay slot fills. Use regfix. |
| Branch delay slot tools | All 1,038 filled delay slots in stub targets are GCC-scheduled, not ASPSX. Don't build pipeline tools; fix C control flow instead. |
| Re-scoring with permuter after source integration | Different externs, different codegen. Permuter score going UP after correct integration is normal. Only `make` (SHA1) is authoritative post-integration. |
| Manual asm diff before `dc.sh regfix-suggest` / `gen-regfix` | Automates 80% of swaps, delay-slot fills, reorders. Run it first. |
| Changing stack arg type from `s32` to `u16` in callee | Caller's lw/sh sequence shifts AND PsyQ ABI reads u16 from offset+2. Keep s32; regfix `subst "lw" "lhu" @ N`. |
| Reading `dc.sh diff` cascade output before `dc.sh diff-align` | 145+ "diffs" that are really 1 structural change + 139 cascade. Always `diff-align` first. |
| `dc.sh verify <func>` for retirement work | Bridge-blind — always reports MATCH for bridged functions. Use `verify-c` instead. |
| `gen-regfix` on pure register-cycle diffs | Floods 200+ "structural diff" lines. Write swap rules manually. |
| Top-level `__asm__("...")` block doing function logic | Not a match. Forbidden as final state unless authorized in `inline_asm_canonical.txt`. |
| `replace_with_asmfile` bridge as a default response to difficulty | Requires explicit per-function authorization. The project accumulated 209 bridges with 0 real-decomp progress before this rule was added. When stuck, escalate **technique**, not artifact. |
| Trying the same C variant twice expecting different result | GCC flattens different C structures. Same score twice → switch from C-level to permuter/regfix. |
| Quick-win hunting (picking functions by ease) | Forbidden. Take the next function in queue order. The queue is in (loose) complexity order. |

## Common gotchas

### G1. Build-cache regression trap

**Symptom:** "MATCHED first try", `verify` says MATCH, `verify --all` green, SHA1 matches, commit lands. Weeks later, fresh checkout + `make` produces a different binary.

**Cause:** `make` uses cached `build/src/*.o`. Edit one `.c` → only that `.o` recompiles. New C may have different register allocation than the prior `.o`, but link still produces matching bytes because OTHER cached `.o` files happen to cancel out the difference.

**Fix:** `bash tools/dc.sh verify --clean` before commits involving more than 2 register-asm pins, more than 3 regfix rules, or `move`/`addu` placement tricks. The commit hook does this automatically per match; run it yourself when committing a series of related matches in quick succession.

### G2. Bridge false-match trap

**Symptom:** During retirement, `dc.sh verify <func>` says MATCH, clean-rebuild SHA1 passes, commit goes through. Later: removing the bridge shows 60+ instruction diffs.

**Cause:** Build ends with `asmfix.py`. An active `replace_with_asmfile` line overwrites `.o` bytes with raw asm regardless of the C body. `dc.sh verify` reads bytes at the function's address in `build/bb2.exe` — for a bridged function, those are always the bridge's bytes.

**Fix:** `bash tools/dc.sh verify-c <func>` — bridge-aware; refuses if active bridge in asmfix.txt. Use this during retirement, NOT plain `verify`.

### G3. Pure-C stub label drift

**Symptom:** Adding a pure-C stub (or full decomp) to a `.c` file breaks subsequent sibling functions' asmfix renames and regfix substs that reference `.L<N>` literals.

**Cause:** GCC numbers `.L<N>` per compilation unit, monotonically increasing. Adding ANY new function (even a tiny stub) increments the counter by +1, shifting all SUBSEQUENT functions' label numbers.

**Fix:**

1. `bash tools/dc.sh fix-asmfix-drift --apply` (build-driven).
2. `bash tools/dc.sh fix-label-drift --apply` (linker-driven).
3. If still broken: `bash tools/dc.sh fix-label-drift --also-verify` (slow ~3–5 min; catches wrong-but-existing label trap).
4. Preemptive: `bash tools/dc.sh regfix-drift-immune --apply` to rewrite substs with literal `.L<N>` to `\.L\d+` regex.

### G4. Caller arity cascade regression

**Symptom:** After matching `func_X`, 500+ sibling regressions across the file.

**Cause:** Your decl arity is narrower than callers' actual call. GCC dead-codes extra-arg computations in callers, changing their bytes.

**Fix:** `dc.sh inline-replace` auto-runs `caller-audit` and BLOCKS if arity is wrong. If it blocks: widen the decl to max observed arity (use `s32 unused_arg` for the dead one). NEVER override `caller-audit`.

### G5. GCC scheduler reorders independent stores

**Cause:** GCC moves trailing stores up into a `jal` delay slot when target has a `nop`.

**Fix:** Memory-clobber barrier between the store and the `jal`:

```c
do_work();
__asm__ volatile ("" ::: "memory");
foo();
```

Or wrap the store in a multi-insn `asm volatile` body to lock its position.

### G6. WSL CRLF / `$N` expansion in heredocs

**Symptom:** Build silently produces wrong bytes. cc1 emits inline-asm with empty operands; maspsx errors "Unable to parse load/store instruction" OR silently emits malformed function.

**Cause:** `wsl bash -c 'python3 -c "... $N ..."'` — outer bash expands `$0`, `$1`, `$8` inside the inner string BEFORE Python sees it. `register asm("$8")` becomes `register asm("")`.

**Fix:** Save Python edit scripts to a file (`python3 tools/tmp_edit.py`) or use Edit/Write tools directly (no shell layer). For build files, always write through WSL via `python3 <file>` or `dc.sh replace`. Never raw native-Windows Edit/Write — CRLF silently breaks the GNU toolchain.

### G7. `addu` in inline asm — forbidden

**Symptom:** Function byte-matches; `refresh-queue` reclassifies as `inline_asm_debt`; back to top of queue.

**Cause:** `audit_inline_asm.py` SUSPECT regex catches `\baddu\b`.

**Fix:** Use the MIPS pseudo-mnemonic `move` (expands to the same bytes `addu rd, rs, $zero`, not in the regex):

```c
__asm__ volatile ("move %0, %1" : "=r"(v0) : "r"(a2));
```

### G8. Permuter score 0 but build SHA1 mismatch

**Cause:** Permuter only checks `.text`, not `.rodata`. Switch jump table case ordering wrong.

**Fix:** Reorder `case` statements in your C switch to match target's jtbl order. Always `make` to verify post-permuter-match.

### G9. Function content matches but build SHA1 fails

**Cause:** Symbol address moved. Probable `regfix reorder @ 0` or `delete @ 0` — function-label preamble (`func_NAME:`, `.ent`, `.frame`) attaches to first instruction.

**Diagnostic:** `mipsel-linux-gnu-nm build/bb2.elf | grep <func>` — compare symbol address vs original.

**Fix:** Use two substs that swap the *content* of idx 0 and idx N (substs replace text in place) instead of reordering.

### G10. `s32` → `u16` stack arg breaks caller and callee

**Symptom:** Caller's compiled `lw`/`sh` sequence shifts after changing callee arg type.

**Cause:** PsyQ ABI reads u16 stack slots from offset+2 not offset+0 (high halfword of 32-bit slot). Caller now zero-extends with `andi 0xFFFF` instead of sign-extending with `sll/sra`.

**Fix:** Keep stack args as `s32` and use regfix `subst "lw" "lhu" @ N` for the load.

## Decision tree: "I'm stuck, what now?"

1. **Did you read the penalty list?** (`dc.sh debug <func_dir>`) Route via the table at the top of this doc.
2. **If only Reg ≥ 1:** regfix swaps. Don't run permuter or `gen-regfix`.
3. **If Stack Differences dominates:** `dc.sh frame-shift --apply` or 4-5 rule regfix.
4. **If Ins/Del ≥ 1 with structural pattern:** check `dc.sh diff-align`; pick a named recipe if it matches.
5. **If hoist mismatch on callee-saves:** `dc.sh diagnose-hoist <func>` → register-asm pin (force) or inline-asm constant load (defeat).
6. **If structural and no named recipe fits:** longer permuter (5–10 min), then 30 min, then overnight.
7. **If score > 3000 + 30+ inst label-bounded region:** `dc.sh asmfix-slice` (widest viable slice within function body).
8. **If still stuck:** build a new tool / new regfix op / new named recipe. The project's pipeline is open-ended; extending it is expected.

What stuck does NOT mean:

- A reason to skip the function.
- A reason to bridge with `replace_with_asmfile` without authorization.
- A reason to leave inline `__asm__()` as the final answer.

Read [`CONTRIBUTING.md`](../CONTRIBUTING.md) for the full escalation ladder and the bridge-authorization process.
