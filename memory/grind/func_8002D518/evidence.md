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
