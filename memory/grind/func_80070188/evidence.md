# Evidence bank — replay_camera_attack

- [s1] [fable-blitz 2026-07-07] Queue: distance 696, ASM-STRUCTURAL, parked (canonical audit 2026-06-09 REJECTED -- standard compiled C). Single rule = asmfix.txt:185 replace_with_asmfile. Stub at src/text1b.c:16470 (file stem text1b CONFIRMED from queue.json). No WIP for THIS function; memory/wip/replay_camera_Init is a DIFFERENT small function (39 insns, code6cac_b2_post) -- kinship is subsystem-only (same replay/SpecialCam globals family); its lesson that transfers: the a1-preservation register-rotation wall it documents is function-specific, do NOT import its rejected_forms wholesale.

- [s1] [fable-blitz 2026-07-07] Signature: void replay_camera_attack(ctx *s7) -- only a0 used. Frame 0x90; saves ra+fp+s0..s7 (ALL callee-saved). fp = *(s32*)(D_800A35A8->0x74) (replay slot ptr array, held across whole body); s6 = 0xFF held as a REGISTER CONSTANT across the entire loop (s:46) -- loop.c invariant hoisting of the repeated `== 0xFF` comparand; s4 = s16 player-loop var; s5 = s4 - D_800A3554 (pad index); s1/s2/s3/s0 per-iteration derived ptrs.

- [s1] [fable-blitz 2026-07-07] Preamble (s:19-43): sw 0 -> sp+0x28, 0x100 -> sp+0x38 and sp+0x3C (descriptor constants stored ONCE before the loop -- the struct persists across iterations, fields partially rewritten per draw); jal saMotionSet(*fp, 0x60) with *fp also stored to sp+0x18; initTexPage(s7->0x18, 1, 0, saMotionSet_ret); ot_Link(D_800A374C + 0x20, s7->0x18); s7->0x18 += 0xC.

- [s1] [fable-blitz 2026-07-07] Main loop bound: while (s4 < D_800A35B0 + (s16)D_800A3554 + 1), bound RECOMPUTED at entry (s:37-44) and at loop bottom (s:659-668). CODEGEN FACT: D_800A3554 is read TWICE with different widths back-to-back -- lhu $a3 (s:37, feeds subu s5 = s4 - a3) then lh $v0 (s:40, feeds the +1 bound) -- a u16-typed read AND an s16-typed read of the same global (rule: u16-global-lhu-lbu-low-byte); the C likely has one u16 use and one s16 use (or a cast).

- [s1] [fable-blitz 2026-07-07] Per-iteration setup (s:48-79): s2 = &D_800A3588[s4] (s16 col), s3 = &D_800A358C[s4] (s16 row), s1 = s4*3; grid cell byte = D_8009BC40[*s2*2 + *s3*12]; s0 = &D_8009BC7C[cell]; D_800A3561[s4*3] = cell. Branch on D_800A3560[s4*3] == 0xFF (s6) -> edit-mode arm vs .L80070628 display arm.

- [s1] [fable-blitz 2026-07-07] Edit arm: sh 0x1E -> ((s16*)D_800A35C4)[s4] (D_800A35C4 is a POINTER global, lw then index); pad word D_800A354C tested against masks (0x4000|0x1000|0x2000|0x8000|0x40|0x10) << ((s5 sign-extended s16) << 4) -- i.e. per-player halfword select via sllv on the 32-bit pad word (sll 16 / sra 12 = s16*16 shift pattern, s:84-85). FOUR near-identical grid-walk blocks: up (s:99-133, (*s3)++ wrap >=5 -> 0), down (s:148-178, (*s3)-- wrap <0 -> 4), right (s:200-243, (*s2)++ wrap > D_800A35B4 -> 0, extra exit test (*s0&1) and *s2<4), left (s:260-300, (*s2)-- wrap <0 -> D_800A35B4). Each: func_8005C650(sfx,0x7F,0x7F); *s0 &= ~4; do { step+wrap; recompute cell+s0; D_800A3561[s1]=cell; } while (*s0 & 4 [and for lateral: (*s0&1)==0 && *s2 < 4]); then *s0 |= 4. The 4 walks are OPEN-CODED (not cross-jumped) -- each keeps its own wrap constants; translate as 4 copy-paste blocks, not a helper (no jal between them).

