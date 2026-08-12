/* MoveImage — DISPROVEN FORMS, grind session 3 (structural).
 * Every form here was measured with `sandbox MoveImage --disable all` on the
 * floor-2 candidate base and gated on build_insns == 49.  Raw tables:
 *   tmp/grind/MoveImage/s3/sweep7_results.md  (alias exemption on the rect)
 *   tmp/grind/MoveImage/s3/sweep8_results.md  (opaque packet pointer / reorder)
 *   tmp/grind/MoveImage/s3/sweep9_results.md  (rect[1] in the inter-store window)
 *   tmp/grind/MoveImage/s3/sweep11_results.md (dev-table placement, G5 base)
 *
 * ============================================================
 * FAMILY 1 — make the rect reads alias-EXEMPT from the stores.
 * ============================================================
 * Mechanism tried: sched.c:834-839 exempts a (MEM_IN_STRUCT && varying) ref
 * from a (non-struct && fixed) ref, and sched.c:828 exempts an RTX_UNCHANGING
 * read from any non-unchanging store.  So reading the rect words through a
 * struct pointer (COMPONENT_REF -> MEM_IN_STRUCT_P) or through a `const s32 *`
 * (RTX_UNCHANGING_P) should drop the rect[1] load's depth.
 * Result: the exemption FIRES, and it destroys the H2 delay-slot steal every
 * time — the freed rect load beats `sll $v0,$s1,16` to the head of the
 * post-guard block.
 *      C1 two s32 members, natural order          21 / 47
 *      C2 same + src hoisted into a local         21 / 47
 *      C3 only the SECOND word struct-typed       21 / 47
 *      C4 s16 RECT struct incl. the w/h guard     21 / 47
 *      C5 const s32 * rect, walking pointer       20 / 48
 *      C6 const s32 * rect, plain rect[0]/rect[1] 20 / 48
 *      C8 const struct pointer                    20 / 48
 * VERDICT: KILLED.  Neither MEM_IN_STRUCT_P nor const-qualification may be
 * applied to the RECT side of this function.
 *
 * struct form (representative — C1):
 *      typedef struct { s32 w0, w1; } MvWords;
 *      MvWords *r = (MvWords *)arg0;
 *      D_8009BF28 = packed;  *bf24 = r->w0;  D_8009BF2C = r->w1;
 * const form (representative — C5):
 *      const s32 *rect = arg0;  src = *rect++;  ... D_8009BF2C = *rect;
 *
 * ==================================================================
 * FAMILY 2 — opaque (multi-set) packet pointer, to give the dev-table
 * symbol load a true memory predecessor.
 * ==================================================================
 * Mechanism tried: sched.c canon_rtx() resolves the packet pointer to its
 * SYMBOL via reg_known_value (sched.c:421-433 — REG_EQUAL with reg_n_sets==1,
 * or REG_EQUIV), and two distinct symbols never conflict, so the dev-table load
 * has no predecessor and priority 1.  A packet pointer with TWO sets loses its
 * known value, and an opaque-REG store DOES conflict with a symbol load.
 * Result: the edge is created — and it is the WRONG edge.  A true store->load
 * dependence forces the dev-table load BELOW the packet store, whereas target
 * emits it ABOVE (asm/funcs/MoveImage.s, `lui $v1,%hi(D_8009BE6C)` at 8007B734
 * is 5 instructions ahead of `sw $a0,0x0($a1)` at 8007B748).
 *      E1 `bf24 -= 2;` after the dev-table read     9 / 49
 *      E2 `bf24 -= 2;` before the dev-table read    9 / 49
 *      E3 `bf24 = (s32 *)((u8 *)bf24 - 8);`         9 / 49
 *      E4 base-then-advance (`&D_8009BF24 - 2` then `+= 2`)  2 / 49 (cse folds
 *         it back to a single set — inert, NOT a counterexample)
 *      E8 E1 combined with the store reorder       10 / 49
 * VERDICT: KILLED.  Target's build does NOT have this dependence; any form
 * that creates it sinks the dev-table load.
 *
 * representative (E1):
 *      *bf24 = src;  D_8009BF2C = *rect;
 *      p = (s32 *)g_gpu_dev_table;  fn = (s32 (*)())p[2];
 *      bf24 -= 2;  return fn(p[6], (s32)bf24, 0x14, 0);
 *
 * =========================================================================
 * FAMILY 3 — rect[1] read anywhere BEFORE the packet store (extends H7a).
 * =========================================================================
 * H7a had established that hoisting the rect[1] read above the D_8009BF28
 * SYMBOL store loses the steal.  The measured dependence graph shows the real
 * boundary is the PACKET store (insn 75), not the symbol store: the rect[1]
 * load's depth comes from `75 -> 78`, and D_8009BF28 is a symbol store whose
 * edge into 78 contributes nothing.  So the untested window was "after the
 * D_8009BF28 store but before the packet store".  It is now tested and dead:
 *      F1 wh read between the two stores, walking   10 / 48
 *      F2 same, plain arg0[1]                       20 / 48
 *      F3 both reads after the BF28 store           14 / 48
 *      F4 same + BF2C store before the packet store 10 / 48
 *      F5 same + dev-table read hoisted             10 / 48
 *      E5/E6/E7 D_8009BF2C store emitted before the packet store  10-20 / 48
 * VERDICT: KILLED.  Any form in which the rect[1] read (or the D_8009BF2C
 * store) precedes the packet store drops to 48 instructions — the steal dies.
 * Exactly ONE rect read may sit ahead of the packet store, and rect[0] holds
 * that slot.
 *
 * ================================================================
 * FAMILY 4 — dev-table read placement on the plain-argument base.
 * ================================================================
 * On the alternate base (memory/grind/MoveImage/candidate_alt_plain_arg.c,
 * 4 / 49) the sole residual is the dev-table pair being 4 slots late.  Eleven
 * source placements of `p = (s32 *)g_gpu_dev_table;` (first statement of the
 * function, before the pointer setup, before/between/after each of the three
 * stores, with and without `fn = p[2]` and `q = p + 6` moved alongside) were
 * measured: TEN are byte-identical at 4 / 49, and the one that moved
 * (`p` + `fn` both hoisted above the stores) is 10 / 48 — the steal again.
 * VERDICT: KILLED.  The dev-table load's emitted slot is not steerable from
 * source order on either base; it is set by its scheduler priority, which is
 * what the next probe must attack.
 */
