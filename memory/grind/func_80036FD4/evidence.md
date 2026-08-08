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

## Session 2 (structural, 2026-07-29) — floor 9 -> 2, build_insns 78 -> 79 == target

### Headline
All 79 instructions now match target in opcode AND register. The residual 2 is two
RELOCATION ADDENDS, not two instructions, and links to identical bytes. See
docs/grind/decisions.md (2026-07-29, func_80036FD4) for the integration handoff.

### The causal chain, fully resolved (this replaces s1's H4/H5 guesses)
s1 correctly identified "the reload's schedule position is the whole remaining gap"
but attributed it to sched1 + move_by_pieces. Both details were wrong; the mechanism
is:

1. The aggregate copy was ALREADY a single block-move insn all along — cc1 -da
   combine dump insn 53:
   `(set (mem:BLK (symbol_ref "D_80101E6C")) (mem:BLK (reg 78)))`, pattern
   `177 {movstrsi_internal}`. So s1's H5 ("force emit_block_move instead of
   move_by_pieces and it will hard-block the hoist") is KILLED AT ITS PREMISE: the
   copy is opaque already, and the scheduler sinks it past the reload anyway.
2. The actor is sched2 (the POST-RELOAD scheduler), not sched1. Both passes make the
   same move, but the emitted order comes from sched2 — confirmed by mapping the
   sched2 trace onto the objdump one-for-one.
3. sched2 schedules the block backwards. The reload (`lh`, insn 62) cannot be placed
   at T-14 because its consumer (`sll`, insn 65) is at T-13 and the load latency is
   2. At T-14 the block move (insn 53) is the ONLY ready insn, so it fills the slot:
   `;; ready list at T-14: 53 (1), now 53` / `;; launching 62 before 53 with no
   stalls at T-15`.
4. That ONE decision produces BOTH s1-observed symptoms. It consumes the load-delay
   slot (so no `nop` — 78 insns instead of 79) AND it leaves the reload's pseudo live
   across the block move, so reload assigns the movstrsi scratches $a1/$a2 instead of
   target's $a0/$a1. s1 was right that they are one coupled problem; the direction is
   scheduling -> allocation, not the reverse.

### The lever (CONFIRMED): the store and the reload must share a base symbol
The only way to keep the block move out of that load-delay slot is a real memory
dependence block-move -> reload. sched.c:817 `true_dependence` calls
`memrefs_conflict_p (SIZE_FOR_MODE (mem), ..., SIZE_FOR_MODE (x), ..., 0)`, and
`SIZE_FOR_MODE(BLKmode) == 0`. For two CONSTANT addresses the recursion bottoms out at
`if (CONSTANT_P (y)) return rtx_equal_for_memref_p (x, y) && (xsize == 0 || ysize == 0
|| ...)` — the size-0 clause makes a BLKmode store conflict with anything at the same
base, and `rtx_equal_for_memref_p` makes two DISTINCT symbol_refs never conflict.

Therefore: no record shape, member layout, declared alignment, or padding can create
the dependence while the store and the load name different symbols. The base is the
variable; the shape never was.

Declaring the block at 0x80101E60 as one record (store at base+0xC BLKmode, halfword
at base+0) creates the conflict. Measured: floor 9 -> 4, build_insns 78 -> 79, and the
registers fall into target's $a0/$a1 with no register-directed work whatsoever.

### Second lever (CONFIRMED): INSN_LUID tie-break decides the last 2 instructions
At floor 4 the only diff was an adjacent-pair reorder in the index computation.
Target: `sll v0,a0,16` / `lui+addiu v1,SpecialCam` / `sra v0,v0,13` — the table base is
materialised BETWEEN the sign-extend's sll and its sra. `sched.c rank_for_schedule()`
breaks priority ties on `INSN_LUID (tmp) - INSN_LUID (tmp2)`, i.e. on original RTL
order, so C statement order decides this directly. Measured sweep:

