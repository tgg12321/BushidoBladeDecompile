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

## [s4] permuter modality — the v21 chassis is a strict local minimum

- [s4] CHASSIS (fourth consecutive session): `src/code6cac_b.c` at dispatch did
  NOT carry the ledger candidate — HEAD still had the pre-s1 register-pin/.word
  island. Re-applied `candidate.c` with
  `tmp/grind/func_8002D518/s4/splice.py memory/grind/func_8002D518/candidate.c`
  and re-measured: **score 7, build_insns 144 == target_insns 144**. The floor
  is unchanged from s3; s4 did not lower it.
- [s4] A REUSABLE full-TU permuter workspace now exists and is validated:
  `tmp/grind/func_8002D518/s4/mkws.sh` builds `tmp/perm_d518/` from scratch
  (preprocessed `src/code6cac_b.c` as base.c, an engine-faithful compile.sh, and
  target.o assembled from `asm/funcs/func_8002D518.s` + the r3000-patched
  permuter prelude). Its self-validation prints `base insns: 154  target: 154`
  and a diff that is EXACTLY the known 7-slot residual — so the permuter metric
  here is the honest residual, not an artefact.
  - Two gotchas cost turns and are banked: (i) maspsx emits `.ent`/`.end` at
    COLUMN 0 (no leading tab), so a `/^\t\.ent\tfunc/` extraction awk silently
    produces an empty object — match `/^\.ent\tfunc_8002D518$/`; (ii) the Bash
    tool's quoted heredoc COLLAPSES `\n` to `\n` when writing a generator
    script, which turns the island's `"...\n"` escapes into real newlines and
    makes base.c unparseable ("missing terminating \" character"). Write
    generator scripts with the Write tool, or repair afterwards with
    `tmp/grind/func_8002D518/s4/mkperm2b.py`.
- [s4] RANDOM CAMPAIGN (`tmp/perm_d518`, label `s4-v21-chassis`, -j 6):
  **33,881 iterations / 1,132 s wall / ZERO finds.** permuter base score 35.
  Score histogram of the whole run: **35 x8,286 (the base, never beaten)**,
  39 x21, 40 x11, 45 x347, 50 x54, everything else higher. The v21 chassis is a
  STRICT LOCAL MINIMUM under decomp-permuter's random pass — every mutation the
  randomizer can make is neutral or uphill. Per-iteration scores banked at
  `tmp/grind/func_8002D518/s4/campaign_random_scores.txt`.
- [s4] DIRECTED CAMPAIGN (`tmp/perm_d518b`, label `s4-directed-guard-perm`):
  the exhaustive 3x4x2 = **24-point cross-product** of {outer `disc < 0` exit
  shape} x {inner LZCS guard shape} x {slow-path table-index operand} —
  i.e. the interaction of the three axes s2/s3 had only measured one at a time.
  permuter enumerated all 24 and exited. **None below base 35**; six are exactly
  base-equivalent, eighteen strictly worse (230/235/430/545/605/740/800). There
  is no interaction effect on these axes; they are closed as a space, not just
  as individual probes.
- [s4] CONSEQUENCE FOR THE FRONTIER: the residual 7 is NOT reachable by generic
  restructuring of this function. 33,881 random samples plus a 24-point directed
  cross-product all bottom out at the base. The `disc`-in-$v1 allocation must be
  attacked through the mechanism s3 named (global.c allocno priority / live-range
  length), not by respelling the block. The next productive modality is
  forensics on `.greg` (which allocno ordering change would flip 117 off $3, and
  what source-visible property produces it), not more sampling.

- [s4] [s4] CHASSIS, fourth consecutive session: src/code6cac_b.c at dispatch did NOT carry the ledger candidate (HEAD still had the pre-s1 register-pin/.word island). Re-applied candidate.c via tmp/grind/func_8002D518/s4/splice.py and re-measured with `sandbox func_8002D518 --disable all`: score 7, build_insns 144 == target_insns 144. Confirmed again at end of session with the edits still in place.

- [s4] [s4] A reusable, VALIDATED full-TU permuter workspace recipe for this function now exists: tmp/grind/func_8002D518/s4/mkws.sh builds tmp/perm_d518/ from scratch and self-validates by printing 'base insns: 154  target: 154' plus a diff that is exactly the known 7-slot residual (slots 85-89 subu/bgez/sltiu/j/move, 91 the copy source, 93 the table-index addu, 97 the bltz). Any future session can rebuild it in one command.

- [s4] [s4] TOOLING GOTCHA A: maspsx emits `.ent` / `.end` at COLUMN 0 (no leading tab), unlike raw cc1 output. An extraction awk written as /^\t\.ent\tfunc_.../ silently produces an EMPTY object and the workspace validation then fails with a bare grep exit-1 and no message. Match /^\.ent\tfunc_8002D518$/.

- [s4] [s4] TOOLING GOTCHA B: the Bash tool's QUOTED heredoc still collapses `\\n` to `\n` when writing a generator script, which turns the LZCS island's `"...\n"` escapes into real newlines and makes the generated base.c unparseable ('missing terminating " character'). Write generator scripts with the Write tool; the repair pass is tmp/grind/func_8002D518/s4/mkperm2b.py. This is the same class of corruption that hit candidate.c between s2 and s3.

- [s4] [s4] RANDOM CAMPAIGN measurement: 33,881 iterations, 1,132 s wall, -j 6, ZERO finds. Full score histogram bottom: 35 x8,286 (the base, never beaten), 39 x21, 40 x11, 45 x347, 50 x54. The v21 chassis is a STRICT LOCAL MINIMUM under decomp-permuter's random pass.

- [s4] [s4] DIRECTED CAMPAIGN measurement: the exhaustive 24-point cross-product of the three guard-region axes returned nothing below base. Six spellings are exactly base-equivalent (so the base spelling is not uniquely privileged, merely tied) and eighteen are strictly worse. There is NO interaction effect between the three axes; they are now closed as a SPACE, not merely as individual probes.

- [s4] [s4] CONSEQUENCE: the residual 7 is not reachable by generic restructuring of this function. Two independent searches - 33,881 random samples and a complete directed enumeration - bottom out at the base. The `disc`-in-$v1 allocation must be attacked through the mechanism s3 already named (global.c allocno priority for pseudo 117: 5 refs over a 6-insn live range makes it the FIRST of 24 allocnos, and `;; 117 preferences: 3` then hands it $3), not by respelling the block.

- [s4] [s4] Both campaigns were harvested with --stop inside this turn; `pgrep -af permuter.py` returns nothing, so no campaign outlives the session.

---

## s5 (forensics) — the `disc`→$6 question is now CLOSED ARITHMETICALLY

Chassis re-measured at dispatch: HEAD src/code6cac_b.c **again** did not carry
candidate.c (FIFTH consecutive session — the Grinder commits the ledger, not
src/). Re-applied candidate.c verbatim → `sandbox --disable all` score **7**,
build_insns 144 == target_insns 144. All s5 numbers are on that chassis.

Dumps: `pwsh tools/grinder/dump.ps1 func_8002D518` (canonical cc1; the stock
`-da` dumps named every pass involved, so no BB2_*_DEBUG instrumented run was
needed). Saved slices in `tmp/grind/func_8002D518/s5/`.

### E1 — WHERE disc's `$3` actually comes from (three passes, named)

`.greg` header for func_8002D518 (base chassis):

    ;; 24 regs to allocate: 117 122 72 121 78 79 123 76 77 132 157 131 105 104 107 75 74 106 118 103 102 116 108 73
    ;; 117 conflicts: 108 116 117 2 29
    ;; 117 preferences: 3

pseudo 117 == `disc` (confirmed in `.lreg`: insn 209 `(set (reg/v:SI 117)
(minus (reg 118) (reg 121)))`, then bgez 212, sltiu 224, table index 234, copy
245 `(set (reg 132) (reg 117))`).

The `preferences: 3` is INHERITED, not direct:

1. `.lreg` insn 207 `(set (reg 121) (mult (reg 119) (reg 120)))`.
   `global.c:set_preference` (called from `mark_reg_store`, global.c:1484) does
   `if (GET_RTX_FORMAT(GET_CODE(src))[0] == 'e') src = XEXP (src, 0)`, i.e. for
   a non-copy SET it takes the **first operand** of the RHS — reg 119.
   Local-alloc had already placed 119 (`dist_sq >> 9`) in $3
   (`.lreg` → `;; Register 119 in 3.`), so allocno 121 gets
   `hard_reg_preferences |= {3}`.
