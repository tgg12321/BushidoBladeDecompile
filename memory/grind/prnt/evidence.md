# prnt — evidence

## Ruling 10 dossier (manual session 2, 2026-09-25)

Owner Ruling 10 ("verified original source, verbatim reuse",
.claude/rules/ordinary-c-judge-decidable.md, landed c3e7a0b9e). Every prong
below was checked against the fetched source files and against
asm/funcs/prnt.s (the target), not taken from the earlier ledger.

### Correction: the source version is 4.3BSD-Tahoe doprnt.c 5.35, not Reno 5.39
Session 1 (below) and the ruling's background text name 4.3BSD-Reno
doprnt.c 5.39. **The target does not follow 5.39.** The deciding statement is
the `ARG()` macro:
- 5.35 (Tahoe): `flags&SHORTINT ? va_arg(argp, short) : va_arg(argp, int);`
  for every conversion, so `%ho/%hu/%hx` sign-extend.
- 5.36 (SCCS delta 1988-10-24, Keith Bostic: "%h is broken ... make sure your
  fix doesn't sign extend on %hu") changed it to `ARG(basetype)` with
  `(short basetype)va_arg(argp, int)`, i.e. zero-extension for o/u/x. 5.37,
  5.38 and 5.39 keep that form.
- Target: all four ARG sites load the short arg with `lh` (signed):
  0x8007947C (d/i), 0x80079500 (o), 0x800795A8 (u), 0x800795E8 (x).
- Measured on unspliced main (where both carry the same 2-point jtbl addend
  artifact): the 5.39 ARG(int)/ARG(unsigned) spelling scores 5, i.e. 3
  source-level hunks, `lh` -> `lhu` at exactly the o/u/x sites; the 5.35
  spelling scores 2, i.e. 0 source-level (rejected/reno-5.39-arg-lhu-5.c vs
  the candidate, 2026-09-25).
- Lower bound: `case ' ': if (!sign) sign = ' ';` (target 0x800792FC
  `bnez $fp`) first appears in 5.33; 5.33/5.34/5.35 have the same code
  (5.33->5.34 only splits `flags = dprec = fpprec = width = 0;` into four
  statements, 5.34->5.35 only the copyright). 5.35 is the version shipped in
  the public 4.3BSD-Tahoe release, so it is the cited version.

### (A) Public, pinned original source
- Primary: https://raw.githubusercontent.com/dspinellis/unix-history-repo/b98826995697c37ced684813f008619460bd7ff8/usr/src/lib/libc/stdio/doprnt.c
  (tag `BSD-4_3_Tahoe` -> commit b98826995697c37ced684813f008619460bd7ff8,
  "BSD 4_3_Tahoe release").
  In-file version: `static char sccsid[] = "@(#)doprnt.c	5.35 (Berkeley) 6/27/88";` (:19).
  16131 bytes, SHA-256 780140314613409c8dab6d48520a2f0813509b8653ee00bf97c69d8170b4b8ba.
- Second copy (another archive): TUHS Unix Tree,
  https://www.tuhs.org/cgi-bin/utree.pl?file=4.3BSD-Tahoe/usr/src/lib/libc/stdio/doprnt.c
  (the `<pre>` block, HTML-unescaped): byte-identical, same SHA-256
  780140314613409c8dab6d48520a2f0813509b8653ee00bf97c69d8170b4b8ba. Caveat:
  unix-history-repo's README says its release snapshots were imported from the
  TUHS distributions, so these two share an upstream.
- Third copy, independent derivation: the CSRG SCCS archive (McKusick's CSRG
  Archive CD-ROMs) as reconstructed on unix-history-repo branch `BSD-SCCS`.
  Delta 5.35 = commit b51e0a0da5370c7cb6206089fe8f2f2efa3d97e8 (Keith Bostic
  1988-06-28, "install approved copyright notice"), path
  usr/src/lib/libc/stdio/vfprintf.c (the SCCS file was renamed doprnt.c ->
  vfprintf.c in 1991, so the history lives under the later name). SHA-256
  c7b3f1709785fd3e56b5a8a0dbc58c72fd48a3ccf519c6e5f785dadb8773d823. It is
  identical to the Tahoe release file except the sccsid line (:19), which in
  the reconstruction reads `"@(#)vfprintf.c	5.35 (Berkeley) %G%"` (module
  name already expanded to the later file name, date keyword unexpanded);
  rewriting that one line to the release's `doprnt.c ... 6/27/88` makes the
  files `cmp` equal — so every cited line is
  confirmed character for character by a copy that did not come from the
  release tape.
- Reno 5.39 for comparison: tag BSD-4_3_Reno (commit
  e36aea743b64478ee491d1f605752fe02928c9b1), SHA-256
  a9c68f6cda563407c832552830d38aa9918b400cb4c3d9e3aded0e28513554a0; TUHS copy
  byte-identical.
- SCCS delta commits located by bisection (tmp/prnt_r10/bisect_versions.py):
  5.35 b51e0a0da537, 5.36 14eb6926d4cc (1988-10-24), 5.37 e34c9549ddfb
  (1989-03-27, "embedded assignments are dangerous" — splits `n = size` out
  of the `if`), 5.38 0573a111d780 (copyright), 5.39 ce0774d20653 (hp300
  isspecial).
- Not a decompilation: 4.3BSD source as released by UC Berkeley CSRG. The
  third-party file is NOT committed to this repo (tmp/ only).

### (B) Transcription map, whole function (5.35 line -> target)
Same in the port (statement for statement): :89-90 `fmt = fmt0; digs = ...`,
:91 `for (cnt = 0;; ++fmt)`, :109-110 `if (!ch) return (cnt)`, :112-114 flag
resets, the whole switch for ` # * - + . 0 1-9 L h l c D d i n O o p s U u X x`
with the same fallthroughs and `goto rflag/nosign/number/pforw`, :319-329 the
digit loop, :350-374 pforw sizing, padding and prefix, :383-384 the string
loop, :386-387 fpprec zeros (kept; fpprec is always 0 here, target
0x800797FC-0x8007980C still runs the loop), :389-393 left padding and `cnt +=`,
:395-396 `case '\0': return (cnt)` (jtbl index 0 -> 0x800792A4), :397-399
`default: PUTC((char)*fmt); cnt++;`.

Every difference (each checked in asm/funcs/prnt.s):
1. Signature: `_doprnt(fmt0, argp, fp)` (K&R) -> `s32 prnt(s32 fd, u8 *fmt0,
   char *argp)`. FILE *fp is gone; a new FIRST parameter is never read ($a0 is
   first written at 0x80079294 `lbu $a0`); printf passes 1 (src/text1b_b.c
   printf). argp moves to the third parameter ($a2 -> $s1 at 0x8007924C).
2. :82-87 FILE flag setup and `return (EOF)` removed; replaced by
   `if (fmt0 == NULL) return 0;` (0x80079270 `bnez $a1` / `addu $v0,$zero,$zero`).
3. :92-108 the FILE-buffered copy loop for ordinary characters removed. The
   port reads one char per outer iteration: `if (!(ch = *fmt)) return cnt;
   if (ch != '%') { putchar(ch); continue; }` (0x80079294-0x800792B0 ->
   0x80079888 `jal putchar` -> 0x80079890 `++fmt`). Ordinary characters are
   NOT added to cnt (no cnt update on that path; cnt lives at 0x38($sp)).
4. PUTC(c) (`putc(c, fp)`) -> `putchar(c)` everywhere (10 `jal putchar`).
5. :377-382 the bcopy fast path (`if (fp->_cnt - (n = size) >= 0 && ...)
   {...} else`) removed; its `n = size` assignment is kept as the statement
   `n = size;` in the same position, followed by the else-arm loop
   (0x800797C8 `addiu $s0,$s6,-1` = `--n` of `n = size`).
6. Floating point removed: `_double`, `softsign` and the :200-238
   `e E f g G` case (jtbl entries 0x45/0x47/0x65/0x66/0x67 all go to default
   0x80079878); `cvt/round/exponent` are not in the object.
7. BUF: 348 (`MAXEXP+MAXFRACT+1`) -> 40 (buf at 0x10($sp); `t = buf + BUF` is
   `addiu $s2,$sp,0x38` at 0x8007962C).
8. va_arg: PsyQ/GCC rounded-size va_arg (`AP += 4; *(T *)(AP - 4)`) instead of
   Tahoe varargs.h `((mode *)(list += sizeof(mode)))[-1]` — the short case
   advances 4 (0x80079478 `addiu $s1,$s1,4` / `lh -4($s1)`), which the Tahoe
   macro would not. Spelled `prnt_va_arg`; ARG() otherwise verbatim 5.35.
9. String literals -> the named rodata arrays D_80015A68 ("0123456789abcdef"),
   D_80015A84 (upper), D_80015A7C ("(null)", cast `(char *)`), for the
   literal-pooling reason in the source comment (layer-2 cleared 2026-09-25).
10. Types: `u_char` -> `u8`, `u_long` -> `u32`, `int` -> `s32`, `char *digs`
   -> `const char *digs` (the arrays are const); `register` dropped;
   `char *p, *memchr();` -> `char *p;` (memchr defined in the same file).
11. ctype macros written out: `isascii(c) ((unsigned)(c)<=0177)` ->
   `((u32)(c) <= 0177)`, `isdigit(c) ((_ctype_+1)[c]&_N)` with `_N 04` ->
   `((&_ctype__plus_0x1)[c] & 4)` (Tahoe include/ctype.h :25, :17, :5).
12. Flag bit names prefixed PRNT_ (same values :48-54).
Version-sensitive statements touching `n`: only :377 (5.35/5.36 embed `n =
size` in the `if`; 5.37+ make it a statement). After the FILE path is removed
both give `n = size;` before the loop, so the target cannot tell them apart;
the cited version (5.35) is fixed by ARG (above), and the target follows it.

### (C) `n` is the original's own variable, verbatim
5.35 :64 `register int n;		/* random handy integer */`. Its writes/reads in
_doprnt, in order, against the port:
| 5.35 | original | port |
|---|---|---|
| :92,:95,:97,:100,:103,:107 | FILE copy loop counter | absent (difference 3) |
| :148 | `n = va_arg(argp, int);` | `n = prnt_va_arg(argp, s32);` |
| :150 | `n = 0;` | `n = 0;` |
| :152 | `n = 10 * n + todigit(*fmt++);` | same |
| :155 | `prec = n < 0 ? -1 : n;` | same |
| :167 | `n = 0;` | same |
| :169 | `n = 10 * n + todigit(*fmt);` | same |
| :171 | `width = n;` | same |
| :359 | `for (n = realsz; n < width; n++)` | same |
| :370 | `for (n = realsz; n < width; n++)` | same |
| :373 | `for (n = fieldsz; n < dprec; n++)` | same |
| :377 | `(n = size)` inside the FILE `if` | `n = size;` (difference 5) |
| :379-381 | `fp->_cnt -= n; bcopy(..., n); fp->_ptr += n;` | absent (difference 5) |
| :383 | `while (--n >= 0)` | same |
| :390 | `for (n = realsz; n < width; n++)` | same |
No other statement of the port reads or writes `n` (checked by grep of the
body: the only other `n` token is `case 'n':`). Type `s32` (= int);
`register` not carried. In the target every one of these sites is $s0
(0x80079358, 0x8007936C-0x800793C0, 0x800793D4-0x80079420, 0x80079708,
0x80079778, 0x8007979C, 0x800797C8-0x800797E0, 0x8007982C).

### (D) Annotation
At the declaration in the body (candidate.c / src/text1b_b.c): cites Ruling 10
+ c3e7a0b9e, 5.35 :64, the pinned URL, and every reuse line.

### (E) Receipts (re-measured 2026-09-25 session 2, on the landing chassis;
unspliced-main numbers carry +2 for the jtbl addend artifact, so subtract 2)
- one variable per role (precn/widthn/padn/zeron/left): 23 -> 21 (419 insns)
- both accumulators split: 23 -> 21; prec accumulator only: 11 -> 9 (419);
  width accumulator only: 7 -> 5 (418). Session-1 numbers (21/21/9/5) confirmed.