- [s1] [fable-blitz 2026-07-07] Edit arm tail (s:303-313): D_800A3530[s4*2] = 7; D_800A3534[s4*2] = 0; jump to join .L80070718. Display arm .L80070628 (s:315-377): D_800A3530[s4*2] = 9; if (((s16*)D_800A35C4)[s4]) decrement; anim counter D_800A3534[s4*2]++ capped at 12, written to (fp->0xC)+2 as byte; descriptor draw #1: p0 = fp->0xC, p1 = p0+0xC, x = *s2*116 + (*s2 s16 /2)*20 + 0x6E (synth_mult sll3-subu-sll2-addu-sll2 chain + sra 17 for /2, s:352-364), y = *s3*16 + 0x80, field+0x14 = 7, in_tex = s7->0x10; jal func_8007352C(sp+0x18); s7->0x10 = ret.

- [s1] [fable-blitz 2026-07-07] Join .L80070718: confirm button (0x40 << pad shift): if D_800A3560[s4*3] == 0xFF: if (*s0 & 1) { D_800A3562[s4*3] = 0xFF; D_800A3590[s4*2] = 2; D_800A3560[s4*3] = D_8009BC41[idx] (NOTE: 0xB41 base = D_8009BC40+1, second byte of cell pair); func_8005C650(D_8009BC40[idx] + 0xB, 0x7F, 0x7F); D_800A35C8 = 0xF; D_800A35CA = 0x14; if (D_800A35BC == 2 && (D_800A3568->0x14 & 0x20000) && s1... s4==0) { D_800A358A = (D_8009BC7C[D_800A3561] & 2) ? 2 : 0; D_800A358E = 0; if (*(s16*)D_800A35C4 == 0) { D_800A3564 = D_8009BC40[D_800A358A*2]; D_8009BC7C[that] |= 4; } BREAK out of the player loop (j .L80070B64, s:468+489) } } else func_8005C650(0xA,...). Cancel button (0x10 << shift, .L800708EC): if D_800A3578 == 0: func_8005C650(2,...); 3-way unwind on D_800A3554/D_800A3563/D_800A3560 state (s:516-571).

