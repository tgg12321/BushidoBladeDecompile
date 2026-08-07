/* s83 F20 probe: symmetric idx_1495 comma-op dereference at HEAD of both vblank if-arms.
 * Structural modality attempt to lift p78 refs at flow-time before local-alloc.
 * Result: masked=2 (INERT vs h5 baseline), target_insns=160, build_insns=160.
 * Mechanism killed: comma-op unused-value reads are DCE-eliminated at flow-time by
 * delete_noop_moves/DCE BEFORE local-alloc counts refs (same mechanism confirmed at
 * s78 for local scalar pointer carriers p73/p77/p78/p79).
 * KILLED subvariant: byte-neutral (0-emit-delta) idx_1495-lift is DCE-invisible for
 * the poll-region symmetric arm insertion. Complements s78's +2-insn-visible subvariant.
 * Closes the F20 frontier item (last un-measured p78-lift subvariant).
 */
      if (status & 4)
      {
        if (D_800A11B8 != 0)
        {
          ((void (*)(u8, void *)) D_800A11B8)((*idx_1495, *idx_1495), &D_800F19A8);
        }
      }
      if (status & 2)
      {
        if (D_800A11B4 != 0)
        {
          ((void (*)(u8, void *)) D_800A11B4)((*idx_1495, *idx_1494), &D_800F19A0);
        }
      }
