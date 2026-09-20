/* func_8006ECF4 — session 1 (recon). NO real C body exists yet for this function;
 * src/text1b.c still carries `INCLUDE_ASM("asm/funcs", func_8006ECF4);` and the only
 * prior C artifact (pre-include-asm-body.c) is a Campaign-4 placeholder stub with the
 * WRONG signature (4 args; the asm shows exactly one) and no decompiled logic — do not
 * apply it, do not treat it as progress.
 *
 * This file intentionally carries NO candidate body. Recording that fact here (rather
 * than a stub) so no future session mistakes an absent candidate.c for "not yet looked
 * at" — it means "looked at; nothing here is real; start the structural draft from
 * evidence.md/hypotheses.md F2 next session."
 *
 * Verified single-argument signature from asm/funcs/func_8006ECF4.s: `$a0` is moved to
 * `$s2` at entry and used as a struct pointer throughout (`arg0[0]->0x54`, `arg0->0x4`
 * read-modify-write via func_80073728). No return value convention observed at the
 * epilogue. See evidence.md for the full traced control flow (loop bound, nested
 * D_9BC7C/D_9BC40/D_800A3588/D_800A358C/D_800A3561 lookup chain, 6-way jtbl_800159D0
 * switch, D_800A32F4 8-byte struct copy into LoadImage).
 */
