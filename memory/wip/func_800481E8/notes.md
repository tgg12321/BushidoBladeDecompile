# func_800481E8 (text1b.c) — BLOCKED: forbidden dead-frame floor (canonical-asm)

## TL;DR
Target frame is **72 bytes** (`addiu sp,sp,-72`, saves s0/s1/s2/ra at 56/60/64/
68); GCC produces a **40-byte** frame. The ~32 extra bytes have **no recoverable
semantic local** — the function uses only registers + one stacked 5th call arg to
efc_buki_draw_zanzou. The 12 regfix rules patch the frame size (40->72) + all 8
register save/restore offsets (24->56, 28->60, 32->64, 36->68 and the lw mirror)
+ a prologue reorder + an INLINE_MOVE_ALIASING `addu $16,$18,$5 -> addu $16,$16,$5`.
HEAD also carries a `register asm("$16")` pin + `__asm__("move %0,%1")`
INLINE_MOVE_ALIASING in the body. Honest distance 14.

## Why it's not pure-C (FORBIDDEN class)
Per [[dead-vars-local-array]] (FORBIDDEN 2026-05-31): matching the 72-byte frame
requires either `s32 pad[8];` (forbidden dead local array — the detector refuses
completion) or the register pins / frame-size regfix (cheats). There is NO local
with clear semantic meaning that accounts for the 32 dead bytes. The rule's
explicit guidance for this exact situation: park / request canonical-asm
authorization; do NOT add a pad array.

Sibling precedent: `AddTbpOfst_80047EE8` (same file) was PARKED 2026-06-07 for
the identical "dead-vars-local-array frame floor" (commit 0244c4dd).

## Endpoint
Canonical-asm authorization (or a genuine identification of what the 32 frame
bytes semantically held in the original — none found; the call-loop uses no stack
buffer). Blocked. This is a user/orchestrator decision, not worker pure-C work.
