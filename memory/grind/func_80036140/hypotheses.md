# func_80036140 — hypotheses (ruled out / open)

## Ruled out (with how)
- **Scalar increment spellings to get the `la`+0(reg) member form** (++x, x++, x+=1, x=x+1, in/after
  a call-result branch) — all emit direct `lw sym`/`sw sym` (tmp test t1.c, cc1 -O2 -G0). The
  member form comes only from COMPONENT_REF/ARRAY_REF (memory_address force_reg). Invented pointer
  locals would be allocator-steering locals (not landable).
- **Non-aggregate spelling of the ATV copy** — lwl/lwr/swl/swr only from an unaligned BLKmode move;
  a `u32` deref gives aligned lw/sw; byte-wise copy = 4x lbu/sb (candidate.c, 50). Pointer pun is
  banned (dossier: per-use cast/pointer pun on split-aggregate = auto-FAIL).
- **CdlATV merge under the current model** — (a) -G0 cse shares the base address across CdMix
  (s1/s0) instead of gp-rel val0 + fresh `la` at the copy; (b) maspsx gp-rel's `sym+1..3`
  (injected sdata extern, not .comm). Measured 34 / siblings 10 and 6.
- **Global maspsx "no gp for extern+offset"** — 9 matched sites need gp on `sym+N`
  (gp_off_census.py), so it would break text1a_post/text1b.
- **sdata_exclude for the ATV objects** — keyed by base symbol, kills the base gp-rel access too.
- **Real `.comm` definitions in C** — cc1 emits 3-arg `.comm sym,4,1` (maspsx parse crash, cf. CD_cw);
  and maspsx would emit .bss storage in the object (conflicts with the linker-script address).
- **-G8 alone** — 14 (the +1..+3 bytes still gp-rel'd); **.comm knowledge alone (-G0)** — 22.

## Open (the frontier)
1. **Owner ruling needed** (docs/grind/borderline.md 2026-09-26 func_80036140): may the build model
   (i) compile the CD module TU with -G8 and (ii) tell maspsx that g_cd_atv / D_800A36B8 were COMMON
   in the original TU (offset forms not gp-rel, no storage emitted)? With both + the record
   extension the body is instruction-complete (scratch score 2 = jtbl operand only).
2. Record extension past 0x80101E99 (E9A..EA7) rests on this function's member-form codegen only —
   same evidence class as the func_800620B8 question (compiler-behavior evidence for a merge).
   Independent-evidence search done: main EXE lui 0x8010 formations into 0x80101E58..EC7 (117,
   every one at a label's own address) and MOVOVL.EXE (0) — integration/scan_ovl.py.
   Nothing but the func_80036940 E8C-0x20 / E98-0x2C pair crosses a label.
3. Landing mechanics once ruled: split the CD module into its own TU (rodata base 0x80010938),
   -G8 via GP_FILES (+ engine/buildconfig.py mirror), func_80036940 linked as asm
   (LINKED_ASM_FUNCS) or split around; delete jtbl_80010938 from code6cac_b_rodata_post.c and keep a
   leading zero word there until func_80036940 lands; oracle + all siblings.
