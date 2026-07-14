/* REJECTED — distance 3 in every spelling (s2, 2026-07-13).
 * Any form where p WALKS (`p += 2`) and its only value uses are the two p+0xC
 * address givs: loop.c combines the load+store givs and reduces them to a
 * BIASED walker (addiu s1,s2,12; lhu/sh 0(s1)) vs target's addu s1,s2,zero;
 * lhu/sh 0xC(s1). Measured 3 for ALL of: (a) plain, no wrap; (b) wrap around
 * the two calls kept (committed shape minus alias); (c) alias kept but wrap
 * removed (cse copy-propagates the alias away before loop analysis);
 * (d) p += 2 inside its own do-while(0); (e) whole body inside do-while(0)
 * — .loop dump proves biv verified + reduced identically in (d)/(e).
 * Also: RMW statement inside do-while(0) = 5; `return (s16 *)p;` post-loop
 * use = 20 (keeps biased giv AND spills an extra callee-save).
 * The committed new_var-alias form worked only because `new_var = p` inside
 * the wrap survives cse as a zero-add DEST_REG giv that anchors combine_givs
 * at bias 0. The clean fix is deriving p from i (see candidate.c). */
u8 *p = base;
do {
    /* ... */
    *(u16 *)(p + 0xC) = (u16)(*(u16 *)(p + 0xC) + val);
    i++;
    p += 2;
} while (i < 6);
