# Evidence bank — mario_getMarioVoiceData_8005BE84

## Confirmed baseline (s1 recon, git HEAD 38921159)
- Applied WIP candidate.c to src/text1b.c (replaced HEAD cheat form with $16/$3 pins).
- `canonical` verdict: **C** (pure-C target, distance 4, 46 target insns).
- `sandbox --disable all` score: **4** (46 build insns / 46 target insns; scorable=true;
  rules_dropped=0; cheat_asm_stripped=393 project-wide).
- `diagnose`: all 4 diffs are CONTROL-FLOW (branch/jump rename/retarget/reorder).
- No sibling near-clones ≥0.65 similarity (find_duplicates.py --threshold 0.65 over 1419 fns).

## Diff shape (target 0x8005BE84 vs sandbox build)
Both builds emit 46 insns. Prologue/epilogue byte-identical. Diverge in ONE zone:
scheduling of `arg0 << 1` and `+1` around the bltz.

Target (asm/funcs/mario_getMarioVoiceData_8005BE84.s):
```
lh    v0, 0(s1)
nop
bltz  v0, .L8005BF18
 sll  s0, s0, 1          # DELAY SLOT: arg0<<1 (s0 = arg0<<1, callee-save)
jal   func_80085F98
 nop
jal   func_80085EE4
 addu a0, zero, zero
addu  a0, zero, zero
jal   func_80085E4C
 addu a1, zero, zero
lh    a0, 0(s1)
jal   func_80085EE4
 nop
lh    a0, 0(s1)
jal   md_game_check_change_main_mode_katinuki
 addu s1, v0, zero
addiu a0, s0, 1          # LATE: temp_a0 = (arg0<<1)+1 into FRESH reg a0
sll   a0, a0, 16
sra   a0, a0, 16
jal   func_80085E4C
 addu a1, a0, zero
```

Sandbox build (candidate.c, floor 4):
```
lh    v0, 0(s1)
sll   s0, s0, 1          # HOISTED EARLY (before lh in bltz delay reorg)
lh    v0, 0(s1)
nop
bltz  v0, ...
 addiu s0, s0, 1         # DELAY SLOT: +1 (coalesced back into s0)
jal   func_80085F98
 nop
...
jal   md_game_check_change_main_mode_katinuki
 move s1, v0
sll   a0, s0, 0x10       # sign-extend s0 (which ALREADY has +1)
sra   a0, a0, 0x10
jal   func_80085E4C
 move a1, a0
```

## Root-cause mechanism
Register-allocation *coalescing* + delay-slot filler. My C form
`temp_a0 = (arg0 << 1) + 1;` computes the whole value up front. GCC's RA sees
arg0 (s0) is dead after the shift (only temp_a0 remains) and COALESCES the
addi destination back into s0 (`sll s0,s0,1; addiu s0,s0,1`). The delay-slot
filler (reorg.c fill_slots_from_thread) then picks the LAST insn before bltz —
the addiu — because it's closer / equally safe.

Target keeps s0 = arg0<<1 across the entire call chain (s0 is callee-save),
then reads s0 at `addiu a0, s0, 1` into a FRESH register a0. This requires
the shifted value to be LIVE across the call chain — which forces GCC to keep
it in s0 and NOT reuse s0 for temp_a0.

## Rejected forms (banked; do not re-propose)
- `s16[][2]` indexing without cached base -> recomputes lui/addu per access (score 16-21).
- Shift arg0 in place (variants e/i/j: `arg_save <<= 1; temp_a0 = arg_save + 1` inside if) -> score 15-17 (breaks bltz delay-slot fill AND addu operand order).
- `char* base + arg0*4` cast (variant g) -> bi=47 extra insn, score 16.
- Precompute temp_a0 WITHOUT base-first ptr -> score 8 (wrong addu operand order).

- [s1] sandbox --disable all: score 4, 46 build insns / 46 target insns, scorable=true.

- [s1] canonical: verdict C, distance 4.

- [s1] diagnose: 4/4 diffs are CONTROL-FLOW.

- [s1] Diff localized to bltz delay slot + late +1: target uses `sll s0,s0,1` in delay slot and `addiu a0,s0,1` late; build has `sll s0,s0,1` hoisted early and `addiu s0,s0,1` in delay slot.

- [s1] Target keeps s0=arg0<<1 live across 5 function calls (callee-save); build coalesces temp_a0 into s0 because temp_a0 has no live-across-call use.

- [s1] Prologue/epilogue/mid-body call sequence: byte-identical between build and target.

- [s1] No sibling near-clones at similarity >= 0.65.

- [s1] WIP-banked rejects (banked, do NOT re-propose): s16[][2] no cached base (16-21), shift-in-place variants e/i/j (15-17), char* base + arg0*4 cast (16), precompute temp_a0 without base-first ptr (8).