2. `global.c:expand_preferences` then merges 121's preference into 117: the subu
   (insn 209) is a `single_set` whose dest is 117 and which carries
   `REG_DEAD (reg 121)`, and 117/121 do not conflict → preferences OR'd both ways.
3. `global.c:find_reg` allocates 117 **first** (see E3) and takes $3.

**This preference is NOT the cause of the divergence.** mips.h defines no
`REG_ALLOC_ORDER`, so find_reg's fallback is the ascending hard-reg scan; with
$2 in 117's hard-conflict set the first free reg is $3 regardless. Measured:
swapping the multiply operands (`(c_val << 2) * (dist_sq >> 9)`) → score **10**,
worse (`rejected/mult-operand-swap-score10.c`). Frontier item (iii) from s4 —
"re-associate the discriminant so the $3-preference source lands elsewhere" — is
therefore **KILLED**. Only CONFLICTS can move disc.

### E2 — WHY disc dies at the copy: cse.c, not global.c

`.rtl` (pre-cse) line 9946: the inner LZCS guard is
`(if_then_else (ge:SI (reg/v:SI 117)) ...)` — it reads **disc**.
`.cse` line 8943: the same jump_insn now reads `(ge:SI (reg/v:SI 132))` — **ud**.
Ref counts across passes confirm the single substitution happens exactly at cse
(117: 30→27, 132: 11→12 between `.rtl`/`.jump` and `.cse`; unchanged thereafter).

The predicate is `cse.c:make_regs_eqv (new=132, old=117)`. 132 replaces 117 as
`qty_first_reg` (the canonical register cse substitutes for the whole quantity)
iff BOTH:

* `uid_cuid[regno_last_uid[132]] > cse_basic_block_end`
  (or `uid_cuid[regno_first_uid[132]] < cse_basic_block_start`), and
* `uid_cuid[regno_last_uid[132]] > uid_cuid[regno_last_uid[117]]`
  — i.e. **`ud`'s last use is later than `disc`'s last use**.

Both hold: `ud` is used by the `__asm__` island and by `ud >> shift` in the join
block, while `disc`'s last pre-cse use is the bltz. So cse rewrites the bltz to
read `ud`, `disc` dies at the copy, and its live range collapses to 6 insns.
Target keeps `bltz $a2` on `disc` (asm/funcs/func_8002D518.s, 0x8002D698) while
the island and the `srlv` use `$a0` (= `ud`) — so in the original build the same
two pseudos exist but cse did NOT canonicalise onto `ud`.

Emitted asm (base chassis, `tmp/grind/func_8002D518/dumps/code6cac_b.s`) confirms
the copy lands in the beqz delay slot exactly like target
(`beq $2,$0,.L276 / move $4,$3` vs target `beqz $v0,.L8002D698 /
addu $a0,$a2,$zero`); the ONLY difference in that whole region is that our
`bltz $4` reads ud where target's `bltz $a2` reads disc.

### E3 — the exact allocno priority table (reproduced by hand, model validated)

`global.c:allocno_compare` sorts descending by
`floor_log2(n_refs) * n_refs / live_length` (×10000×size). From `.lreg`
("Register N used R times across L insns"), base chassis:

| pos | allocno | refs/len | pri | gets |
|----|----|----|----|----|
| 1 | **117 disc** | 5/6 | 16667 | $3 |
| 2 | 122 | 6/11 | 10909 | $6 |
| 3 | 72 threshold | 9/26 | 10385 | $4 |
| 4 | 121 | 2/2 | 10000 | $3 |
| 5 | 78 | 4/9 | 8889 | $3 |
| 6 | 79 | 4/9 | 8889 | $3 |
| 7 | 123 result | 4/9 | 8889 | $5 |
| 8 | 76 | 4/10 | 8000 | $5 |
| 9 | 77 | 4/10 | 8000 | $5 |
| 10 | **132 ud** | 4/11 | 7273 | $4 |
| 11 | 157 | 2/3 | 6667 | $3 |
| 12 | 131 lzcr | 3/5 | 6000 | $3 |
| 13-21 | 105 104 107 75 74 106 118 103 102 | | | |
| 22 | **116 dist_sq** | 3/33 | 909 | $5 |
| 23 | 108 | 3/36 | 833 | $7 |
| 24 | 73 r_sq | 2/46 | 435 | $9 |

This reproduces the printed `;; 24 regs to allocate:` order exactly, including
the 8889 and 8000 ties (broken by ascending allocno index). The model is
therefore validated and can be used PREDICTIVELY by later sessions — compute
`floor_log2(refs)*refs/len` from any `.lreg` and you know the allocation order
before you build.

### E4 — what `disc` in $6 would REQUIRE (closed form)

`find_reg` blocks a hard reg for allocno A only via (a) A's hard-reg conflict
set, or (b) a hard reg already assigned to an allocno that CONFLICTS with A.
Non-conflicting allocnos freely share a hard reg, so "who else already has $6"
is irrelevant. $6 is reachable for 117 only when 2, 3, 4 and 5 are all blocked.
In this function:

* **$2** — permanent hard conflict of 117. Free.
* **$3** — becomes a hard conflict of 117 as soon as disc is live across the
  lzcr region (observed directly in the s5 M1 variant:
  `;; 117 conflicts: 108 116 117 131 2 3 12 29`). Reachable.
* **$4** — the ONLY conflicting-allocno candidate is **132 (`ud`)**, pri 7273,
  position 10. So 117 must (i) conflict with 132 — i.e. `disc` must still be
  live at/after the copy — and (ii) sort BELOW 132, i.e. pri(117) < 7273.
* **$5** — candidates holding $5 are 123 (pri 8889, pos 7), 76/77 (8000, pos
  8/9) and 116 `dist_sq` (909, pos 22). 117 already conflicts with 116, but 116
  is 22nd, so using it needs pri(117) < 909. With 6 refs
  (`floor_log2(6)=2`) that means `12/L < 0.0909` → **live_length > 132 insns** in
  a 144-insn function. **Impossible.** So the $5 blocker must be 123, 76 or 77,
  none of which currently conflicts with disc.

Combining the $4 and $5 constraints, the target allocation requires
**simultaneously**:

1. `disc` live past the `ud` copy (conflict with 132) — so `ud` must still exist
   as its own allocno; and
2. `disc` conflicting with one of 123 / 76 / 77 (an early-allocated $5 holder); and
3. `pri(117) < 7273` — with 6 refs `live_length >= 17`, with 7 refs `live_length >= 20`.

Requirement 1 is in direct tension with cse (E2): the only measured way to keep
117 alive past the copy is to make `ud`'s last use earlier, and that makes cse
merge 132 away entirely (E5). Requirement 2 has no current mechanism at all —
123 (`result`) is only live on the `disc < 0` arm and at the join, where disc is
dead; 76/77 are the early z-range locals.

**Standing conclusion for later sessions: "get disc into $6 by lowering its
allocno priority / lengthening its live range" — the s1..s4 frontier head — is
DEAD as a single-axis attack.** Priority alone can never do it, because the only
$5 holder disc conflicts with (116) sits at position 22 and would need
live_length > 132. Any future attempt must produce a NEW conflict edge from disc
to an early $5 holder, which is a different (and currently unmechanised) axis.

### E5 — the M1 measurement (equal score, strictly worse platform)

Variant: slow-path index spelled `(&D_8008D118)[(u32)disc >> shift]` instead of
`[ud >> shift]`, so `ud`'s last use becomes the `__asm__` island.
Measured: **score 7**, 144 == 144 (a tie, not a gain).

Forensics on the M1 dumps: `;; 23 regs to allocate: 122 72 121 117 78 79 …` —
allocno **132 is gone**. cse's E2 predicate flipped, 117 stayed canonical, every
`ud` use was rewritten to 117, and 117 absorbed the refs
(`Register 117 used 7 times across 15 insns`, pri = 2*7/15 = 9333, position 4).
Its conflict set grew to `108 116 117 131 2 3 12 29` (note the new hard 3, and
hard 12 from the island's `"$12"` clobber), and disc moved **$3 → $4**.

That is one hard reg closer in raw terms but structurally a dead end: it deletes
the only allocno (132) that can ever block $4 for disc. Banked as
`rejected/srlv-reads-disc-merges-away-ud-allocno-score7.c`.

Side note worth keeping: the M1 variant is an equal-score, *different* 7-slot
residual with disc in $4. If a later session ever finds a way to block $4 by a
hard conflict rather than by allocno 132, M1 becomes the better starting chassis.

### E6 — artifacts

* `tmp/grind/func_8002D518/s5/greg.func.txt` — base `.greg` slice (allocno list,
  conflicts, preferences, dispositions, post-reload RTL).
* `tmp/grind/func_8002D518/s5/lreg.func.txt` — base `.lreg` slice (pseudo RTL,
  per-register refs/live-length, local-alloc dispositions incl. `Register 119 in 3`).
* `tmp/grind/func_8002D518/s5/jump2.func.txt` — base `.jump2` slice, captured for
  the frontier's slot-88 cross-jump item. NOT analysed this session (the s5 turn
  budget went entirely to the allocation item); it is a free head start for s6.
