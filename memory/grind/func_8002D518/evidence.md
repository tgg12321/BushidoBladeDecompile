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
