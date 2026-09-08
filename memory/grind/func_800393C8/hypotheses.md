# Hypothesis ledger — func_800393C8

## s1 (2026-09-08, recon) — all measured on the -mel -msoft-float chassis, no FAKE constructs anywhere

| # | Hypothesis | Mechanism | Probe | Result | Verdict |
|---|---|---|---|---|---|
| 1 | The 2026-08-18 WIP candidate (26 on the hard-float chassis) measures lower on the softfloat chassis because its loop-1 residual was a single LICM hoist of `li -1` | loop.c LICM threshold halved by -msoft-float (58 vs 122 allocatable regs) | apply recovered WIP body, sandbox --disable all | 10 (loop 1 byte-identical; all residual in loop 2) | CONFIRMED |
| 2 | Loop 2 as a real `while`/`for` loop (not a goto-loop) lets LICM hoist the `-1` sentinel into the preheader like the target | goto-loops get no loop notes, so loop.c never sees them | var_A_while / var_D_for | 8 / 8 | CONFIRMED |
| 3 | Loop 2 as `if (idx < 0xB4) do { if (x) break; ... } while (D_800A3714 < 0xB4)` | rotated do/while duplicates the exit test | var_F_if_dowhile_inner | 15 (126 insns) | KILLED (instance; measured on the s1 body with no FAKE constructs) |
| 4 | An `s32 next` temp for the new index (`next = idx + 1; idx = next; D_800A3714 = idx` or store `next`) produces the target's second copy | second pseudo for the stored value | var_B_while_next / var_E | 10 / 10 (124 insns — an SImode temp adds a truncation copy) | KILLED (instance; s1 body, no FAKE) |
| 5 | An `s16 cur` temp (`cur = idx + 1; D_800A3714 = cur; slot += 0x10; idx = cur;`) is the second HImode pseudo the target carries (`move v1,v0; sh v1; ...; move t0,v1` in the delay slot) | two HImode pseudos: loop-carried idx (t0) and the stored value (v1); reorg fills the back-branch delay slot with the `idx = cur` copy | var_G / H / I / K | 2 (123 insns) each | CONFIRMED |
| 6 | The same two-pseudo shape can be had WITHOUT the temp by re-reading the global (`D_800A3714 = idx + 1; idx = D_800A3714`) | cse would substitute the stored pseudo | var_K2_no_cur | 6 (122 insns) — only one copy is generated | KILLED (instance; s1 body, no FAKE) |
| 7 | `mode = arg0 << 12` must be computed AFTER the `D_800A3714 == 0xB4` check so its pseudo does not conflict with the `lh v1` temp and can take v1; reorg lifts the `sll` into the `bne` delay slot | global.c conflict with the compare temp; reorg.c fall-through delay-slot fill | var_L (local after check) / var_M (inlined into the store) | 0 / 0 | CONFIRMED |
| 8 | `u8 age` instead of `s32 age; cnt = age & 0xFF` is byte-neutral (the `andi 0xFF` is the QImode zero-extension for the comparisons) | expand: zero_extend of a QImode pseudo for SImode compare; `age + 1` byte store stays unextended | var_N_u8age / var_final | 0 / 0 | CONFIRMED |

Killed spellings (all instance kills on this chassis, no FAKE constructs present):
- loop 2 goto-form (WIP candidate as recovered): 10
- loop 2 `if + do/while` with inner break: 15
- loop 2 with `s32 next` temp: 10
- loop 2 storing `cur` before `slot += 0x10`: 4
- loop 2 without the `s16 cur` temp (global re-read): 6
- `mode` local computed before the full-table check: 2

Frontier: none — sandbox 0 and oracle SHA1 match with the C in place (candidate-ready).
