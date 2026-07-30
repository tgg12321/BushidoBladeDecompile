# Hypothesis ledger — func_80036FD4

## Resolved in session 1

**H0 (CONFIRMED).** The honest 17 floor is two independent things: (a) 3 missing
instructions caused by cse folding the reload of D_80101E60 into the arg0
sign-extend chain, and (b) a register-allocation residual (the 8 regfix rules).
(a) is solved by giving cse a memory-flushing store between the `sh` and the
read. Mechanism, the measurement, and the killed alternatives: see evidence.md.

**H1 (CONFIRMED).** An aggregate (BLKmode-destination) assignment writing
D_80101E6C/D_80101E70 as one 8-byte record flushes cse's memory table
(`note_mem_written` → `writes->all`), restoring both target reloads of
D_80101E60 and target's `lw/lw/sw/sw` shape. Floor 17 → 9, build_insns 76 → 78.

**H2 (KILLED).** "An intervening `sw` to a different absolute global, or an
`extern volatile` on one of those globals, defeats the store-to-load forward."
It does not — cse's `invalidate_memory` only removes varying-address entries
unless `writes->all` is set, and a volatile store merely goes unrecorded. The
pre-session build had both and still folded.

## Live frontier (session 2+)

**H3 — declaration cleanup (owed before any completion claim).** The working
form uses the probe spelling `*(CamPair *)&D_80101E6C = *(CamPair *)entry;`.
That is a typed re-view of globals (pointer-alias family) and should not be
committed as-is. The clean spelling is to declare the record as a struct-typed
global in `include/code6cac.h` (the file already treats `&D_80101E6C` as a
buffer pointer at code6cac_b2_post.c:277) and assign it directly, with the same
treatment for the source side (`SpecialCam` as an array of that record type,
matching `(s32)&SpecialCam + idx*8` at code6cac_b2_post.c:365 and :508/:513).
*Next probe:* introduce the struct declaration, drop both casts, re-sandbox and
confirm the score stays at 9 (bytes must be unchanged); then the pre-existing
`extern volatile s32 D_80101E70;` cheat at code6cac_b2_post.c:45 can also be
dropped for this function.

**H4 — the reload's schedule position is the whole remaining gap.** Target's
`lh` sits after both `sw`s and pays a load-delay `nop` (the missing 79th
instruction); our cc1 sched1 hoists it above the two entry loads because it has
the longer critical path to the jal argument. In target the reload's destination
is `$a0`, which is anti-dependent on `sw a0, %lo(D_80101E6C)` — that
anti-dependence is what pins it late, and it is a *consequence* of the words
landing in `$a0/$a1`, so the register residual and the nop are one coupled
problem, not two.
*Next probe:* dump cc1's sched1 output (`-da`, keep the `.sched` / `.greg` dumps
in `tmp/grind/func_80036FD4/sN/`) and read the ready-list priorities for the
`lh` versus the two `lw`s. Then look for a C shape that lengthens the entry-copy
chain or shortens the reload's chain to the jal — e.g. splitting the
`D_80101E74 = BcdToFrames(...) + ... - 0x96` statement so the call argument is
computed in its own statement AFTER the record copy, or expressing the call
argument via the already-live `entry` pointer relationship instead of a fresh
index computation.

**H5 — force `emit_block_move` instead of `move_by_pieces`.** If the aggregate
copy stayed an opaque block-move insn through sched1 rather than being expanded
into two SImode moves at expand time, it would be a hard scheduling barrier and
the `lh` could not be hoisted above it — which is precisely target's shape. Our
8-byte, 4-byte-aligned struct takes the `move_by_pieces` path.
*Next probe:* measure whether a differently-shaped record (e.g. a declared
alignment or member layout that makes GCC take the `movstrsi` path) changes the
schedule — and, importantly, whether it does so WITHOUT changing the emitted
load/store shape. Reject immediately if the record shape has no independent
justification from the data layout; the record's real shape must come from how
the bytes at 0x80101E6C are used elsewhere, not from what schedules well.

## [s1] The honest 3-instruction shortfall (76 vs 79) is GCC cse forwarding the just-stored arg0 to the reload of the s16 global D_80101E60, replacing the target's lui/lh/nop/sll with a second 'sra (arg0<<16),13'.
- mechanism: cse.c records a store's MEM in the src's equivalence class; ordinary sw stores to distinct absolute symbols only set writes->var, which invalidate_memory (cse.c:1700) uses to remove varying-address entries only, so D_80101E60's constant-address entry survives and the read folds. The pre-session source bought the reload with asm volatile("" ::: "memory"), which the sandbox strips - hence the 17 floor.
- probe: Read asm/funcs/func_80036FD4.s against the cheat-stripped sandbox disassembly (tmp/grind/func_80036FD4/s1/dis.sh); read tools/gcc-2.7.2/cse.c note_mem_written (7539) / invalidate_memory (1700) / the dest-recording guard at 7310-7340.
- result: Target reloads D_80101E60 twice (once before the jal, once after); our build emitted the pre-jal one as a duplicate sra off arg0<<16. Exactly 3 instructions, exactly the shortfall.
- verdict: CONFIRMED

## [s1] Writing D_80101E6C and D_80101E70 as ONE aggregate assignment gives the store a BLKmode destination, which sets note_mem_written's writes->all, flushes cse's entire memory table, and restores the target reload in pure C.
- mechanism: note_mem_written (cse.c:7539) sets *writes_ptr = everything for a BLKmode written MEM; invalidate_memory then removes every in_memory entry including constant-address ones. move_by_pieces then emits the 8-byte aggregate as two SImode symbol-addressed stores, which is target's lw/lw/lui+sw/lui+sw shape (target's %lo(D_80101E70) == the aggregate's sym+4). Independent justification for the record: code6cac_b2_post.c:277 already passes &D_80101E6C to cdrom_BcdToFrames/cdrom_FramesToBcd as a buffer.
- probe: Replaced the two scalar stores + the asm memory barrier with a single 8-byte record assignment; sandbox func_80036FD4 --disable all.
- result: score 17 -> 9, build_insns 76 -> 78 (target 79). Both D_80101E60 reloads now present, load-load-store-store ordering matches target.
- verdict: CONFIRMED

## [s1] An intervening sw to a different absolute global, or an extern volatile qualifier on one of the stored globals, defeats the store-to-load forward.
- mechanism: Claimed cse memory invalidation from a plain or volatile store. In fact invalidate_memory only removes varying-address entries unless writes->all is set, and a volatile MEM store merely sets do_not_record for that store (canon_hash, cse.c:1943) without calling invalidate_memory.
- probe: The pre-session build already had both (two sw to distinct globals between the store and the read, and extern volatile s32 D_80101E70 at code6cac_b2_post.c:45) and was measured at floor 17 with the reload folded away.
- result: Fold survived both. Neither is a flush mechanism; the volatile on D_80101E70 is dead weight for this function and can be dropped.
- verdict: KILLED

## [s1] Computing the entry index from the just-stored global (D_80101E60 * 8) instead of from the parameter ((arg0 << 16) >> 13) changes the emitted first index.
- mechanism: cse forwards the first read to the stored arg0 and materialises sign_extend*8 as sll 16 / sra 13 either way, so both spellings are byte-identical.
- probe: Swapped the spelling with the aggregate store in place; sandbox --disable all.
- result: score unchanged at 9, identical bytes. Kept the D_80101E60 * 8 spelling as the single-source-of-truth form (matches the sibling at code6cac_b2_post.c:365).
- verdict: KILLED