- [s1] [fable-blitz 2026-07-07] Loop tail .L800709FC (both arms): sp+0x40 flag byte = (((s16*)D_800A35C4)[s4] != 0); src ptr = (D_800A3554 && s4 != 0) ? fp[s4+1] : fp[s4] (sra 14 = *4 word index, s:599-604); pulse color: math_Sin((D_800A35C4->0x8 & 0x1F) << D_800A3530[s4*2] + s4*511 (sll 9 - self, s:619-620)); rgb bytes 0x41..0x43 = (sin*63 >> 12) - 0x40; x uses +0x4E bias (vs 0x6E in draw #1); second func_8007352C; s4++; recompute bound; loop.

- [s1] [fable-blitz 2026-07-07] Post-loop .L80070B64 (s:669-725): if (D_800A3580 == 0 && D_800A3560 != 0xFF && *(s16*)D_800A35C4 == 0): if (D_800A35BC == 2 && (D_800A3568->0x14 & 0x20000)) D_800A3554 = 1; then if (D_800A3563 == 0xFF || ((s16*)D_800A35C4)[1] == 0) if (D_800A35B0 + (s16)D_800A3554 == 0) skip -- else { D_800A3578 = 1; D_800A3558 = 0; if (D_800A35BC == 2) { D_800A3565 = 0xFF; D_800A3592 = 2; } D_800A3584 = 1; } (the a0=1 value is shared between the two sh stores -- single `1` local). Epilogue: jal func_8005C6D0(); restore 10 regs.

- [s1] [fable-blitz 2026-07-07] The descriptor struct = the S60C8 family already typed and byte-matched in COMPLETED sibling func_800600C8 (src/text1b.c:12969, same TU): {p0,p1,in_tex,pad0C,zero10,arg2@0x14,width@0x18,zero1C,pad20,pad24,byte28,3 rgb bytes}. Here: sp+0x18=base, +0x10=0 (once, pre-loop), +0x14=7 (per draw), +0x18=x, +0x1C=y, +0x20/+0x24=0x100 (once, pre-loop), +0x28=flag byte, +0x29..0x2B=rgb. 25 func_8007352C call sites in text1b.c to crib arg idioms from.

- [s1] [fable-blitz 2026-07-07] m2c reference generated: tmp/blitz/m2c_replay_camera_attack.c (372 lines, valid syntax, exit 0). No near-dup lead in tmp/duplicates_leads.txt. Callees all known symbols: saMotionSet, initTexPage, ot_Link, func_8005C650 (SE trigger), func_8007352C (prim emit), math_Sin, func_8005C6D0.

## Manual session 2026-09-29 (s2) — from scratch, 698 -> 12 (all remaining hunks GPREL-name artifacts)

Tooling (memory/grind/func_80070188/tools/, run from repo root under WSL):
- sc.py = `sandbox --disable all --candidate` clone that applies the landing-time
  overrides to the COPY only (sc_reps.py: SelectEntryE534 `pad`->`unk1`, the record
  merge across all consumers from merge_reps.py; sdata_exclude/sdata_syms diffs in
  tools/*.landing.diff). `--funcs=` scores several functions of the TU. `--mini` +
  mkpre.py: text1b.c prefix with every other body stripped (1 s per compile; verified
  equal to the full-TU score).
- Old ledger names: this function was `replay_camera_attack` in s1; symbols unchanged.

Measured path (scores are the sc.py full-TU/mini score for func_80070188):
- First full body from the asm/m2c decode (tmp/func_80070188/m2c.c): 142 (sandbox).
- s.x as `*col * 116 + 0x6E + (*col >> 1) * 20` and `s.y` before `s.ot_idx`:
  126 -> the x chain matches (fold-const factors 4 out of `*col*116 + (*col>>1)*20 + K`
  (20), `K + ...` orders 30-36; only `a*116 + K + b*20` / `K + a*116 + b*20` match).
- LOOP BOUND: `for (i = 0; i < 1 + D_800A35B0 + (port_ofs = D_800A3554); i++)` +
  `port = i - port_ofs;`: 118 -> 83. The target loads D_800A3554 TWICE in each copy of
  the loop test (`lhu a3` for the body's `subu a0,s4,a3`, `lh v0` for the bound;
  asm 37/40 and 659/662). An `lhu` in the test block whose value only the body uses
  can only come from the loop condition itself (stmt.c expand_end_loop moves the exit
  test, jump.c duplicates it; nothing else moves a load into it). Also fixes the frame:
  vars 64 -> 80 (target 0x90 = 80 + 24 args + 40 regs); the assignment used as a value
  in the duplicated test costs 8 phantom bytes per copy (phantom-frame-slots-gcc272).
  Comma form `port_ofs = D_800A3554, i < ...` also 83 / vars 80 (reads the global twice).
- RECORD MODEL (the big one): 83 -> 16 by declaring 0x800A3560 as an array of 3-byte
  records `{u8 unk0, unk1, unk2}` and writing `D_800A3560[i].unk1` etc. With the u8
  array every `D_800A3560[i * 3 + k]` expands (expr.c ARRAY_REF -> PLUS(&arr, MULT)
  with EXPAND_SUM, then memory_address/force_operand) to `reg = sym+k; reg2 = idx + reg`,
  so cse shares the ADDRESS (`sb v1,0(s1)`) and loop.c hoists `sym+k+idx` whole out of
  the walk loops. The target shares the INDEX (s1 = i*3; `lui at; addu at,at,s1;
  sb %lo(D_800A3561)(at)` / `lbu %lo(D_800A3560)(at)`; walks copy it `move a1,s1` or
  recompute it after a join). A COMPONENT_REF of a variable-index record expands its
  offset with expand_expr(offset) into a REG (get_inner_reference), giving exactly the
  target's in-MEM `(plus idx sym+k)` form and index sharing. Every u8-array spelling
  measured 118: `[i*3+1]`, `[1+i*3]`, `(&D_800A3560[1])[i*3]`, `*(D_800A3560+i*3+1)`,
  `*(i*3+D_800A3560+1)`, `[i+i*2+1]`, `(D_800A3560+1)[i*3]`; a loop-body `rec = i*3`
  named intermediate 139 (walks then use rec directly: no `move a1,s1`, no recompute).
  The same record model removes func_8006F97C's FAKE `rec` and func_80070C70's FAKE
  `ctx` named intermediates byte-identically (their FAKE comments describe exactly this
  address-expansion effect).
- Cancel arm: `D_800A3560[D_800A3554].unk0` (inside `D_800A3554 == 1`): 12 -> 10. With
  `D_800A3560[1].unk0` read + store, the constant address is forced into a register
  (explow.c memory_address force_reg; two uses -> cse keeps the register): `lui v1;
  addiu v1,3; lbu 0(v1)` / `sb s6,0(v1)`. cse folds the index load to 1 (jump equivalence
  `D_800A3554 == 1`) after expansion, so the MEM keeps a constant address -> both gp_rel.
- Timers: plain `D_800A35C8[0] = 0xF; D_800A35C8[1] = 0x14;` matches here (10); the
  func_800720FC pointer alias is NOT needed (reverse order 12).

Current candidate.c (2026-09-29): full-TU sc.py 12/698 (698/698 insns), every hunk
operand-only and every one a GPREL16 naming artifact: `%gp_rel(D_800A3588+2)` vs
target `%gp_rel(D_800A358A)` (same address; engine/score.py resolves named HI16/LO16
pairs but not GPREL16). Other consumers with the merge: E534 2, ECF4 0, F100 1,
F97C 1, 70C70 0, 71C20 1, 71C4C 1, 720FC 0 — all GPREL artifacts only
(`D_800A3560+1/+4` vs `D_800A3561/D_800A3564`).

Landing-time config needed (tools/*.landing.diff), each a no-op for the current tree:
- sdata_exclude.txt: func_80070188 drops D_800A3588/D_800A358C (target stores
  D_800A358A/D_800A358E gp_rel; the only non-gp D_800A3588 use here is the `la` for
  &D_800A3588[i], which maspsx never gp-converts); func_8006F97C and func_80071C4C drop
  D_800A3560 (their only non-gp D_800A3560 uses are indexed; after the merge their
  D_800A3564 / D_800A3561 reads are D_800A3560+4 / +1 and must stay gp as in target).
- sdata_syms.txt: add D_800A3590 (target `sh %gp_rel(D_800A3592)` = D_800A3590[1];
  every other D_800A3590 access in the tree is indexed or `la`).

### Layer-2 FAIL 2026-09-29 (landing attempt 1) — rejected/record-merge-e534-word-pun-0.c
- Landed form: candidate.c + the record merge (tools/land.py, full diff in
  rejected/record-merge-e534-word-pun-0.landing.diff): sandbox 0/698 and full-build
  SHA1 == oracle. Reverted; oracle re-verified green.
- Objection (only ground): func_8006E534's pre-existing `*(s32 *)D_800A3560 = -1;`
  (target `sw -1,%gp_rel(D_800A3560)`, func_8006E534.s:85) is a per-use pointer pun over
  the merged object — aggregate-merge prong (d) bans it for every consumer; pre-existing
  is no exemption. Member stores give 4x sb. The message also quoted only half of (d).
- PASSED on merits (keep): i*3 object-model evidence + record shape; alias rows for
  func_80070F78; the sdata edits; the loop-bound assignment; `[D_800A3554].unk0`;
  `flags` (one value, not a reused local); pad->unk1.
- A union declaration `union { Unk800A3560Record rec[2]; s32 word; } D_800A3560;` with
  `D_800A3560.word = -1;` in func_8006E534 matches every consumer (tools/sc.py with
  MERGE=union; tools/union_candidate.c): same scores as the struct form (GPREL-name
  artifacts only). Filed as policy-question in docs/grind/borderline.md 2026-09-29.
- No-merge probes: a FAKE typed pointer view `Rec3560 *recs = (Rec3560 *)D_800A3560;`
  (pointer-alias family) = 96/688 — the base stays in a callee-saved register
  (`lui s8; addiu s8`), so no in-MEM `(plus idx sym+k)`.

### No-merge search after the FAIL (2026-09-29, mini TU, per-byte symbols as on main)
- Same body with u8-array spellings `D_800A3560[i * 3 + k]`, D_800A3561/D_800A3564
  scalars: 76 (all remaining source-level hunks are the record-index address form).
- Per-site index intermediates recover it piece by piece:
  top `s32 rec = i * 3;` + a do-body `idx = i * 3;` just before each walk store: 41 -> 14
  (walks then get `move a1,s1` / recompute exactly as the target);
  confirm `D_800A3560[sel = i * 3]` in the && operand + cancel `k = i * 3` after the
  sound call + L9B4 `k2 = i * 3`: frame stays vars=80.
  Best: nomerge_best.c = 14/698 (8 GPREL artifacts + the cancel arm: the `D_800A3560[0]`
  / `[3]` constant stores share a forced `lui/addiu v1` base with the
  `[D_800A3554 * 3]` read; [3]/[3] 12 but 700 insns; h1/h2 reorders 14).
- That form carries ~8 FAKE named intermediates (one per site) plus an assignment in a
  condition; the record declaration (struct or union) needs none. Under the
  fewest-no-purpose-constructs principle the union form is the landing candidate if the
  owner allows it (borderline.md 2026-09-29).

### No-merge close: slot accessors (2026-09-29) — candidate.c, sandbox 0 + oracle
- Five `static inline` accessors for the record bytes (`slot_get0/1(rec)`,
  `slot_set0/1/2(rec, v)`, `rec` = slot * 3 byte offset). An inline call's argument is
  expanded into its own register before the body is integrated, so inside the body
  `D_800A3560[rec + k]` has a REG index: memory_address keeps `(plus rec sym+k)` in the
  MEM (the target's `lui at; addu at,at,idx; lbu/sb %lo(D_800A356k)(at)`), and cse
  shares the `i * 3` argument inside an extended block / recomputes it after a join,
  exactly the target's pattern. Declarations untouched (u8 D_800A3560[] as on main).
  Measured (mini TU, sc.py MERGE=0): plain u8 spellings 76; accessors with a u8-returning
  getter 37-39 (all operand-only: cse matches the QImode return pseudo with the constant
  0xFF on the `== 0xFF` path, so the confirm store is `sb v1` instead of the hoisted 0xFF
  register, which drops that pseudo's refs and swaps s6/s7/s8 seats); getter returning
  s32: 10 = GPREL-name artifacts only (full TU 12, same class). Constant-offset accesses
  direct or through the accessors are byte-equal (d_z/d_post/d_all 10); the landing uses
  the accessors for every record byte (one handle in this function, D_800A3561/64 not used).
- Landing (tools/land2.py): body + accessors over INCLUDE_ASM; SelectEntryE534
  `pad`->`unk1`; sdata_exclude func_80070188 row -> `g_gpu_ot_ptr` only (D_800A3588 /
  D_800A358C blocked the target's gp stores to D_800A358A/358E; D_800A3562 is not named by
  this C at all); sdata_syms + D_800A3590. verify-oracle --rebuild --allow-dirty SHA1 ==
  62efab4f... and sandbox 0/698 (2026-09-29).

### Layer-2 FAIL 2026-09-29 (landing attempt 2) — rejected/slot-accessors-0.c
- Objection: the five static inline accessors are the named-intermediate device in
  another spelling (identity wrappers whose only effect is a fresh index pseudo per
  site), with none of the family's tests (FAKE annotation, mechanism, exhaustion);
  precedent decisions.md:21698. The message's cancel-arm sentence also misdescribed
  the measured direct-[3] result (measured: [3] read + [3] store = 12 at 700/698,
  i.e. two extra insns; the `lui/addiu v1` base appears with the [D_800A3554 * 3] read,
  nomerge_best.c 14).
- PASSED unchanged: loop-bound assignment, `flags`, the sdata edits (incl. dropping
  D_800A3562), pad->unk1, the s32 getter type.
- Orchestrator frontier: per-site FRESH once-written FAKE named intermediates.

### Per-site named intermediates (2026-09-29, after attempt 2)
- nomerge_best.c already is that form: top `rec`, a do-body `idx` per walk (assigned
  just before the store), confirm `sel` (assigned in the && operand), cancel `k`, L9B4
  `k2` — one fresh once-written local per site. Score 14 = 10 GPREL-name artifacts +
  4 in the slot-1 cancel arm only.
- Slot-1 cancel arm (`if (slot1 byte0 == 0xFF) { D_800A3554 = 0; slot0 byte0 = 0xFF; }
  else slot1 byte0 = 0xFF;`), every direct spelling measured: read [D_800A3554*3]
  + store [3] + [0] 14 (the read's force_operand leaves `sym` in a pseudo that cse gives
  the slot-0 store: `lui/addiu v1` + `sb s6,0(v1)`); [3]/[3] 12 at 700; [3] read +
  [D_800A3554*3] store 12 at 700; both [D_800A3554*3] 14; slot-0 store as
  `[(D_800A3554 - 1) * 3]` before zeroing 14; `[D_800A3554]` after zeroing 14;
  `[D_800A3554 + D_800A3554 + D_800A3554]` 14 (v14 sweep).
- What closes it (accessor `slot_get0(3)` / `slot_set0(3, 0xFF)`, and the struct
  `[D_800A3554].unk0`): the index must already be a register when the address is
  expanded, and cse then folds it to 3, leaving `(mem (const sym+3))` direct (gp).
  With the u8 array that needs a local index whose value on that path is 3 — a dummy
  constant local in a subscript, refused by Q22 (named-local-fake-exception.md:128-154).
  No ordinary spelling found.

### After the Q22 ruling on `ofs` (2026-09-29)
- port_ofs lead (tools/port_ofs_sweep.py): `D_800A3560[port_ofs * 3]` read/store 72-73
  (710 insns), `port_ofs == 1` test 72-77: port_ofs then lives across the calls in a
  callee-saved register and the index is a runtime multiply (cse cannot fold it: the
  target tests the global), so the arm no longer compiles to constant gp addresses.
- Cancel-arm expression sweeps (tools/cancel_arm_sweep2/3.py): every product spelling
  of slot*3 (`D*3`, `3*D`, `D*2+D`, `D+D*2`, `(D<<1)+D`, `D*3+0`, pointer form) 14;
  [3]/[D+2] mixes 12 at 700. ONLY `D_800A3560[D_800A3554 + 2]` for both the read and the
  store closes it (10 = GPREL artifacts only; m3): PLUS(load, const) keeps the load in a
  register inside `(plus reg sym+2)`, which cse folds to `sym+3`. It is the Q22 device
  without a local (an index expression fixed at 3 on that path, `+2` has no reading of
  its own), so it is NOT proposed as ordinary C.
- Permuter campaign A (tools/mkws.sh workspace, PERM_RANDOMIZE on the D_800A3578 == 0
  block, -j 2, launched 2026-09-29T10:57Z): best early find output-120 = `new_var = 3;`
  constant holder as the multiplier (`D_800A3560[D_800A3554 * new_var]`) — a Q22 dummy
  local in a subscript; refused.
  Campaign A harvested/stopped after ~25 min: later finds (180-220) were the same
  dummy index (`new_var = D_800A3554 * 3`) or moved `D_800A3560[0] = 0xFF` out of its
  arm (changes the program). Nothing admissible.
- Permuter campaign B (plain u8 body nm/base.c = sandbox 76; full randomization, -j 2,
  ~20 min): base 3610 -> best 2715, which re-derives the index local itself
  (`D_800A3560[new_var = i * 3]` in the confirm test, the `sel` of nomerge_best.c);
  no novel find in the last 10-minute window; stopped.

### Session-end state (s2 bank, 2026-09-29)
- Floor without the merge: 14 (nomerge_best.c): per-site FAKE named intermediates
  (rec / idx x4 / sel / k / k2; annotations still to be written) reach everything except
  the slot-1 cancel arm (4 points; the rest are GPREL-name artifacts).
- Known closes, none landable today: (1) union declaration (owner question,
  borderline.md 2026-09-29); (2) struct records + a pun-free func_8006E534 word store
  (none found); (3) Q22-class fixed-value index in the cancel arm (`new_var = 3`,
  `ofs = D_800A3554 * 3`, `[D_800A3554 + 2]`) — refused / not proposed.
- Killed: record merge (FAIL, pun), inline accessors (FAIL, un-annotated device),
  port_ofs (72-77), every slot*3 product spelling (14), permuter A/B.
- Frontier: an ordinary cancel-arm spelling whose index is a register at expansion
  without being a fixed-value dummy (e.g. a restructure where the arm is reached with the
  slot index in a real variable), or the owner's answer on the union.

## Manual session 2026-09-29 (s3) — cancel arm closed without a record merge; floor 14 -> 6 (GPREL-name artifacts only)

Mechanism (compiler source + dumps, memory/grind/func_80070188/s3_dump_excerpts.txt):
- Constant-subscript element (`D_800A3560[3]`): expr.c:4589 ARRAY_REF with a constant index
  falls through to the COMPONENT_REF path; change_address -> explow.c:385 memory_address, which
  force_regs every constant address ("By passing constant addresses thru registers we get a
  chance to cse them", explow.c:396). cse1 then gives both cancel-arm stores the read's pseudo
  (excerpt C: insn 1508 `(plus r701 -3)`, insn 1520 `r701`), so the base survives as
  `lui/addiu v1` (700 insns). A scalar VAR_DECL's DECL_RTL `(mem (symbol_ref))` never passes
  through memory_address: no pseudo, nothing for cse to share, both accesses stay gp-direct.
  (With -G8 small data the constant would win cse's find_best_addr tie, cse.c:2622 +
  mips.h:2897 ADDRESS_COST(REG)=1 vs mips_address_cost SYMBOL_REF_FLAG ? 1 : 2; text1b is -G0,
  so not a lever, recorded for completeness.)
- Variable index (`D_800A3560[i * 3 + k]`): expr.c:4659 builds *(&arr + index); EXPAND_SUM gives
  (plus (mult i 3) sym+k); not legitimate -> explow.c:447 force_operand -> expand_binop copies
  the symbol into its own pseudo (excerpt A: r199 = sym+1, r203 = r202 + r199). With the index
  already a pseudo, (plus reg sym+k) is legitimate at explow.c:419 and stays in the MEM
  (excerpt B: insn 341). This is the per-site named-intermediate mechanism.

Closing lever: the slot-1 state byte at a constant address is spelled with its own splat symbol,
`extern u8 D_800A3563;` (and `D_800A3565` for the post-loop store), exactly as main already spells
slot 0's / slot 1's cell bytes `D_800A3561` / `D_800A3564` next to `D_800A3560[i * 3 + 1]` (same
function mixes both on main: func_8006F100, text1b.c:11790-11798, COMPLETED-C). Variable-index
accesses keep `D_800A3560[...]`. No fixed-value index, no new type.
- nomerge_best.c + scalar cancel arm (tmp d63): mini 14 -> 8; `k` then unnecessary (ablated 8).
- + post-loop `D_800A3563 != 0xFF` / `D_800A3565 = 0xFF` for consistency: full TU 6/698, every
  hunk operand-only GPREL-name (`%gp_rel(D_800A3588+2)` vs `D_800A358A` etc.).
- `sel` moved into the `if (*flags & 1)` block as an initialized local (no assignment inside the
  && condition): 6 (in the condition 6; a statement before the if 19).
- `idx` initialized at the top of each do-body: 8 (a `move a1,s1` schedules one slot early);
  kept assign-before-store.
- Ablation on the final candidate (tools/ablate_s3.py, full TU): inline rec 28; all four idx 47
  (each alone 12/12/16/16); sel 28; ofs (was k2) 18. Candidate = 6.
- candidate.c (2026-09-29 s3) = this form with FAKE annotations; landing config = the reviewed
  s2 config, minimal form: sdata_exclude func_80070188 row drops D_800A3588 / D_800A358C (keeps
  D_800A3562, g_gpu_ot_ptr), sdata_syms + D_800A3590, SelectEntryE534 pad -> unk1.

func_8006E534 side (task item 2), result: no pun-free spelling of its `sw -1` exists under a record
or 2-D declaration. GCC 2.7.2 emits a single SImode store only for an SImode lvalue: it has no
store merging; a BLKmode struct copy/constructor of 1-byte-aligned records goes through
move_by_pieces / store_constructor per field (byte stores; MIPS STRICT_ALIGNMENT keeps a 4 x u8
struct BLKmode); a struct copy from an initialized object loads (lw) instead of `li -1`. Every
consumer of 0x800A3560..65 in asm/funcs is a byte access except that one sw (grep, s3), so there is
no independent evidence of a word-sized member. The only cast-free word lvalue is the union
(borderline.md 2026-09-29). A 2-D `u8 D_800A3560[][3]` reaches get_inner_reference like the struct
(expr.c:4620 only takes the *(&arr+i) path for a variable OUTER index), so it would close the same
way, but E534's pun stays a pun over it: not proposed.
