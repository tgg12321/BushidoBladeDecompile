# Probe grid — cc1 crash class isolation (2026-07-13)

Verifying `docs/escalations/closer-cc1-fork-divergence.md` gate 4.

Compiled with production flags:
`-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w`
via `tools/gcc-2.7.2/build/cc1`.

| File | Shape | Volatile? | -O level | Result |
|---|---|---|---|---|
| a.c | `while ((*(vol u16*)MMIO) & 0x30) { if (++i > 0xF00) break; }` | yes | O2 | **CRASH** |
| probe1_for.c | `for (i=0; (*(vol u16*)MMIO) & 0x30; ) { if (++i>K) break; }` | yes | O2 | **CRASH** |
| probe2_infloop.c | `while(1) { if(!cond) break; if (++i>K) break; }` | yes | O2 | compiles (structurally different) |
| probe3_dowhile.c | `if (cond) do { if (++i>K) break; } while (cond);` | yes | O2 | compiles (current candidate; structurally different) |
| probe4_nonvolatile.c | `while (g & 0x30) { if (++i>K) break; }` | **NO** | O2 | **CRASH** |
| probe5_no_break.c | `while (g & 0x30) { ext(); }` | no | O2 | compiles (no break) |
| probe6_break_only.c | same as probe4 | no | O2 | **CRASH** |
| probe7_break_no_cond.c | `while (g & 0x30) { if (h) break; }` (no `++i`) | no | O2 | compiles (no pre-increment-compare) |
| probe8_o1.c | same as probe4 | no | **O1** | **CRASH** |

## Isolated trigger

**Crash class**: `while (LOOP_COND) { if (++LOCAL_COUNTER > CONST) break; }` at O≥1.

- Volatile-ness of LOOP_COND is INCIDENTAL (probe4/6/8 crash on plain `u16 g`).
- The `++LOCAL_COUNTER > CONST` break INSIDE a while's body is load-bearing (probe7 with plain `if (h) break;` compiles; probe5 with no break compiles).
- Reformulating to `while(1)` or `if + do-while` avoids the crash but drops the loop notes that anchor `mostly_true_jump`'s likely=1 verdict on the back-edge — which is the precondition for the delay-slot `+1` fill and the reorg increment-compensation `-1` that target contains.

## Why non-crashing alternatives cannot match

Target (asm/funcs/DispUpdateStatusMessage.s:150-160):
```
    addu   $v1,$0,$0
    addiu  $v1,$v1,0x1        ; pre-increment before loop
.L80088BF0:
    sltiu  $v0,$v1,0xF01
    beqz   $v0,.L80088C14
    nop
    lhu    $v0,0x1AA($a0)      ; volatile
    andi   $v0,$v0,0x30
    bnez   $v0,.L80088BF0
     addiu $v1,$v1,0x1         ; delay-slot fill (increment)
    addiu  $v1,$v1,-0x1        ; reorg compensation (dead on normal exit)
```

Both compiling alternatives (probe2, probe3) emit an increment BEFORE the compare (not filling the delay slot) and lack the `-1` compensation. 3-instruction structural gap, deterministic across body-order permutations.

## cc1psx counter-exhibit

`t_psx.s` — cc1psx compiles `a.c` (the crashing input for our fork) with exit 0. Loop region matches target byte-for-byte.
