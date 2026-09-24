# Evidence bank — func_80079A30

- [s1] [fable-blitz 2026-07-07] Rule inventory: ONE rule -- asmfix.txt:219 replace_with_asmfile; stub src/text1b_b.c:1968 `void func_80079A30(s32 arg0, s32 arg1, s32 arg2, s32 arg3)`. Distance 533; floor 533 until a draft lands. Park = rejected distance>500 canonical misroute; the body is textbook GCC-compiled C.

- [s1] [fable-blitz 2026-07-07] IDENTIFICATION: sprintf-class format interpreter. arg0(s3)=dest buffer, arg1=fmt; returns output count s2 after NUL-terminating (asm :590-592). Main loop: copy non-'%' bytes (:534-537); on '%' (0x25) parse flags '-'(bit1) '+'(bit2) ' '(pad char sp+0x211) '#'(bit4) '0'(bit8) in a self-looping chain (:39-81), then width ('*' -> va_arg with negative->negate+bit1, or digit accumulation *10-0x30 at :104-123) into sp+0x214, then '.' precision ('*' or digits) into sp+0x218 with bit 0x10 (:125-175); '0' flag is CLEARED when '-' present (v1 & ~8 at :177-184).

- [s1] [fable-blitz 2026-07-07] State struct: 12 bytes at sp+0x210 initialized by 3-word copy from D_8009BE10 (:29-36) = {u32 flags; u8 signch; ...; s32 width; s32 prec} -- the lw/lw/lw+sw/sw/sw block is a struct assignment from a static default (D_8009BE10 has no C decl yet; needs extern + data segment check). Modifier chars l/h set flags 0x20('h' short: sign-extend via sll16/sra16 at :226-229)/0x40/0x80 via jtbl cases .L80079CF8/.L80079D04/.L80079D10.

- [s1] [fable-blitz 2026-07-07] Conversion switch: `switch (c - 0x4C)` sltiu 0x2D + jtbl_80015CA4 (45 entries, ALREADY migrated to C rodata at src/text1a_b_post_rodata.c:174). Cases: 'd/i' .L80079D3C (signed dec, neg->'-' in signch, bit2->'+'), 'u' .L80079D8C, 'o' .L80079EA4 (andi 7/srl 3), 'x' .L80079FA8 / 'X' .L80079F94-adjacent with hex digit tables D_80015C90 (lowercase) / D_80015C7C (uppercase), both committed rodata strings (src/text1a_b_post_rodata.c:164,169); 'p'-like case sets flags|=0x50 prec=8 then shares the hex path; 'c' .L8007A0B0 (single byte from va); 's' .L8007A0D0 (bit4 -> Pascal-style length byte *s++, else strlen func_800791D8; bit0x10 -> memchr func_8007992C(s,0,prec) truncation, returns prec if no NUL); 'n' .L8007A154 (store count: bit0x20 -> sh, else sw); default .L8007A188 (if c=='%' fallthrough to literal emit else terminate).

- [s1] [fable-blitz 2026-07-07] Digit generation: decimal via UNSIGNED magic 0xCCCCCCCD (multu/mfhi/srl 3 = u/10, remainder *(-10) reconstruction, +0x30) writing BACKWARD from s1 = sp+0x210 downward into the 0x200-byte scratch (sp+0x10..0x20F) (:281-294); octal andi7/srl3 (:349-356); hex andi 0xF/srl 4 + table lookup (:433-441). Zero-padding to precision (sb 0x30 loops :303-310, :379-387, :450-457), then '0'+'x' '#' prefix for hex (:463-469 stores conversion char itself then '0' -- reproduces 0x/0X from `c`), sign char prepended for decimal (:311-319).

- [s1] [fable-blitz 2026-07-07] Common emit .L8007A1A0: if width > len and !left-justify, pad ' ' (s4=0x20) decrementing width in place (:550-560); func_8007A28C(dst+s2, s1, s0) copies the s0-byte body (helper = length-bounded memcpy; check its committed form); then left-justify trailing pad (:571-579). s2 accumulates count; s0 = body length; s1 = body start pointer.

