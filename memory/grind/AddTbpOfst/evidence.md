# Evidence bank — AddTbpOfst

## Session 1 (recon, 2026-07-20)

- **Baseline**: canonical verdict C (49 insns, distance 3). Honest floor
  `sandbox --disable all` = **3**. 3 regfix subst rules (all register renames,
  lines 488-490 of regfix.txt — do not touch): andi source $7->$4 @1,
  sll dest $3->$2 @30, addu src $3->$2 @33.
- **Function identity**: am_rmd module (Kengo tag `am_rmd/AddTbpOfst`, 49i),
  sound/VAB table setup. Neighbors func_80087F64 / tslCDFileRead (main.c,
  matched, COMPLETED) share the `D_80102A68[idx]` guard idiom.
- **Two independent diff clusters**:
  1. **andi operand** (@1): target `andi $v0,$a0`; we emit `andi $v0,$a3copy`.
     RTL (tmp/grind/AddTbpOfst/s1/main.i.{rtl,greg}): u16 param makes TWO
     pseudos — 73 (SI incoming, ->$4, dies at insn 24) and 72 (HI var, ->$7,
     lives to the sb). Guard expands as zero_extendhisi2(72); the (s16) idx
     sext expands as shifts on subreg:SI(72) which CSE folds to 73 (that's why
     sll/sra read $4 correctly). CSE cannot fold a bare REG operand (72) to a
     subreg equivalence — only subreg chains. Combine cannot fold insn6
     (72=subreg(73)) into the zero_extend because 72 is multi-use (the sb);
     added_sets_2 PARALLEL fails recog. Neighbor func_80087F64 proves the fold
     WORKS when the param var fully dies (s16 param, `(u16)a0` guard, no raw
     sb use — its andi reads $4 and it has NO $a3 copy). AddTbpOfst target has
     BOTH the copy and the raw andi, so the original had some spelling where
     the sb's value reaches RTL as pseudo-73-derived. Not found yet.
  2. **sll placement/register** (@30/@33): target `sll $v0,$v1,4` at slot 32
     (BEFORE the two sb's), fresh value in $2, then `addu $v0,$v0,$a0` after
     the sb's. Target's slt @18 keeps sa1 in $3 => the shifted value is
     PROVABLY a fresh pseudo, not in-place sa1. Fresh-subexpr spelling
     `entry = *((s32*)(((sa1<<4)+v1)+8));` gets regs RIGHT (sll $2, addu $2)
     but emits sll at slot 36 (after sb's) — the do{}while(0) fence between
     the sb's blocks the hoist. In-place `sa1=sa1<<4` gets the PLACEMENT
     right (scheduler hoists above sb1) but reg wrong ($3). Named temp `ofs`
     perturbs RA globally (7). Fence removal scrambles the tail (19/20).
- **Fence is load-bearing**: do{}while(0) between the sb's is required; no
  fence = 19, fence before both sb's = 5 (order sb1,sll,addu,sb2 — close but
  wrong), fence after both = same as A1.
- **Score table**: baseline 3 | A1 (banked) 3 | A2/A3 named-temp 7 | A4
  no-fence 19 | A5 entry-early 20 | A6 fence-first 5 | B s32-params 6 |
  C/D s16-param-family 7 | E u16-saved 3 (no-op) | E2 u8-saved 5 (50 insns).
- Artifacts: tmp/grind/AddTbpOfst/s1/ (main.i, main.i.{rtl,greg,combine,...},
  diffasm.sh side-by-side differ, rtldump.sh).

- [s1] Honest floor 3 (sandbox --disable all, 49/49 insns); canonical verdict C, distance 3; 3 regfix substs = pure register renames

- [s1] RTL dumps (main.i.rtl/.greg): u16 param a0 = pseudos 73 (SI incoming, $4, dies insn 24) + 72 (HI var, $7=$a3, lives to sb); guard = zero_extendhisi2(72) -> andi from $7; (s16) sext = shifts on subreg(72) which cse folds to 73 -> reads $4 correctly; 72 conflicts with hard reg 4 so RA can never fix the andi

- [s1] Matched neighbor func_80087F64 (s16 param, (u16)a0 guard, no raw-value store) proves combine folds the copy into the zero-extend when the param var fully dies: andi from $4, no $a3 copy. AddTbpOfst target has BOTH the copy and the raw andi

- [s1] Target slt $v0,$v1,$v0 @18 proves sa1 stays in $3 => shift value is a fresh pseudo in $2 (in-place spelling structurally cannot match)

- [s1] Score table: baseline 3, A1(banked) 3, named-temp 7, no-fence 19, entry-early 20, fence-first 5, s32-params 6, s16-family 7, u16-saved 3(no-op), u8-saved 5
