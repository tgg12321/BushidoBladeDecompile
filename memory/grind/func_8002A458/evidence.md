# func_8002A458 — evidence (manual session 1, 2026-09-25)

Queue: distance 416, ASM-PARTIAL (two gte_Lzc LZCS/LZCR islands, census row
:69 of the 2026-08-17 cop2 cluster, `.claude/rules/cop2-addressing-preamble-cluster.md`).
No prior ledger. Caller func_8002AB08 passes (obj, &mask_a, &mask_b, quiet).

## Result
candidate.c: sandbox --disable all 0 (416/416, 0 scored hunks); spliced into
src/code6cac_b.c (INCLUDE_ASM replaced; the file's `Vec3i` typedef moved up
from above func_8002C61C so this body can use it) → verify-oracle --rebuild
--allow-dirty build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa. Prepared for
layer-2 review (READY_FOR_REVIEW), not committed.

## What the function does
Weapon-segment sweep for character `obj` (id = obj+4, partner = obj+0).
Scratch record 0x1F8002B8 (`scr`): +0x60 = scr, +0x64 = scr+0xC, tip copied to
+0xC8, d = tip - base. diff = |partner->0x1D8 - ratan2(dx,dz)| folded to
0..0x800. dx²+dz²-dy² < 0 → printf("ILLEGAL GUN MOTION : %d\n") and return.
Pitch/yaw into +0xF8/+0xFA (+0xFC = 0), optional func_80032854(id==0, 0xB, ...).
q = (d<<12)/|d|; base := tip; tip += q*4. If diff < 0x400: func_8002E838(scr),
22 records of D_800F5F68[id] (stride 0x14) vs scratch points 0x1F8000A8 +
id*0x108 + i*0xC via func_8002EA24 → bits in *hit / *deep (6..9 skipped
unless obj+0x26C). Base pulled back by q/4 into +0xA8, func_80053614 stage
cast; on a hit not of material 7: if *hit, clear both masks when the stage
point is nearer the base than obj+0xF4 (else skip the sfx); optional
func_80032854(id==0, 0xA, scr+0x100, 0). D_800A37E8/EA/EC = -q.

## Floor progression (sandbox --disable all)
97 (straight transcription) → 88 (dz/dx allocno tie) → 54 → 39 → 12 (end block
reuses dx/dy/dz) → 6 (`lz = ~1; lz &= sp_tmp;`) → 4 (site-1 `len` holds the
squared length then its root; `lzc_in` = LZC-input copy that stages the table
byte) → 0 (do-while(0) around `rec = &D_800F5F68[id * 0x1B8];`, found by the
permuter (perm2, iter 716) as a wrap of the final func_80032854 call on a
header-exact-island chassis, then applied by hand at the rec init, which fixes
both seat pairs with merged islands).

## Allocation facts that drove the search
global.c priority = floor_log2(refs)*refs/live_len (live_len = sched.c
post-sched1 value; refs weighted by loop depth in flow.c). Target seats:
s0 partner/diff/rec+0x12 giv/scr+0x100, s1 dz/bit, s2 dx/pos, s3 dy/i, s4 rec,
s5 scr, s6 i*12 giv, s7 id, fp obj; spilled hit, deep, quiet, qx/qy/qz, id*0x108.
- obj (8 refs, live 242) beats id (8 refs, live 251) and scr (31/232 hdr,
  31/224 merged) beats rec (8/44) in the natural source order. The do-while(0)
  doubles the weight of rec's set and the three id refs of id*0x1B8 → both
  flip. Without it the only fixes are source-order hacks that move `lh s7` /
  `move s3,zero` (score 4-6).
- The header-exact PsyQ form (6 asm statements per gte_Lzc) also fixes
  rec/scr by lengthening scr (+8 insns), but the engine strips the GPR-only
  `move $12,%0` and `nop` statements as cheat-asm, so it cannot be scored;
  banked as rejected/header-exact-islands-objid-swap.c.
- Site-1 copy register: n needs a0; v1 is first free unless the copy also
  stages the table byte (lzc_in << 16 is local-allocated to a0 → preference)
  AND the squared length stays canonical in CSE (len reused as the root keeps
  its last use after lzc_in's). Only that pair reaches it (gen5 sweep, 256
  spellings).

## Layer-2 FAIL (2026-09-25) and current honest frontier
The sandbox-0 body (rejected/len-lzcin-dxyz-multiwrite-0.c) FAILED layer-2 on two
items only. The islands, census-row path, canonical row, region hashes, the
do-while(0) around the rec init, the `lz` Ruling-4 split and the Vec3i move
were all ruled SOUND. They land only together with a passing body.
- `lzc_in` / `len`: lzc_in is a bare copy invented to be borrowed (fails the
  staged-value bound 2; fails R5 1(a)/(e) and R6 (D)). len holds three
  quantities (fails R5 1(a)/(b) and R6 (D)).
- dx/dy/dz: each is written 3x with different quantities, twice in one
  block (fails R5 1(b)/(d) and R6). The staged-value annotation also lacked
  a named pass and a ledger pointer (bound 4).
candidate.c is now the HONEST form: separate h_sq + single-write copy +
separate tbl at site 1, six fresh end-block locals, and dz/dy/dx declared in
that order. It keeps the do-while(0) and the lz split. It scores 30
(417 insns): about 2 from the site-1 copy landing in $v1, and about 28 from
the end block. Fresh end-block deltas are block-local, so local-alloc puts
them in $v0.., while the target has them in $s2/$s3/$s1, dx/dy/dz's own seats.
A non-call-crossing single-role pseudo cannot get those seats, because
local-alloc and find_reg take caller-saved registers first and there is no
copy preference to an s-register. The target bytes therefore imply the
original reused dx/dy/dz ("point minus segment base" at all three sites).
Tried and killed on the honest side: an inline `lut_sqrt()` helper (22 fast
lines; the two sites share one LZCR slot; it would also move the islands out
of the body).
Frontier: a policy question to the owner on (1) reusing dx/dy/dz as the
point-minus-base delta at three sites and (2) the in-place `len = sqrt(len)`
plus the LZC-input copy. Alternatively, find a single-role structure whose
end-block deltas are call-crossing or s-register-preferred. None is known.
