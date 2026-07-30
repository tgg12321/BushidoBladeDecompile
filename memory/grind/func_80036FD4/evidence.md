# Evidence bank — func_80036FD4

## Session 1 (recon, 2026-07-29)

### Baseline
- `canonical func_80036FD4` → verdict **C**, 79 target insns, distance 17.
- `sandbox --disable all` at session start: **score 17**, build_insns 76, 8 regfix
  rules dropped, 18 cheat-asm instances stripped.
- The 8 regfix rules (regfix.txt:79-95) are ALL `reorder` / `subst` register
  renames — no `insert`. They encode only the register-allocation residual, not
  the missing instructions.
- The pre-existing source carried `asm volatile("" ::: "memory")` between the
  D_80101E60 store and the following block (a scheduling/CSE barrier cheat). The
  sandbox strips it; that strip is exactly what costs 3 instructions (76 vs 79).

### Structural read of the target (asm/funcs/func_80036FD4.s)
Straight-line body after the `D_80101E62 != 0` early-out:
1. `sll v0,a0,16 / sra v0,v0,13` — index = (s16)arg0 * 8 (arg0 is s32; the
   sll16/sra13 pair is the sign-extend folded with the *8 scale).
2. `lui/addiu v1, SpecialCam` — base kept live all the way to the jal delay slot.
3. `sh a0, %lo(D_80101E60)` — the store of arg0.
4. `lw a0,0(v0) / lw a1,4(v0)` then `lui/sw` to D_80101E6C and `lui/sw` to
   D_80101E70 — load-load-store-store, the aggregate-copy shape.
5. `lui a0 / lh a0, %lo(D_80101E60)` — **a real RELOAD of the just-stored
   global**, then `nop` (load delay), `sll a0,a0,3`, `jal cdrom_BcdToFrames`,
   delay `addu a0,a0,v1`.
6. After the call, a SECOND reload of D_80101E60 for the D_8008EC38 index (that
   one is free — the call flushes cse's memory table).

### The mechanism (CONFIRMED, both from GCC source and by measurement)
`tools/gcc-2.7.2/cse.c:7539 note_mem_written()` decides how much of cse's memory
hash table a store invalidates, and `invalidate_memory()` (cse.c:1700) consumes
it. The table is flushed WHOLESALE (`writes->all = 1`) only when the written MEM:
- is BLKmode, or
- is `(mem (scratch))`, or
- has a varying (register) address that is NOT (`MEM_IN_STRUCT_P` or a PLUS
  address), or
- has a varying address and is QImode.
A CALL_INSN also flushes everything (cse.c:7243). An ordinary `sw` to a distinct
absolute symbol sets only `writes->var`, which removes only entries whose address
varies — so a constant-address entry for D_80101E60 SURVIVES it.

Consequence: with scalar assignments `D_80101E6C = entry[0]; D_80101E70 = entry[1];`
nothing between the `sh` and the reload flushes memory, so cse forwards the
stored arg0 to the read and the load is replaced by a second `sra (arg0<<16),13`.
That fold is worth exactly the 3 missing instructions.

Empirically KILLED as flush mechanisms (measured on this function — do not
re-test):
- Two `sw` stores to different absolute globals between the store and the read
  — the pre-session build had exactly this and still folded.
- `extern volatile s32 D_80101E70;` (already present at code6cac_b2_post.c:45) —
  a volatile MEM store sets `do_not_record` for that store only; it does NOT call
  `invalidate_memory`, so it does not defeat the fold. (It is also a cheat by
  policy; the new form no longer needs it for this function.)

### The lever that worked
D_80101E6C and D_80101E70 are adjacent s32s and are already used as ONE record
elsewhere in the same file: `code6cac_b2_post.c:277` passes `&D_80101E6C` to
`cdrom_BcdToFrames` / `cdrom_FramesToBcd` as a buffer pointer. Writing them with
a single aggregate assignment gives the store a BLKmode destination at expand
time → `note_mem_written` sets `all` → cse's whole memory table is flushed →
both reloads of D_80101E60 appear, exactly as in target. move_by_pieces then
emits the aggregate as two SImode symbol-addressed stores, reproducing target's
`lw/lw/lui+sw/lui+sw` shape (target's `%lo(D_80101E70)` == the aggregate's
`sym+4`).

Measured: **floor 17 → 9**, build_insns 76 → 78 (target 79).

Probe form used (NOT the committable spelling — see hypotheses.md H3):
```c
typedef struct { s32 a; s32 b; } CamPair;
*(CamPair *)&D_80101E6C = *(CamPair *)entry;
```

Also measured NEUTRAL (same score 9, same bytes): computing the entry index from
`D_80101E60 * 8` after the store instead of from `(arg0 << 16) >> 13`. cse
forwards the FIRST read identically either way, so both spellings emit target's
`sll 16 / sra 13`. The `D_80101E60 * 8` spelling is kept because it is the
single-source-of-truth form and matches the sibling at code6cac_b2_post.c:365
(`(s32)&SpecialCam + (s32)D_80101E60 * 8`).

### Remaining residual at floor 9 (one instruction short of 79)
- Target's first reload sits AFTER both `sw` stores and needs a load-delay `nop`
  before `sll a0,a0,3`. That nop is the missing 79th instruction.
- Our build's cc1 sched1 pass hoists the `lh` ABOVE the two entry loads (it has
  the longer critical path to the jal argument), so no nop is required.
