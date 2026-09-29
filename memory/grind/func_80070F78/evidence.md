# Evidence bank — motion_ShiftControl

- [s1] Queue entry: distance=808, verdict=ASM-STRUCTURAL, rules=1, status=parked (2026-06-09 audit REJECTED auto-auth: standard GCC-compiled C). Single rule: asmfix.txt:187 replace_with_asmfile; src/text1b.c body is a stub. 808 = unwritten body. regfix.txt: zero rules.

- [s1] Structure (tmp/blitz/motion_ShiftControl_map.txt): 810 insns @0x80070F78-0x80071C1C, frame 0x70, saves s0-s7+fp+ra. Single jr $ra, void return. NO mult/div (all strength-reduced: *0x3F, *3, <<5/<<6 scaling), NO GTE, NO jump tables.

- [s1] Signature: void motion_ShiftControl(void *arg0, void *arg1). arg0 = draw-context struct (+0x18 prim/tex cursor advanced +0xC after initTexPage, +0x10 and +0x4 prim-list heads updated from func_8007352C/func_80073728 returns); arg1 = sprite work struct (+0x0 image ptr, +0x4 = img+0xC, +0x8/+0xC next links, +0x14 mode, +0x18/+0x1C screen x/y, +0x20/+0x24 scale 0x100->0x80, +0x28 flag, +0x29..0x2B RGB pulse from math_Sin).

- [s1] 3 loops, all over the same bound `D_800A35B0 + (D_800A3558 + 1)` (slot count = base count + extra when D_800A3558!=0): (1) small pre-pass 38-insn loop marking state-5/0x10 slots (sets D_800A3562[i*3]=6/7, sp18=1, timer[i]=0); (2) the 546-insn main per-slot loop (select/confirm/cancel handling); (3) the 90-insn draw loop (func_80073728 sprite per active slot). The bound expression is RELOADED each iteration (calls clobber memory) — write it inline in each loop condition.

- [s1] Loop counters are s16 (var_s3 patterns `var_s3_2 << 0x10 >> 0x10` everywhere; shift-amount computes `(s16)(i - D_800A3558) << 4`) — the pervasive sll/sra 0x10 pairs come from s16 counter locals; declaring them s32 would delete ~50 insns and never match.

- [s1] Pad-input decoding: D_800A354C is a button/trig word tested with per-slot nibble shifts: 0x2000<<(4*k) and 0x8000<<(4*k) = left/right, 0x40<<(4*k) = confirm, 0x10<<(4*k) = cancel, where k=(s16)(i - D_800A3558). Cursor per slot: D_800A3590[i] (s16 pairs, clamped to [var_a3..var_a2] = [0/1..4/5] depending on a D_8009BC7C[D_800A3561[i*3]] & 2 capability bit).

- [s1] State globals cluster (menu state machine): D_800A3560[i*3] slot state (5/0x10 = locked), D_800A3562[i*3] selected id (0xFF = none; 6/7 forced), D_800A3561[i*3] char id, D_800A3544[i] anim rate (7 or 9), D_800A3540[i] frame 0..0xC, D_800A35C4 timer array (s16, +4/+6 fields, set 0x1E), D_800A3578 mode latch (0->3->0x101), D_800A3584/D_800A3550/D_800A355C/D_800A3558/D_800A3565/D_800A3563 exit latches, D_800A3594[i] column, D_800A35BC game mode (2 = vs?, 3 = auto-confirm), D_800A3568->0x14 & 0x20000 flag, D_800A35A8 resource block (+0x74 image table, +0x7C VRAM x base, +0x60, +0x14 CLUT table indexed [id*8 + (capBit?0:4)]).

- [s1] Call pattern is the codegen-sensitive area: func_8005C650 (play SE) called 10x with VARYING arg counts — 4 args `(0,0x7F,0x7F,var_a3)` and `(2,0x7F,0x7F,var_a3)` vs 3 args `(1,0x7F,0x7F)`, `(2,0x7F,0x7F)`, `(6,0x7F,0x7F)` — an unprototyped K&R callee; preserving the per-site arg counts matters for a3 homing and for cross-jump-call-merge behavior (target may merge identical call suffixes).

- [s1] gpu_LoadImage x2 + gpu_DrawSync(0) x2 (confirm-path texture upload: src VRAM rect = D_800A35A8->0x7C + (i<<6) + id*8, data ptr = *(D_800A35A8 + id*8 + 0x14 + (capBit ? 0 : 4))) — the second instance (locked-slot re-upload at m2c lines 314-319) is nearly identical inline code, kept separate by control flow (no merge).

- [s1] Distinctive idioms for the draft: `(&D_800A3562)[-(var_s3_2 == 0) & 3]` = index (i==0 ? 3 : 0) via -(cond)&3 arithmetic; `((cond) == 0) * 4` CLUT-offset multiply of a comparison; RGB pulse `(math_Sin(((timer & 0x1F) << rate[i]) + i*0x1FF) * 0x3F >> 12) - 0x40` stored to 3 consecutive s8 fields (0x2B,0x2A,0x29 in that order); scale reset 0x100/0x100 inside loop vs 0x80/0x80 footer.

- [s1] Tail block (m2c 401-411): exit-condition check — if (D_800A3562 != 0xFF && timer[0]==0) { if (mode==2) D_800A3558=1; if (((D_800A3565!=0xFF && timer[1]==0) || (D_800A35B0+D_800A3558)==0) && sp18 != 1) { D_800A3578=0x101; D_800A3584=2; D_800A3550=1; D_800A355C=0; } } — sp18 is a genuine stack local (flag set 1 in pre-pass, 2 in locked-path).

