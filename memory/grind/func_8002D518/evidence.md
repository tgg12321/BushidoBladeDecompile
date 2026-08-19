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
