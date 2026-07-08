/* s44 rejected: `cnt = D_800F19BC++;` (single-statement postfix outer counter)
 * masked 4 — novel spelling in vT40 basin (10th known distinct spelling), no
 * floor drop. Compound postfix compiles to the identical lw/addiu/sw sequence
 * as the split 2-statement form; outer BB structure unchanged; do_timeout
 * pair-window insn stream identical to vT40. Banking to keep the rejected
 * catalog complete; do not re-derive.
 */
/* body: identical to tmp/grind/marionation_Exec/s44/v01_postinc_cnt.c */
