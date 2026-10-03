# Lane X1 10: s32 D_8009060C[38] + s16 D_800906A4[39][2] (func_8003DA8C). Standalone. OWNER QUESTION inside.
Files: include/code6cac.h (StatusUpBuf / D_800906A4 scalars -> 2 arrays), src/code6cac_c2.c,
asm/data/7D920.data.s (StatusUpBuf dlabel merged into D_800906A4), undefined_syms_auto.txt, named_syms.txt,
symbol_addrs.txt (-StatusUpBuf).
## Finding: CONFIRMED
`&D_800906A4 + arg0*2` (s16 *, i.e. byte arg0*4), `(u8 *)&StatusUpBuf + arg0*4` (StatusUpBuf = 0x800906A6) and
`(u8 *)&D_8009060C + arg0*4`: two per-stage tables, 38 s32 at 0x8009060C and s16 pairs at 0x800906A4 ([0]
tested, [1] passed on), 39 pairs to D_80090740 (the last {0,0}). Plus the permuter locals new_var / new_var2.
Note: Kengo's symbol table has a 154-byte `StatusUpBuf` - exactly 0x800906A6..0x8009073F. The name was pulled
from symbol_addrs.txt by size; in BB2 its only reader treats the bytes as the [1] halves of 4-byte pairs.
## Changed body
func_8003DA8C 0 (86/86): `rec = D_800906A4[arg0]; if (rec[0] != 0)`, `ptr = &D_8009060C[arg0]; ... *ptr`,
permuter `new_var` / `new_var2` gone (plain `if (dist < 0x1770)`).
## FAKEs (2, added) - the first needs an owner ruling
`s32 idx = arg0 * 4; ... *(s16 *)((u8 *)D_800906A4 + 2 + idx)` (twice) instead of `D_800906A4[arg0][1]`:
an element read is MEM_IN_STRUCT and sched.c true_dependence (tools/gcc-2.7.2/sched.c:834-839) lets a varying
in-struct HImode read pass the fixed scalar store `D_80090608 = ...` just above, so both reads move ahead of the
`sh` (score 8). `*(s16 *)(s32)&D_800906A4[arg0][1]` (15), a pointer local read twice (15), a const view (18) fail.
Question: item 4 forbids a raw-offset cast to the declared object; item 3 admits a FAKE-labelled codegen
construct. Is this byte-offset read on the declared array admissible as a FAKE? Alternative honest-by-type form
is (a) one struct over 0x800905F8..0x8009073F (no evidence beyond adjacency - not done) or (b) leave HEAD.
Second FAKE (same mechanism): `rec = D_800906A4[arg0]; if (rec[0] != 0)` - a direct `D_800906A4[arg0][0]` read
is scheduled above the D_800905F8 store (score 9). Labelled, as it exists only for that.
## Verification
cmpall c10: code6cac_c2 all 75 functions 0 (relocs renamed), other 29 TUs identical; 7D920.data.s re-assembled:
section bytes identical, only StatusUpBuf(.NON_MATCHING) symbols gone.

## Draft commit message (if the FAKE is admitted)
cheat-cleanup: D_8009060C[38] / D_800906A4[39][2] - func_8003DA8C indexes the per-stage tables; StatusUpBuf retired

func_8003DA8C read two per-stage tables through byte offsets off three scalars, the pair's second half under its
own name StatusUpBuf (0x800906A6). Declared in code6cac.h, the dlabel merged, the name retired; the permuter
locals go. Two FAKEs: the pair read through `rec`, and its [1] read off the array base (MEM_IN_STRUCT reads pass
the D_800905F8 / D_80090608 scalar stores in sched.c).

Verification:
  - harness: func_8003DA8C 0; code6cac_c2 all 75 functions 0; 30 code6cac.h TUs section-identical; data .s identical
  - verify-oracle --rebuild SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa (at landing); layer-2 (at landing)
