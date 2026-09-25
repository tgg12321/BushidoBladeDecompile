# prnt — evidence (manual session 2026-09-25)

## Provenance / reference
- Sony PsyQ 4.0 LIBC2 PRNT (libscan census 2026-07-09). No published matched C:
  psyz (tmp/psyz-ref @a438bda) has `libc2/prnt.c` as INCLUDE_ASM only; SOTN's
  psxsdk has LIBC `sprintf.c` (a different routine, already landed here).
- The target is 4.3BSD-Reno `_doprnt` (lib/libc/stdio/vfprintf.c, 1990) with the
  FILE buffering replaced by `putchar`: same flag bits (LONGINT 1, LONGDBL 2,
  SHORTINT 4, ALT 8, LADJUST 0x10, ZEROPAD 0x20, HEXPREFIX 0x40), same case set
  and fallthroughs (`*`->`-`, `D`->`d`, `O`->`o`, `U`->`u`, `X`->`x`), same
  `ARG()` ladder, `fpprec` (always 0 here, float cases absent), `dprec`/`fieldsz`
  /`realsz` padding, `digs` reset after each conversion, BUF = 40
  (buf at sp+0x10, number end at sp+0x38).
- BB2 differences from Reno: `if (fmt0 == NULL) return 0;`, first param unused
  (printf passes 1), ordinary characters are NOT counted in the return value
  (non-% chars go straight to putchar), default case counts then prints.

## Measured
- First transcription of Reno: 14; PsyQ/GCC generic `va_arg`
  (`AP += rounded; *(T *)(AP - rounded)`, gives `lh -4`) -> 10. Remaining 10 =
  four lo16 string addends + the jtbl addend (literal vs D_ symbol), 0 source-level.
- Literals land 40 bytes short at link (rejected/string-literals-oracle-40B-short.c):
  GCC 2.7.2 pools identical STRING_CSTs per translation unit, so sprintf's
  "0123456789ABCDEF"/"...abcdef" (same file, separate object in the original link,
  its own copies at 0x80015C7C/90) fold into prnt's. Referencing the existing
  named arrays D_80015A68 / D_80015A7C / D_80015A84 keeps both copies; only
  jtbl_80015A98 is deleted (compiler-emitted).
- Landed form: verify-oracle --rebuild --allow-dirty GREEN
  (62efab4f73f992798c43e8c730aa43baa10bb4fa); sandbox --disable all vs spliced src = 0 (418/418).
- The single Reno `n` ("random handy integer") is load-bearing. Per-role split
  (rejected/split-n-per-role-21.c) = 21 (419 insns). Partial splits: parse
  accumulators only 21; loop counters only 21; prec-parse only 9; width-parse
  only 5; any ONE loop counter alone 0.
- Default-case order: Reno's `PUTC; cnt++` and `cnt++; putchar` both score the same;
  landed Reno order.

## Layer-2 verdict 2026-09-25: FAIL (the single `n` only)
- rejected/single-n-reno-verbatim-0.c = the sandbox-0 / oracle-GREEN body. FAIL
  ground: `n` is written as the precision and width accumulators, five pad/zero
  loop counters and the remaining-char count, so it fails Ruling 5 1(a)/(b)/(f);
  Ruling 6 n/a (not a record pointer); Ruling 8 is vmNoiseOn-only; the owner
  declined "reused scratch matching the original library source, as a class"
  on 2026-09-24 (decisions.md Ruling 8 entry).
- Everything else CLEARED by layer-2: t/digs/size/flags/sign, the named arrays
  (truthful data layout), the jtbl_80015A98 removal, the repeated
  __va_rounded_size, the `(char *)D_80015A7C` cast. The landing chassis
  (keep D_80015A68/7C/84, delete jtbl_80015A98, splice body) is proven.
- Provenance of the shared `n` (not committed to the repo; fetch to tmp/):
  https://raw.githubusercontent.com/dspinellis/unix-history-repo/BSD-4_3_Reno/usr/src/lib/libc/stdio/doprnt.c
  sccsid "@(#)doprnt.c 5.39 (Berkeley) 6/28/90". Excerpts:
    :67   register int n;		/* random handy integer */
    :151  n = va_arg(argp, int);  :153 n = 0;  :155 n = 10 * n + todigit(*fmt++);
    :158  prec = n < 0 ? -1 : n;
    :170  n = 0;  :172 n = 10 * n + todigit(*fmt);  :174 width = n;
    :362/:373/:393  for (n = realsz; n < width; n++)
    :376  for (n = fieldsz; n < dprec; n++)
    :385-386  n = size; ... while (--n >= 0) PUTC(*t++);
  (vfprintf.c on the same tag is only the wrapper calling _doprnt.)

## Why the honest per-role form sits at 21 (sb diff, rejected/split-n-per-role-21.c)
- The two scored source-level hunks + the operand-only cascade are all the
  parse accumulators: split out, the prec accumulator lands in $a2 and the width
  accumulator in $a1 (caller-saved: neither crosses a call), plus an extra
  `move s0,a2` for `prec = n < 0 ? -1 : n` (+1 insn). The target has both in
  $s0, the register the call-crossing pad/print loop counters use. A pseudo
  that never lives across a call gets a callee-saved register from global.c
  only by sharing an allocno with one that does, so $s0 on the accumulators is
  the direct signature of ONE pseudo shared with the loop counters.
- Measured partial splits: prec-accum alone 9 (419), width-accum alone 5,
  both 21; any single loop counter alone 0; all three loop roles 21.
- Frontier: an honest spelling where the accumulator value itself is
  call-crossing (none found; `prec`/`width` directly as accumulators change the
  '*'/clamp code) — or an owner ruling on original-source reuse (borderline.md
  2026-09-25 prnt entry).
- Accumulate straight into the role variable (tmp/prnt/mk_direct.py shape):
  prec as its own accumulator + `if (prec < 0) prec = -1;` = 14 (418) — prec
  then takes $s0 and the loop pseudo $s4 (swapped), and the target's
  `bgez s0 / move s4,s0 / li s4,-1` ternary shape is lost (the target needs an
  accumulator distinct from prec); width as its own accumulator 98 (408:
  width is a spilled slot), both 62. Dead ends.
- candidate.c (2026-09-25) = the honest per-role body, floor 21/418.