- [s1] m2c decompiles CLEAN standalone (no jtbl needed): tmp/blitz/motion_ShiftControl_m2c.c (412 lines, --valid-syntax). Control flow fully structured; block_20/21/23 and block_94/95 are short-circuit condition joins (the mode==2 && !0x20000 && i==1 special-case check appears 3x — likely a repeated inline condition, not a helper).

- [s1] Function family: sibling UI functions func_8007352C / func_80073728 (sprite-emit helpers, both called here) and func_8005C650 (SE play) — check their completion status when drafting; the misapplied name is from Kengo matching but no kengo annotation exists on this function's stub.

- [s1] Ledger existed as empty skeleton (session_count=0); no WIP checkpoint.

## Manual session 2026-09-29 (s2, laneA) — from scratch, 808 -> 6 (record model, mini TU)

Tooling (memory/grind/func_80070F78/tools/, copies of tmp/func_80070F78/): sc.py (sandbox clone,
`--mini --pre=<prefix>`), mkpre.py (text1b.c prefix up to the INCLUDE_ASM line with bodies
stripped; sc_reps.py retypes the `func_80070F78` prototype + caller cast to DescF97C *), gen.py
(variant generator), dump.py (RTL dumps on the mini prefix), scr.sh (scores against
tmp/func_80070F78/mini/pre_rec.c = mini prefix with `u8 D_800A3560[]` replaced by a 3-byte
record array `{u8 unk0, unk1, unk2}` — the record model, NOT on main).

Measured path (mini TU, sandbox-equivalent score):
- First draft (m2c-guided, u8 array model `D_800A3560[i * 3 + k]`, per-byte scalars for constant
  offsets): 242/827. Same body in the record model: 116. Record-model bounds as siblings
  (`1 + D_800A35B0 + D_800A3558`, loop 2 `(port_ofs = D_800A3558)`): included.
- Loop 1: the two arms each carry their own `flag = 1; timer = 0;` (cross-jump re-merges them):
  cse sees the top block's i*2 in both arms (target's shared `a0`). 118 -> 75 with the timer
  if/else below and the tail order fixes.
- else-arm order x, y, ot_idx (as func_80070188).
- Loop 3 prologue: `s->header = sheets[0];` BEFORE the scale/has_color/ot_idx stores: any store
  through `s` invalidates cse's memory table (cse.c note_mem_written: varying address -> nonscalar;
  QImode -> all), so `cells = s->header + 0x24` reloads as the target does (70 -> 63).
- vram: `vram = *(u8 **)(D_800A35A8 + 0x7C); vram += i << 6;` (Ruling 4 compound split) at both
  sites: the load lands in vram's own register (target `lw s4,0x7C(v0)`); one-statement form puts
  it in a temp (expr.c binop subtarget is cleared inside loops, preserve_subexpressions_p). 63 -> 51,
  insn count 810/810.
- Draw tail order: colors, x, y, pad0C, ot_idx (permutation sweep of 5 items, 120 bodies: only
  CXYPO = 43).
- Image-table load `LoadImage(vram + id * 8, <slot>)`: the target computes id*8, +0x14, + base,
  + k*4, then lw 0 — every address-context spelling (INDIRECT_REF, ARRAY_REF, 2-D cast, struct view)
  expands under EXPAND_SUM, where expr.c both_summands floats the constant outward (lw 20(...)).
  Only an ASSIGNMENT (modifier 0 -> binop) keeps (id8 + 20) as its own insn: a pointer local
  `tim = (s32 *)(D_800A35A8 + 0x14 + id * 8 + k * 4); LoadImage(vram + id * 8, *tim);` (43 -> 21).
- Selector variables: ==3 site `k` (if/else -> jump.c store-flag, sltiu), confirm site `sel`
  (1 / special / 0), locked site `k2`: the target has three different registers (a1 local, a1 global,
  v1 local), so one shared selector cannot be it (l_* sweep).
- Locked site: `id` must be a single-set value there (inline `D_800A3560[i].unk2` twice, cse'd):
  sched1 adjust_priority birthing boost (reg_n_sets == 1) puts the unk2 load after the store-flag,
  as the target (21 -> 6).
- Remaining 6: (a) GPREL-name artifact `%gp_rel(D_800A35C8+2)` vs `D_800A35CA` (operand-only);
  (b) locked-arm other-player timer select: target `lw v0,D_800A35C4; bnez s0; addiu v1,v0,4;
  addiu v1,v0,6; lh v0,0(v1)` (base v0, pointer v1, beq delay slot filled from the target);
  ours base v1 / pointer v0. Forms measured: pointer local if/else both orders, ?: forms,
  `&[i==0?3:2]`, `+= ?:` all 27-30 (extra insns); `+2; if (i==0) +3` / `timer++` / `&[2]`/`&[3]`
  / block-local = 6.

Open construct questions for landing (not yet settled):
- Record model needs the 0x800A3560 record merge across all consumers + func_8006E534's word
  store (union word view = Q33, being encoded 2026-09-29) — or a per-byte fallback with per-site
  FAKE record-offset locals (70188 style), unmeasured on this body yet.
- `tim` pointer locals (three sites) and the selector variables `k`/`k2` separate from `sel`.