- single `n`: 2 on unspliced main (the jtbl lo16 addend only, operand-only);
  0/418 against the spliced src; full-build SHA1 == oracle.

### (F) Everything else
Unchanged from the session-1 layer-2 clearance, except three edits toward the
5.35 text (all byte-neutral, measured): ARG drops the `(long)` casts
(verbatim 5.35), `isdigit` drops a `(u8)` cast (verbatim ctype.h), `isascii`
is `<= 0177` (verbatim ctype.h). The source comment now cites 5.35.

## Session 1 (2026-09-25) — kept for the record; version attribution superseded above

### Provenance / reference
- Sony PsyQ 4.0 LIBC2 PRNT (libscan census 2026-07-09). No published matched C:
  psyz (tmp/psyz-ref @a438bda) has `libc2/prnt.c` as INCLUDE_ASM only; SOTN's
  psxsdk has LIBC `sprintf.c` (a different routine, already landed here).
- Session 1 attributed the body to 4.3BSD-Reno doprnt.c 5.39 (SUPERSEDED: the
  target follows 5.35, see the correction above). Same flag bits, case set and
  fallthroughs, `fpprec`, `dprec`/`fieldsz`/`realsz` padding, `digs` reset,
  BUF = 40 (buf at sp+0x10, number end at sp+0x38).

