# Evidence bank — note2pitch

## s1 (recon, 2026-09-08) — matched on first measured form

- CHASSIS: HEAD main, cc1 -mel -msoft-float, form A (psyz `decomp/src/libsnd/vm_n2p.c`
  `note2pitch` adapted to BB2 names) applied at src/main.c in place of
  `INCLUDE_ASM("asm/funcs", note2pitch);`. `canonical note2pitch` = verdict C, distance 0.
  `sandbox note2pitch --disable all` = score 0 (49/49 insns). `verify-oracle` = ok: true
  (full build+link SHA1 == oracle) with the C in place. The 13 `cheat_asm_stripped` in the
  sandbox JSON are main.c-wide canonical islands in OTHER functions, not in note2pitch.
- OBJECT MODEL: D_800A26E4 — MATCHES (measured score 0). It is NOT a sub-symbol of
  g_alarm_active_sentinel (0x800A26DE): asm/data/7D920.data.s:32539 gives it its own
  `dlabel D_800A26E4` and the contents 0x1000, 0x100E, 0x101D, ... through 0x1FC5 at
  0x800A2860 are psyz's 192-entry `pitch_table` verbatim (0x180 bytes). Both note2pitch and
  the already-COMPLETED-C note2pitch2 address it via their own `%hi/%lo(D_800A26E4)` with
  an index in $at, never via a base register from 0x800A26DE; the +6 "SUB-SYMBOL" signal is
  splat name adjacency (the sentinel is a 1-byte flag at 0x800A26DE, 5 bytes of unrelated
  data separate them). Declaration `extern u16 D_800A26E4[];` (pre-existing in src/main.c for
  note2pitch2) reproduces the bytes; no aggregate merge is warranted.
  _svm_cur_plus_0x2 / _plus_0x10 / _plus_0x11 — MATCHES: they are `_svm_cur.note`,
  `.tone_center`, `.tone_shift` of the already-merged `struct struct_svm`
  (include/sound.h:22-27); the three `lbu` loads and their %lo offsets (+2, +0x10, +0x11)
  reproduce byte-exact from the struct fields. The named_syms rows for those three symbols
  are stale splat names; the header already carries the aggregate.
- Codegen notes for future readers: the `bgez/addiu 7/srl 3` sequence is `int / 8` on the
  u8-promoted tone_shift (GCC 2.7.2 signed power-of-two division idiom; the srl comes from
  the result being narrowed to `u16 step`). `lui/ori 0x2AAAAAAB; mult; mfhi; sra 1; subu`
  is the `/ 12` magic on `(s16)note`. `sltiu $a1,0x10 ; addiu $a1,15` is the unsigned
  `step >= 16` clamp — step must be UNSIGNED (u16) for sltiu; a signed step would emit
  slti. The `note + (60 - tone_center)` grouping still emits `addiu 0x3C; subu` (GCC
  reassociates the constant), so the psyz spelling was kept verbatim.
- Constructs: none. Ordinary C, no FAKE, no volatile, no pins.