* `tmp/grind/func_8002D518/s5/base.c`, `m1_src.c` — the two measured TU states.
* `tmp/grind/func_8002D518/s5/evidence_s5.md` — this text.

- [s5] CHASSIS: HEAD src/code6cac_b.c did NOT carry candidate.c for the FIFTH consecutive session (the Grinder commits the ledger, not src/). Re-applied it verbatim; sandbox --disable all = score 7, build_insns 144 == target_insns 144. All s5 numbers are on that chassis, and it is the state src/ is left in.

- [s5] pseudo 117 == `disc`, confirmed from .lreg RTL: def at insn 209 `(set (reg/v:SI 117) (minus (reg 118) (reg 121)))`, then bgez 212, sltiu 224, table index 234, and copy 245 `(set (reg 132) (reg 117))` (== `u32 ud = disc;`). Base stats: 5 refs across 6 insns.

- [s5] VALIDATED ALLOCNO PRIORITY TABLE (base chassis; pri = floor_log2(refs)*refs/live_length x10000), reproduces the printed allocation order exactly: 117 disc 5/6 = 16667 -> $3 (pos 1) | 122 6/11 = 10909 -> $6 | 72 threshold 9/26 = 10385 -> $4 | 121 2/2 = 10000 -> $3 | 78 4/9 = 8889 -> $3 | 79 4/9 = 8889 -> $3 | 123 result 4/9 = 8889 -> $5 | 76 4/10 = 8000 -> $5 | 77 4/10 = 8000 -> $5 | 132 ud 4/11 = 7273 -> $4 (pos 10) | 157 2/3 = 6667 -> $3 | 131 lzcr 3/5 = 6000 -> $3 | ... | 116 dist_sq 3/33 = 909 -> $5 (pos 22) | 108 3/36 = 833 -> $7 | 73 r_sq 2/46 = 435 -> $9. Later sessions can use this predictively: compute the ratio from any .lreg and you know the order before you build.

- [s5] `;; 117 conflicts: 108 116 117 2 29` on the base chassis — disc conflicts with NOTHING that holds $3, $4 or $5 at the time it is allocated, and it is allocated FIRST of 24. That, not the preference, is why it takes $3.

- [s5] CLOSED-FORM REQUIREMENT for disc -> $6 (evidence.md E4): (1) disc must be live past the `ud` copy so it conflicts with allocno 132 ($4, position 10) — which means `ud` must SURVIVE as its own allocno; AND (2) disc must conflict with an early-allocated $5 holder, i.e. allocno 123 (`result`, pos 7) or 76/77 (pos 8/9) — 116 `dist_sq` is unusable because it sits at position 22; AND (3) pri(117) < 7273, i.e. live_length >= 17 at 6 refs or >= 20 at 7 refs. Requirements (1) and (3) are in direct tension with cse (see the cse.c finding), and (2) has no known mechanism at all.

- [s5] The emitted asm already matches target everywhere in this region EXCEPT the register: our `beq $2,$0,.L276 / move $4,$3` is the exact analogue of target's `beqz $v0,.L8002D698 / addu $a0,$a2,$zero` (the ud copy in the beqz delay slot, stolen from the branch target by reorg's fill_slots_from_thread, which is why the label advanced past it in BOTH builds). The single structural difference is `bltz $4` (ud) vs target `bltz $a2` (disc).

- [s5] The M1 variant (slow-path index on disc) is an equal-score-7 but STRUCTURALLY DIFFERENT chassis with disc in $4 and no separate `ud` allocno. Recorded as a fallback: if a later session ever finds a way to block $4 by a HARD conflict (rather than via allocno 132), M1 becomes the better starting chassis.

- [s5] NOT analysed this session: the slot-88 / jump2 cross-jump residual (frontier item b). Its .jump2 slice was captured anyway and is banked at tmp/grind/func_8002D518/s5/jump2.func.txt as a free head start.

---

## s6 (forensics) — floor UNCHANGED at 7 (re-measured on the re-applied candidate; 144 == 144)

Chassis note, SIXTH consecutive session: `src/code6cac_b.c` at dispatch did NOT
carry `memory/grind/func_8002D518/candidate.c`. HEAD held a hybrid pre-s3 form
(`register s32 t4_v asm("t4")` island + `s32 sq = sqrt_val << 9;` divide tail).
The candidate was re-applied and re-measured at score 7 / 144 insns before any
probe, and restored to that exact state at end of session.

### E7 — cse's register canonicalisation is STRICTLY EITHER/OR. The target's two-register `disc`/`ud` split CANNOT come from a source-level plain copy. (closed form; KILLS the s5 frontier item 3)

Read from `tools/gcc-2.7.2/cse.c:826` (`make_regs_eqv`) and
`cse.c:8008` (`cse_end_of_basic_block`), not hypothesised:

* `make_regs_eqv(new=132 ud, old=117 disc)` places BOTH regs in ONE quantity `q`
  and sets `qty_first_reg[q]` to exactly one of them. Every subsequent *read* of
  that quantity anywhere in the extended block is rewritten to `qty_first_reg`.
* Therefore whichever register loses canonicality has **zero surviving
  consumers**, and its defining copy is dead. There is no cell of the predicate
  in which both `disc` and `ud` keep consumers.
* Both halves are already measured:
  - base / if-else guard (shape A): `ud` wins → the bltz reads `ud`, `disc`'s
    live range collapses to 6 insns (`Register 117 used 5 times across 6 insns`).
  - s5 M1 / `[(u32)disc >> shift]` and the s3 v8/v9/v10 "init lzcr then if"
    shape (shape B): `disc` wins → allocno **132 disappears entirely**
    (`;; 23 regs to allocate`), i.e. the copy is deleted.
* **Consequence.** The s5 frontier's "untested 2×2 cell" (win the cse extended
  block extent so that condition (a) fails while (b) still holds) is **not a
  free cell**: winning it only flips WHICH register survives. It can never
  satisfy s5/E4 requirement 1 (`disc` live past the copy *while* allocno 132
  still exists). Requirement 1 is unreachable from any plain-copy spelling.
  The target's `addu $a0,$a2,$zero` at slot 97 must therefore come from
  something other than a C-level `u32 ud = disc;` — a reload/asm-operand copy or
  a source in which the two values are not cse-equivalent.

### E8 — the exact extended-block mechanism (source-attributed, so later sessions need not re-derive it)

`cse_end_of_basic_block` (cse.c:8039) scans `while (p && GET_CODE (p) != CODE_LABEL)`
— the block ends at the first CODE_LABEL — with exactly two escapes:
* **TAKEN** (cse.c:8100): follow a conditional jump when
  `LABEL_NUSES (JUMP_LABEL (p)) == 1` **and the target label is preceded by a
  BARRIER**. Our if/else guard compiles to `bgez disc,L_else / <then> / j L_join /
  L_else: <island> / L_join:` — `L_else` *is* barrier-preceded, so cse follows it,
  scans the island, and then stops dead at `L_join`. `ud`'s last use (the
  `srlv` after `L_join`) is therefore OUTSIDE the block → predicate (a)
  `uid_cuid[regno_last_uid[132]] > cse_basic_block_end` is TRUE → `ud` promoted.
  That is the whole reason the base has `ud` canonical.
* **AROUND** (cse.c:8149): follow a conditional jump that branches *around* a
  block, i.e. the target label is NOT barrier-preceded (fall-through). The
  "init `lzcr = 0` then plain `if`" spelling — which is literally what target's
  `bltz $a2,.L8002D6BC` + fall-through island reads like — takes this path, so
  the block extends PAST the join label and over the `srlv`, predicate (a) goes
  FALSE and `disc` stays canonical. Which is exactly the shape that s3 measured
  as losing the copy, and E7 explains why: keeping `disc` canonical *is* what
  deletes `ud`.