| C form for the entry address                              | emitted order | score |
|---|---|---|
| `entry = (s32*)(&SpecialCam + rec->unk00 * 8);`           | sll, sra, sym | 4 |
| `u8 *cam = &SpecialCam;` as first initialiser, then index | sym, sll, sra | 4 |
| `idx = rec->unk00; cam = &SpecialCam; entry = cam+idx*8;` | sll, sym, sra | **2** |

Mechanism for the winning form: `idx = rec->unk00` expands the sign-extend as
sll16+sra16; `cam = &SpecialCam` is the next insn; `idx * 8` expands as sll3, and
combine fuses sra16+sll3 into `sra 13` AT THE sll3's position — i.e. after the symbol.
So the sll stays first, the symbol lands second, the sra third.

### The residual 2 (measured, and why it is the floor for this mechanism)
`objdump -dr tmp/sandbox/func_80036FD4/code6cac_b2_post.o`:

    628: lui at,0x0    R_MIPS_HI16 D_80101E60
    62c: sw  a0,12(at) R_MIPS_LO16 D_80101E60      target: %lo(D_80101E6C), addend 0
    630: lui at,0x0    R_MIPS_HI16 D_80101E60
    634: sw  a1,16(at) R_MIPS_LO16 D_80101E60      target: %lo(D_80101E70), addend 0

The instruction words are identical; only the symbol+addend pairs differ.
0x80101E60+12 == 0x80101E6C, +16 == 0x80101E70, and GNU ld resolves an o32
R_MIPS_HI16 using the addend of the following R_MIPS_LO16 — so both spellings LINK to
the same words. Splat's per-word symbol naming in the target .s is the artifact
(cf. memory/project/splat-symbol-names-are-not-evidence.md); the sandbox compares
unlinked object words, so it scores 2.

Base-choice arithmetic (why 2 is minimal): base 0x80101E60 puts addends on the two
`sw`s only = 2 words. Base 0x80101E6C would give `sw`#1 addend 0 but moves addends
onto `sw`#2 (+4), the `sh` (-12) and the pre-jal `lh` (-12) = 3 words. Varying-address
(pointer) spellings of the store change the addressing to `lui/addiu/sw/sw` and cost
more. So 2 is the minimum under the only dependence mechanism that exists.

### The 8 regfix rules are now ACTIVELY HARMFUL
`sandbox func_80036FD4` with rules ENABLED scores 4 (vs 2 with `--disable all`). The 8
reorder/subst rules at regfix.txt:79-95 were written against the old codegen and now
corrupt output that is already correct. They must be deleted (`retire`) before any
full build can match — that is the integration blocker and it is outside a grind
session's surface. NOT build-verified this session; the link-identity claim rests on
the relocation arithmetic plus the HI16/LO16 pairing rule, and wants one `retire` to
confirm.

### Record justification (independent of codegen — required for the commit)
- This function already hands the block's interior to another function:
  `base = (u8 *)&D_80101E62 - 0xA` == 0x80101E58, passed to `tslPolyF4Init`.
- code6cac_b2_post.c:277 passes `&D_80101E6C` to `cdrom_BcdToFrames` /
  `cdrom_FramesToBcd` as one 8-byte buffer — the `pair` member is one object.
- Target's own call passes `&SpecialCam + i*8`, so SpecialCam is an array of that
  same 8-byte record.
- Layout recovered from the use sites: +0x00 s16 (E60), +0x02 s16 (E62), +0x04 s16
  (E64), +0x06 unknown s16, +0x08 s16 (E68), +0x0A s16 (E6A), +0x0C 8-byte pair
  (E6C/E70), +0x14 s32 (E74).

### Cleanup still owed (unchanged in kind from s1's H3, now better justified)
`(ReplayCamRec *)&D_80101E60` is a probe spelling (pointer-alias family). The
committable form declares the record once in include/code6cac.h and drops the per-word
externs. Deliberately NOT done here: those symbols are referenced from other files and
unifying them changes their mutual aliasing, so it needs a full-build SHA1 check and is
owner-scope. It does not change this function's score.

