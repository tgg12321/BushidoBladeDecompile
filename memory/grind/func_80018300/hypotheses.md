# func_80018300 — hypotheses (manual lane, 2026-09-24)

## CONFIRMED
- H1 island clobbers "$12".."$15" (inline_o.h text) drive the $s0/$s1/$s2/$t8/$t9 seats. 81 -> 54 (scr-var body), 44 on ablation of final.
- H2 constant operands as literal `"r"((s32 *)0x1F80000C)` rather than a reassigned `scr` local: the local lets cse reuse the 0x1F80000C register for the following `sum` loads (`lw v0,0(v0)` vs target `lui/lw 12`). 54 -> 43.
- H3 LZC arm variable reuse (len carries count+shift, sum takes table byte). 43 -> 16.
- H4 packed link-index word staged through `thresh`. 16 -> 8. The own-variable pseudo cannot conflict with dx/dy/p2 in any spelling where its live range is [lw, node-pointer calc]; only sharing thresh's pseudo seats it in $t1.
- H5 frame slack = oversized live LZC object (carve-out). 8 -> 0 with lz[5]/lz[6].

## KILLED (frame: phantom-slot alternatives before the carve-out)
Probe harness tmp/f18300/probe.py (instrumented cc1, BB2_FRAME_DEBUG) on the score-8 body; none moves `vars=` off 8:
- assignment-as-value: `if ((sum = ...) < 0x400)` at both sites; `nthresh = -(thresh = radius * 3)` at both sites.
- scratch stores through a `scr` pointer variable, a struct-typed constant base, `((s32 *)0x1F800000)[i]`.
- 19 single spelling swaps (u16/s16 halves, *64 vs <<6, `/=` vs `= /`, s32 vs u32 sum/len/lz, scratch array reads, `& -2`, cast placement, for vs while, radius+radius*2, s16 count, pointer-typed base, data/out `+=` forms, midpoint operand order, `/ 8` vs `>> 3`): all vars=8; 14 instruction-neutral, 5 change instructions (cnt_s16 mid_sum radius3 sh3div sum_s32; label-normalized compare, tmp/f18300/probe_cmp.py).
- Outgoing-args phantom is impossible: lz sits at 0($sp), so the 16 bytes are above it inside the locals region.

## KILLED (pair)
- data[1] re-read per arm (no variable): 79, 313 insns.