### E9 — s5/E4 requirement 2 IS reachable in ordinary C, but it is ANTI-CORRELATED with the allocation order (measured, score 11)

Probe: hoist the default value — `s32 result = 0;` before the discriminant, guard
spelled `if (disc >= 0) { ... }`, inner `result = 0;` dropped.
* Result: **the new conflict edge appears** —
  `;; 117 conflicts: 108 116 117 123 2 29` (123 = `result`, the $5 holder).
  s5's "requirement 2 has no current mechanism at all" is superseded.
* But the same edit is what makes the edge: `result` must be live across `disc`,
  so `Register 123` goes 4 refs/9 insns → **3 refs/37 insns**, pri 8889 → **811**,
  allocno position **7 → 23 of 24**. It is now allocated long AFTER 117 and
  blocks nothing; it took `$8`. Score 11, build_insns 143 (parity lost).
* **General law for this function:** any edit that lengthens a $5 holder's live
  range enough to overlap `disc`'s 6-insn range simultaneously divides that
  holder's `allocno_compare` priority by the same length. Creating the edge and
  keeping the blocker early are in direct arithmetic tension.

### E10 — slot 88 is ALLOCATION-NEUTRAL; the s5 frontier's "(a) and (b) are one problem" is FALSE (measured, score 10, dumps read)

Probe: `if (disc < 0) return 0;` (the s3 form) re-measured **with dumps**.
* `.greg`: `;; 117 conflicts: 108 116 117 2 29` — **unchanged**; no 123 edge.
* `.lreg`: 117 unchanged bit-for-bit (5 refs/6 insns, pri 16667, position 1,
  `;; 117 preferences: 3`, gets `$3`). 123 goes 4 refs/9 → 3 refs/7, pri
  8889 → 4286, position 7 → **13**: it moves DOWN the order, i.e. *further* from
  being able to block `$5`.
* So solving slot 88 does not and cannot create the `$5` blocker. Frontier
  items (a) and (b) are independent.

### E11 — the slot-88 loss is NOT a `find_cross_jump` selectivity question (the s5 frontier's premise is refuted)

Counted directly in the dumps for both the base chassis and the `return 0;` variant:
* `.jump` (pre-reload) carries **9** distinct `(set (reg/i:SI 2 v0) (const_int 0))`
  return-0 blocks; `.jump2` (post-reload) carries **1** — in BOTH builds.
  jump2's cross-jumper is not selective at all: it merges every one of them.
* The final target asm nevertheless contains **9** `addu $v0, $zero, $zero`
  instructions, and so does our 144-insn base build. Those 9 copies are
  RE-MATERIALISED after jump2, by `reorg.c` filling each `j <label>` delay slot
  with the instruction at the jump target (and redirecting the jump past it).
* **Consequence.** The s5 frontier statement "target and our build BOTH already
  carry two UN-merged copies … so find_cross_jump is selective here" is wrong.
  Slot 88 is a **reorg/delay-slot** question (which `j`+`addu` pairs reorg
  re-expands), not a jump2 cross-jump question. Any future probe on slot 88
  should read `.dbr`, not `.jump2`.

- [s6] CHASSIS, sixth consecutive session: src/code6cac_b.c at dispatch did NOT carry memory/grind/func_8002D518/candidate.c (HEAD held a hybrid pre-s3 form: `register s32 t4_v asm("t4")` island + `s32 sq = sqrt_val << 9;` divide tail). The candidate was re-applied and re-measured at score 7 / build_insns 144 == target_insns 144 before any probe, and restored to exactly that state at end of session (re-verified score 7 / 144).

- [s6] cse.c:826 make_regs_eqv puts pseudo 132 (`ud`) and pseudo 117 (`disc`) into ONE quantity and rewrites every later read of that quantity to qty_first_reg - so exactly one of the two ever has consumers, and the other's defining copy is dead. Both halves are already measured (base: ud canonical, 117 = 5 refs/6 insns; s5 M1 and s3 v8-v10: disc canonical, `;; 23 regs to allocate`, allocno 132 gone).

- [s6] cse.c:8008 cse_end_of_basic_block scans `while (p && GET_CODE (p) != CODE_LABEL)` with exactly two escapes: TAKEN (cse.c:8100 - needs LABEL_NUSES(JUMP_LABEL)==1 AND the target label BARRIER-preceded) and AROUND (cse.c:8149 - needs the target label NOT barrier-preceded). Our if/else LZCS guard takes the TAKEN path, which ends the block at the inner join label and leaves `ud`'s last use (the srlv) outside it, so make_regs_eqv condition (a) holds and `ud` is promoted. The init-lzcr-then-if shape takes the AROUND path, which extends the block past the join over the srlv, so `disc` stays canonical - and that is exactly why that shape loses the copy.

- [s6] MEASURED: hoisting `s32 result = 0;` above the discriminant DOES create the conflict edge that s5/E4 called mechanism-less - `;; 117 conflicts: 108 116 117 123 2 29`. score 11, build_insns 143. But `Register 123` goes 4 refs/9 insns (pri 8889, allocno position 7) to 3 refs/37 insns (pri 811, position 23 of 24) and takes $8 instead of $5.

- [s6] MEASURED: `if (disc < 0) return 0;` gives score 10 / build_insns 142 with allocno 117 bit-for-bit unchanged (5 refs/6 insns, pri 16667, position 1, preference 3, gets $3) and conflicts unchanged at `108 116 117 2 29`; 123 goes 4 refs/9 to 3 refs/7 (pri 8889 to 4286, position 7 to 13).

- [s6] MEASURED: .jump = 9 return-0 blocks, .jump2 = 1, in both the base chassis and the return-0 variant; the target asm and our 144-insn base build each contain 9 `addu $v0, $zero, $zero`. The duplication is reorg.c delay-slot filling, not surviving jump2 blocks.

- [s6] NEW ARITHMETIC OPENING (not yet probed): global.c allocno_compare priority is floor_log2(refs)*refs/live_length. s5 closed the priority axis using disc's current 5-6 refs, where floor_log2 = 2 and beating 116 (`dist_sq`, pri 909, holds $5, and ALREADY conflicts with 117) needs live_length > 132. But floor_log2(2) = floor_log2(3) = 1, so a 3-ref disc needs only live_length > 33 and a 2-ref disc only > 22. The reference COUNT on the pseudo that must land in $a2, not its live length alone, is the untried variable.

## s7 (forensics) — FLOOR 7 -> 4 at held parity 144 == 144. s5/E4's closed-form "impossible" was based on an INCOMPLETE model of find_reg's blockers.

Chassis at dispatch: src/code6cac_b.c did NOT carry the s6 candidate (7th time).
Re-applied `memory/grind/func_8002D518/candidate.c` and re-measured: score 7,
144 == 144. All s7 numbers below are relative to that re-measured base.

### E12 — THE CORRECTION. A pseudo's hard-reg conflict set is fed by `local_alloc`'s block-local assignments, not only by RTL hard regs. This is the third blocker channel s5/E4 never modelled, and it is the one that opens `$a2`.

s5/E4 stated: "`find_reg` blocks a hard reg for allocno A only via (a) A's
hard-reg conflict set, or (b) a hard reg already assigned to an allocno that
CONFLICTS with A", and then derived requirement 2 (a NEW conflict edge from
`disc` to an early-allocated `$5` holder) as the only route. Channel (a) was
treated as fixed. It is not — it is a **source-controllable function of the live
range**, because `global.c`'s `global_conflicts` treats every pseudo with
`reg_renumber >= 0` (i.e. every pseudo `local_alloc` already placed) as a live
HARD REG. Read directly out of the base `.lreg`:

    ;; Register 94 in 4.   ;; Register 95 in 4.   ;; Register 96 in 5.
    ;; Register 100 in 3.  ;; Register 119 in 3.  ;; Register 124 in 2.
    ;; Register 128 in 2.  ;; Register 133 in 3.  ;; Register 137 in 4.
    ;; Register 141 in 3.  ;; Register 142 in 4.  ;; Register 154 in 5.  (etc.)

The proof that this is the operative channel is allocno **122** (`sqrt_val`) in
the base: `;; 122 conflicts: 108 116 122 123 2 3 4 5 29 64 65 66` — hard 2,3,4,5
all blocked, and 122 duly gets **$6**, while there is no RTL hard reg 3/4/5
anywhere in the function's RTL (the only RTL hard regs are `$4..$7` at insns
4/6/8/10 (incoming args), `$2` at each `return`, `$29/$30`, HI/LO, and `$12`
from the island's clobber — verified by scanning the `.lreg` RTL). Base `disc`
(117) had `;; 117 conflicts: 108 116 117 2 29` purely because its live range was
6 insns (209 def, 212 bgez, 224 sltiu, 234 fast-path index, 245 copy) and only
`$2`-assigned locals (124 in block 20, 128 in block 21) overlapped it.

