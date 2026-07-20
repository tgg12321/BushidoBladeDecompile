/* REJECTED s3: hoist new_var computation to end of loop (right before
 * the call). Score 13. Loop-body reordering breaks maspsx's scheduling
 * of the multi-lhu sequence; new diffs appear across the load block.
 * Mid-loop position (between a2v and a3v reads) is load-bearing. */
