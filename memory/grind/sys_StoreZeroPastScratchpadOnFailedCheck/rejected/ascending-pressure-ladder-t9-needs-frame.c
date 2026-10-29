/* REJECTED s6 (solver): ascending-pressure ladder. With N values loaded and live across
 * the store in one block, the scratchpad address pseudo seats at $(N+2) ascending: $10..$15
 * frame-free (N=8..13), then $16=$s0 WITH a frame at N=14, and $25=$t9 only at N=23 (framed).
 * Generator + table: tmp/grind/func_80052C10/s6/ladder.py, ladder.txt. Form below = N=23. */
typedef volatile unsigned int vu32; extern volatile int G[];
void f(void) {
  int v0 = G[0];
  int v1 = G[1];
  int v2 = G[2];
  int v3 = G[3];
  int v4 = G[4];
  int v5 = G[5];
  int v6 = G[6];
  int v7 = G[7];
  int v8 = G[8];
  int v9 = G[9];
  int v10 = G[10];
  int v11 = G[11];
  int v12 = G[12];
  int v13 = G[13];
  int v14 = G[14];
  int v15 = G[15];
  int v16 = G[16];
  int v17 = G[17];
  int v18 = G[18];
  int v19 = G[19];
  int v20 = G[20];
  int v21 = G[21];
  int v22 = G[22];
  *(vu32 *)0x1F800400 = 0;
  G[100] = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 + v21 + v22;
}