**LAW for this function:** to move a global allocno onto a specific hard reg,
lengthen its live range so it spans block-locals that `local_alloc` has already
placed in the registers you want blocked. Priority/order is not the only lever
and, per E9, is the arithmetically self-defeating one.

### E13 — the lever that does it: `disc` and `sqrt_val` are ONE C variable (sanctioned variable-reuse), which is what target's register file literally shows

Target keeps the discriminant, the square root and `sqrt<<9` all in `$a2`
(0x8002D668 `subu $a2,$a0,$v1` … 0x8002D694 `srl $a2,$v0,3` … `sll $a2,$a2,9`).
Deleting the separate `sqrt_val` local and reusing `disc` merges the two pseudos
into one:

    base : Register 117 used  5 times across  6 insns; ;; 117 conflicts: 108 116 117 2 29
    s7   : Register 117 used 13 times across 19 insns;
           ;; 117 conflicts: 108 116 117 122 131 2 3 4 5 12 29 64 65 66

117 stays allocno **position 1 of 23** (pri floor_log2(13)*13/19 = 20526), so
nothing else in the allocation order moves — but with hard 2,3,4,5 blocked
`find_reg` can only return **$6 = $a2 = target's register**. All six `disc`
slots close in one edit. Measured: score 7 -> 6 (build_insns 146, +2).

### E14 — the second edit: on the s7 chassis the init-then-overwrite LZCS guard is worth 2 slots, reversing the s2/s3 verdict

`lzcr = 0; if (disc >= 0) { island; lzcr = sp_tmp; }` — the shape s2/s3 rejected
because cse's AROUND escape (E8) keeps `disc` canonical and deletes the
`u32 ud = disc;` copy. On the s7 chassis that deletion is cheaper than the
if/else's extra `j`: **score 6 / 146 insns (if/else) -> score 4 / 144 insns
(init guard)**. Parity is restored and the diff drops to 4 slots. The s3
candidate-header instruction "the LZCS guard is a real if/else — do not undo" is
SUPERSEDED for the variable-reuse chassis.

### E15 — the 4-slot residual, normalised (tmp/grind/func_8002D518/s7/align3.py)

    T[ 74] addu $2,$0,$0     B addu $5,$0,$0     <- slot 88, the disc<0 arm
    T[ 76] addu $4,$6,$0     B --                <- the `ud` copy is folded away
    T[ 84] addu $12,$4,$0    B addu $12,$6,$0    <- island input reads $6 not $4
    T[ 93] srlv $2,$4,$3     B srlv $2,$6,$3     <- slow-path index reads $6 not $4

(The two `lui $1,%hi(...)` entries the script also prints are unlinked-reloc
noise, not real diffs.) So the residual is exactly TWO items: the `ud` copy
(3 slots) and slot 88 (1 slot) — the same two items s3 named, minus the six
`disc` register slots.

### E16 — both residual items re-measured on the s7 chassis; both verdicts UNCHANGED from s6

* **`ud` hoisted above the `(u32)disc < 0x400u` test** (E8's idea of moving the
  copy into an earlier extended block so the island read and the `srlv` read
  both fall outside it): **score 4 / 144 — exactly neutral**, byte-identical
  residual. cse still makes `disc` canonical and still deletes the copy. s6/E7's
  either-or law holds on the new chassis: with a plain C copy, target's
  simultaneous `$a2`-with-consumers + `$a0`-with-consumers split is unreachable.
  Banked `rejected/s7-ud-hoist-over-0x400-neutral-score4.c`.
* **`if (disc < 0) return 0;`** for slot 88: **score 8 / 142 insns** — still 2
  insns short, still the jump2 cross-jump of the 2-insn return-0 block (s6/E10,
  E11). Banked `rejected/s7-disc-lt0-return0-still-crossjumps-score8-142insns.c`.

### E17 — artifacts

`tmp/grind/func_8002D518/s7/`: `apply.py` (brace-matched form splicer),
`align3.py` (normalised target-vs-build diff), `dis.sh`, `base.lreg.txt`,
`base.greg.txt` (re-measured floor-7 chassis), `m1.lreg.txt`, `m1.greg.txt`
(the `[(u32)disc >> shift]` control: 117 = 7 refs/15 insns, hard conflicts
{2,3,12,29} only — no 4, no 5, which is why M1 never reached $6),
`vr.lreg.txt`, `vr.greg.txt` (the variable-reuse build showing the
{2,3,4,5} hard-conflict set), `base_src.c`, `m1.c`, `v_reuse.c`,
`v_reuse_initguard.c` (= the new candidate), `v_ig_udhoist.c`, `v_ig_ret0.c`,
`v/*.dis`.

- [s7] Chassis re-check at s7 dispatch: src/code6cac_b.c did NOT carry the s6 candidate for the SEVENTH consecutive session (HEAD held a hybrid `sq` + `[(u32)disc >> shift]` form). Re-applied candidate.c and re-measured the stated base: score 7, build_insns 144 == target_insns 144.

- [s7] END-OF-SESSION STATE: src/code6cac_b.c carries the new s7 form and re-measures score 4, build_insns 144 == target_insns 144. memory/grind/func_8002D518/candidate.c is that exact text with a full s7 header.

- [s7] s5/E4's standing conclusion ('get disc into $6' is DEAD; find_reg blocks a hard reg only via the allocno's own hard-reg conflict set or via a conflicting allocno that already holds it) was CORRECT about the two channels it listed but WRONG to treat channel (a) as fixed: global.c counts local_alloc-placed block-locals (reg_renumber >= 0) as live hard regs, so channel (a) is a source-controllable function of live-range length. This is E12 and it is the whole session.

- [s7] Direct proof in the base .lreg: ';; Register 94 in 4. ;; Register 95 in 4. ;; Register 96 in 5. ;; Register 100 in 3. ;; Register 124 in 2. ;; Register 128 in 2. ;; Register 133 in 3. ;; Register 137 in 4. ;; Register 141 in 3. ;; Register 142 in 4.' - and allocno 122 (sqrt_val), whose range spans the $3/$4/$5 ones, carries ';; 122 conflicts: ... 2 3 4 5 ...' and is assigned $6.

- [s7] The function's RTL contains NO hard reg 3, 4 or 5 outside insns 4/6/8/10 (the incoming-arg copies), so 122's hard conflicts with 3/4/5 cannot come from RTL hard regs - only from local_alloc.

- [s7] Base allocno table re-validated bit-for-bit against s5/E3: 117 = 5 refs/6 insns (pri 16667, position 1 of 24, ';; 117 preferences: 3', gets $3); ';; 117 conflicts: 108 116 117 2 29'. Its refs are insn 209 (subu def), 212 (bgez), 224 (sltiu), 234 (fast-path table index), 245 (the ud copy).

- [s7] The M1 control ('[(u32)disc >> shift]', no reuse) measured this session: 117 = 7 refs/15 insns, allocno position 4, ';; 117 conflicts: 108 116 117 131 2 3 12 29' - hard 3 present but NOT 4 and NOT 5, which is the precise reason M1 never reached $6 despite the longer range. Score 7 / 144.

- [s7] s7 form: 117 = 13 refs/19 insns, still allocno position 1 of 23, ';; 117 conflicts: 108 116 117 122 131 2 3 4 5 12 29 64 65 66' -> $6 = $a2 = target.

- [s7] Target's register file corroborates the variable-reuse reading directly: $a2 carries the discriminant (0x8002D668 `subu $a2,$a0,$v1`), the square root (0x8002D694 `srl $a2,$v0,3`) and the <<9 result (`sll $a2,$a2,9`) - one register, one C variable.

- [s7] The 4-slot residual, normalised (nop / reg-name / pseudo-mnemonic / reloc insensitive): T[74] `addu $2,$0,$0` vs `addu $5,$0,$0` (slot 88); T[76] target's `addu $4,$6,$0` MISSING in our build (the folded ud copy); T[84] `addu $12,$4,$0` vs `addu $12,$6,$0` (island input); T[93] `srlv $2,$4,$3` vs `srlv $2,$6,$3` (slow-path index). Two independent items: the ud copy = 3 slots, slot 88 = 1 slot.

