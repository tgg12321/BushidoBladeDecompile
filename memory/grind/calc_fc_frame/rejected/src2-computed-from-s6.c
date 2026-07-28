/* REJECTED (s1, floor 13): computing the +4 pointer into a fresh local
 * `src2 = src_base + 4;` makes GCC compute it from the callee-save copy
 * (addiu a0,s6,4) instead of target's in-place `addiu a0,a0,4`, and frees
 * $a0 too early so the scan/count temporaries allocate a0/a1 instead of
 * target's a1/a2. The original C increments the PARAM itself
 * (src_base += 4) and keeps a separate `src_orig = src_base;` copy for the
 * post-loop call arg — param stays live in $a0 to its natural position. */
    src2 = src_base + 4;
    data_ptr = (s32)orig_dest + (count + 2) * 4;
    ...
        src_ptr = (s32 *)src2;
        ...
            func_800520B8(src_base + start, data_ptr, size);