### Artifacts (session 2)
- `tmp/grind/func_80036FD4/s2/dis.sh` — side-by-side target-vs-sandbox disassembly
- `tmp/grind/func_80036FD4/s2/cmp.py` — normalised diff (opcode+register only, so
  relocation/label spelling noise is filtered out); reports 12 lines of which 10 are
  normaliser artifacts and 2 are the addend words
- `tmp/grind/func_80036FD4/s2/dump.sh` — regenerates cc1 `-da` dumps for the file and
  slices out this function
- `tmp/grind/func_80036FD4/s2/rtlsum.py` — flattens an RTL dump to one line per insn
- `tmp/grind/func_80036FD4/s2/dump-cur/` — the distinct-symbol (score 9) dumps; the
  `f-base.i.sched2.txt` trace lines quoted above are here
- `tmp/grind/func_80036FD4/s2/dump-cam/` — the symbol-first variant's dumps
- `tmp/grind/func_80036FD4/s2/decision_entry.md` — the decisions.md entry as filed

- [s2] [s2] Floor 9 -> 2 with sandbox --disable all; build_insns 78 -> 79 == target_insns 79. All 79 instructions match asm/funcs/func_80036FD4.s in opcode AND in register operand.

- [s2] [s2] The residual 2 is two relocation addends, not two instructions: objdump -dr shows `sw a0,12(at)` / `sw a1,16(at)` with R_MIPS_LO16 against D_80101E60, versus target's addend-0 relocs against D_80101E6C / D_80101E70. 0x80101E60+12 == 0x80101E6C and +16 == 0x80101E70, and GNU ld resolves an o32 R_MIPS_HI16 using the following R_MIPS_LO16's addend, so both spellings link to identical words.

- [s2] [s2] The actor is sched2 (the POST-RELOAD scheduler), not sched1 as s1 assumed. Mapping the sched2 trace onto the objdump matches one-for-one; the pre-fix decision is in the dump verbatim: ';; ready list at T-14: 53 (1), now 53' then ';; launching 62 before 53 with no stalls at T-15'.

- [s2] [s2] The missing nop and the $a1/$a2-vs-$a0/$a1 register residual are ONE symptom of that single scheduling decision, and the direction is scheduling -> allocation: with the block move sunk past the reload, the reload's pseudo is live across the block move, so reload avoids $a0 for the movstrsi scratches. Fixing the schedule fixed the allocation with no register-directed work.

- [s2] [s2] sched.c:817 true_dependence -> memrefs_conflict_p: SIZE_FOR_MODE(BLKmode) == 0, and for two constant addresses the recursion bottoms out at `rtx_equal_for_memref_p (x, y) && (xsize == 0 || ysize == 0 || ...)`. A BLKmode store therefore conflicts with anything at the SAME base symbol and with NOTHING at a different symbol — at any record size, member layout, or declared alignment.

- [s2] [s2] The aggregate copy was already a single movstrsi block-move insn before this session (cc1 -da combine dump insn 53, pattern 177 {movstrsi_internal}), so s1's H5 premise was false; an opaque block-move insn is not a scheduling barrier.

- [s2] [s2] sched.c rank_for_schedule() breaks equal-priority ties on INSN_LUID (original RTL order), so C statement order directly decides adjacent-pair emission order. Every insn in this region has priority 1-4, so ties dominate.

- [s2] [s2] Measured order sweep for the entry-address computation: `&SpecialCam + rec->unk00 * 8` -> sll,sra,sym -> score 4; `u8 *cam = &SpecialCam;` first -> sym,sll,sra -> score 4; split (`idx = rec->unk00; cam = &SpecialCam; entry = cam + idx*8`) -> sll,sym,sra == target -> score 2.

- [s2] [s2] The 8 regfix rules at regfix.txt:79-95 are now ACTIVELY HARMFUL: `sandbox func_80036FD4` with rules enabled scores 4 versus 2 with --disable all. They were written against the old codegen and corrupt output that is already correct, so no full build can match until they are deleted.