- [s7] Both residual items were re-measured on the new chassis and both s6 verdicts held: ud-hoist-above-0x400 is exactly neutral (4/144, identical residual), and `if (disc < 0) return 0;` still loses 2 insns to jump2 (8/142).

- [s7] Reusable tooling written this session and worth inheriting: tmp/grind/func_8002D518/s7/apply.py (brace-matched splice of a form file into src/code6cac_b.c - removes the chronic re-apply friction) and tmp/grind/func_8002D518/s7/align3.py (normalised target-vs-build diff that survives the sandbox's dropped multu_pad nops, so the residual is readable without the engine's scorer).

- [s8] **FUNCTION SOLVED - honest sandbox distance 0** (`--disable all`, 33 rules dropped, 289 cheat-asm lines stripped, build_insns 144 == target_insns 144). Chassis re-measured at dispatch: the s7 candidate re-applied cleanly and reproduced floor 4, so the s7 ledger was chassis-accurate. Final form saved to memory/grind/func_8002D518/candidate.c and IN PLACE in src/code6cac_b.c.

- [s8] E14 (the ud copy, the frontier head since s1) - **the copy is a cse.c survival question, not a reload/local_alloc question**. s7's frontier guessed "post-cse, by reload satisfying the asm's r operand or by a local_alloc range split". That guess is WRONG and is now closed by dump. With ONE `u32 ud = disc;` the .cse dump (tmp/grind/func_8002D518/s8/dumps_nodup/code6cac_b.cse) shows the copy insn is GONE - the block runs `(code_label 240) -> (note 242) -> (insn 248 set (reg 130) (const_int 0))` with no reg-reg set anywhere, and every `ud` read has been rewritten to `(reg/v:SI 117)`. cse deletes it outright. Writing the assignment a SECOND time inside the `if (disc >= 0)` guard arm makes the pseudo multiply-defined; in the resulting .cse dump (dumps_v6) BOTH copies survive verbatim as `(insn 245 (set (reg/v:SI 131) (reg/v:SI 117)))` before the guard and `(insn 255 (set (reg/v:SI 131) (reg/v:SI 117)))` inside it.

- [s8] E15 - the allocation that follows is exactly target's. In dumps_v6/code6cac_b.greg the same two insns read `(set (reg/v:SI 4 a0) (reg/v:SI 6 a2))`, i.e. pseudo 117 (disc) -> $a2 and pseudo 131 (ud) -> $a0, and the island's asm input operand is `(reg/v:SI 4 a0)`. The redundant second copy is dropped before final output, so insn parity holds at 144 and the surviving copy is reorg-hoisted into the `beqz` delay slot at 0x8002D680 - target's `addu $a0,$a2,$zero`, in target's register, for free.

- [s8] E16 - statement ORDER inside the else-arm decides which of the two delay slots each insn lands in. `lzcr = 0; ud = disc;` gives score 3 with `addu $3,$0,$0` in the beqz slot and `addu $4,$6,$0` in the bltz slot (i.e. the two are swapped vs target); `ud = disc; lzcr = 0;` gives score 1 with both in target's slots. reorg fills the beqz slot from the first insn of the branch-target thread, so source order maps directly onto delay-slot occupancy here.

- [s8] E17 (slot 88 / T[74], dead since s6 as a "reorg delay-slot re-materialisation question") - **it was never a reorg question; it is a two-locals question.** Target's `disc < 0` arm writes the RETURN register (`j .L8002D774` + `addu $v0,$zero,$zero` in the delay slot) and jumps PAST the join's `addu $v0,$a1,$zero` at 0x8002D770. Spelling the arm `return 0;` is measured dead a second time (s8/v5: score 5, build_insns 142 - jump.c cross-jumps the `$v0=0; j epilogue` block into an earlier return-0 site, which leaves `bgez`+`j` adjacent and then inverts them to a single `bltz $6, <early ret0>`; the disassembly shows the branch going to 0x1930). The real reading of `addu $v0,$a1,$zero` is that `result` (the returned value, $v0) and the comparison flag ($a1) are two DIFFERENT locals: writing `s32 flag = 0; if (t1_val >= 0) flag = t2_val < 0x101; result = flag;` puts `result` in $v0, so the `disc < 0` arm's `result = 0` is already a write of $v0 and its `j` targets the epilogue. Score 1 -> 0.

- [s8] E18 (negative results banked this session): assigning `ud` in BOTH arms of an explicit if/else guard costs 3 insns (score 6 / 147) whether the island reads `ud` (s8/v1) or `disc` (s8/v2) - the arms do not cross-jump and the copy is materialised twice. A fresh m2c decompile (tmp/grind/func_8002D518/s8/m2c_raw.c, via `python3 tools/m2c/m2c.py`, NOT `python3 -m m2c.main` which is not installed) collapses $a0 into $a2 exactly as our folded builds do, so m2c carries no information about the copy; its only useful signal was the two-variable `var_a2`/`var_a1` split, which is E17.

## [s9] 2026-08-19 — synthesis: the merged attack, and the closed-form proof that the last 3 slots REQUIRE a second definition

(This session follows the s8 run whose sandbox-0 candidate was bounced by the
layer-1 cheat-reviewer on 2026-08-19 07:16 — a FAMILY-CITATION failure, not a
codegen failure. Numbered [s9] because the ledger's [s8] slot is taken.)

### E18 — chassis re-verified from scratch

`src/code6cac_b.c` at dispatch did NOT carry `candidate.c` (same trap as s2 —
it still had the HEAD `register s32 t4_v asm("t4")` + `.word` island). Applied
`candidate.c` verbatim and measured **score 0 / build_insns 144 == target 144 /
rules_dropped 33 / cheat_asm_stripped 289**. The s8 form is REAL and
reproducible; the ONLY thing between this function and COMPLETED-C is the
disposition of one construct. Apply script (reusable, LF-safe, handles an
optional `/* BB2GRIND-HELPER */ … /* BB2GRIND-HELPER-END */` prefix block):
`tmp/grind/func_8002D518/s8b/apply.py <variant.c>`.

### E19 — the residual on any copy-less chassis is EXACTLY the copy, 3 slots, nothing else

Disassembled the copy-less chassis (`tmp/grind/func_8002D518/s8b/v9.dis`,
generated by `tmp/grind/func_8002D518/s1/dis.sh`). At score 3 the three
differing slots are, and are only:
1. the missing `addu $a0,$a2,$zero` in the `beqz` (0x400-test) delay slot —
   we emit a `nop`;
2. `move $12,$6` where target has `addu $t4,$a0,$zero` (island operand reads
   `$a2` instead of the copy);
3. `srlv $2,$6,$3` where target has `srlv $v0,$a0,$v1` (slow-path shift reads
   `$a2` instead of the copy).
Every other insn in the 144 — both entrance chains, the mult cascade, the
c_val/dist_sq ordering, the LZCS island bytes, the div scaffolds, the two-local
`flag`/`result` tail — is byte-identical. There is no second residual item left
anywhere in this function.

### E20 — THREE new copy spellings measured, all KILLED (this is the synthesis contribution)

All measured on the s8 chassis with the FAKE duplicate REMOVED, so the delta is
purely the copy spelling. Banked in `rejected/`.

* **s9-A — `static inline` helper, the argument copy as the source of `$a0`.**
  The most attractive "honest original source" reading of this function: the
  redundant `bltz $a2` at 0x8002D698 (dominated by `bgez $a2` at 0x8002D66C, so
  provably always-taken-false) and a spare register holding the same value are
  the classic fingerprint of an INLINED helper with its own internal
  negative-guard and its own parameter pseudo. Built
  `static inline s32 sqrt_lzcs(u32 ud)` carrying the island, the `lzcr` guard
  and the mantissa-table tail, called as `disc = sqrt_lzcs(disc);`.
  RESULT: GCC 2.7.2 at `-O2` DOES inline it (no `jal`, no standalone body
  emitted, parity held at 144) — but the parameter copy folds exactly like a
  plain local copy. **Score 3.** integrate.c's parameter pseudo is an ordinary
  pseudo to cse; it buys nothing. This kills the whole "the original was an
  inlined helper" reading as a matching lever (it may still be historically
  true; it is codegen-irrelevant).
  → `rejected/s9-static-inline-helper-param-copy-folds-score3.c`
