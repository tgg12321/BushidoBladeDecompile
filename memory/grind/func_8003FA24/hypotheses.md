# func_8003FA24 hypotheses

## CONFIRMED

### H1 — packed data model (session 1, still holds)
Point list via `D_80103608[*(s16*)(obj+4)][*(s16*)(obj+2)]`; three halfwords per
point copied into 8-byte records; first stream pass computes
`count*3` or `count*6` (bit 3 of flags) and advances by `D_80094AEC[(flags>>3)&3] * count`;
second pass emits type-3 (5 halfwords) / type-4 (6 halfwords) scratchpad packets at
`0x1F800000`; then `func_80045230`, `func_8003FE40`, `func_80017D84`, aligned cursor
returned. All of it reproduced.

### H6 — the store constants must not be LICM-hoisted; a multi-set `s16` user
variable is the lever (session 2). Measured: `-dL` shows threshold 58 (soft-float),
our loop 50 insns → always hoists. `s32` scratch is folded away by CSE and changes
**nothing**; `s16` scratch works; reusing the already-live `flags` works best.
119 → 97 → 89.

### H7 — `short` counter + `while (--count != -1)` produces the target's
`addiu v0,a3,-1 / move a3,v0 / sll / sra` idiom (session 2).

### H8 — the three alignment sites are ternaries
`cur = ((u32)cur & 3) ? cur + 2 : cur;` (session 2).

### H9 — `cc1`'s `.frame ... vars=` IS `get_frame_size()`, and is a far better
gradient than the sandbox score for frame questions (session 2, `tmp/frame.sh`).

## KILLED

- **H2** phase-specific counter split (s1): 129 → 137. Re-killed independently in s2
  in a different chassis: a separate packet counter takes vars 48 → **56**.
- **H3** dedicated `s16 *` walker for pass 2 (s1): 129 → 140.
- **H5** permuter proposal inserting `count = mode` (s1): semantic lie, rejected
  without scoring.
- **H10** (s2) `s32` scratch variable to defeat the hoist — byte-identical output;
  CSE re-creates the single-set constant pseudo. Only `s16` survives.
- **H11** (s2) one shared `u8 *aligned` local across all three alignment sites —
  collapses one site, 263 → 260 insns, score 107.
- **H12** (s2) `n = *(s16*)src++; count = n;` to get the outer load's `lh`+`move`
  without the assignment-in-condition — GCC coalesces `n` into `count`, the `move`
  vanishes, and score goes 83 → 85. They never conflict, so coalescing is
  unavoidable without an artificial later read of `n`.
- **H13** (s2) blaming the +16 `vars` on declared block-scope locals — converting
  all of them to ternaries/returns left vars at 48. It is inherent to the
  assignment-in-condition spelling (see evidence.md frontier item 1).
- **H14** (s2) pressure reduction as a route to E's frame — inlining `mode`,
  block-scoping `value`, and shortening the scratch's live range all still give
  vars 48.

## NEUTRAL

- **H4** (s1) named stride/field-selector intermediates — byte-identical.

## FRONTIER (ranked)

1. A spelling of the outer packet-loop count read that yields the target's
   `lh v0,0(s0)` + `move a3,v0` **and** keeps `vars = 32`. Every
   assignment-in-condition form measured costs exactly 16 phantom bytes; every
   split-statement form loses the `move`. This is the single highest-value item.
2. An honest source construct that puts `packet_type & 2` in the **type-3** loop
   body so `loop.c` hoists the target's dead `andi a2,v1,0x2` into that preheader.
   Worth 3 insns (the `andi` itself plus the two delay-slot nops our redundant
   `li t1,-1` blocks). Note the two preheaders are byte-identical in the target —
   whatever it is, it is in BOTH loops.
3. The register cascade — expect it to largely follow (1) and (2).
