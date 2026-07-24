/* REJECTED s2, score 12. Post-loop Triple copy references copy_end instead of
 * src (legal since src==copy_end at loop exit), attempting to extend copy_end's
 * live range past the do-loop toward the tail.
 * MEASURED: copy_end(77) nrefs=3->4, livelen only 5->6 (the Triple copy is
 * ADJACENT to the loop, so the range barely grows), pri rose to 13333, stayed
 * in t0. AND it breaks the tail bytes: target's Triple lw uses $a2 (src); using
 * copy_end($t0) as the base changes those 3 lw -> mismatch. Dead: nothing
 * structurally uses the end pointer after the loop, so its range can't extend.
 */
        } while (src != copy_end);
        *(Triple *)dst_q = *(Triple *)copy_end;   /* <- breaks bytes; range +1 only */