- [s1] [fable-blitz 2026-07-07] VARARGS: pre-prologue homing `sw a1,4(sp); sw a2,8(sp); sw a3,0xC(sp)` BEFORE addiu sp,-0x248, fmt walked via homed copy at sp+0x24C, va pointer sp+0x220 starts at sp+0x250 = caller-frame a2 home. This is GCC 2.7.2 stdarg homing for `int func(char *dst, const char *fmt, ...)`. The stub's current 4-arg signature must become real varargs OR the project's fake-varargs-explicit-homing technique (on-demand rule; the committed sibling debug_printf at src/text1b_b.c:1899 uses exactly the explicit `s32 *ap = &fmt; ap[1]=a; ap[2]=b; ap[3]=c;` spelling and compiled to the same three sw's). CHECK the rule before drafting -- caller ABI must stay identical since callers pass through registers.

- [s1] [fable-blitz 2026-07-07] The ENTIRE surrounding TU region is the libc island, COMPLETED-C: strcpy (src/text1b_b.c:1884-area), strlen func_800791D8 (:1889), toupper/tolower func_800798CC/func_800798FC (:1911/:1919), memchr func_8007992C (:1927), putchar-ish func_8007997C (:1944). These prove the compilation idioms (goto-based control, (&D_8009BD8D)[c] ctype table) this file's original used. func_80079244 (stub at :1906, asmfix twin) is the SAME interpreter shape for console output (called by debug_printf) -- solving 79A30's skeleton solves 79244; keep the two ledgers linked.

- [s1] [fable-blitz 2026-07-07] This is stock PSX-era libc (SN Systems/PsyQ prnt.c family). The decomp.me corpus (gcc2.7.2-psx class) very likely has a scratch for sprintf/vsprintf/prnt with overlapping target asm -- highest-leverage external reference for the exact source shape (flag bits 1/2/4/8/0x10 and the 0x211 sign-char slot are distinctive).

- [s1] [fable-blitz 2026-07-07] Width/precision interplay details for the draft: 'd' path with bit8&&signch!=0 -> prec = width-1 (:261-269 reads width into prec then decrements if sign present); prec<=0 -> prec=1 (:271-275); 'x' with bits 8|4 -> prec = width-2 (:417-423); 'u' with bit8 -> prec = width (:335-339). The 0x50 flag-set case .L80079F94 (prec=8, flags|=0x40|0x10) is likely '%p'-adjacent spelling: check jtbl index (c-0x4C) to name it precisely when drafting.

- [s1] [fable-blitz 2026-07-07] m2c reference: tmp/blitz/m2c_func_80079A30.c (398 lines, clean) with jtbl at tmp/blitz/jtbl_80079A30.s. Loop counters s0/s2 are plain s32 (no re-extension); fmt pointer kept in MEMORY (sp+0x24C reload every use, not a register) -- characteristic of address-taken va/fmt locals, matches stdarg + heavy struct-in-stack usage.

## 2026-09-23 manual upstream sweep — SOTN reference scores 2/535 (insn count exact)

- Source: SOTN `src/main/psxsdk/libc/sprintf.c` (local clone tmp/sotn @8bd7c77; the
  psxsdk tree is gone from sotn master as of 02bc3bd). psyz (a438bda) still INCLUDE_ASM.
- Verbatim adaptation (static default -> `extern printf_info D_8009BE10`, hex strings ->
  D_80015C7C/D_80015C90, bool bitfields -> u32) = 9/535, build_insns 535 == target.
- Two edits to reach 2/535 (candidate.c here):
  1. `case 'c'`: `va_arg(args, s32)` not `va_arg(args, char)` — target reads the slot with
     `lw`, char spelling emits `lbu` (9 -> 8).
  2. Drop SOTN's `do { if (prependPlus) ... } while (0); // FAKE` -> plain
     `} else if (info.prependPlus) {` — fixes the s5/s6 swap of the '-'/'+' constants (8 -> 2).
     BB2 links a different build; SOTN's fake is not needed (plain-C form is better here).
  (Removing the isHalf do-while(0) wrappers too is WORSE: 25.)
- The remaining 2 is the jump-table load `lw v0,0(at)` vs ours `lw v0,24(at)` — our jtbl
  lands at text1b_b.o .rodata+0x18. NOT a code difference: it is rodata placement.
- MEASUREMENT REQUIRES a header change: include/code6cac.h:560 `extern s32 sprintf();`
  conflicts with a varargs definition in GCC 2.7.2 — changed to
  `extern s32 sprintf(char *, char *, ...);` for the sandbox run only (reverted after;
  tmp/upstream/sb_sprintf.ps1 does this automatically).
- LANDING needs rodata re-attribution: text1b_b.o(.rodata) is 0x80015A3C (0x18 B); the
  original sprintf.o rodata = the two hex literals + jtbl (0x80015C7C..0x80015D58) inside
  text1a_b_post_rodata.c, preceded by 0x80015A54..0x80015C7B items. Per the TU re-split
  recipe (.claude/rules/jtbl-rodata-split-infrastructure.md) the intervening items must move
  into text1b_b.c (or text1a_b_post_rodata split) so the literals + jtbl emit at 0x80015C7C.

## 2026-09-23 manual landing attempt — full-build SHA1 == oracle, layer-2 FAIL

- Landed form (rejected/sotn-args-frame-walk-0.c) + rodata re-attribution (jtbl_80015A54
  and prnt's D_80015A68/7C/84 + jtbl_80015A98 moved into text1b_b.c ahead of their owners;
  D_80015C7C/C90/jtbl_80015CA4 deleted from text1a_b_post_rodata.c, emitted by sprintf as
  literals + switch table) + variadic prototype in include/code6cac.h:
  sandbox 0/535, full build SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa.
  The rodata move, prototype, u32 bitfields, extern D_8009BE10, else-if '+', and
  va_arg(args, s32) were all CLEARED by layer-2.
- Layer-2 FAIL on (1) SOTN's `bufPtr = (char*)&args - sizeof(printf_info) - 4;` = reaching
  buf through the address of another local (cross-object derivation, semantic-lie #5), and
  (2) the four isHalf do-while(0) wraps lacking inline FAKE annotation (load-bearing:
  dropping them = 25). (2) is a trivial fix; (1) is the blocker.
- Honest `bufPtr = &buf[sizeof(buf)];` = 96/535 (build 510): args leaves its sp+0x220 stack
  slot for a register. Target leaves $s7 unused, so args is NOT a pressure spill — the
  original takes its address. stdarg spellings measured with the honest bufPtr:
  SOTN macros 96; `*(char **)&ap +=` va_arg 96 (folded); char* va_list 96;
  `char *va_list[1]` 96 (1-word array stays in a pseudo); GCC ginclude va-mips.h
  little-endian va_arg 82 (build 534).
- Frontier: a truthful reason for args to be address-taken (or an owner ruling on the
  SOTN frame-walk line — docs/grind/borderline.md 2026-09-23 sprintf entry).

## 2026-09-24 re-landing under owner Ruling 7 (40813a22e)

- Landed chassis = rejected/sotn-args-frame-walk-0.c + Ruling 7 comment on the bufPtr line.
- Per-wrap ablation of the four isHalf do-while(0) wraps on the landed chassis (sandbox 0):
  dropping only wrap #1 (d/i) = 5/535, #2 (u) = 8, #3 (o) = 5, #4 (hex) = 5. Every wrap is
  individually load-bearing; the unwrapped diff at each site is the va_arg load + isHalf
  flag test before it swapping v0/v1 and reordering (source-level + operand-only hunks).
