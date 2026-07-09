# BREAKTHROUGH: the contested twins ARE PsyQ libcd bios.c functions (2026-07-09)

## Identity (proven by string xref in disc/SLUS_006.63)

- `0x80016240` = `"CD_sync"` — the string cpu_side_move_dir_4 stores into D_800F19C0.
  **cpu_side_move_dir_4 == CD_sync(int mode, u_char *result)**
- `0x80016248` = `"CD_ready"` — marionation_Exec's D_800F19C0 value.
  **marionation_Exec == CD_ready(int mode, u_char *result)**
- `0x8001626C` = `"CD_cw"` — **saEft01Init == CD_cw** (the third twin).
- `0x80016274` = `"$Id: bios.c,v 1.86 1997/03/28 07:42:42 makoto Exp $"` — the exact
  source file + version. BB2 links libcd bios.c **v1.86**.
- fmt `0x800161C8` = `"%s:(%s) Sync=%s, Ready=%s"`, banner `0x800161B8` = `"CD timeout: "`.

The Kengo-derived names (cpu_side_move_dir_4 / marionation_Exec / saEft01Init) are
BOGUS symbol-collision artifacts ("x4 size collision" comment in system.c admits it).
These are Sony library functions; the original author is Sony (makoto), not Lightweight.

## Reference source: sotn-decomp psxsdk/libcd/bios.c (v1.77, same fn set)

Fetched to `tmp/closer/sotn_bios.c` (MIT-licensed matched decomp, GCC 2.7.2 PSX
toolchain class). BB2 = v1.86 vs SOTN = v1.77 — minor diffs possible, window
strings identical.

## Symbol mapping (all verified against BB2 data segment + asm)

| BB2 | libcd original | Notes |
|---|---|---|
| D_800A1494/1495/1496 | `static volatile CD_intr Intr` = `{u8 sync; u8 ready; u8 c;}` | ONE 3-byte volatile struct; IRQ-mutated (InterruptCallback(2, callback)) |
| D_800A125C | `CD_intstr[8]` (char* table: NoIntr..DiskError,?,?) | SEPARATE object from Intr |
| D_800A11DC / D_800A11D5 | `CD_comstr[]` / `u8 CD_com` | |
| D_800F19B8/19BC/19C0 | `volatile Alarm_t {int time; int count; char *name;}` | ONE struct (volatile in SOTN) |
| D_800F19A0/A8/B0 | `Result_t D_...[3]` (8-byte sync/ready/dataend result bufs) | CD_sync copies 19A0; CD_ready copies 19B0 (c-path) / 19A8 (ready-path) — matches SOTN exactly |
| D_800A147C..1490 | `static volatile u8* CD_reg` pointers (0x1F801800..) MMIO | mmio-volatile-type-level applies |
| func_80080828 | `getintr()` | |
| sys_GetVblankCount | `CheckCallback()` | misnamed |
| tslTm2LoadImage_2 | `puts` | misnamed |
| debug_printf | `printf` | |
| cdrom_ClearIrq | `CD_flush()` | |
| D_800A11B4 / D_800A11B8 | `CD_cbsync` / `CD_cbready` | |
| D_800A1498 | `CD_init_struct D_...` = {&Intr, &result, &CD_com, &CD_status, &CD_pos, rcsid} | its first word POINTS AT D_800A1494 = &Intr |

## The original C for the contested timeout window (SOTN v1.77, get_alarm inlined)

```c
if (Alarm.time < VSync(-1) || Alarm.count++ > 0x3C0000) {
    puts("CD timeout: ");
    printf("%s:(%s) Sync=%s, Ready=%s\n", Alarm.name,
           CD_comstr[CD_com], CD_intstr[Intr.sync], CD_intstr[Intr.ready]);
    CD_flush();
    return -1;
}
```

CD_sync tail: `sync = Intr.sync; if (sync == CdlComplete || sync == CdlDiskError) {
Intr.sync = CdlComplete; _memcpy(result, &D_sync_result, 8); return sync; }` —
matches BB2 asm semantics (==2 || ==5, then store 2).

## Why this matters for the residuals

1. `Intr` is VOLATILE in the original — every .sync/.ready load is an
   unCSEable, unreorderable lbu. 155 grind sessions fought scheduling/RA ties on a
   NON-volatile chassis; the honest volatile struct form was never measured.
   Volatile here is ORIGINAL SEMANTICS (IRQ-mutated), squarely inside
   [[legitimate-volatile-interrupt-touched]] — not a coercion.
2. `CD_intstr[Intr.sync]` / `CD_intstr[Intr.ready]` are member accesses off one
   3-byte struct base — the honest replacement for the h5 cross-symbol lever.
3. The original loop is `while (1)` (s30's while(1) KILL was measured on the wrong
   non-volatile chassis — re-measure on the faithful structure).
4. saEft01Init (=CD_cw, 15 rules) and getintr/CD_flush/CD_init siblings can all be
   respelled from the same reference source.

## Grind-ledger corrections

- s45 "no external-repo analog exists" is WRONG — sotn-decomp contains the same
  function (identity wasn't recognized because scans were asm-shingle similarity,
  not semantic identity).
- The W4 one-struct question (Ruling 1): 125C vs 1494 = SEPARATE objects
  (fakematch branch confirmed for h5), but 1494/95/96 = ONE struct (CD_intr).
