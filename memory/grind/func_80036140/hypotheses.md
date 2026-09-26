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
- **(slotI 2026-09-26) E9C/EA4 as anything other than CdState members** — at -G8: separate
  variables 18, pointer read-modify-write locals 18, function-scope pointer aliases 42 (small-data
  address cost folds the pointer back, cse find_best_addr); a separate 12-byte record at 0x80101E9C:
  func_80036140 8 + cdrom_ReadyCallback 12 (no use_related_value with the CdState base). Dumps in
  landing/dumps/. Only the CdState extension through 0x80101EA7 reaches the oracle.
- **(slotI) cdrom_SetMix at -G0 with the CdlATV merge** — 5 (address register shared with CdMix's
  argument); cc1psx+ASPSX COMMON -G0 differs too. Hence the second -G8 file (cdrom_SetMix +
  func_80035F78).

## Open (the frontier) — 2026-09-26 evening, slotI
1. **Owner question (borderline.md 2026-09-26 "the CD state record must run to 0x80101EA7")**: the
   E9C..EA7 extension is forced by mechanism (evidence.md § Record extension) but (a4′)(2)/(3) would
   make 0x80101E9E..0x80101EA3 an anonymous filler, which (a4′)(5) then fails (other consumers need
   puns). If the owner allows named members typed by the other consumers' accesses, the landing
   below is ready; if not, func_80036140 cannot land (no admissible spelling reaches the target).
2. Everything else is built and oracle-verified in scratch (evidence.md CURRENT STATE): maspsx
   COMMON gate + registration (landing/tools/gate.py, register.py; engine test 805/0), five-file
   split (apply_model.py --split final --merge --ext rec), symbol-row retirement still to script
   (D_800A36B9..BB, g_cd_atv_plus_0x1..3, the "retire with func_80036140" alias rows, E9C/E9E/EA0/EA4
   rows, named_syms comment 0x80101E9B -> 0x80101EA7), header comment update, move-identity check.
3. Landing driver ready (landing/tools/land.sh, run under the lock only): gate.py + register.py
   (saves commit-1 Makefile / engine/buildconfig.py to tmp/func_80036140/c1/), apply_model.py
   --split final --merge --ext rec, symfiles.py (prong (c) rows + one dlabel per CdlATV), the
   gate-lists "current completions" line. Scratch `t_final.sh` = oracle; `movecheck.py` = every moved
   line identical. Commit plan: (1) msg_gate.txt (maspsx gate + registration; commit with the c1/
   versions of Makefile + buildconfig in the worktree), (2) msg_match.txt. Fill @OWNER_RULING@ etc.