- Register residual is coupled to that: with the reload hoisted, the two copied
  words land in `$a1/$a2` and the reload in `$a0`; target has the words in
  `$a0/$a1` with the reload RE-USING `$a0`. In target the anti-dependence on
  `sw a0, %lo(D_80101E6C)` is what pins the reload late. Fix the schedule
  position and the allocation should follow — treat them as ONE problem.

### Siblings / context
- `replay_camera_Init` (code6cac_b2_post.c:242) is the same shape (store
  D_80101E60, read it back, store the pair) and still carries `register asm("$7")`
  / `asm("$8")` pins plus the same memory barrier. It is a likely beneficiary of
  the same aggregate-assignment lever, but it is NOT this function's scope.
- `func_80037110` (code6cac_b2_post.c:365) reads D_80101E60 with no preceding
  store, so it shows nothing about the fold.

### Artifacts
- `tmp/grind/func_80036FD4/s1/dis.sh` — side-by-side target-vs-sandbox
  disassembler for this function (re-runnable: `bash tools/wsl.sh 'sh tmp/grind/func_80036FD4/s1/dis.sh'`)
- `tmp/grind/func_80036FD4/s1/target.txt`, `tmp/grind/func_80036FD4/s1/build.txt`

- [s1] canonical func_80036FD4 -> verdict C, 79 target insns, distance 17.

- [s1] The 8 regfix rules (regfix.txt:79-95) are all reorder/subst register renames - no insert rules - so they encode only the RA residual, never the missing instructions.

- [s1] Session-start sandbox --disable all: score 17, build_insns 76, 18 cheat-asm instances stripped (the asm volatile memory barrier).

- [s1] End-of-session sandbox --disable all: score 9, build_insns 78, and the src no longer contains the memory barrier for this function.

- [s1] GCC 2.7.2 cse flushes its whole memory hash table (writes->all) only for: a BLKmode written MEM, (mem (scratch)), a varying-address write that is not in-struct/PLUS, a varying-address QImode write, or a non-const CALL_INSN (cse.c:7539 note_mem_written, cse.c:7243).

- [s1] Target's pre-jal reload sits after both sw stores and pays a load-delay nop before sll a0,a0,3; that nop is the one instruction our build (78) is still short of 79.

- [s1] Target puts the two copied record words in $a0/$a1 and re-uses $a0 for the reload; the anti-dependence on sw a0,%lo(D_80101E6C) is what pins the reload late. Our build hoists the lh (longer critical path to the jal arg), so the words land in $a1/$a2. The register residual and the missing nop are one coupled problem.

- [s1] The committed-form cleanup is still owed: the working spelling *(CamPair *)&D_80101E6C = *(CamPair *)entry; is a typed re-view of globals (pointer-alias family) and must be replaced by a struct-typed declaration in include/code6cac.h before any completion claim.

- [s1] Sibling replay_camera_Init (code6cac_b2_post.c:242) has the identical store/reload/pair-store shape and still carries register asm("$7")/asm("$8") pins plus the same barrier - a likely beneficiary of the same lever, but out of scope here.
