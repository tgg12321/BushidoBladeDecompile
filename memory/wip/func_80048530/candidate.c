/* func_80048530 — CLEAN pure-C form (no pins, no __asm__, no barriers).
 * sandbox --disable all = 12 (HEAD's cheated honest floor is 11, so this is
 * NOT yet below HEAD — do NOT apply to src/). This is the cheat-FREE base a
 * future agent should resume from: HEAD only "matches" via 5 regfix rules +
 * inline-move-aliasing cheat-asm + "" barriers; this body removes all of them.
 * Three residual diffs (see notes.md): (1) arg0 base-register double-copy
 * routing (target keeps v1=arg0 then move t0,v1, reuses v1 as the pointer; we
 * keep arg0 in one reg), (2) `move a1,a3` hoisted before the bounds-check beqz
 * in target (we emit it in the taken branch), (3) c/d emitted as `lh` where
 * target uses `lhu`+sll+sra (the (s16)(u16) fold). Fixing (3) via u16-hold +
 * (s16)-at-call cascades the arg setup to score 22 — it is coupled to (1). */
s32 func_80048530(s32 arg0, s32 arg1, u32 arg2, s32 arg3) {
    u8 *p;
    s32 count;
    s32 entry_off;
    s32 a, b, c, d;
    p = (u8 *)arg0 + ((s32 *)arg0)[arg1];
    count = *(s32 *)p;
    if (arg2 >= (u32)count) return -1;
    p += 4;
    p += arg2 * 0xC;
    entry_off = *(s32 *)p;
    p = p + 4;
    a = (s32)*(u16 *)p;
    p = p + 2;
    entry_off = entry_off + arg0;
    b = (s32)*(u16 *)p;
    p = p + 2;
    c = (s32)(s16)*(u16 *)p;
    d = (s32)(s16)*(u16 *)(p + 2);
    func_800485EC(entry_off, arg3, (s16)a, (s16)b, c, d);
    return count;
}
