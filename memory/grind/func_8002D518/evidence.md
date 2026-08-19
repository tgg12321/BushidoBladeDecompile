# Evidence bank — func_8002D518

## [s1] 2026-08-19 — recon: baseline, island canonicalization, full diff map

**Chassis at s1:** dispatch said "measurement unavailable"; measured this
session: floor 33 at HEAD form, floor **30** after s1 edits (now in src).
`canonical` verdict: ASM-PARTIAL, 2/144 insns canonical (mtc2 @91, swc2 @93).
33 regfix/asmfix rules dropped by the sandbox (they are inert to the honest
floor; they get retired at completion).

**Cluster membership (settled — do not re-derive):** member of the 2026-08-17
owner cluster ruling, LZCS sub-family, 1 idiom site
(`.claude/rules/cop2-addressing-preamble-cluster.md:74`: L106
`addu $t4,$a0,$zero -> mtc2 $t4,$30 ; nop ; nop`). The island's disposition is
settled; the 4 mechanical inheritance conditions (rule file, "per-function
mechanical check" section) are what the finished state must satisfy:
sandbox==0, zero pins/aliasing/barriers, island-GPR limited to the addressing
preamble, layer-2 + verify-oracle --rebuild before queue done. The direct
same-file accepted sibling spelling is **func_800274BC**
(src/code6cac_b.c:279-318, authorized `inline_asm_canonical.txt:267`).

**Island respell (MEASURED, load-bearing):** HEAD form carried the island as
`register s32 t4_v asm("t4")` + `.word 0x488CF000` / `.word 0xE99F0000`
blocks — cheat-scored, sandbox-stripped, leaving build_insns 141 vs target
144. Respelled in the func_800274BC canonical form (single `__asm__ volatile`,
template `addu $t4,%1,$zero / mtc2 $t4,$30 / nop / nop / addu $t4,$sp,$zero /
swc2 $31,0($t4)`, `"=m"(sp_tmp)`, `"r"(ud)`, clobber `"$12"`):
- sandbox does NOT strip this form → build_insns 144 == target 144;
- island bytes verified word-for-word against target: `addu $12,$4,$0` =
  0x00806021 == target 8002D6A0; `addu $12,$29,$0` = 0x03A06021 == target
  8002D6B0; mtc2 0x488CF000, swc2 0xE99F0000, `lw $3,0($29)` 0x8FA30000 all
  exact. The `%1` operand resolved to $a0 naturally.
- floor 33 -> 30 from this + insn-count parity.

**Full residual map at floor 30** (tmp/grind/func_8002D518/s1/build.dis vs
asm/funcs/func_8002D518.s; both entrance chains + island + div/break scaffolds
already match):

1. **The disc copy (the key structural diff).** Target allocates `disc` to
   $a2 and materializes `addu $a0,$a2,$zero` in the beqz delay slot
   (8002D680); the ELSE branch's island input (`addu $t4,$a0`) and
   `srlv $v0,$a0,$v1` (8002D6CC) read the $a0 copy, while `sltiu` (compare
   vs 0x400), table index `addu $at,$at,$a2`, and `bltz $a2` read $a2.
   Our build: disc lands in $4 directly, no copy, delay slot is a nop.
   => original had TWO registers carrying disc; a fresh local `u32 ud = disc;`
   expresses it but GCC folds the copy (see hypotheses.md H2 for the two
   measured KILLED spellings).
2. **Cascade register diffs** (all plausibly downstream of #1):
   - dist_sq: build $6, target $5 ($a1); its `sra` source reg differs.
   - sqrt_val: build $2, target $6 ($a2) — `srl $2,$2,3` vs `srl $a2,$v0,3`,
     `srlv $2,$4,$2` vs `srlv $a2,$a0,$v0`, `sll $5,$2,9` vs
     `sll $a2,$a2,9` (sq: build $5, target $6).
   - t1_val/t2_val div chain: swapped $3<->$4 vs target (t1 in $3 vs $a0=$4;
     denom in $4 vs $v1=$3); result var: build $6, target $5 ($a1)
     (`move $3,$0`/`move $6,$0`/`slti $6` vs `addu $v1,$zero`/
     `addu $a1,$zero`/`slti $a1`; final `move $2,$6` vs `addu $v0,$a1`).
3. **Two ordering flips:**
   - mult-cascade: build emits `addu $6,$8,$6` (dist_sq) right after
     `mult $2,$2`, BEFORE the c_val chain (`addu/subu/sra`); target emits the
     c_val chain first, dist_sq addu after `sra`, before `mflo` (8002D644-54).
   - div prep: build emits denom `sll $4,$6,1` before `addu $3,$2,$5`
     (neg_b+sq); target emits `addu $a0,$v0,$a2 / sll $a0,8 / sll $v1,$a1,1`
     (denom sll LAST before div, 8002D700).

**Tooling facts (bank):** diagnose SKIPs ("not found in
tmp/scan/code6cac_b/code6cac_b.purec.o" — stale scan artifact); use the
sandbox .o + objdump instead (script: tmp/grind/func_8002D518/s1/dis.sh).
Function spans src/code6cac_b.c:1122-~1225. Caller cluster at :1324-1333
(same file). D_8008D118 is the shared sqrt/log byte table.

- [s1] Cluster membership settled: LZCS sub-family member of the 2026-08-17 cop2-addressing-preamble cluster ruling (.claude/rules/cop2-addressing-preamble-cluster.md:74); direct same-file accepted sibling spelling is func_800274BC (src/code6cac_b.c:279, inline_asm_canonical.txt:267); at sandbox 0 the island takes the driver-executed grant path, no re-escalation

- [s1] canonical verdict ASM-PARTIAL 2/144 (mtc2@91, swc2@93); 33 rules are sandbox-inert and retire at completion

- [s1] Floor 30 residual map (evidence.md): (1) missing addu $a0,$a2,$zero disc copy in beqz delay slot; (2) register cascade dist_sq $6->$5, sqrt_val $2->$6, t1/t2 $3<->$4 swap, result $6->$5; (3) two ordering flips: dist_sq addu placement in the mult cascade, denom sll placement in div prep

- [s1] diagnose SKIPs on this function (stale tmp/scan purec.o); use sandbox .o + tmp/grind/func_8002D518/s1/dis.sh instead

- [s1] s1 edits are IN PLACE in src/code6cac_b.c (floor-30 form == memory/grind/func_8002D518/candidate.c)

## [s2] 2026-08-19 — structural: copy-fold pass attributed to combine; if/else escape lands the copy

**CHASSIS WARNING (load-bearing for every future session).** At s2 dispatch
`src/code6cac_b.c` did **not** carry s1's edits — HEAD still had the old
`register s32 t4_v asm("t4")` + `.word 0x488CF000` / `.word 0xE99F0000` island,
and the honest floor measured **33**, not the ledger's 30. The Grinder commits
the ledger, not `src/`. s2 re-applied the s1 island respell from
`candidate.c` and re-measured floor **30 / build_insns 144 == target 144**
before doing any new work. ALWAYS verify `src/` actually carries `candidate.c`
before quoting a ledger floor.

**Pass attribution for the folded `u32 ud = disc;` copy (s1 frontier item 1) —
SETTLED, do not re-derive.** Dumps in `tmp/grind/func_8002D518/dumps/`
(`pwsh tools/grinder/dump.ps1 func_8002D518`; whole-file dumps, function region
starts at `;; Function func_8002D518`: .rtl:9356, .cse:8396, .combine:8565,
.lreg:11527).
- In the pre-optimisation `.rtl`, `disc` = `reg/v:SI 117`, `ud` = `reg/v:SI 132`,
  and the copy is `insn 248: (set (reg/v:SI 132) (reg/v:SI 117))`.
- In `.cse` the copy **survives**. cse's block extension is real here
  (`;; Processing block from 242 to 289` spans past the join `code_label 259`),
  and `canon_reg` rewrites only the FAR use — `insn 270`, the `srlv`, already
  reads 117 in `.cse` — while `jump_insn 251` and the island `insn 254` still
  read 132.
- In `.combine` the copy is **`(note 248 ... NOTE_INSN_DELETED)`** and every
  remaining use has been rewritten to 117. **The fold is combine's, not cse's.**
  Combine's blocks are CODE_LABEL-bounded (a conditional jump does not end
  them), so in the plain-if shape the copy at 248, the branch at 251 and the
  island at 254 all sit inside one label-to-label region and combine propagates
  across them freely.

**THE ESCAPE THAT WORKS (variant M — now the candidate, in src).** Respelling
the LZCS guard from "initialise then conditionally overwrite"

    s32 lzcr = 0;  u32 ud = disc;  if (disc >= 0) { <island>; lzcr = sp_tmp; }

to a real if/else

    s32 lzcr;  u32 ud = disc;
    if (disc < 0) { lzcr = 0; } else { <island>; lzcr = sp_tmp; }

puts a CODE_LABEL between the copy and the island, combine can no longer reach
it, and **the copy survives to reorg**. Measured: score still 30 but
`build_insns` still 144 — **zero instruction cost** — and the emitted code now
carries `move $4,$3` in the `beqz` delay slot, i.e. build idx 90 vs target
`8002D680 addu $a0,$a2,$zero`. The copy is allocated **$a0 — target's own
register** — and feeds BOTH the island input (idx 98 `move $12,$4` vs target
`addu $t4,$a0,$zero`) and the `srlv` (idx 109 `srlv $2,$4,$3` vs target
`srlv $v0,$a0,$v1`), exactly the target use-pattern. This is the free if/else
escape named verbatim in
`.claude/rules/cse-block-extension-controls-fold-span.md` ("Spell the
conditional as a real **if/else** rather than 'initialise to a default, then
conditionally overwrite' ... ordinary C with a semantic reading"), NOT a
coercion. **s1 frontier item 1 is CLOSED.**

**Full 154-slot alignment (target vs build) at floor 30, both base and M.**
Slots **0-74 are byte-identical including register numbers** — the whole
entrance chain, both `x < -threshold` / `threshold < x` ladders, both `z`
ladders, all six `mult`/`mflo` pairs and their register mapping
($t0=ax_sq, $a2=az_sq, $a0=cx, $t3=cz, $a1=x1_sq, $v1=z1_sq — identical in
build). Divergence begins at slot 75 and the residual is now exactly two
coupled items:
1. **`disc`'s register.** Build M puts `disc` in `$3`, target in `$a2`/`$6`.
   Everything from slot 84 (`subu`) through 92 and the `bltz` at 96 follows
   from this. Note M's `bltz` tests the COPY (`bltz $4`) where target tests
   `disc` (`bltz $a2`) — a cse canonicalisation choice, not an extra insn.
2. **The sched2 mult-shadow ordering flip at 75-78.** The four insns in the
   `mult`(74) -> `mflo`(79) shadow are the same set in both; target orders them
   `addu x1_sq+z1_sq / subu r_sq / sra c_val / addu dist_sq`, build orders
   `addu dist_sq / addu / subu / sra`. **Attributed:** sched1 gets this RIGHT
   (`.sched` log: all of 194/196/197/200 have `priority = 13`, and the backward
   ready lists pick `T-20:194, T-19:196, T-18:197, T-17:200` = target order).
   **sched2 flips it** (`.sched2` log: `ready list at T-19: 197 (44) 200 (18),
   now 197 200` — post-reload priorities are no longer tied, 197=0x44 beats
   200=0x18, 197 is taken at T-19 so 200 lands EARLIER in the block). The flip
   is therefore a POST-RELOAD priority effect, i.e. downstream of allocation,
   which is why every source-level reordering measured neutral (below).
   The tail flip at 118-122 (denom `sll` placement) has the same signature.

**Tooling fact banked:** `tools/grinder/dump.ps1` emits WHOLE-FILE dumps for
`code6cac_b`; slice the function with the `;; Function func_8002D518` line
numbers above. Also: **PowerShell on this box cannot invoke `bash`** — a runner
script that shells out to `bash tools/wsl.sh` from a `.ps1` silently exits 127
and the edit is NEVER applied, so every "measurement" in that loop is a stale
re-score of whatever was last in `src/`. s2 lost one probe round to this. The
working pattern is: generate complete variant `.c` files from the Bash tool,
then a pure-PowerShell loop that `Copy-Item`s each into `src/` and runs
`tools/wteng.ps1 main sandbox`. Runner:
`tmp/grind/func_8002D518/s2/run3.ps1`.

- [s2] Chassis: src did NOT carry s1's candidate at dispatch (floor was 33, not 30); re-applying candidate.c restored 30/144==144. Always verify src before quoting a ledger floor.
- [s2] The `u32 ud = disc;` copy is deleted by COMBINE (insn 248 alive in .cse, NOTE_INSN_DELETED in .combine), not by cse. cse's block does extend past the join (242->289) and canon_reg rewrites only the far `srlv` use.
- [s2] SOLVED: spelling the LZCS guard as a real if/else (`if (disc < 0) lzcr = 0; else { island }`) instead of init-then-overwrite makes the copy survive combine at ZERO instruction cost — copy emitted in the beqz delay slot, allocated $a0 (target's register), feeding both the island input and the srlv exactly as target.
- [s2] Slots 0-74 of the 154-slot alignment are byte-identical including registers; the entire residual 30 is downstream of (a) disc allocated $3 vs target $a2/$6 and (b) the sched2 mult-shadow ordering flip at 75-78.
- [s2] The 75-78 ordering flip is SCHED2's, not sched1's: sched1 ties all four at priority 13 and produces TARGET order; sched2's post-reload priorities (197=0x44 vs 200=0x18) invert it. It is downstream of register allocation, so source-level statement reordering cannot reach it.
- [s2] PowerShell cannot invoke `bash` on this box (exit 127) — .ps1 runners that call `bash tools/wsl.sh` silently skip the edit and re-score stale src. Use pre-generated variant .c files + Copy-Item.

- [s2] CHASSIS: src/code6cac_b.c did NOT carry s1's candidate at s2 dispatch — HEAD still had the register-pin/.word island and measured floor 33. The Grinder commits the ledger, not src/. s2 re-applied candidate.c and re-measured 30 / build_insns 144 == target_insns 144 before doing any new work. Future sessions must verify src before quoting a ledger floor.

- [s2] The disc-copy fold is COMBINE's: insn 248 (set (reg/v:SI 132) (reg/v:SI 117)) is alive in .cse and NOTE_INSN_DELETED in .combine. cse's block DOES extend past the join (Processing block from 242 to 289) but canon_reg rewrites only the far srlv use (insn 270); the guard branch (251) and the island (254) still read 132 after cse.

- [s2] SOLVED (s1 frontier item 1): the if/else-spelled LZCS guard makes the copy survive at zero instruction cost. It is emitted as `move $4,$3` in the beqz delay slot (build idx 90) against target `addu $a0,$a2,$zero` at 8002D680, allocated $a0 = target's register, feeding both the island input and the srlv exactly as target.

- [s2] Full 154-slot alignment: slots 0-74 are byte-identical INCLUDING register numbers — the entire entrance ladder, both x and z chains, all six mult/mflo pairs and their register mapping ($t0=ax_sq, $a2=az_sq, $a0=cx, $t3=cz, $a1=x1_sq, $v1=z1_sq). The whole residual 30 is downstream of slot 75.

- [s2] The residual now reduces to exactly two coupled items: (a) disc allocated to $3 where target uses $a2/$6 — everything at slots 84-92 and the bltz at 96 follows from it; (b) the mult-shadow ordering flip at 75-78 and its tail twin at 118-122 (the denom sll placement).

- [s2] The 75-78 flip is SCHED2's, not sched1's. sched1 ties all four shadow insns at priority 13 and its backward ready lists produce TARGET order (T-20:194, T-19:196, T-18:197, T-17:200). sched2's post-reload priorities are 197=0x44 vs 200=0x18, so 197 is taken at T-19 and 200 lands earlier. Post-reload priority is downstream of allocation — source statement order cannot reach it (A/H/K measured byte-identical).

- [s2] Variable-reuse as an allocation lever for dist_sq is measured NEGATIVE: reusing x1_sq scores 43 (145 insns), reusing z1_sq scores 57 (145 insns). Both add an instruction.

- [s2] TOOLING: PowerShell on this box cannot invoke `bash` — a .ps1 runner that shells out to `bash tools/wsl.sh` exits 127 silently, the edit is never applied, and every measurement in that loop is a stale re-score of whatever was last in src/. s2 lost one probe round to this. Working pattern: generate complete variant .c files from the Bash tool, then a pure-PowerShell loop that Copy-Item's each into src/ and runs tools/wteng.ps1 main sandbox (tmp/grind/func_8002D518/s2/run3.ps1).

- [s2] TOOLING: tools/grinder/dump.ps1 emits WHOLE-FILE dumps for code6cac_b; slice the function with the `;; Function func_8002D518` line numbers (.rtl:9356, .cse:8396, .combine:8565, .lreg:11527).

## [s3] 2026-08-19 — structural: the tail-shape breakthrough (floor 30 -> 7)

**CHASSIS (third consecutive session).** At s3 dispatch `src/code6cac_b.c` again
did NOT carry the ledger candidate — HEAD still had the pre-s1
`register s32 t4_v asm("t4")` + `.word 0x488CF000` island and measured floor 33.
Additionally `memory/grind/func_8002D518/candidate.c` as stored on disk had
**real newlines inside the `__asm__` string literals** instead of `\n` escapes,
i.e. it was not compilable C; s3 repaired the file in place before splicing.
Re-applied and re-measured floor **30 / 144 == 144** before any new work.

**THE RESULT: floor 30 -> 7, at unchanged instruction parity (144 == 144).**
Two independent edits inside the final divide block, both ordinary C:

1. **`sqrt_val <<= 9;` in place of `s32 sq = sqrt_val << 9;`** — target emits
   `sll $a2,$a2,9`, i.e. the shifted value occupies sqrt_val's own register.
   Measured alone: **30 -> 21**.
2. **Naming the first quotient's numerator** (`num1 = (neg_b + sqrt_val) << 8;`)
   and assigning it BEFORE `denom = dist_sq * 2;` — target computes the
   denominator LAST before the div (`addu $a0,$v0,$a2 / sll $a0,8 /
   sll $v1,$a1,1`). Measured alone (with `sq` still present): **30 -> 19**.

Both together (variant v21): **7**.

**What those two edits fixed — the whole s1 residual cascade.** With v21 the
full 154-slot alignment (objdump `-dz`, so nop-runs are not elided; script
`tmp/grind/func_8002D518/s3/align.py`) shows only 15 raw differing words, of
which 8 are scorer-masked (4 j-targets, 4 `%hi`/`%lo` D_8008D118 relocation
words). Confirmed against the greg dispositions for v21: `dist_sq` (pseudo 116)
now allocates **$a1/5** (was $6), `sqrt_val` (122) **$a2/6** (was $2), `result`
(123) **$a1/5** (was $6); the t1/t2 divide chain, both div/break scaffolds, the
second numerator, the `mflo` destinations and the final `bltz/slti/move` tail
are all byte-identical to target. **The 118-122 "tail twin" ordering flip is
gone**, and so is the 75-78 mult-shadow flip. s2's attribution (post-reload
sched2 priority, unreachable from statement order) was right about *statement
reordering* — what moved it was removing one live pseudo from the divide chain
(variable reuse), which changed allocation and therefore the post-reload
priorities.

**The entire remaining residual is 7 slots and exactly TWO items:**

- **(a) 6 slots — `disc` allocates `$v1`/3 where target uses `$a2`/6.** Slots
  84 (`subu`), 85 (`bgez`), 86 (`sltiu`), 90 (the copy's source register), 92
  (the table-index `addu`), 96 (the `bltz`). Same opcodes, same operand roles,
  same order — only the register number differs.
- **(b) 1 slot — slot 88.** Target emits `addu $v0,$zero,$zero` and jumps to
  `.L8002D774`, the epilogue label *past* `addu $v0,$a1,$zero`; that is a real
  `return 0;` for the `disc < 0` arm. We emit `move $5,$0` and jump to the join.

**Allocation forensics for (a) (fresh dumps, v21 in src).** greg for
func_8002D518: `;; 24 regs to allocate: 117 122 72 121 78 79 123 76 77 132 157
131 105 104 107 75 74 106 118 103 102 116 108 73`. `disc` is pseudo **117** and
is allocated **FIRST** (highest global.c priority — 5 refs across a 6-insn live
range), and it carries `;; 117 preferences: 3`, so `find_reg` hands it `$3`
before any other allocno is considered. Its conflict set is tiny
(`108 116 117 2 29`). In the target compile `disc` must have had either no `$3`
preference or a longer live range: target's `bltz` at slot 96 reads **disc**
($a2) where ours reads the **copy** ($4), so target's `disc` is live ~6 slots
longer than ours and would sort later in the priority order. `set_preference`
(tools/gcc-2.7.2/global.c) records the preference from
`(set (reg 117) (minus ...))` via `XEXP (src, 0)` after `reg_renumber`, so the
preference tracks which hard register local-alloc gave the first `mflo`.

**Measured and KILLED this session (all complete files, Copy-Item runner):**

- v1 inverted LZCS guard (`if (disc>=0){island} else {lzcr=0}`): **32 / 146** —
  materialises the dead arm.
- v2 duplicated `ud = disc;` into both guard arms: **32 / 146** on the floor-30
  chassis, **9 / 146** as v26 on the v21 chassis. It DOES make the `bltz` read
  `disc`, but pays 2 instructions for it.
- v4 `denom` computed immediately after `dist_sq`: **37 / 144**.
- v6 `ud` hoisted above the 0x400 test with every fast-path use routed through
  it: **31 / 145** (re-confirms s2 H6 on the new chassis).
- v7 island reads `disc` instead of `ud` (the asymmetric-operand probe from the
  s2 frontier): **byte-identical to base** — cse canonicalisation makes the asm
  operand spelling irrelevant. s2 frontier item 3 is CLOSED, negative.
- v8/v9/v10 the LITERAL target shape (`u32 ud = disc; s32 lzcr = 0;
  if (disc >= 0) { island }`): score 30 but the **copy re-folds in combine**
  (delay slot back to `nop`, disc back to `$4`). The s2 if/else guard is
  load-bearing and the target's `bltz`-delay-slot `lzcr = 0` is a reorg
  artefact, not evidence about the source shape.
- v11 / v24 `if (disc < 0) return 0;`: produces the target shape for slot 88 but
  **jump2 cross-jumps** the 2-insn block into the entrance return-0 block,
  giving **142 insns** (score 33 on the floor-30 chassis, **10** on the v21
  chassis). Blocked by cross-jumping, not by the spelling.
- v13 split-init accumulation for `disc` was a **C89 declaration-after-statement
  compile error** (91 "insns") — NOT a measurement, discarded.
- v14 named `dsq`/`c4` intermediates, v15 `result`/`sqrt_val` declaration swap,
  v16 `result` declared before `disc`: all **30 / 144 and byte-identical** to
  base — declaration order remains codegen-neutral here.
- v22 naming the SECOND numerator as well (with `sq` kept): **19 / 144** — the
  gain is specifically from `sqrt_val <<= 9` plus naming `num1`, not from naming
  numerators generally.
- v23 `if (t1_val < 0) return 0;` in the result tail: **20 / 142**.
- v25 both products named with the second product first: **27 / 144**.
- v29 dropping `ud` entirely (island and srlv both read `disc`): **7 / 144** —
  same score as v21 but slot 90 is a **nop** where target has
  `addu $a0,$a2,$zero`. Structurally strictly worse; banked as runner-up.

**Tooling banked.** (i) Disassemble with `objdump -dz`, NOT `-d` — plain `-d`
elides nop runs as `...`, which silently mis-aligns every index-based comparison
against `asm/funcs/*.s` (cost s3 one confused turn). (ii) The alignment script
`tmp/grind/func_8002D518/s3/align.py <variant>` prints the 154-slot diff and the
raw-diff count; the masked classes are j-targets and `%hi`/`%lo` reloc words
(raw 15 == engine 7 + 8 masked). (iii) The variant runner is
`tmp/grind/func_8002D518/s3/run.ps1 -Variants a,b,c` and MUST be invoked with
the call operator (`& tmp/.../run.ps1 -Variants ...`) from the PowerShell tool —
`pwsh script.ps1 -Variants a,b` uses `-File` semantics, binds the whole list as
one string, silently copies nothing and re-scores stale src. (iv) Python on this
box reads repo sources as cp1252, so any variant generator must open/write with
`encoding='latin-1'` or the em-dash in the island comment breaks every anchor
match.

- [s3] BREAKTHROUGH: floor 30 -> 7 at unchanged parity 144 == 144, from two ordinary-C tail edits: `sqrt_val <<= 9;` in place (target reuses sqrt_val's register for the shifted value) and naming the first quotient's numerator so `denom = dist_sq * 2` is assigned last. Measured separately 21 and 19; together 7.
- [s3] Those two edits fixed the ENTIRE s1 cascade: dist_sq now $a1, sqrt_val $a2, result $a1, both div scaffolds and the whole tail byte-identical; the 75-78 mult-shadow flip and its 118-122 tail twin are both GONE. s2 was right that statement REORDERING cannot reach a post-reload sched2 priority — but removing a live pseudo from the divide chain changes allocation and therefore those priorities.
- [s3] The residual 7 is exactly two items: (a) 6 slots where `disc` is $v1/3 vs target $a2/6 (slots 84, 85, 86, 90, 92, 96), (b) 1 slot (88) where the `disc < 0` arm should be a real `return 0;` writing $v0 and jumping to the epilogue label past the result move.
- [s3] `disc` = pseudo 117, allocated FIRST of 24 allocnos (5 refs / 6-insn live range) and carries `;; 117 preferences: 3`, so it takes $3 before anything else is considered. Target's disc is live ~6 slots longer (its bltz reads disc, ours reads the copy), which would sort it later in global.c's priority order.
- [s3] `if (disc < 0) return 0;` gives the exact target shape for slot 88 but jump2 CROSS-JUMPS the 2-insn block into the entrance return-0 block: 142 insns vs target 144, score 10. Blocked by cross-jumping, not by the spelling.
- [s3] The LITERAL target shape for the LZCS guard (init lzcr = 0 then plain if) re-folds the disc copy in combine every time (v8/v9/v10). The target's bltz-delay-slot lzcr = 0 is a reorg artefact; the s2 if/else spelling is load-bearing.
- [s3] The asymmetric-operand probe from the s2 frontier (island reads disc, srlv reads ud) is BYTE-IDENTICAL to base — cse canonicalisation makes the asm operand spelling irrelevant. That frontier item is closed, negative.
- [s3] TOOLING: use `objdump -dz`; plain `-d` elides nop runs as `...` and silently mis-aligns index-based comparison against asm/funcs/*.s. Alignment script: tmp/grind/func_8002D518/s3/align.py.
- [s3] TOOLING: invoke the variant runner with the call operator (`& tmp/.../run.ps1 -Variants a,b`); `pwsh script.ps1 -Variants a,b` uses -File semantics, binds the list as one string, copies nothing and re-scores stale src.
- [s3] TOOLING: candidate.c as stored by s2 had REAL newlines inside the __asm__ string literals (not compilable C). s3 repaired it. Verify before splicing.

- [s3] CHASSIS (third consecutive session): src/code6cac_b.c did NOT carry the ledger candidate at dispatch - HEAD still had the pre-s1 register-pin/.word island and measured floor 33. The Grinder commits the ledger, not src/.

- [s3] memory/grind/func_8002D518/candidate.c as stored by s2 contained REAL newlines inside the __asm__ string literals instead of \n escapes - it was not compilable C. s3 repaired the file before splicing; future sessions must check this.

- [s3] Floor is 7 with build_insns 144 == target_insns 144, edits in place in src/code6cac_b.c (candidate.c is the same text).

- [s3] The two edits that did it, both ordinary C: `sqrt_val <<= 9;` in place of `s32 sq = sqrt_val << 9;` (measured alone 30 -> 21) and naming the first quotient's numerator `num1 = (neg_b + sqrt_val) << 8;` assigned BEFORE `denom = dist_sq * 2;` (measured alone 30 -> 19). Together: 7.

- [s3] greg dispositions at floor 7 now match target for the whole tail: pseudo 116 (dist_sq) in 5/$a1, 122 (sqrt_val) in 6/$a2, 123 (result) in 5/$a1. Both div/break scaffolds, both numerators, the mflo destinations and the final bltz/slti/move tail are byte-identical.

- [s3] The 75-78 mult-shadow ordering flip and its 118-122 tail twin are BOTH GONE. s2's attribution was right that statement REORDERING cannot reach a post-reload sched2 priority - but removing one live pseudo from the divide chain changes allocation and therefore those priorities.

- [s3] The entire residual 7 is two items: (a) 6 slots where `disc` is $v1/3 vs target $a2/6 - slots 84 subu, 85 bgez, 86 sltiu, 90 the copy's source, 92 the table-index addu, 96 the bltz - same opcodes, same roles, same order, only the register number differs; (b) 1 slot (88) where the disc < 0 arm should be a real `return 0;` writing $v0 and jumping to the epilogue label past the result move.

- [s3] Allocation forensics: greg prints `;; 24 regs to allocate: 117 122 72 121 78 79 123 76 77 132 157 131 105 104 107 75 74 106 118 103 102 116 108 73` - `disc` is pseudo 117, allocated FIRST (5 refs across a 6-insn live range = highest global.c priority) and carries `;; 117 preferences: 3`, so find_reg hands it $3 before any other allocno is considered. Its conflict set is just `108 116 117 2 29`.

- [s3] Target's `disc` is live ~6 slots longer than ours: target's bltz at slot 96 reads disc ($a2) where ours reads the copy ($4). A longer live range sorts 117 later in global.c's priority order, which is the most likely reason target's disc did not get first pick of $3.

- [s3] set_preference (tools/gcc-2.7.2/global.c) records a hard-reg preference for a pseudo from `(set (reg N) (minus ...))` by taking XEXP(src, 0) after reg_renumber, so 117's $3 preference tracks the hard register local-alloc gave one of the two mflo results.

- [s3] The scorer masks j/branch targets and %hi/%lo relocation words: the v21 alignment shows 15 raw differing words = 7 real + 8 masked (4 j-targets, 4 D_8008D118 reloc words).

- [s3] TOOLING: disassemble with `objdump -dz`, never plain `-d` - plain -d elides nop runs as `...` and silently mis-aligns every index-based comparison against asm/funcs/*.s.

- [s3] TOOLING: invoke the variant runner with the call operator (`& tmp/grind/func_8002D518/s3/run.ps1 -Variants a,b,c`); `pwsh script.ps1 -Variants a,b` uses -File semantics, binds the list as a single string, copies nothing and re-scores stale src.

- [s3] TOOLING: Python on this box reads repo sources as cp1252, so a variant generator must open/write with encoding='latin-1' or the em-dash in the island comment breaks every anchor match.