* **s9-B — `u32 ud = disc;` hoisted above the 0x400 test AND consumed by the
  small path.** Rationale: reorg fills the `beqz` delay slot from the insn
  PRECEDING the branch, so target's copy was emitted before the 0x400 test;
  and giving `ud` a use in the fall-through (small-path) block as well as in
  the branch-taken block puts consumers on both sides of the split. Spelled
  `u32 ud = disc; if ((u32)disc < 0x400u) { disc = table[ud] >> 3; } else {…}`.
  RESULT: **score 3**, parity 144. cse canonicalises both uses to `disc` and
  deletes the copy; an extra use in the fall-through block does not perturb
  `make_regs_eqv` at all. (Distinct from the s7 `ud-hoist-over-0x400` reject,
  which did not add the small-path use.)
  → `rejected/s9-ud-hoisted-smallpath-also-reads-ud-score3.c`
* **s9-C — `ud` defined ONLY inside the guard arm (single def, never a dead
  store).** `u32 ud; lzcr = 0; if (disc >= 0) { ud = disc; <island>; lzcr = … }`
  with the post-join `table[ud >> shift]` read. This is the only single-def
  PLACEMENT the ledger had never measured (s1-s8 all defined `ud` before the
  guard). RESULT: **score 8, build_insns 145 — parity LOST.** Worse than the
  copy-less baseline; the arm-local def costs an insn and does not survive.
  → `rejected/s9-ud-defined-only-inside-guard-arm-score8-145insns.c`

### E21 — CLOSED FORM: the surviving copy REQUIRES a second definition of `ud` or `disc`, and every second definition that is not a same-value re-store costs bytes

Merging s6/E7 + s6/E8 + s8's confirmed duplicate finding + E20 gives a complete
decision procedure for this slot, so no future session needs to sample spellings
again:

1. For the copy insn to reach output, BOTH `disc` and `ud` must still have
   consumers after cse. (E19: target reads `$a2` three times — `sltiu`,
   small-path table index, `bltz` — and `$a0` twice — island operand, slow-path
   `srlv`.)
2. cse's `make_regs_eqv` (cse.c:826) puts the two regs in ONE quantity and
   rewrites every read in the extended block to `qty_first_reg`, so exactly one
   of them keeps consumers — **unless the quantity is invalidated between the
   two reads.** (s6/E7; both cells of the extended-block predicate are measured:
   if/else guard → `ud` wins, `disc` dies, score 6/146; init-then-`if` guard →
   `disc` wins, `ud` dies, score 3/144.)
3. The quantity is invalidated only by a SET of `ud` or of `disc` between the
   island read and the `srlv` read. cse does not invalidate registers for a
   volatile `asm` (it invalidates memory only), so the island cannot do it.
4. In that window the target's own register file shows `$a2` (`disc`) and `$a0`
   (`ud`) are BOTH unwritten — every intervening write goes to `$v1` (`lzcr`,
   then `shift`) or `$v0`. So the invalidating set cannot be a live computation
   reusing either variable: reusing `disc` for `shift` or for the table index,
   or reusing `ud` for `ud >>= shift`, forces that pseudo's single hard register
   to serve two different target registers and costs slots elsewhere.
5. Therefore the invalidating set must store a value equal to the one already
   there — i.e. a redundant same-value re-store of `ud` (or of `disc`). That is,
   by definition, the dead-store / self-assignment construct, and it is the
   construct s8 measured as the unique 3-slot closer.

**Consequence for the pipeline.** The last 3 slots of func_8002D518 are not a
search problem any more; they are a POLICY question about one construct whose
family (`.claude/rules/dead-store-fake-exception.md`, "dead conditional store"
/ self-assignment class, itself on the frozen SOTN-accepted list) is sanctioned
and whose four strict prerequisites this function satisfies in full — but which
the driver's `banned_constructs` tripwire now mechanically rejects, because the
layer-1 FAIL that created the ban was a FAMILY-CITATION defect and the ban text
was minted from the construct rather than from the defect. See s9 hypotheses and
the outcome's `ruling_question`.

## E22 (s10, synthesis) — the s8 match is chassis-stable, and the block was a citation, not the construct

The chassis check at dispatch reported "measurement unavailable" and the ledger's
last recorded floor was 4, so the s8 result was treated as unproven and
re-measured from scratch. The s8 body was applied verbatim to
`src/code6cac_b.c` (it had NOT been left in the tree — HEAD still carried the
pre-s8 form with two `register s32 t4_v asm("t4")` pins and `.word`-encoded cop2
instructions), and the only edit made to it was to the FAKE annotation's
family reasoning.

MEASURED, this session, current chassis:

    & tools/wteng.ps1 main sandbox func_8002D518 --disable all
    { "score": 0, "target_insns": 144, "build_insns": 144, "scorable": true,
      "rules_dropped": 33, "cheat_asm_stripped": 289 }

So the honest floor is **0**, not 4: the 3-slot `ud`-copy closer and the 1-slot
`flag`/`result` split both reproduce on this chassis. Note what else that number
says — the applied form ALSO deletes the two register pins and the two `.word`
cop2 encodings the tree had been carrying for this function, replacing them with
the constraint-bound island in the `func_800274BC`-accepted shape.

**The block was never substantive.** docs/grind/decisions.md:6517 records the
layer-1 FAIL, whose single confirmed defect is the FAMILY CITATION
(duplicated-statement-into-arms, whose scope does not describe this code shape);
decisions.md:6521 records the Judge narrowing the ban to exactly
"this construct cited under duplicated-statement-into-arms" and authorising
resubmission under dead-store-fake-exception. This session therefore did the one
thing the ledger's own s9 conclusion asked for: re-derived the justification
under the family the construct actually matches, checked that family's four
strict prerequisites one at a time against the source and the ledger, and
rewrote both the in-source annotation and `self_vet.md` with verbatim scope
sentences and file:line precedents
(`.claude/rules/dead-store-fake-exception.md:24` for the scope block, `:28` for
the `x = x;` self-assignment sub-scope, `.claude/rules/no-new-park-categories.md:260`
for the frozen-list confirmation, `docs/grind/decisions.md:6521` for the Judge's
own reading of this exact construct). No duplicated-statement-into-arms claim
survives anywhere in the source or the vet.

**One correction to the previous vet's family bookkeeping, worth inheriting.**
The named-intermediate frozen-list entry
(`.claude/rules/no-new-park-categories.md:189`) qualifies only ONCE-WRITTEN,
ONCE-READ intermediates (`:196-:204`; multi-write carriers are explicitly not
that entry — the `y1` FAIL at decisions.md:1833 stands). Of the three
intermediates the old vet claimed under it, only `num1` fits. `ud` has two live
reads and `flag` has two writes — so neither is claimed as a matching lever at
all; both stand as ordinary C on their own semantics (`ud` is the island's
operand and the table index; `flag` is a default-0 boolean the join copies into
the return value). Claiming a family you do not need is how the previous
submission failed layer-1; the narrower claim is the durable one.

**Remaining work is entirely process.** Bytes are proven cheat-free at distance
0 with all 33 rules dropped. What is still owed is: a fresh layer-1 review of
this diff, the Judge, then the operator path — `engine build` (SHA1 == oracle),
`engine retire func_8002D518` (drops all 33 regfix/asmfix rules),
`engine verify-oracle --rebuild`, `engine queue done func_8002D518`.

## [s11] 2026-08-19 — synthesis: the copy CAN survive with no dead store (first time in 11 sessions), and the exact price of that is measured

Chassis re-verified from scratch at dispatch. `src/code6cac_b.c` again did NOT
carry `candidate.c` (HEAD still had the pre-s8 `register s32 t4_v asm("t4")` +
`.word` form — the eleventh consecutive session to find that). Applied the
copy-less control and the distance-0 form and measured BOTH this session:

    v_nodup  (candidate minus the FAKE re-store)  -> score 3, build_insns 144 == target 144
    base_dup (candidate.c verbatim)               -> score 0, build_insns 144 == target 144
                                                     rules_dropped 33, cheat_asm_stripped 289

So the ledger's two numbers are both chassis-accurate right now: the honest
cheat-free floor of a form with NO annotated construct is **3**, and the
distance-0 form is real and reproducible.

### E23 — five new spellings measured; the ud/tval and shift/half variable reuses are exactly codegen-neutral

All five are cheat-free ordinary C (variable reuse only), all at parity 144.
Target's own register file motivates each: `$a0` carries BOTH `ud` and the
mantissa-table value (`lbu $a0` at 0x8002D6D8, `sll $a0,$a0,16`), and `$v1`
carries `lzcr`, then `shift`, then `half` (`srl $v1,$v1,1` at 0x8002D6DC).