- [s2] [s2] NOT build-verified this session. The link-identity claim rests on the relocation arithmetic plus the documented HI16/LO16 addend-pairing rule. `retire func_80036FD4` (which deletes those 8 rules and full-build SHA1-verifies with auto-rollback) is the single decisive test, and it is outside a grind session's surface.

- [s2] [s2] Record layout recovered from use sites, independent of codegen: +0x00 s16 (D_80101E60), +0x02 s16 (E62), +0x04 s16 (E64), +0x06 unknown s16, +0x08 s16 (E68), +0x0A s16 (E6A), +0x0C 8-byte pair (E6C/E70), +0x14 s32 (E74). Justification: this function passes `(u8 *)&D_80101E62 - 0xA` (== 0x80101E58) to tslPolyF4Init as a struct pointer; code6cac_b2_post.c:277 passes &D_80101E6C to cdrom_BcdToFrames/cdrom_FramesToBcd as one 8-byte buffer; target's own call passes &SpecialCam + i*8, making SpecialCam an array of that same 8-byte record.

- [s2] [s2] The body carries zero cheats: no register pins, no __asm__, no volatile, no alias rename, no dead store, no do-while(0). s1's `asm volatile("" ::: "memory")` barrier is gone and was not replaced. The one construct needing a layer-2 ruling is the `(ReplayCamRec *)&D_80101E60` typed re-view (pointer-alias family), which is a probe spelling the header-declaration cleanup removes.

## OPERATOR — 2026-08-08 integration-handoff execution round (bytes PROVEN, then layer-2 FAIL; reverted)

Executed per the 2026-07-29 escalation's operator steps under owner
delegation (2026-08-07):

1. Candidate spliced into src/code6cac_b2_post.c with post-wave identifier
   mapping (cdrom_BcdToFrames->CdPosToInt, tslPolyF4Init->CdControlB,
   array-decay s0 init) — structure exactly as banked.
2. sandbox --disable all: score 2, 79/79, 8 rules dropped (matches ledger).
3. **retire func_80036FD4: SHA1 == 62efab4f... FULL-BUILD GREEN with all 8
   rules deleted. The link-identity claim is now PROVEN by measurement**
   (previously arithmetic-only). The residual 2 is confirmed addend-class
   (objdump -dr verified by the layer-2 reviewer).
4. Fresh layer-2 cheat-reviewer: **FAIL** on `(ReplayCamRec *)&D_80101E60`
   as spliced — (a) missing the mandatory /* FAKE */ annotation required by
   pointer-alias-fake-exception.md; (b) the ledger's own "CLEANUP STILL
   OWED" gate (header-record clean form) was skipped; (c) the record-merge
   is centrally justified by the sched.c true_dependence mechanism (6-test
   T3); (d) cross-file record evidence judged weaker than claimed (s0-0xA
   = 0x80101E58, not the record base; only the E6C/E70 pair sub-object is
   independently attested).
5. Clean-form feasibility MEASURED and blocked tree-wide: D_80101E70 is
   `extern volatile s32` in code6cac_b2_pre.c but plain s32 in 6 other TUs;
   D_80101E74 is s16 in 5 TUs / s32 in 1 — a shared record cannot satisfy
   both (width cures are themselves gated on the SOTN-evidence pathway).
   TU-scoped form would rewrite ~28 use sites across ~8 completed functions
   in b2_post whose codegen depends on distinct-symbol non-dependence (the
   candidate's own sched2 mechanism cuts both ways).
6. Working tree reverted to HEAD (old body + 8 rules restored); oracle
   green in committed state. NEEDS_USER question filed in decisions.md —
   per review-discipline hard rule 1, not self-resolvable under delegation.

Constraint for future sessions: do NOT re-splice the probe spelling; the
open question is representation policy, not bytes. Bytes are proven.
