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

## Manual session 2 (2026-09-28) — current stock cc1 (PLUS->IOR patch removed)
Re-measured (sandbox --disable all): candidate.c 30 (417); s2/dxyz-reuse-2.c = candidate
+ end block reusing dx/dy/dz only = **2** (416/416, one operand-only hunk: the site-1
LZC-input copy seats $v1, target $a0). s2/closing-form-0.c (dx/dy/dz reuse + `len`
holding the squared length then both roots + `lzc_in` copy that then stages the table
byte) = **0** (416/416). Splits of that form: site-2 root in its own local 11
(s2/site2-len-split-11.c); copy moved into the else arm 23; copy+table-byte reuse
without the `len` reuse 4 (s2/copy-tbl-only-4.c: copy in $a0 but `lzc_in` becomes the
CSE-canonical register, so the < 0x400 test and LUT index read $a0, target $a1).
dx/dy/dz Ruling 11 ablations: full per-value 30; value 1 (segment delta) own, 2+3
shared 30; value 2 own 8; value 3 own 8.

### Why the copy seats $v1 (instrumented cc1, BB2_FINDREG_DEBUG=88 on dxyz-reuse-2)
Copy pseudo 88: conflicts v0 a1 t4 s5 sp; someone_prefers / own_copy_prefs /
own_full_prefs EMPTY -> first free in numeric order = v1 (ord 13). In closing-form-0
the same pseudo (lzc_in) also holds the table byte, so it is live across the lz
computation (hard conflict with v1, which local-alloc gives that code) and
`(set X (ashift lzc_in 16))` with X local-allocated to a0 gives own_full_prefs = a0
(global.c set_preference, first-operand rule, :1681). A copy dead at the island has
neither. `len` reuse keeps the squared length canonical in cse (make_regs_eqv,
cse.c:844-857: the copy becomes canonical only if its last use is later than the chain
head's), which is what keeps the test/index on $a1.

### Single-role site-1 spellings: floor 2 (s2/gen1.py, s2/gen1_scores.txt)
320 spellings on the dxyz chassis: sum into h_sq or n (either copy direction), copy
before/after the < 0 check, every consumer (printf, < 0, < 0x400, LUT, island,
shift-index) reading either variable, root into `len` or back into the sum variable.
Distribution: 96 x 2, 64 x 4, 88 x 13, 64 x 15, 8 x 21. None below 2.

### Other routes killed this session
- Inline helper `static inline s32 lut_sqrt(u32 x)` (the pattern in src/system.c):
  15 (s2/inline-helper-15.c). GCC 2.7.2 allocates the inlined frame with
  assign_stack_temp(keep=1) (integrate.c:2092) inside expand_assignment's temp level
  (expr.c:2660-2664), so both calls share ONE LZCR slot (sp+280); the target has two
  (0x118/0x11C). An unmodified parameter is substituted without a copy; a reassigned
  one copies at BOTH sites (target: site 1 only). Dead.
- Staging the site-1 LZC input through the existing `len_sq` (its real job is site 2):
  cc1 coalesces the copy into h_sq's $a1 and site 2 moves to $a1 too. Dead.

### Status
The closing form needs a local that holds a bare copy (the LZC input) and then the
table byte — Ruling 11 text (ordinary-c-judge-decidable.md:1340-1343) names
func_8002A458 `lzc_in` as a (C)(3) bare-copy FAIL, and the 2026-09-25 layer-2 failed
it under the staged-value rule (bound 2). This needs an owner ruling; asked 2026-09-28.
dx/dy/dz and `len` are Ruling 11 candidates on their own (real computations).

## LANDED 2026-09-28 — COMPLETED-INLINE-ASM-CANONICAL (manual session 2)
auth: 6038c278f (inline_asm_canonical.txt row, inline_o.h class) -> Match: 9f53bf788
(src/code6cac_b.c + region hashes) -> queue: 61b3b4331. Full-build SHA1
62efab4f73f992798c43e8c730aa43baa10bb4fa; sandbox --disable all 0 (416/416) from src;
check_completion_integrity OK. Layer-2: round 1 FAIL (temp proof: cse argument presented as
universal, segment-length $a1 seat unproven), fixed in r11/proof.md; round 2 PASS (18 more
counter-spellings banked, none at 0). Closing constructs: dx/dy/dz, temp, temp2 under Ruling 11
(temp2's copy under Q28), header-exact gte_Lzc islands, one do-while(0). Proof: r11/proof.md.