* **s11-A — `ud` reused for the table value** (`ud = (&D_8008D118)[ud >> shift];`
  then `disc = (ud << 16) >> (0x13 - ((u32)shift >> 1))`): **score 3**, byte-
  identical residual to the control. The reuse is exactly neutral: it does NOT
  invalidate the cse quantity, because the second def of `ud` sits AFTER `ud`'s
  last read.
* **s11-C — A + `shift` reused for `half`** (target's `srl $v1,$v1,1` shape):
  **score 3**, again byte-identical. Neutral.
* **s11-B — C + the `u32 ud = disc;` copy hoisted above the `(u32)disc < 0x400u`
  test** (target's insn ORDER — the copy is in the `beqz` delay slot, i.e. it
  precedes the branch in RTL): **score 3**. Neutral. Confirms s7/E16 and s9-B on
  the reuse chassis.
* **s11-D — `disc` itself reused as the `shift` carrier**, i.e. a REAL
  value-carrying second definition of `disc` between the island's read of `ud`
  and the `srlv`'s read of `ud`
  (`disc = 0x16 - (lzcr & ~1); tval = tbl[ud >> disc]; disc = (tval<<16) >> (0x13 - ((u32)disc>>1));`):
  **score 10 — AND THE COPY SURVIVES.** The aligned diff
  (`tmp/grind/func_8002D518/s11` + `s7/v/vD.dis`) shows `addu $4,$6,$0` present
  in the build for the first time without any dead store; it lands in the `bltz`
  delay slot instead of the `beqz` slot (swapped with `lzcr = 0`), and the
  separate `tval` local takes `$v1`/`$a0` in the wrong order.
* **s11-E — D + A (`disc` as the shift carrier AND `ud` reused for the table
  value): score 4**, and the aligned diff is only FOUR real slots:

      T[ 84] addu $12,$4,$0     B addu $12,$6,$0   <- island operand reads disc, not ud
      T[ 92] subu $3,$3,$2      B subu $6,$3,$2    <- shift lives in $a2, target $v1
      T[ 93] srlv $2,$4,$3      B srlv $2,$4,$6    <- ditto (index source $4 is CORRECT)
      T[ 97] srl $3,$3,1        B srl $3,$6,1      <- ditto

  The copy `addu $4,$6,$0` is present, in target's slot, in target's registers,
  and the slow-path `srlv` reads it — i.e. two of the three copy slots that have
  been the frontier head since s1 are CLOSED by ordinary C with no annotated
  construct at all.
  Banked `rejected/s11-disc-as-shift-copy-SURVIVES-but-shift-reg-wrong-score4.c`.

### E24 — CLOSED FORM (upgrade of s9/E21 from deduction to measurement): every value-carrying invalidation costs strictly more than it buys

E23-D/E identify the mechanism precisely: the cse equivalence `ud ≡ disc` is
broken by ANY second definition of `disc` in the window, and a real
value-carrying one works just as well as a same-value re-store. So the question
is no longer "does an honest invalidation exist" (it does) but "can the value it
carries live in the hard register target uses". Reading target's register file:

* `$a2` (`disc`) is read at 0x8002D66C `bgez`, 0x8002D670 `sltiu`, 0x8002D688
  (small-path table index) and 0x8002D698 `bltz`, and is next WRITTEN only at
  0x8002D694 (small path) / 0x8002D6EC (slow-path result). In the whole window
  between the copy and the `srlv`, target writes `$v1` and `$v0` only.
* Therefore any C that invalidates by writing `disc` forces `disc`'s single
  pseudo to carry a value target holds in `$v1` — measured cost exactly 3 slots
  (E23-E: T92/T93/T97).
* The island's operand can only read the COPY's register if the equivalence is
  already broken BEFORE the island, and the only insn between the copy and the
  island in target is the `bltz` guard test, which reads `disc` and defines
  nothing. The window therefore admits no value-carrying write at all — which is
  why E23-E still emits `addu $12,$6,$0` (T84).
* Writing `ud` instead is not available: `ud` must still hold `disc` at the
  island and at the `srlv`, so any write to it is by definition same-valued.

**Conclusion (measured, not deduced).** The three-slot copy residual is closable
in exactly one way: a redundant same-value re-store of `ud` in the guard arm
(the s8 form, distance 0). Every alternative invalidation is now measured, and
each one pays 3 or more slots for the 3 it buys. This retires the s1-s10
frontier item "find an honest spelling of the copy" as CLOSED-NEGATIVE, and it
is the strongest available evidence for the construct's necessity: it is not a
convenience, it is the unique zero-cost invalidation.

### E25 — the remaining blocker is a driver-state artifact, not codegen

`memory/grind/func_8002D518/state.json` still carries
`banned_constructs = ["`u32 ud; ud = disc; lzcr = 0; if (disc >= 0) { /* FAKE ... */ ud = disc; <island>; lzcr = sp_tmp; }`"]`,
banked by `Set-FailRouting` from the 2026-08-19 07:16 layer-1 FAIL
(docs/grind/decisions.md:6517), whose ONE confirmed defect was the sanctioned-
family CITATION (duplicated-statement-into-arms). The Judge then ruled PASS at
07:34 (decisions.md:6521) and narrowed the ban to "this construct cited under
duplicated-statement-into-arms", authorising resubmission under
dead-store-fake-exception. The narrowing landed in `judge_constraints` only:
`grindlib.py` has `ban` / `constrain` but **no un-ban path**, and
`check_banned_constructs` is a term-overlap tripwire over the self-vet's
`CONSTRUCTS:` block, so the authorised resubmission is mechanically discarded
before any reviewer sees it. That is what happened to the session immediately
before this one (its self-vet tripped on `disc, (disc, disc, island)`), and a
third consecutive discard on this function trips `Circuit-Break`.

The only honest dispositions available to a session are therefore (a) worsening
the form to avoid the construct (floor 3 instead of 0), or (b) an INTEGRATION
HANDOFF. Wording a `CONSTRUCTS:` line to stay under the tripwire's 4-hit
threshold while still using the construct would be evading a mechanical gate, so
it was not done. This session takes (b).

- [s8] Chassis re-verified from scratch: src/code6cac_b.c did NOT carry candidate.c at dispatch (eleventh consecutive session) - HEAD still had the two register-asm t4 pins plus .word cop2 encodings. Applied forms measure: candidate.c -> score 0, build_insns 144 == target_insns 144, rules_dropped 33, cheat_asm_stripped 289; the same body minus the annotated re-store -> score 3 / 144.

- [s8] The distance-0 body also REMOVES the two register pins and the two .word cop2 encodings the tree carries for this function, replacing them with the constraint-bound GTE LZCS island in the func_800274BC-accepted shape (.claude/rules/cop2-addressing-preamble-cluster.md:74).

- [s8] s11-A/B/C (ud reused for the table value; shift reused for half; copy hoisted above the 0x400 test) are exactly codegen-NEUTRAL: score 3, parity 144, byte-identical residual. A second def placed after the variable's last read cannot invalidate a cse quantity.

- [s8] s11-D (disc reused as the shift carrier) makes target's `addu $a0,$a2,$zero` survive with NO dead store - score 10, copy present but in the bltz delay slot; s11-E (D + ud reused for tval) is score 4 with the copy in target's slot and registers and the slow-path srlv reading it. First honest materialisation of that copy in 11 sessions.

- [s8] Closed form (evidence.md E24, now measured rather than deduced): every value-carrying invalidation forces disc's pseudo to also carry a value target holds in $v1 (measured 3-slot price for a 3-slot gain), and no invalidation can precede the island read because target emits only the `bltz` guard between the copy and the island. The same-value re-store of `ud` is therefore the unique zero-cost invalidation.

- [s8] The remaining blocker is driver state, not codegen: state.json banned_constructs still holds the construct text banked by Set-FailRouting from the 2026-08-19 07:16 layer-1 CITATION FAIL (docs/grind/decisions.md:6517), while the 07:34 Judge ruling (decisions.md:6521) PASSED and narrowed the ban to that citation alone and authorised resubmission under dead-store-fake-exception. grindlib.py exposes `ban` and `constrain` but no un-ban command, and check_banned_constructs is a term-overlap tripwire over the self-vet CONSTRUCTS: block - so the authorised resubmission is discarded as INVALID before any reviewer sees it (exactly how the previous session died, matched on `disc, (disc, disc, island)`).

- [s8] Wording a CONSTRUCTS: line to stay under the tripwire's 4-hit threshold while still using the construct would be evasion of a mechanical gate, so it was not attempted; the honest alternatives were a deliberately worse form (floor 3) or this integration handoff.
