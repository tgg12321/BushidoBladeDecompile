/* KILLED s2: walking pointer bp = base+0x1A; b=*bp; r=*(bp-2); g=*(bp-1).
 * Hypothesis: Different base pointer pseudo yields distinct MEM alias set,
 * reorganizing sched.c dependence graph, giving b higher priority.
 * Result: score stayed at 2, emission still [r,g,b]. GCC 2.7.2 (combine/CSE)
 * folded `bp - 2` / `bp - 1` back to base+offset form BEFORE sched1 ran, so
 * all three MEMs share the identical (mem:QI (plus (reg:SI player) (const_int K)))
 * shape. Same dependence graph as literal form => same schedule. */
u8 *bp = (u8 *)player + 0x1A;
b = *bp;
r = *(bp - 2);
g = *(bp - 1);
gnd_load_tex(b | ((r << 16) | (g << 8)));
