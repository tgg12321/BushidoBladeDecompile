# camera_CalcAngles (0x80047384) — evidence

## 2026-10-01 — reopened by owner ruling Q84 (Q65 adoption, step 14, A8)

Was COMPLETED-C in sound.c (pre-include-asm-body.c: two s16 statics written `D_800A33C8 = -ratan2(..);
D_800A33CA = s0; return &D_800A33C8;`). Q65's A8 (owner ruling Q79: Sony ASPSX + PSYLINK place every `.lcomm`
static 4-aligned, memory/grind/q65-adoption/q56/adopt/lcomm_align_probe.*) makes D_800A33CA the second
halfword of the static `s16 D_800A33C8[2]` in text1b (the M3 merge of sound + text1b). Q65 step 14 (generator
memory/grind/q65-adoption/q56/adopt/s14_apply.py) puts this function back to INCLUDE_ASM (asm/funcs/
camera_CalcAngles.s = func_80047384.s with its label renamed) and re-queues it (engine `queue reopen`).

candidate.c is the array spelling on the Q65 tree (after step 08's casts). Residual: +1 instruction under our
cc1 -G0. The element store's constant address is not a legitimate -G0 MIPS address, so cc1 puts it in a
register (memory_address), and cse shares that register with the returned address:
target `negu v1,v0 / lui v0 / addiu v0 / sh v1,gp(D_800A33C8) / sh s0,gp(D_800A33CA)`, ours
`lui v1 / addiu v1 / negu a0,v0 / move v0,v1 / sh a0,0(v0) / sh s0,gp+2`.

Spellings measured (scripts memory/grind/q65-adoption/q56/adopt/try_cam*.sh, measureA2.sh), all +1 insn:
`D_800A33C8[0]` + `return D_800A33C8;`; `*D_800A33C8 = ...`; `return &D_800A33C8[0];`; a two-member struct
(`.a` / `.b`, `return &D_800A33C8.a;`); [1] stored before [0] via a temporary; a temporary for the first value;
a do-while(0) around the two stores; each before the switch (extern array) and after it (static array).
cam_probe.sh / cam_probe.out: the same shape (a static s16[2] written [0], [1], its address returned) compiles to
the target's `la / sh D_pair / sh D_pair+2` under cc1psx -G8 and under our cc1 -G8; our cc1 -G0 adds the move.
text1b under cc1 -G8 (GP_FILES) is not neutral for the rest of the file (owner Q83 refused).

Frontier: a C spelling that keeps the [0] store's address a plain symbol under -G0 without sharing it with the
return value, or a measured, evidence-backed per-file cc1 -G8 for the file that holds this function.
