# Evidence bank — func_80017D84

## Session 1 (recon, 2026-09-02) — MATCHED: sandbox --disable all = 0, verify-oracle ok:true

### E-s1-1. Function shape and chassis
66-insn object-slot allocator (asm/funcs/func_80017D84.s). Walks 8 slots of 0x34 bytes in
g_file_data_buf (= D_800F6740) for the first slot whose word 0 is zero; returns -1 if none;
raises the u8 high-water mark D_800A30E8 to the slot index; fills slot fields from the
descriptor param (u16 @+0 -> slot+4, s32 @+4 -> slot+0, s16 @+2 -> slot+8 as s32, s32 @+0x10 ->
slot+0xC and slot+0x10 = that + (slot h4 << 6), slot+6 = 0) and copies 32 bytes from
*(ptr @+0xC) into slot+0x14..+0x33 (two batches of 4 lw / 4 sw through v1,a0,a1,a2 = GCC 2.7.2
mips block move, MAX_MOVE_REGS=4); then calls func_80017A44(desc, slot) and returns the index.
canonical verdict C (pure-C distance 36 measured on the first draft; driver-dispatch floor was
"unavailable" because src carried INCLUDE_ASM). The retired-chassis body
(retired-chassis-2026-08/body.c, floor 37) was entirely register-asm pins + asm barriers; nothing
from it was reused.

### E-s1-2. cc1 -w SILENTLY DROPS a struct assignment whose cast names an UNDECLARED typedef
Draft 1 wrote the block copy as `*(MATRIX *)(p + 0x14) = **(MATRIX **)(a0 + 0xC);`. gte.h is not
included by src/ings.c (only code6cac_b.c and sound.c include it), so MATRIX is undeclared in
that TU. cc1 with -w produced NO error and emitted NO code for the statement: sandbox 36,
build_insns 45 (the 16 block-move insns + the `move t0,a0` param copy were absent, and the slot
pointer landed in a1 instead of a3 because a0 was never clobbered). Raw cc1
(tmp/grind/func_80017D84/s1/cc1.sh) confirmed the drop is cc1's, not the sandbox stripper's.
Banked in rejected/undeclared-matrix-type-statement-dropped.c. Lesson for siblings: a missing
struct copy in the build with no compile error means "check the type name is declared in THIS TU".

### E-s1-3. Block move + tail order: the ONLY residual after the struct copy was scheduling
With a TU-local `typedef struct { s32 v[8]; } ObjBlock;` the block move, the `move t0,a0` param
copy and the a3 slot pointer all appear (register allocation matched with no coercion). The
remaining diff was the order of the 12 tail insns. A raw-cc1 sweep (tmp/grind/func_80017D84/s1/
sweep.py) of all 24 legal orders of the five independent tail statements {w8: slot+8 = s16 desc+2;
c = desc+0x10; h6: slot+6 = 0; wC: slot+0xC = c; w10: slot+0x10 = c + (h4<<6)} (c before wC/w10)
gave exactly ONE zero-mismatch order: (w8, c, h6, wC, w10); every other order mismatched 10-11 of
the 12 tail insns. Applied that order: sandbox --disable all = 0 (66/66), verify-oracle ok:true.
The final raw .s is tmp/grind/func_80017D84/s1/func_80017D84_final.s. The banked losing order is
rejected/tail-order-w8-h6-c-wC-w10.c.

### E-s1-4. sandbox "cheat_asm_stripped: 3" is NOT this function
src/ings.c has two file-scope `__asm__(` blocks at lines 91 and 117 (other functions); the count
is theirs. func_80017D84's candidate contains no asm of any kind.

### E-s1-5. Kengo lead is empty
kengo_matches.csv maps the neighbour func_80017A44 -> coli_MakeKatanaVec as "size-only-ambiguous"
and has no row for func_80017D84; Kengo/ holds only the disc image + symbol lists, no C source.
No duplicate-lead entry in tmp/duplicates_leads.txt either. Not needed: the function matched.
