# Evidence — func_800174F4 (src/ings.c)

## Session 1 (recon, 2026-08-13) — floor 25 -> 14

### Function shape
136 target instructions. `void func_800174F4(void)` — the per-frame draw
dispatcher: early-returns when `g_disp_enable == 0xFF`, otherwise builds a
DRAWENV in a 0x68-byte stack buffer (`sp+0x20`), `SetDefDrawEnv` /
`PutDrawEnv`, `ClearOTagR(sp+0x18, 2)`, then a 4-way `switch (g_disp_enable)`
(cases 1/2, 10, 20, default), then `DrawOTag(D_800A374C + 4)` + `DrawSync(0)`.
Carries exactly ONE regfix rule: `func_800174F4: $3 <-> $5 @ 27-41`
(regfix.txt:11) — i.e. the cheat covers only the switch-selector register.

Target register roles (from asm/funcs/func_800174F4.s):
  $s0 = &sp20 (drawenv buffer address), later the case-1/2 loop COUNTER.
  $s1 = 0xF0 (drawenv height), later the case-1/2 loop LIMIT, the case-20
        `div` dividend, and the `D_800A37A8[]` table value. One register,
        four roles.
  $s2 = the prim pointer (`&D_800F33D8` -> func_8005D46C -> func_8005D554 ...).

### The cheat scaffold in the inherited body was worth ZERO distance
The pre-session src/ings.c body carried `register s32 s0_var asm("s0")`,
`register s32 s2_var asm("s2")`, `register s32 a1_val asm("a1")`, an
empty-body dead-read `if ((a1_val && a1_val) && a1_val) { }`, and the
`new_var` / `new_var2` constant-holder locals. A from-scratch pure-C rewrite
with ALL of that removed measured the SAME honest floor (25). MEASURED, not
inferred. There is therefore no reason for any future session to keep or
re-derive any of it.

### Measured floor ladder (all `sandbox func_800174F4 --disable all`)
| form | score | insns |
|---|---|---|
| inherited body (pins + dead-read + constant holders) | 25 | 136 |
| clean pure-C rewrite, same structure | 25 | 135 |
| + drop the `a0_temp` copy local from the case-1/2 loop | 25 | 135 |
| + split the loop counter out of `env` into its own `s32 i` | **18** | 135 |
| + move `i++` to AFTER the call inside the loop | **14** | **136** |

At 14 the instruction SEQUENCE matches target exactly (136 vs 136, zero
insert/delete/reorder in the aligned diff). All 14 residual points are
register-NAME substitutions.

### The two residual clusters at floor 14
(A) switch-selector web — 6 diffs, insns 28/30/31/33/35/41:
    ours `lbu v1,%gp_rel(D_800A3768)` + `beq v1,..` / `slti v0,v1,11` /
    `slti v0,v1,3` / `beq v1,..` / `beqz v1,..`; target uses `$a1` throughout.
    This is precisely what the surviving regfix rule `$3 <-> $5 @ 27-41`
    papers over.
(B) callee-save web split — 8 diffs, insns 50/52/53/58/60 and 116/119/120:
    ours  i->$s1, loop-limit->$s0, table-value->$s0
    target i->$s0, loop-limit->$s1, table-value->$s1.
    GCC splits the single C variable `h` into three independent allocnos
    (0xF0/div web, loop-limit web, table-value web). Ours places web #1
    correctly in $s1 but webs #2/#3 in $s0; target keeps all three in $s1.

### Mechanism reference (read from the frozen toolchain source)
`tools/gcc-2.7.2/global.c:allocno_compare` — allocation order is by
    priority = floor_log2(n_refs) * n_refs / live_length   (× 10000 × size),
ties broken by allocno number (i.e. first-use order). Higher priority is
allocated first and takes the first free reg in `reg_alloc_order`, so for the
callee-saves the higher-priority allocno gets $s0. This is the lever surface
for cluster (B): the split is decided by refs/live-length of the individual
webs, not by anything the C names.

### Artifacts
`tmp/grind/func_800174F4/s1/` — `diffdump.py` (aligned normalized-insn diff
tool built on `engine.score.normalized_insns`, ours vs `build/src/ings.o`),
`diff.txt` (baseline 25), `diff_goto_notemp.txt`, `diff_18.txt`, `diff_14.txt`,
`ings.i` + `ings.i.greg` (cc1 `-dg` RTL/global-alloc dump of the baseline
form; `;; Register dispositions` for this function starts at greg line ~5945),
plus snapshots `ings_clean25.c`, `ings_split_i_18.c`, `ings_incr_after_14.c`.

- [s1] Baseline honest floor was 25 (sandbox func_800174F4 --disable all); best form this session is 14 with build_insns == target_insns == 136.

- [s1] MEASURED: the inherited body's entire cheat scaffold is worth ZERO honest distance -- a from-scratch pure-C rewrite that removes all three `register T x asm(...)` pins, the empty-body dead-read `if ((a1_val && a1_val) && a1_val) { }`, and the `new_var`/`new_var2` constant-holder locals scored the SAME 25. No future session should keep or re-derive any of it.

- [s1] func_800174F4 carries exactly one regfix rule: `func_800174F4: $3 <-> $5 @ 27-41` (regfix.txt:11), whose scope is precisely the switch-selector register cluster.

- [s1] Target register roles: $s0 = &sp20 drawenv buffer then the case-1/2 loop counter; $s1 = 0xF0 height, then the loop limit, then the case-20 `div` dividend, then the D_800A37A8[] table value (one register, four roles); $s2 = the prim pointer chain.

- [s1] At floor 14 the residual is exactly two register-name clusters: (A) switch selector ours $v1 vs target $a1 at insns 28/30/31/33/35/41 (6 pts); (B) callee-save webs ours counter->$s1 + limit->$s0 + table-value->$s0 vs target counter->$s0 + limit->$s1 + table-value->$s1 at insns 50/52/53/58/60/116/119/120 (8 pts).

- [s1] GCC 2.7.2 splits the single C variable `h` into three independent allocnos (0xF0/div web, loop-limit web, table-value web); our build places web #1 correctly in $s1 but webs #2/#3 in $s0.

- [s1] Mechanism reference read from the frozen toolchain: tools/gcc-2.7.2/global.c:allocno_compare -- allocation order is priority = floor_log2(n_refs) * n_refs / live_length (x10000 x size), ties broken by allocno number; higher priority is allocated first and takes the first free register in reg_alloc_order, so for the callee-saves the higher-priority allocno gets $s0. Declaration order/naming only enters the tie-break, so it is NOT a lever for these (untied) webs.

- [s1] The cc1 -dg dump of the BASELINE body (tmp/grind/func_800174F4/s1/ings.i.greg, function section at ~line 5945) shows `;; Register dispositions: 72 in 17  73 in 16  74 in 18 ...` -- i.e. the two callee-save pseudos were swapped relative to target at baseline. This dump predates the floor-14 form and must be re-taken before it is used for cluster (B).

- [s1] tmp/grind/func_800174F4/s1/diffdump.py is a reusable aligned normalized-instruction differ (ours vs build/src/ings.o) built on engine.score.normalized_insns; it is what turned an opaque score into the per-insn cluster map above.
