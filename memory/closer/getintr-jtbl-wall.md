# getintr (func_80080828) — the system.c jump-table placement wall (2026-07-10)

## The function
LIBCD BIOS `getintr` (static), 0x80080828..0x80080DB0, 353 words, dist 352
(whole-body asmfix splice). SOTN bios.c `getintr()` maps 1:1; v1.86 deltas
identified from the target asm:
- DiskError reporting is TWO independently-guarded printfs
  (`if (D_800A11C0 > 0) debug_printf(...)` twice — jump-threading of the
  CSE'd guard reproduces the exe's single-skip shape), strings
  D_800161E4 "DiskError: " + D_800161F0 "com=%s,code=(%02x:%02x)\n";
- default case: puts(D_8001620C "CDROM: unknown intr") + debug_printf
  (D_80016220 "(%d)\n", nReg);
- symbol map: D_800A147C/1480/1484/1488 = CDRegister0-3 pointer globals;
  D_800A11C0 debug level; D_800A11C4/C8 CD_status/1; D_800A11CC CD_nopen;
  D_800A11D5 CD_com; D_800A11DC comstr tbl; D_800A137C = D_80032C68 tbl;
  D_800A127C = D_80032B68 tbl; D_800A1494/95/96 = Intr.sync/ready/c;
  Result[3] = D_800F19A0/A8/B0 (u8[8] each); jtbl_8001622C = switch table.
- `volatile u8 nReg` @sp+0x10, `volatile u8 buf[8]` @sp+0x18, i in s0,
  bHasError in s1, inline _memcpy with NULL guard (loop: init 7, bne vs -1).

## Blocker 1 — jump-table placement (STRUCTURAL, new class for system.c)
The switch's table must land at 0x8001622C, inside
src/text1a_b_post_rodata.c's region (hand `const u32 jtbl_8001622C[5]`).
bb2.ld places system.o(.rodata) AFTER display.o's — a GCC-emitted switch
table from system.c physically cannot land at 0x8001622C, and deleting the
hand const shifts all downstream rodata. main.c does NOT have this problem
(S_SCA's switches emit into main.o's own region correctly — that's why
func_8008AF9C closed). No committed C in the tree uses
computed-goto-through-extern-jtbl (`goto *(void *)jtbl_8001622C[nReg-1];`)
— that spelling would emit the right bytes but needs an owner ruling (it is
semantically truthful post-link but exists only because of TU layout).
Same wall applies to ANY system.c/display.c-region function whose jtbl
lives in a *_rodata TU: check before starting (grep the jtbl symbol).

## Blocker 2 — Intr member volatiles (grant class, same as proposal §3)
Cases 4/5 need `Intr.ready = Intr.c = 4` chained stores WITH re-read
(sb; lbu; sb) — requires volatile D_800A1494/95/96 (= g_cd_status_a/b/c,
which the twins' HEAD bodies already declare volatile as ungranted debt).
Ruling-4 class (BIOS.OBJ census-proven, codegen-measured). NOTE: granting
these changes the TWINS' sandbox scores (their volatile decls stop being
stripped) — coordinate with Phase 2 before proposing.

## Ready-to-run when unblocked
Full SOTN-based C sketch exists in the session-8 transcript; the la/at-form
law from cdread_triple_README.md §1 predicts the mixed forms in getintr's
cases (the la-form D_800A1494 stores in cases 3/5 = volatile-cast or
struct-first-access spellings; at-form 1495/1496 = volatile-decl direct).
Estimated one focused session once both blockers clear.
