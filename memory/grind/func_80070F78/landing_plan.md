# func_80070F78 — landing plan (per-byte model; BANKED, not landed — 2026-09-29)

Status: held by the orchestrator pending the owner's ruling on the 0x800A3560 object model
(the per-byte model `u8 D_800A3560[]` + scalars D_800A3561..D_800A3565 is under audit on
func_8006E534 / func_80070188). This plan lands the per-byte form; if the owner rules for a
record/union declaration, see evidence.md "Union / record merge vs this function" first
(under -G0 the union form measures 67/822; per-file -G8 would be needed).

## Files and edits (all in one Match commit)
1. `src/text1b.c`
   - replace `INCLUDE_ASM("asm/funcs", func_80070F78);` with memory/grind/func_80070F78/candidate.c
     verbatim (its five leading externs included: D_800A3540[], D_800A3544[], D_8009BC38[],
     D_800A3594[], D_800A3562);
   - prototype `extern void func_80070F78(s32 a0, s32 *prim);` ->
     `extern void func_80070F78(s32 a0, DescF97C *s);`;
   - delete the `typedef struct PrimC70 {...} PrimC70;` block (no other user);
   - func_80070C70: `PrimC70 prim;` -> `DescF97C prim;`, field renames p_geom->header,
     p_static->table, link->out, zero10->semi, code->ot_idx, mode->x, zero1C->y, width->scale_x,
     height->scale_y, byte28->has_color; its comment "clears mode/zero1C a second" -> "clears x/y a
     second"; the call `func_80070F78(arg0, (s32 *)&prim);` -> `func_80070F78(arg0, &prim);`.
   Mechanically: `python3 memory/grind/func_80070F78/tools/land.py memory/grind/func_80070F78/candidate.c`
   (needs tools/sc_reps.py next to it at tmp/func_80070F78/ — the scripts reference tmp paths).
2. No config edits: sdata_exclude.txt's `func_80070F78: D_800A3560, g_gpu_ot_ptr` row is already
   right (indexed D_800A3560 accesses never gp-convert; D_800A3561/2/3/5 constant accesses go gp via
   sdata_syms.txt, which already lists them); no undefined_syms rows change (D_800A3540, D_800A3544,
   D_800A3594, D_8009BC38, D_800A3562 rows stay: C now names them).
3. After PASS: `queue done func_80070F78`, commit engine/queue.json, check_completion_integrity,
   ledger note recording the layer-2 PASS (Q39).

## Proof so far
- tools/sc.py full TU with the edits: func_80070F78 1/810 (GPREL-name artifact
  `%gp_rel(D_800A35C8+2)` vs `D_800A35CA`), func_80070C70 0/194, func_80070188 0/698,
  func_800720FC 0/690, func_80071C4C 0/270, func_80071C20 0/11.
- Full build of the spliced tree with candidate.c exactly (tmp/func_80070F78/final.c, 2026-09-30
  under the laneA lock): build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa, build_matches true;
  reverted and rebuilt green (earlier u9 body also matched 2026-09-29).

## Constructs a reviewer must judge (receipts)
- FAKE record offsets `rec` (loop 1), `rec` (loop 2), `ofs`, `idx` (x2), `other`: ablations
  52 / 14 / 22 / 32 / 33 / 24 (ablations/*.c), mechanism = func_80070188's.
- FAKE `tim` (x3): ablations 9 / 10 / 15.
- Ruling 11 `vram`, `sheets`, `cells`: r11/README.md (per-value 24; dumps r11/r11info.txt,
  r11/allocdbg.txt; permuter campaigns r11-all-final / r11-all-final-2). Weak point: the second
  `vram` and `sheets` writes re-load the same lvalue after calls (README (B)(2)).
- Ruling 4 `vram = load; vram += i << 6;` (one statement 29).
- `((s16 *)D_800A35C4 + 2)[i == 0 ? 1 : 0]` for the other player's timer.
- The prototype / func_80070C70 descriptor type change (removes a cast).

## Pre-submission checklist (2026-09-29)
- [x] staged body == measured body: candidate.c == tmp/func_80070F78/final.c; every ablation and
      per-value spelling is generated from it (tools/abl_final.py).
- [x] every FAKE: family named (named-intermediate entry), annotation present, cited path
      memory/grind/func_80070F78/evidence.md exists.
- [x] every multi-value local: Ruling 11 named, prongs in r11/README.md, innermost-scope declarations.
- [x] merged/retyped symbols: none merged; DescF97C retype of func_80070C70's descriptor sandboxed 0.
- [x] no single-use helper/intermediate without annotation; no fixed-value index (`other` is 3 or 0).
- [ ] commit message: tmp/func_80070F78/msg_match.txt (draft; re-check numbers at landing).
- [x] verify-oracle on candidate.c exactly (2026-09-30); sandbox on the spliced src to re-run at landing.
