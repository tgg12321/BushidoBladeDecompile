# Evidence bank — func_80038170

- WIP rejected_form: {'form': 'dummy0/dummy1 + __asm__ "m" (HEAD)', 'score': 'matches-via-cheat', 'why': 'FORBIDDEN frame-coercion; sandbox strips it -> honest 12'}

- WIP rejected_form: {'form': 'strip all cheats, s1,s2,s3 order (clean pure C)', 'score': 12, 'why': 'pure +8-byte frame gap; body byte-identical'}

- WIP rejected_form: {'form': 's32 s3=0,s2=0,s1=0 (reversed decl order) — rejected/decl-order-prologue-flip.c', 'score': 'would be 12 same', 'why': 'FAIL: cheat-reviewer — prologue-save-order manipulation, param-local-alias-prologue-pair-flip family'}

- WIP rejected_form: {'form': '{ s32 tmp[2]; } (dead array nested block, no stores)', 'score': 'gives vars=16/frame=56 with correct save offsets, no extra body stores', 'why': 'FORBIDDEN: dead-vars-local-array; engine detector refuses completion'}

- WIP rejected_form: {'form': '{ s32 tmp[2]; tmp[0]=0; tmp[1]=0; } (dead array with stores)', 'score': 'gives vars=16 but adds 2 extra sw insns + shifts register alloc', 'why': 'FORBIDDEN: dead-vars-local-array + produces wrong body asm'}

- WIP rejected_form: {'form': 's32 tmp1,tmp2; (dead scalars)', 'score': 'vars=8', 'why': 'GCC eliminates dead scalar stack slots entirely'}

- == imported from memory/wip notes.md ==
# func_80038170 (code6cac_c_mid.c) — BLOCKED: phantom 8-byte frame local

## TL;DR
Same class as func_8001924C. Target frame -56, cc1 naturally gives -48. The ONLY pure-C
mechanism that gives vars=16/frame=56 without extra body asm is `{ s32 tmp[2]; }` (dead array,
no stores) — **FORBIDDEN** per `dead-vars-local-array` (engine detector refuses completion).
Declaration-order reversal (s3,s2,s1) also **FAILS** cheat-reviewer (prologue-save-order
manipulation). Honest distance = **12, all from +8-byte frame delta**. 141-insn body is
otherwise byte-identical. No recoverable semantic local. Card BLOCKED.

## The gap (precise)
- natural cc1 (clean pure C): `addiu sp,sp,-48`, vars=8, saves at 24/28/32/36/40
- target:                       `addiu sp,sp,-56`, vars=16, saves at 32/36/40/44/48
- Body: NO sp-relative accesses except prologue saves + epilogue restores.
- The 8 extra bytes at sp+16..sp+31 (target) are NEVER accessed. Phantom local.

## What was tried (full lever record)

1. **HEAD cheats stripped** → score 12 (all frame delta). Body byte-identical.
2. **s3,s2,s1 declaration order** → gives correct save ORDER naturally (s0,s3,s2,s1,ra).
   FAILS cheat-reviewer: prologue-save-order manipulation (param-local-alias-prologue-pair-flip).
   Saved under `rejected/decl-order-prologue-flip.c`.
3. **Dead array `{ s32 tmp[2]; }` (no stores, nested end block)** → vars=16/frame=56, correct
   save offsets, NO extra body stores, no register-alloc shift. FORBIDDEN: dead-vars-local-array.
   This IS the compiler mechanism but the technique is forbidden by policy.
4. **Dead array with stores** → vars=16 but adds 2 extra `sw $0,16($sp)` instructions + shifts
   register allocation. Does not match target body even if frame is right.
5. **Dead scalars `s32 tmp1,tmp2`** → vars=8. GCC eliminates scalar dead-stack slots entirely.
6. **Arg-area expansion** → func_80079194 called with only 2 args; expanding prototype doesn't
   change arg-build area (only actual passed args count).
7. **Structural variants** (all s1/s2/s3 orderings, outer j/k/v loops, mask-before-i) → all
   give vars=8/frame=48. No structural variant produces vars=16.

