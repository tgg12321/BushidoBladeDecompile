/* _SendPAD (0x80079000, LIBAPI SENDPAD, src/main/psxsdk/libapi/sendpad.c). Best honest C:
 * sandbox --disable all = 4 of 10 (2026-10-03), operand-only: GCC loads the pointer into $v0 and
 * saves $ra at 0x10(sp); the original uses $t1 and 0x14(sp) (frame 0x18 either way, the load
 * already ahead of the frame). cc1psx-check: the original cc1psx scores the same 4.
 * Also 4: pass-through (int a0..a3) returning the call; an int global cast to a function
 * pointer at the call; a local copy of the pointer. */
extern void (*D_800A362C)(void);

void _SendPAD(void) {
    D_800A362C();
}