### Measured
- First transcription: 14; PsyQ/GCC generic `va_arg` (`lh -4`) -> 10.
  Remaining 10 = four lo16 string addends + the jtbl addend (literal vs D_
  symbol), 0 source-level.
- Literals land 40 bytes short at link (rejected/string-literals-oracle-40B-short.c):
  GCC 2.7.2 pools identical STRING_CSTs per translation unit, so sprintf's
  digit strings (same file, separate object in the original link, its own
  copies at 0x80015C7C/90) fold into prnt's. Referencing the existing named
  arrays D_80015A68 / D_80015A7C / D_80015A84 keeps both copies; only the
  jtbl_80015A98 const is deleted (compiler-emitted).
- Default-case order: `PUTC; cnt++` and `cnt++; putchar` score the same;
  landed the original order.

### Layer-2 verdict 2026-09-25 (session 1): FAIL (the single `n` only)
- rejected/single-n-reno-verbatim-0.c. FAIL ground: `n` fails Ruling 5
  1(a)/(b)/(f); Ruling 6 n/a; Ruling 8 vmNoiseOn-only; the owner had declined
  original-source reuse as a class 2026-09-24. Ruling 10 (2026-09-25) is the
  owner's answer to that borderline entry.
- Everything else CLEARED: t/digs/size/flags/sign, the named arrays, the
  jtbl_80015A98 removal, the repeated __va_rounded_size, the
  `(char *)D_80015A7C` cast.

### Why the honest per-role form sits at 21
- Split out, the prec accumulator lands in $a2 and the width accumulator in
  $a1 (caller-saved: neither crosses a call), plus an extra `move s0,a2` for
  `prec = n < 0 ? -1 : n` (+1 insn). The target has both in $s0, the register
  the call-crossing pad/print loop counters use: the signature of ONE pseudo
  shared with the loop counters.
- Accumulating straight into the role variable: prec 14 (418), width 98, both
  62. Dead ends. rejected/split-n-per-role-21.c is the honest per-role body.
