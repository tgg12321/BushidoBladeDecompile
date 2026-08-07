/* s53 REDERIVE: m2c --reg-vars verbatim statement-order transplant.
   Shape: arg5 chain (shift + add + load) placed BEFORE t0 chain
   (shift + tbl-add), mirroring m2c-regvars output line ordering:
     t0 = idx_1494[0];    // lbu a0
     v0 = idx_1494[1];    // lbu v0
     v0 <<= 2;            // sll v0  (arg5 shift first)
     v0 += (s32)tbl_125c; // addu v0 (arg5 addr, in-place)
     arg5 = *(s32*)v0;    // lw v1
     t0 *= 4;             // sll a0  (t0 shift AFTER arg5 chain)
     t0 = (s32)((u8 *)tbl_125c + t0); // addu a0

   Rationale: m2c --reg-vars v0,v1,a0 produced a shape where arg5 addr
   comp (v0<<2, v0+=tbl, *v0) is *fully sequenced* before t0's shift/add.
   Novel: prior s2 E_arg5_first flipped only the head lbus; this variant
   preserves the head-lbu order but flips the whole shift+add cluster.

   Result: masked=15, target_insns=160, build_insns=160 (+13 vs h5=2).
   REGRESSION identical to the v0-triple-set-addr-walk sibling probe:
   the same s39 v0-triple-set-carrier basin. The t0-first head-load
   preservation does NOT protect the h5 basin when the arg5 subseq
   fully precedes the t0 subseq — sched.c reorders both chains through
   the same qty allocations regardless of C statement placement.

   KILL: m2c-regvars-derived statement ordering (arg5-fully-before-t0)
   is NOT a novel h5-preserving axis. Confirms s10 conclusion that the
   pair-swap window is coupled through qty arithmetic; source-order
   permutation within the block does not decouple the two chains'
   LAUNCH signatures.
*/
    t0 = idx_1494[0];
    v0 = idx_1494[1];
    pp = (void **)&D_800F19C0;
    v0 <<= 2;
    v0 += (s32)tbl_125c;
    arg5 = *(s32 *)v0;
    t0 *= 4;
    t0 = (s32)((u8 *)tbl_125c + t0);
