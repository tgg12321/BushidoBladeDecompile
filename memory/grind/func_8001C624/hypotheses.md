# Hypothesis ledger — func_8001C624

## s2 (structural) — 2026-07-13

All scores are `sandbox func_8001C624 --disable all` (target 127 insns).
"normalised diff" = instruction streams of the sandbox `.o` and the pristine
`build/src/code6cac.o` compared through the SAME objdump, with %hi/%lo relocs
resolved to ABSOLUTE addresses (tool: `tmp/grind/func_8001C624/s2/difffunc.py`).
That normalisation is what let this session separate real reordering from
reloc-spelling noise — the raw sandbox score conflates them.

| # | Hypothesis | Probe | Score / insns | Verdict |
|---|---|---|---|---|
| H1 | The nested `do{}while(0)` wrappers are load-bearing sched1 fences, not RA weighting | read `tools/gcc-2.7.2/sched.c`; RTL dump of the matching form | 0 labels, 0 barriers, 2×LOOP_BEG/END notes | **CONFIRMED** |
| H2 | Wrapper-free plain-symbol C is not merely "worse codegen" — it is SEMANTICALLY WRONG | normalised diff of v1 | 61 / 127 | **CONFIRMED** |
| H3 | Whole-region struct typing in the header (Judge avenue #2) closes it | v10 `extern s32 D_80101F80[105]` | 94 / **99** | **KILLED** |
| H4 | Per-vector struct/array typing closes it | v4 (Vec4/Vec3 structs), v7 (`s32[3]`) | 67 / **125** | **KILLED** |
| H5 | Anchoring only the fed words (cast form) closes it | v8 (FB0 grp), v9a/v9b (all 3 grps) | 61 / 71 / 75, 127 | **KILLED** |
| H6 | A multiply-assigned destination pointer makes copy addresses opaque to `canon_rtx` → total fence | v14 | 61 / 127 (output *identical* to v1) | **KILLED** |
| H7 | Re-basing every region global on one symbol via casts reaches the target | v13 | **normalised diff EMPTY, 127/127** | **CONFIRMED — and SELF-REJECTED as a cheat** |
| H8 | The sandbox distance can score a base+offset respelling | v13 links byte-identically yet scores 33 | 33 | **KILLED** (metric limitation) |

### H1 — the wrappers are total scheduling barriers (this KILLS the pending ruling)
`sched.c:2066-2088`, verbatim comment: *"If there is a LOOP_{BEG,END} note in
the middle of a basic block, then we must be sure that no instructions are
scheduled across it."* The code then, for the insn following the note, adds an
ANTI dep on **every** `reg_last_uses[i]`, a dep on **every** `reg_last_sets[i]`,
sets `reg_pending_sets_all = 1`, and calls **`flush_pending_lists()`** — a total
register + memory barrier.

The RTL dump of the committed (matching) form shows **0 code_labels and 0
barriers** — the function is ONE basic block — with exactly 2 × LOOP_BEG /
LOOP_CONT / LOOP_END notes (the two wrappers). So the wrappers do nothing except
plant two `flush_pending_lists` scheduling barriers inside a straight-line block.

**Consequence for the open ruling.** The s1 ledger and the Judge left open
"the nested wrappers can be re-considered under `do-while-zero-exception` on
their merits." They cannot. That rule's sanctioned mechanism is
`NOTE_INSN_LOOP_BEG → LABEL_OUTSIDE_LOOP_P → reorg.c relax_delay_slots`, and s1
already established this function is straight-line so that mechanism is absent.
What is actually operating here is `sched.c`'s explicit all-register+all-memory
fence — i.e. precisely the **"scheduling barrier"** family that CLAUDE.md lists
as a cheat alongside register pins. The wrappers are not a borderline do-while(0);
they are a scheduling barrier wearing a do-while(0) costume. Do not re-submit
them for sanction.

### H2 — our source is not just unlucky, it is wrong
In the wrapper-free build, `*(Blk12*)&D_801020C0 = *(Blk12*)&D_80101FB0;` is
hoisted to sit immediately after `sw zero, %lo(D_80101FB0)` — i.e. ABOVE the
`D_80101FB4 = -0x384;` and `D_80101FB8 = 0;` stores that feed it. The emitted
code copies STALE FB4/FB8. The committed (byte-matching) build copies
`(0, -0x384, 0)`. So the wrapper is not a codegen hint — with our spelling it is
load-bearing for the program's MEANING. A correct C source cannot depend on it:
GCC must be able to SEE the dependence. It currently cannot, because —

### The root mechanism (traced end-to-end in sched.c)
The three copies are `(mem:BLK (reg))` in RTL (one `movstrsi` insn each; the
lw/sw pairs appear only at final). `true_dependence` → `memrefs_conflict_p
(SIZE_FOR_MODE(mem), ...)`, and **`GET_MODE_SIZE(BLKmode) == 0`**. Every branch
of `memrefs_conflict_p` short-circuits on `xsize == 0 || ysize == 0` **once the
canonicalised bases are `rtx_equal`**. Therefore:

* a block copy conflicts with **every** access to the **same base symbol at any
  offset** (size-0 ⇒ unknown extent ⇒ assume overlap), and
* a block copy conflicts with **nothing** at a **different symbol** — the
  `CONSTANT_P` branch requires `rtx_equal_for_memref_p(x, y)`.

`D_80101FB0` and `D_80101FB4` are different `symbol_ref`s. Hence zero dependence.
(`MEM_IN_STRUCT_P` is a red herring here: the RTL dump shows the copies are plain
`(mem:BLK ...)` with no `/s`, and the scalar globals are plain `(mem:SI ...)`, so
both `!MEM_IN_STRUCT_P` clauses of `true_dependence` are inert.)

### The contradiction that defines the frontier
The target's bytes pin BOTH of these simultaneously:
* **(a)** per-store at-form addressing (`lui $at ; sw ..,%lo(D_80101FDC)($at)`)
  and per-copy `lui/addiu` bases ⇒ the codegen of **separate scalar symbols**.
  An aggregate declaration provably cannot produce it (H3/H4: GCC forces the
  object's base into a register and CSE commons it — 99 insns).
* **(b)** the copies do not float ⇒ under the mechanism above, a **shared base
  symbol** is required.

(a) and (b) are contradictory under GCC 2.7.2's alias rules. H7 shows the only
way to satisfy both is a cast re-view that lies about (a) — a cheat. Therefore
**one of my premises about the original source must be wrong**, and the only
remaining free variable is the one never swept under the plain-symbol spelling:
**statement order**. sched.c's `rank_for_schedule` breaks priority ties by
`INSN_LUID` — i.e. by SOURCE ORDER — so source order is a first-class lever, and
the s1-era note "order is INERT (11 of 12 identical at 75)" was measured under
the ANCHORED spelling only and does not transfer.

## Frontier (next session)
1. **Source-order sweep under the PLAIN spelling** (baseline v1 = 61). Never
   done. `rank_for_schedule` ties → LUID → source order. Sweep permutations of
   the store groups / copy placement / load placement against the normalised
   diff, not the raw score.
2. **`pending_lists_length > 32` flush placement** (`sched.c:1754`). The flush is
   a total memory barrier and lands at a position determined purely by the COUNT
   of memory refs before it. Counting the current form puts it inside copy3.
   Adding/removing/reordering memory ops upstream MOVES that barrier. This is an
   honest, source-level lever that has never been probed.
3. **Are the copies really `movstrsi`?** Check whether an element-wise copy
   (`d.x=s.x; d.y=s.y; ...`) can be made to emit the target's `lui/addiu`+pairs
   shape. If it can, the copies stop being BLKmode and the whole size-0 alias
   pathology disappears — the dependences become exact and honest.

## [s2] The nested do-while(0) wrappers are load-bearing sched1 fences, not loop-note RA weighting (s1's guess), and their mechanism decides whether do-while-zero-exception can sanction them.
- mechanism: sched.c:2066-2088 -- 'If there is a LOOP_{BEG,END} note in the middle of a basic block, then we must be sure that no instructions are scheduled across it': for the insn after a loop note GCC adds an ANTI dep on EVERY reg_last_uses[i], a dep on EVERY reg_last_sets[i], sets reg_pending_sets_all=1, and calls flush_pending_lists() -- an all-register + all-memory barrier.
- probe: cc1 -dj -dS RTL dump of the committed matching form (tmp/grind/func_8001C624/s2/rtl/head/) + read tools/gcc-2.7.2/sched.c.
- result: RTL shows 0 code_labels and 0 barriers (the function is ONE basic block) with exactly 2x LOOP_BEG/LOOP_CONT/LOOP_END notes = the two wrappers. They contribute nothing but two total sched1 barriers in straight-line code.
- verdict: CONFIRMED

## [s2] The wrappers can be re-considered under do-while-zero-exception on their merits (the Judge's standing offer).
- mechanism: do-while-zero-exception's sanctioned mechanism is NOTE_INSN_LOOP_BEG -> LABEL_OUTSIDE_LOOP_P -> reorg.c relax_delay_slots. s1 already established this function is straight-line, so that mechanism is absent.
- probe: Identify which GCC pass actually consumes the loop notes here (above).
- result: The operating mechanism is sched.c's explicit total barrier -- i.e. the 'scheduling barrier' family CLAUDE.md lists as a cheat alongside register pins. Not the sanctioned family. The wrappers must NOT be re-submitted for sanction.
- verdict: KILLED

## [s2] Removing the wrappers only degrades the schedule (it is a codegen-quality problem).
- mechanism: n/a -- checked the emitted code, not the score.
- probe: Reloc-normalised instruction diff of the wrapper-free plain-symbol form vs the pristine build/src/code6cac.o.
- result: FALSE, and this is the key finding: GCC hoists `*(Blk12*)&D_801020C0 = *(Blk12*)&D_80101FB0;` to sit immediately after `sw zero,%lo(D_80101FB0)` -- ABOVE the D_80101FB4/FB8 stores that FEED it -- so the wrapper-free code copies STALE data while the byte-matching build copies (0,-0x384,0). Our C's MEANING currently depends on the barrier. A correct source must let GCC SEE the dependence.
- verdict: KILLED

## [s2] ROOT MECHANISM: GCC cannot see the dependence because the block-copies are BLKmode and the region words are separate symbols.
- mechanism: The 3 copies are `(mem:BLK (reg))` (one movstrsi insn each; lw/sw pairs appear only at final). true_dependence -> memrefs_conflict_p(SIZE_FOR_MODE(mem),...) and GET_MODE_SIZE(BLKmode)==0. Every branch short-circuits on `xsize==0 || ysize==0` ONCE the canonicalised bases are rtx_equal. So a block copy conflicts with EVERY access to the SAME base symbol at ANY offset, and with NOTHING at a different symbol. D_80101FB0 vs D_80101FB4 are different symbol_refs => zero dependence. (MEM_IN_STRUCT_P is a red herring: RTL shows the copies as plain (mem:BLK) with no /s, so both !MEM_IN_STRUCT_P clauses of true_dependence are inert.)
- probe: Read sched.c memrefs_conflict_p / true_dependence / canon_rtx; confirm MEM flags in the RTL dump.
- result: Confirmed and predictive -- it explains every variant's score this session.
- verdict: CONFIRMED

## [s2] JUDGE AVENUE #2 -- whole-region struct typing in the header closes the function.
- mechanism: One object => shared base symbol => the BLKmode size-0 rule makes every copy conflict with every region store => sched1 loses all freedom => source order => target.
- probe: v10 `extern s32 D_80101F80[105]` + index accesses; also v4 (per-vector Vec4/Vec3 structs), v7 (`extern s32 D_80101FB0[3]`), v2 (`s32 *rg` pointer view).
- result: DEAD. The aliasing is fixed but the ADDRESSING FORM breaks: GCC 2.7.2 expands a COMPONENT_REF/ARRAY_REF of an aggregate VAR_DECL by forcing the object base into a REGISTER (expand_assignment->store_field) and CSE commons it across all members. v10 = 99 insns (target 127): 28 `lui`s vanish and the copies become `lw v0,-48(v1)` / `sw v0,272(v1)` off a shared base. v4/v7 = 125 insns (`sw a0,0(v1)` reg-base at the offset-0 field + block-copy shape change). The TARGET emits at-form per-symbol stores and per-copy lui/addiu bases -- the codegen of SEPARATE SCALARS -- so the original did NOT declare this region as an aggregate.
- verdict: KILLED

## [s2] Partial cast-anchoring of only the words that feed each copy closes it.
- mechanism: `((s32*)&D_80101FB0)[1] = -0x384;` keeps a constant address (at-form survives) while sharing the copy's base symbol, so the BLKmode size-0 rule creates the dependence.
- probe: v8 (FB0 group only), v9a (all three groups), v9b (all three groups + the final read also cast).
- result: DEAD. v8 = 61: it DOES fix copy2's hoist (FB0/FB4/FB8 become contiguous) but copy1 and copy3 then float freely past the untyped stores. v9a = 71, v9b = 75 -- partial anchoring is strictly WORSE than none. v9b also reproduces and EXPLAINS the s1-era 75: `((s32*)&D_80101F80)[1]` used twice makes CSE materialise a shared address pseudo.
- verdict: KILLED

## [s2] A multiply-assigned destination pointer makes the copy addresses opaque to canon_rtx, giving an honest total fence with every global keeping its own symbol (and therefore scoreable at sandbox 0).
- mechanism: canon_rtx resolves a pseudo to a symbol only via reg_known_value, which init_alias_analysis populates ONLY for single-set pseudos. reg_n_sets>1 => unresolvable => memrefs_conflict_p falls through to its default `return 1` => conflicts with everything.
- probe: v14 -- `void *dst;` reassigned before each of the three copies.
- result: DEAD. GCC const-propagates the pointer away before sched1; the RTL shows fresh SINGLE-set pseudos per copy (88/89, 92/93, 96/97). Emitted code is byte-identical to the plain form; score 61, unchanged. Any spelling cse/copy-prop could NOT fold would be a pure opacity construct, i.e. a cheat.
- verdict: KILLED

## [s2] Re-basing every region global on one symbol via casts (`((s32*)&D_80101F80)[k]`) reaches the target -- and would be a legitimate closing form.
- mechanism: The cast (unlike an aggregate DECLARATION) keeps each address a compile-time constant, so the at-form survives; the shared base symbol makes the BLKmode size-0 rule fence every copy against every store. Best of both.
- probe: v13 -- all ~28 region globals re-spelled; copy2's source kept as &D_80101FB0 to dodge the CSE that degrades the FB0 store.
- result: It REACHES THE TARGET EXACTLY: 127/127 insns, EMPTY reloc-normalised diff vs the pristine object, no wrappers, no dead temps, no pins. BUT SELF-REJECTED AS A CHEAT per no-new-park-categories: no semantic purpose (the words have their own names), invisible in the emitted output, justification unstateable without naming memrefs_conflict_p, pointer-pun/typed-re-view family -- and affirmatively FALSE as a reconstruction, since an aggregate declaration provably emits a shared base register, so the target's at-form codegen proves the original did NOT hold this region as one object. NOT surfaced for sanction; recorded as a killed family.
- verdict: KILLED

## [s2] The engine sandbox can score a base+offset reloc respelling.
- mechanism: n/a -- metric property.
- probe: v13 links byte-identically (`((s32*)&D_80101F80)[23]` relocates as D_80101F80+92; the target as D_80101FDC+0; the linker resolves both to 0x80101FDC) yet the .o disassembly text differs (`sw zero,92(at)` vs `sw zero,0(at)`).
- result: NO. v13 scores 33 while being byte-exact. Any reloc-respelled form is structurally unable to reach sandbox 0 even when byte-perfect; only a full build + SHA1 can adjudicate that class. Relevant to the driver's `candidate-ready == sandbox 0` contract.
- verdict: KILLED

## s3 (structural) — 2026-07-13

All scores are `sandbox func_8001C624 --disable all` (target 127 insns), measured
through the s2 reloc-normalised diff. s3 swept the two levers the s2 frontier
named as never-probed, and re-opened the one avenue s2 killed on the wrong grounds.

| # | Hypothesis | Probe | Score / insns | Verdict |
|---|---|---|---|---|
| H9  | Statement order under the PLAIN spelling can pin the copies (s2's headline frontier) | 14 source orders (b0, o1-o3, y2, y3, z1-z8) | 58-67 / 127 | **KILLED** |
| H10 | The `pending_lists_length > 32` flush can fence the copies (s2 frontier #2) | y1: F/E/SV hoisted to stack ~34 mem refs ahead of the copies | 89 / 125 | **KILLED** (mechanism CONFIRMED, lever unusable) |
| H11 | The copies are BLKmode `movstrsi`, and aggregate typing gives them a shared base symbol -> honest dependence | w1: Blk16/Blk12 typing of the 3 copied regions | 73 / **125** | **CONFIRMED — semantically correct, 2 deltas short** |
| H12 | Under aggregate typing, statement order is still a lever | x2-x6: 5 orders over w1 | 73/73/73/74/73 | **KILLED** (DAG saturated => order inert) |
| H13 | s2's "aggregate decl forces the object base into a register" (H3/H4) is the true obstacle | RTL dump of w1 (`-dr`/`-ds`) | offsets +4/+8 ARE at-form | **KILLED — s2's characterisation was wrong** |

### H9 — statement order is a TIE-BREAKER; it cannot create a dependence
`rank_for_schedule` sorts by INSN_PRIORITY, then by class-vs-`last_scheduled_insn`,
then by INSN_LUID (source order). The LUID tie-break is real — but under the plain
spelling each block copy has **exactly one** dependence: the store to its own base
symbol. The +4/+8 stores are different `symbol_ref`s and `memrefs_conflict_p`'s
`CONSTANT_P` branch requires `rtx_equal_for_memref_p`, so GCC sees no dependence
on them at all.

The decisive probe is o1: moving copy2 to the very END of the source still emitted
it **immediately after `sw zero,%lo(D_80101FB0)`** — and dragged that store to the
TOP of the block with it. The (store, copy) pair is glued and floats as a unit
wherever the source puts either half. A tie-break cannot pin an insn the scheduler
believes is unconstrained. The s2 frontier's premise ("the one variable never
swept") was correct that it was unswept and wrong that it could matter.

### H10 — the flush is real, fires around memory-op 33, and is structurally too late
`flush_pending_lists` (sched.c:1754) IS a genuine, coercion-free total memory
fence, and y1 proves it: with F/E/SV hoisted ahead of the copies, all three copies'
base registers are materialised AFTER every store that feeds them. But the fence
only exists *because* 33+ memory ops were pushed ahead of them — which is exactly
the store order the target does NOT have. In the target's own byte order only **12**
memory ops precede copy1. There is no source order that both puts >32 memory refs
before the copies and emits the target's store sequence. Score 89 (worse than 61):
copies fixed, everything else destroyed. Dead.

(Kept: the flush DOES fire in the normal form, in the D_80101FDC..FF4 group. That
is why the setVector stack stores land after those six zero stores in BOTH the
target and every build we make — that part of the schedule is already correct, and
it is the flush doing it.)

### H11/H13 — s2 killed aggregate typing on the wrong grounds
s2 recorded (H3/H4): *"GCC 2.7.2 expands a COMPONENT_REF of an aggregate VAR_DECL
by forcing the object base into a REGISTER (expand_assignment -> store_field), then
CSE commons it across all members."* **The RTL dump falsifies the general claim.**
Expand does put every member store's address in a pseudo, but **combine folds it
back into the MEM whenever the pseudo is single-use** — and it does, for offsets
+4 and +8, which come out at-form in the emitted asm (`sw v0,%lo(D_80101FB4)($at)`).

Only the **offset-0** member degrades, for a precise reason:

```
(insn 43 (set (reg:SI 84) (symbol_ref:SI ("D_80101FB0"))))     ; bare symbol
(insn 45 (set (mem/s:SI (reg:SI 84)) (const_int 0)))           ; D_80101FB0.a = 0
```

The offset-0 pseudo is the **bare `(symbol_ref FB0)`** — rtx-identical to what the
block copy's `copy_addr_to_reg` needs in a register. CSE commons the two, the
pseudo becomes MULTI-USE, and combine can no longer fold it. The store is stuck at
`sw zero,0(v1)` and loses its `lui`. Offsets +4/+8 are `(const (plus (symbol_ref
FB0) N))`, stay single-use, and fold. Insensitive to statement order (H12) — it is
an expand+CSE fact, not a scheduling one.

So the aggregate form is NOT "99 insns, degenerate". It is **125/127, semantically
correct** (every copy reads FRESH data — verified in the emitted stream), and short
by exactly two mechanical register-allocation artifacts:

1. **offset-0 loses its at-form `lui`** — -1 insn x 3 groups (above).
2. **block-copy chunking `num_regs=3` instead of 2** — net +1 insn.
   `mips.md`'s `movstrsi_internal` always requests 4 scratch regs; `mips.c`'s
   `output_block_move` (line 2445) counts how many do NOT collide with the two
   address registers (`safe_regs`) and recurses with that count. The target got 2
   (`lw,lw,sw,sw` pairs); w1 gets 3. Pure RA fallout — not spellable in C.

The arithmetic closes exactly: **125 + 3 - 1 - 1 + 1 = 127.**

### The contradiction, restated more sharply than s2 could
s2: *"(a) at-form per-symbol stores and (b) copies that do not float are
contradictory in pure C."* s3 can now name the exact theorem:

> For a copy to fence against a store, `canon_rtx` must resolve both to the SAME
> base `symbol_ref`. For the store to keep at-form addressing, its address must
> stay foldable by combine (single-use pseudo, or no pseudo). Under **aggregate**
> spelling the offset-0 member's address pseudo is necessarily rtx-identical to the
> copy's base, so it is always commoned and never folds. Under **scalar** spelling
> the stores keep at-form but only the offset-0 word shares the copy's base symbol,
> so only it fences. **The cast spelling (`((s32*)&SYM)[k]`) is the unique form with
> both properties** — it yields a MEM at a compile-time-constant address (never a
> pseudo) that also shares the base symbol. That is exactly why v13 reached the
> target byte-for-byte, and exactly why it is a cheat.

This is not a search failure; it is a proof that **one of our premises about the
symbol layout is wrong**. The remaining honest degrees of freedom are: (i) the
region's true object boundaries (does the copied object begin at a symbol we have
not named?), and (ii) whether the target's `sw zero,%lo(D_80101FB0)($at)` is really
an at-form store of a *scalar* or an at-form store of `%lo(bigobj+0x30)` that splat
merely NAMED as a symbol.

## Frontier (s4)
1. **Resolve the symbol-layout premise — this is now the load-bearing question.**
   `%lo(D_80101FB0)` and `%lo(bigobj+0x30)` are the same reloc after linking (s2's
   H8). Read the .data/.bss segment around 0x80101F80-0x80102120 and the OTHER
   functions that touch this region to determine the real object boundaries. If the
   region is one named object in the original, the honest aggregate declaration puts
   every store at a NON-zero offset — and delta (1) evaporates, because the offset-0
   collision that defeats combine only happens when a stored word sits at offset 0
   of the very object being block-copied.
2. **Attack delta (1) directly: keep the offset-0 store's address pseudo
   single-use.** It is multi-use only because the block copy needs the SAME bare
   symbol in a register. Any honest spelling in which the copied object's base is
   NOT itself a stored lvalue in this function breaks the collision. Check whether
   a wider enclosing struct (so that FB0/F80/FCC are all at non-zero offsets) does
   it — that is the direct test of avenue 1.
3. **Delta (2) (`num_regs` 3 vs 2) is downstream of live-range pressure** and should
   NOT be attacked directly. It will fall out if and when delta (1) is fixed and the
   schedule matches. Do not spend a session on it in isolation.

## [s3] Statement order under the PLAIN (separate-scalar-symbol) spelling can pin the three BLKmode block copies below the stores that feed them. This was the s2 ledger's headline 'un-swept lever'.
- mechanism: sched.c's rank_for_schedule sorts ready insns by INSN_PRIORITY, then by class-vs-last_scheduled_insn, then by INSN_LUID = SOURCE ORDER. With almost no memory dependences nearly every store is priority-tied, so source order should directly select the emitted order.
- probe: Swept 14 statement orders over the wrapper-free plain baseline (b0, o1-o3, y2, y3, z1-z8), scoring each with the s2 reloc-normalised diff (tmp/grind/func_8001C624/s2/difffunc.py), not the raw sandbox number.
- result: Range 58-67 vs baseline 61 (target 127). The copies NEVER land correctly in any order. The decisive probe is o1 (score 58): moving copy2 to the very END of the source still emitted it IMMEDIATELY after `sw zero,%lo(D_80101FB0)` — and dragged that store to the TOP of the block with it. Under the plain spelling each copy has EXACTLY ONE dependence (the store to its own base symbol; the +4/+8 stores are different symbol_refs and memrefs_conflict_p's CONSTANT_P branch requires rtx_equal_for_memref_p). Order is only a TIE-BREAKER, and a tie-breaker cannot CREATE a dependence. The (store, copy) pair is glued and floats as a unit wherever the source puts either half.
- verdict: KILLED

## [s3] The sched.c:1754 pending-list flush (`if (pending_lists_length > 32) flush_pending_lists(insn)`) is an honest, source-level scheduling barrier whose POSITION is set purely by the COUNT of memory refs preceding it, and it can be moved to fence the three copies.
- mechanism: flush_pending_lists makes the flushing insn depend on every pending read and write and sets last_pending_memory_flush, which every subsequent memory op then depends on — a total memory fence GCC creates by itself, with no coercion construct.
- probe: y1: hoisted the F, E and SV groups above the three copies (source order S0,L,A,B,C,F,E,SV,D,P1,P2,P3), stacking ~34 memory refs ahead of copy1 so the flush provably fires before it.
- result: The mechanism is REAL and it WORKS — in the emitted stream all three copies' base registers are now materialised AFTER every store that feeds them (e.g. `addiu a2,a2,%lo(D_80101FB0)` lands after all of FB0/FB4/FB8). But the fence exists ONLY BECAUSE 33+ memory ops were pushed ahead of the copies, which is precisely the store order the target does NOT have: in the target's own byte order only 12 memory ops precede copy1. There is no source order that both puts >32 memory refs before the copies AND emits the target's store sequence. Score 89 — WORSE than the 61 baseline: copies fixed, everything else destroyed. Confirmed as a mechanism, dead as a lever. (Corollary kept: the flush DOES fire in the normal form, in the D_80101FDC..FF4 group — which is why the setVector stack stores land after those six zero stores in BOTH the target and every build we make. That part of the schedule is already right.)
- verdict: KILLED

## [s3] s2's H3/H4 verdict — 'a whole-region/per-vector aggregate declaration forces GCC 2.7.2 to materialise the object base in a REGISTER (expand_assignment->store_field) and CSE commons it across all members, so the target's at-form per-symbol stores prove the original did NOT declare this region as an aggregate' — is correct.
- mechanism: If true, aggregate typing can never reproduce the target's at-form stores, and the (a)/(b) contradiction s2 identified is unresolvable in pure C outside the cast (cheat) spelling.
- probe: Re-measured aggregate typing with the known-bad `i`-literal construct removed (w1: Blk16/Blk12 typing of the three copied regions, `i` kept as a variable), then dumped cc1's RTL (-dr expand / -ds CSE) via tmp/grind/func_8001C624/s3/rtl.py.
- result: REFUTED. Expand DOES put every member store's address in a pseudo, but COMBINE FOLDS IT BACK into the MEM whenever the pseudo is SINGLE-USE — and it does: offsets +4 and +8 come out at-form in the emitted asm (`sw v0,%lo(D_80101FB4)($at)`). ONLY the offset-0 member degrades, because its pseudo is the BARE (symbol_ref FB0) — `(insn 43 (set (reg:SI 84) (symbol_ref:SI ("D_80101FB0"))))` / `(insn 45 (set (mem/s:SI (reg:SI 84)) (const_int 0)))` — rtx-identical to what the block copy's copy_addr_to_reg needs in a register, so CSE commons the two, the pseudo becomes MULTI-USE, and combine can no longer fold it. Aggregate typing is therefore NOT degenerate: it is 125/127 insns (sandbox 73) and SEMANTICALLY CORRECT — verified in the emitted stream, every block copy reads FRESH data, the first such wrapper-free cheat-free form measured. It is short by exactly two named register-allocation artifacts and the arithmetic closes exactly: 125 + 3 (restore the three at-form luis) - 1 - 1 + 1 (block-copy chunking num_regs 3->2) = 127.
- verdict: CONFIRMED

## [s3] Under aggregate typing, statement order remains a lever (so the s2 order hypothesis could still pay off on the corrected spelling).
- mechanism: Same rank_for_schedule LUID tie-break as H9.
- probe: x2-x6: five statement orders over the aggregate form w1 (loads first, copies late, C+P1 hoisted, P2 last, grouped-local).
- result: INERT — 73/73/73/74/73, all 125 insns. Once the shared base symbol creates the real dependences, the DAG is saturated and the LUID tie-break has nothing left to decide. This also independently confirms that the offset-0 at-form loss is an expand+CSE fact, not a scheduling one.
- verdict: KILLED

## s4 (permuter) — 2026-07-13

Mandated modality: directed permuter. The campaign ran and was harvested+stopped
in-session (27,651 iterations, 1,517 finds). It converged entirely on the BANNED
construct — and that negative result, plus five isolated compiler probes, closes
the honest search space. **s3's entire frontier (all three items) is FALSE.**

| # | Hypothesis | Probe | Result | Verdict |
|---|---|---|---|---|
| H14 | (s3 frontier #1) An enclosing object puts the stores at non-zero offsets, so the offset-0 collision never arises and the 3 at-form `lui`s return | probe_offset0.c: `f_aggN`, copied region at BIG+184 | degrades IDENTICALLY to offset 0 | **KILLED** |
| H15 | (s3 frontier #2) Some honest aggregate spelling keeps the offset-0 store's address single-use | probe_{array,ptrview,baseptr,arrcast}.c — 7 spellings | all 7 degrade identically | **KILLED** |
| H16 | (s3 frontier #3) num_regs 3→2 is downstream RA fallout that falls out once #1 lands | same probes | it is the SAME shared base register | **KILLED** |
| H17 | Our GCC 2.7.2 port diverges from PsyQ's cc1psx here (which would explain everything) | cc1psx_wrapper.sh on every probe | **byte-identical output** | **KILLED** |
| H18 | `volatile` (true_dependence's other path) fences the copies | probe_volatile.c: `n_vol`, `n_vol_both` | half-volatile: no fence. both-volatile: `jal memcpy` | **KILLED** |
| H19 | Directed permuter over the plain chassis finds an honest closer | 27,651 iters, 1,517 finds, triaged | every top find is a barrier; best honest 3035 vs 875 | **KILLED** |

### H14–H16 — the aggregate family is structurally dead (the headline)

The target simultaneously contains, for **each** of the three copied regions:

```
lui $at,%hi(SYM) ; sw $zero,%lo(SYM)($at)      (a) at-form offset-0 store
lui $a1,%hi(SYM) ; addiu $a1,$a1,%lo(SYM)      (b) INDEPENDENT copy base register
lw,lw,sw,sw / lw,nop,sw                        (c) block copy, num_regs=2
```

GCC did **not** common (a)'s address with (b)'s register. Every spelling that gives
the copy a shared base **symbol** (the dependence that pins it) forces GCC to
materialise that base in a **register** at the offset-0 access; CSE then commons it
with the copy's base, so (a) degrades to `sw $0,0($3)` (−1 `lui` × 3 groups) **and**
(c) degrades to num_regs=3 (3×lw + 3×sw). s3 recorded these as two independent
deltas. **They are one phenomenon — the shared base register — and neither is
fixable alone.**

Seven spellings measured, one outcome (`la $3,BASE ; sw $0,0($3)` + num_regs=3):
struct VAR_DECL at offset 0; struct at a **non-zero** offset in a wider enclosing
object (BIG+184); array VAR_DECL `ARR[k]`; array via cast `((int*)ARR)[k]`; typed
struct view through a constant pointer `((Work*)&SYM)->a`; const local pointer;
base pointer into a real array `int *p = ARR; p[k] = …`.

**Why frontier #1 was wrong.** The collision is between the copy's base address and
the address of the store to the **first copied word** — and those are the same rtx at
*any* enclosing offset, because the copy's base **is** the address of the first
copied word. All three copied regions have their first word stored in this function
(D_80101F80, D_80101FB0, D_80101FCC — visible in the target asm). No enclosing object
can remove it.

### H17 — the compiler is not the escape hatch
The original PsyQ `cc1psx` (GCC 2.7.2.SN.1 — the compiler that actually built the
game) produces **byte-identical** output to our decompals port on every probe. Zero
divergence. So the aggregate degradation is real in the original compiler too, and
the target's at-form offset-0 stores prove the original BB2 source did **not** declare
this region as an aggregate.

### H19 — permuter converges on the banned construct, and that is the finding
Base 5615 → best 875, but **every one of the top ~14 finds is a `do{}while(0)`
barrier**; the best CLEAN find (wrapper-free, no volatile/asm/pin, both calls left in
their program-mandated positions) is **3035** — a 3.5× gap. Permuter's improvements
come from exactly two moves, and they are the same move:

1. `do{…}while(0)` — in this straight-line function, purely a `sched.c:2066` total
   barrier (`flush_pending_lists`). s2 killed it for sanction: the
   `do-while-zero-exception` rule's mechanism is reorg.c/`LABEL_OUTSIDE_LOOP_P`,
   which is **absent** here.
2. **Relocating a call** — a call is also a `flush_pending_lists` barrier, and moving
   it is a *spec* change, not a respelling. A naive "wrapper-free" filter misses this;
   `triage.py`'s semantic call-position filter catches it.

**This is how the committed regression was made:** s1's permuter took the top find,
and on this function the top find is always a barrier. The search space contains
nothing else — which is itself evidence that no pure-C *statement-level*
rearrangement fixes the schedule, and that the problem lives in the **declarations**.

### The reframe s2/s3 got backwards
s2/s3 recorded "the wrapper-free plain form is SEMANTICALLY WRONG, our C's meaning
depends on the barrier." **It is not.** The C says: store FB0/FB4/FB8, then copy.
GCC 2.7.2's alias analysis (BLKmode ⇒ `SIZE_FOR_MODE` 0 ⇒ conflicts only with the
same base symbol) cannot see the +4/+8 dependence and reorders across it. That is a
GCC **alias deficiency miscompiling legal C** — the source's meaning is correct and
GCC breaks it. Whatever the original wrote, it did not trigger the miscompile, which
is further evidence its region-word accesses shared a base symbol.

### Where that leaves the function (the s4 ruling question)
Both of `true_dependence`'s paths are now exhausted (shared base symbol → H14–H16;
both-volatile → H18), and the statement-level space is exhausted (H19). The **unique**
C form with both required properties — constant-address MEMs (so the stores keep
at-form and the copy keeps num_regs=2) *and* a shared base symbol (so the copies are
pinned) — is the base+offset re-view over a **scalar** symbol, `((s32 *)&D_80101F80)[k]`,
because a scalar VAR_DECL never engages GCC's aggregate machinery. That is s2's v13:
**byte-exact, link-identical, and self-rejected as a cheat.** Note `((int*)ARR)[k]` on
a *real array* folds back to `ARR[k]` and degrades — so the property comes specifically
from declaring the base a **scalar and indexing past it**, i.e. from a declaration that
is *false about the object*.

## Frontier (s5) — there is no search frontier left; there is a ruling question
1. **RULING (emitted this session).** Given a compiler-confirmed exhaustion proof,
   how is this function to be closed? The honest declaration space is empty; the one
   reproducing form requires a declaration that lies about the object.
2. **The one premise still unexamined: that the three copies are C struct assignments
   at all.** Everything above assumes `movstrsi` from a `*(Blk*)a = *(Blk*)b`. If the
   original produced these lw/sw pairs by some other construct, the whole alias
   analysis is moot. Worth one session of `output_block_move` archaeology before
   accepting the ruling's answer.
3. **Do NOT** re-run a naive permuter campaign on the plain chassis (H19), re-try any
   aggregate spelling (H14–H16), re-try volatile (H18), or re-sweep statement order
   (s3 H9, independently re-derived from `rank_for_schedule` this session).

## [s4] s3's frontier #1 — an enclosing object puts every store at a non-zero offset, so the offset-0 collision never arises and delta (1) evaporates.
- mechanism: The collision was believed to need the stored word at offset 0 OF THE COPIED OBJECT's own symbol; at bigobj+0x30 the store's address would be a different rtx from the copy's base.
- probe: tmp/grind/func_8001C624/s4/probe_offset0.c — f_agg0 (copied struct at offset 0 of its symbol) vs f_aggN (copied struct at BIG+184, inside a wider enclosing object). Isolated TUs, no src/ involvement.
- result: FALSE. f_aggN degrades IDENTICALLY to f_agg0: `la $3,BIG+184 ; sw $0,0($3)` — the offset-0 store still loses its at-form lui, and the block copy still comes out num_regs=3. The collision is between the COPY'S BASE ADDRESS and the address of the store to the FIRST COPIED WORD, and those are the same rtx at ANY enclosing offset, because the copy's base IS the address of the first copied word. All three copied regions have their first word stored in this function (D_80101F80, D_80101FB0, D_80101FCC).
- verdict: KILLED

## [s4] s3's frontier #2 — there is an honest aggregate spelling in which the offset-0 store's address pseudo stays SINGLE-USE, so combine folds it back to at-form and the three luis return.
- mechanism: Believed to be a use-count problem: the pseudo is multi-use only because the block copy needs the same bare symbol in a register.
- probe: Seven spellings measured in isolated TUs: struct VAR_DECL at offset 0; struct at BIG+184; array VAR_DECL ARR[k]; array via cast ((int*)ARR)[k]; typed struct view through a constant pointer ((Work*)&SYM)->a; const local pointer Work *w = (Work*)&SYM; base pointer into a real array int *p = ARR; p[k] = ...
- result: FALSE, and it is not a use-count problem. All seven degrade identically (`la $3,BASE ; sw $0,0($3)` + num_regs=3). k_baseptr degrades with only ONE other use of the base. `((int*)ARR)[k]` folds back to `ARR[k]` and degrades — so the at-form-preserving property belongs specifically to `((s32*)&SCALAR_SYM)[k]`, i.e. to declaring the base a SCALAR and indexing past it: a declaration that is FALSE about the object. Any real aggregate engages GCC's aggregate machinery and forces the shared base register.
- verdict: KILLED

## [s4] s3's frontier #3 — block-copy chunking (num_regs 3 vs the target's 2) is downstream of live-range pressure and will resolve itself once the addressing deltas are fixed.
- mechanism: Believed to be independent register-allocation fallout, not spellable in C, that would fall out once delta (1) landed.
- probe: The same seven probes — every form that loses the offset-0 at-form store ALSO comes out num_regs=3; every form that keeps it (f_scalar, g_cast_copy) comes out num_regs=2.
- result: FALSE as an independent delta. It is the SAME phenomenon: the shared base register held live across the copy is one of the copy's address registers, so mips.c's output_block_move (line 2445) counts fewer safe_regs. It reaches 2 only when the addresses are compile-time constants. s3's "do not attack it in isolation; it will fall out" was right that it is not separately spellable and wrong that it is separately caused.
- verdict: KILLED

## [s4] Our decompals GCC 2.7.2 port diverges from PsyQ's original cc1psx on aggregate member stores, which would explain the target's at-form offset-0 stores and rescue the aggregate family.
- mechanism: If the original compiler expanded a COMPONENT_REF store without forcing the base into a register, the original source COULD have been an aggregate and our port simply cannot reproduce it.
- probe: Ran every probe through tools/cc1psx_wrapper.sh (the original PsyQ cc1psx.exe, GCC 2.7.2.SN.1, via dosemu2) — the sanctioned self-disproof use per the cc1psx-calibration-only rule.
- result: ZERO DIVERGENCE. cc1psx produces byte-identical output to our port on g_arr_nocopy, g_arr_copy and g_cast_copy — same `la $3,ARR ; sw $0,0($3)` degradation, same at-form preservation on the cast form. The aggregate degradation is real in the compiler that actually built the game. This closes the toolchain escape hatch and turns "the original did not declare this region as an aggregate" from an inference into a compiler-confirmed fact.
- verdict: KILLED

## [s4] `volatile` — true_dependence's other path to a conflict — fences the copies below the stores that feed them.
- mechanism: true_dependence returns 1 on `(MEM_VOLATILE_P (x) && MEM_VOLATILE_P (mem))` without consulting memrefs_conflict_p, so volatile could create the dependence WITHOUT a shared base symbol — i.e. keeping at-form stores and num_regs=2. (Measured to close the space, not proposed: volatile coercion is a catalogued cheat absent genuine MMIO/interrupt justification.)
- probe: tmp/grind/func_8001C624/s4/probe_volatile.c — n_vol (volatile region scalars, ordinary block copy) and n_vol_both (both sides volatile).
- result: DEAD both ways. n_vol: the copy STILL floats above the +4/+8 stores, byte-identical to the non-volatile form — true_dependence needs BOTH mems volatile and the copy's mem is not. n_vol_both: GCC 2.7.2 REFUSES to inline a volatile block move and emits `jal memcpy` — the target has an inline lw/lw/sw/sw move and no call, so the shape is structurally wrong at any score. Both of true_dependence's paths are therefore exhausted (the two MEM_IN_STRUCT_P clauses are inert: both require rtx_addr_varies_p, false for symbol addresses).
- verdict: KILLED

## [s4] Directed permuter over the plain-scalar chassis (the mandated modality) finds an honest closing form.
- mechanism: sched1's placement of the copies is a priority/tie-break outcome that is hard to reason about analytically; permuter searches the C space directly.
- probe: tools/permuter_campaign.py, workspace tmp/grind/func_8001C624/s4/perm_a (label "plain-scalars"), minimal TU gated byte-identical to the full TU, target.o assembled from asm/6CAC.s so the relocs match and the score is honest. 27,651 iterations, 1,517 finds, harvested + STOPPED in-session. All finds triaged by construct with a semantic call-position filter (s4/triage.py).
- result: THE BASIN IS A SCHEDULING-BARRIER BASIN. Base 5615 -> best 875, but EVERY ONE of the top ~14 finds is a do-while(0) barrier; the best CLEAN find (wrapper-free, no volatile/asm/pin, both calls in their program-mandated positions) is 3035 — a 3.5x gap. Permuter's improvements come from exactly two moves and they are the same move: (1) do{...}while(0) = a sched.c:2066 total barrier, and (2) RELOCATING A CALL (also a flush_pending_lists barrier — and a SPEC change, not a respelling; the naive wrapper-free filter misses this). This is exactly how the committed regression was made: s1 took the top find, and on this function the top find is always a barrier. The negative result IS the finding: no pure-C statement-level rearrangement fixes the schedule, so the problem lives in the declarations — where s4 proved the honest space is empty.
- verdict: KILLED
