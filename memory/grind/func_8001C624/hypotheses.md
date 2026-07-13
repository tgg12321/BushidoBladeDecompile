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