## HEAD cheats (all forbidden)
- `register s32 i asm("a3")`, `register s32 mask asm("a1")` — register pins
- `s32 dummy0, dummy1; __asm__ volatile ("" :: "m"(dummy0), "m"(dummy1));` — dead-vars frame-coercion
- regfix `reorder 10,11,13,12,9 @ 9-13` — save-order reorder
- `prologue_config.json` entry — replaces prologue with hardcoded -56 frame

## Resume guidance
No remaining pure-C levers. The correct disposition is park (unrecoverable phantom frame local,
no semantic local recoverable from body). Surface for user authorization (canonical-asm or accept
as indefinitely parked INCOMPLETE). Do NOT attempt any dead-array or declaration-order approach.


- == session s1 (2026-07-28, recon) ==

- [s1] Clean-strip baseline re-measured: floor 14 (not the ledger's 12) — 12 frame-delta insns (2x addiu sp + 10 save/restore offsets) + prologue pair-order residue. Metric: engine masked Levenshtein vs build/src reference .o.

- [s1] FRAME GAP SOLVED IN PURE C (kills the WIP-era "unrecoverable phantom local / no remaining pure-C levers" conclusion). Mechanism = phantom-frame-slots-gcc272 (memory/project/): rewriting the if(s3) pair as ONE-table indexing `(&D_8008F19C)[s3*2+0]` / `[s3*2+1]` (D_8008F19D is the splat per-byte auto-symbol for D_8008F19C+1) makes GCC 2.7.2 allocate an 8-byte compiler stack temp: vars 8->16, frame 48->56 == target, zero dead decls, zero extra insns, zero stores to the slot. Floor 14 -> 5. Layer-1 cheat-reviewer: PASS on this construct ("the MORE correct spelling").

- [s1] Phantom-slot trigger matrix (cc1 .frame probes, tmp/grind/func_80038170/s1/): pair off one symbol with shared var index in TWO statements = +8 (vI); single read = 0 (vJ/vL); two-symbol spelling = 0 (vK); named-pointer staging u8*t=&SYM+i*2 = 0 (vM2/vM4); u16 HImode named temp in copy loop = 0. Outer-block pairs (D_8008F1A8 x2) share ONE slot (= the pre-existing natural 8); a pair inside the if() CONDITIONAL allocates a SECOND slot (no reuse across cond scope) -> 16. Copy loops contribute nothing (vD only-copies = vars 0).

- [s1] Residual floor-5 = prologue init/save pair order ONLY: ours s0,s1,s2,s3,ra; target s0,s3,s2,s1,ra; frame + all offsets correct. Order tracks the source order of the three live zero-inits.

- [s1] `s1 = s2 = s3 = 0;` (chained, decl order unchanged) emits s3,s2,s1 (RTL right-to-left) -> floor 1. Layer-1 cheat-reviewer: FAIL (same family as banked decl-order-prologue-flip; "different spelling of the same intent"). Banked: rejected/chained-zeroing-order.c. Do not re-propose order-steering spellings; s1 files a ruling-request instead.

- [s1] The floor-1 residual (with an order lever) is a PROVEN text-only artifact: source emits reloc D_8008F19C+1 where the reference .o (old cheat-form build) spells symbol D_8008F19D. Linked words identical (lui 0x0980 / lbu imm 0xF19D both ways). Word-level diff vs asm/funcs raw words: ONLY the jal func_80079194 reloc word differs pre-link (141/141 insns, count + stream otherwise byte-equal). Once build/ regenerates from this src the sandbox reads 0. Sandbox-0-this-session is unreachable BY CONSTRUCTION for the correct source spelling.

- [s1] Cheat carriers still in tree (driver/retire surface, NOT src): 1 regfix rule (reorder @9-13 — would now MANGLE the already-correct natural prologue if applied) + tools/prologue_config.json func_80038170 entry (hardcoded -0x38 prologue — now redundant: natural cc1 output is identical text). Sandbox filters both (engine/pipeline.py treats prologue_fix as tracked cheat), so floor 5/1 is honest pure-C.

- [s1] Oversized-locals carve-out (2026-07-13 owner ruling, dead-vars-local-array.md) would have covered this function's frame gap (frame-math: 56 - 24 saves - 16 args = 16-byte locals region, 0 bytes written) — NOT NEEDED: the natural live-form closes it. Recorded for family reference only.

- [s1] canonical: verdict C, distance 1..5 range, asm_insns 0 — pure-C target confirmed.
