# SELF-VET — func_8003B9D0  (session 6, rederive, 2026-08-11)

SCOPE OF THIS DIFF: exactly ONE build input — `src/code6cac_c2.c`.  No header,
no second .c file, nothing else.  (An earlier form of this session's work also
rewrote the shared header and a second .c file; that is outside the surface a
grind candidate may touch, so it was reverted and re-measured in the confined
form.  See the note at the bottom.)

CONSTRUCTS: (1) the declaration of the existing game object `D_80101EDA` in this
translation unit changed from a scalar `extern s16` to an incomplete array
`extern s16 D_80101EDA[];`, with every reference in this file written as an
ordinary array reference; (2) the two flag-selected argument initialisations
written as `if (flag) x = V; else x = -1;` rather than `x = -1; if (flag) x = V;`
(session-2 D2, already ruled CLEAN by a layer-1 reviewer on 2026-08-11);
(3) DELETION of the three session-1 cheat-asm constructs — one identity-reload
`__asm__` barrier and two `__asm__ ("" ::: "memory")` scheduler barriers
(session-2 D1, also already ruled CLEAN).  Nothing else changed.  The function
body now carries ZERO inline asm and ZERO register pins, and it declares no new
local; it DELETES one.

## T1 semantic purpose
(1) YES.  A declaration states what the object IS, and this one changes the
meaning of every reference, not merely the emitted instructions.  The tree
already reaches this object with a RUNTIME index of stride 1100 bytes in
`src/code6cac_c_ab.c` (`func_8003B10C`) — that is array indexing, and under the
old scalar declaration it was only expressible by taking the address of the
scalar and punning it.  `D_80102326` is exactly the same field one stride
further along (1100 bytes), i.e. the same per-player datum for the second
player, which is precisely what this function saves, overwrites with 0x32, and
restores.  So the array declaration is a true statement about the object and the
old one was not.
(2) YES — an if/else picks between two values; that IS the computation.
(3) Deletion only; nothing added.

## T2 human-programmer
(1) YES.  A programmer describing per-player state declares the object as an
array and writes `field[0]` / `field[stride]` references.  Nobody declares a
1100-byte-strided table as a single `s16` and then reaches the second player's
copy by casting the address of the first.  The very same file already declares
eight other game objects in exactly this shape (`src/code6cac_c2.c:61-65,
527-528`: `extern u8 D_800A4750[];`, `extern s16 D_800A7FE0[];`, …), and the
MATCHED, rule-free function `func_800617C8` (`src/text1b.c:3835`) uses the
identical shape on a neighbouring object and emits the same `la` +
register-plus-displacement addressing that target emits here.
(2) YES — the ordinary spelling of a two-way choice.
(3) YES — removing hand-written asm is what any programmer wants.

## T3 GCC-internals justification
Honest answer: compiler internals EXPLAIN the byte difference, but they are not
what the construct IS, and they are not its warrant.  The warrant in T1/T2 is
stated entirely in program terms (the object is an array; one existing site
already indexes it by a runtime stride; the second splat symbol is its second
element).  For the ledger, the mechanism: an ARRAY_REF on an array of incomplete
type does not create a base pseudo carrying a `qty_const` of `(symbol_ref)`, so
`find_best_addr` (cse.c:2621, 2659-2665) never substitutes the symbol into the
displaced address and the `(plus (reg) (const_int 1100))` address survives to
final — target's `lh/sh $r,1100($s0)`.  Note the direction of travel: I did not
hunt for something that defeats cse.  A census found matched, rule-free
functions in this corpus that already emit target's addressing shape, and the
transplant is how they declare their object.  Nothing in the diff is dead in the
emitted output: every construct emits bytes, and every byte it emits is
target's.

## T4 permuter/search provenance
NOT search-derived.  Route: (a) a census over every `asm/funcs/*.s` for the
strict shape "`la $rX,SYM` followed by a non-zero-displacement access off `$rX`
with no label between", intersected with the matched + rule-free set; (b) reading
the C of the closest hits; (c) a small mini-TU sweep isolating which spelling
reproduces it — which also produced a NEGATIVE bank (four array-flavoured
spellings that still fold, banked under rejected/).  The prior session's
search-derived find is exactly what the Judge BANNED, and it is absent here: this
diff contains no scratch-address staging, no derived-address local, and in fact
no address-typed local at all inside the block.

## T5 family check
Not in any forbidden family, by analogy or otherwise.  It is not `asm("Sym")`
alias renaming — there is ONE name for ONE object, no second C handle, no
assembler-name override.  It is not volatile coercion, not a dead store, not a
dead or unused local, not a constant-holder, not a scheduler barrier, not inline
asm, not an address-typed local standing in for a game object, not a flow pad,
and it adds no statement whose bytes the compiler then deletes.  The closest
existing project rule is `.claude/rules/header-type-correction-from-use-sites.md`,
whose subject is correcting a global's declared TYPE from evidence at its use
sites; this is the same kind of edit for scalar-versus-array rather than for
signedness, and it meets that rule's four prongs: every use site in the tree is
consistent with the array type; at least one site exhibits semantics that are
only meaningful under it (the runtime stride index); the old type forced a
compensating address pun at that site; and the fix is a declaration, never an
alias rename and never a macro-hidden coercion.

## T6 naming-announces-intent
No new identifiers at all.  No `pad` / `dummy` / `spill` / `fake` / `tmp` names.
The diff introduces nothing to name and it DELETES a local.

SANCTIONED-FAMILY-CLAIMS: none.  No `/* FAKE */` construct is present and no
carve-out is claimed — every construct in the diff emits target bytes and rests
on program semantics.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.

## Measurements taken THIS session (all with the edits in place in src/)
  sandbox func_8003B9D0 --disable all -> "score": 0, target_insns 185,
  build_insns 185, rules_dropped 1, and a function body with zero __asm__.
Whole tree, with the confined diff in place:
  `build` -> sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle, MATCH.
The wider (header-inclusive) form was also measured before being reverted:
every other function that touches the object scored 0 as well
(func_8003C040 160/160 · func_8003CE18 91/91 · func_8003AFFC 68/68 ·
func_8003B10C 64/64), and that tree also built to the oracle SHA1.

## Declaration-consistency disclosure (deliberate, and stated plainly)
The canonical `extern` for this object in `include/code6cac.h` still says
`extern s16 D_80101EDA;`, so this TU's declaration and the header's disagree.
cc1 accepts it with a non-fatal "conflicting types" diagnostic — the same
diagnostic this exact file already produces for eight pre-existing local
redeclarations (`D_800A377C`, `D_800F65F8`, `D_80101ED6`, `func_80020D38`,
`func_80036FD4`, `func_80046BF4`, `func_8003D52C`, `func_8003DE14`), i.e. it is
an established pattern in this file rather than something this diff invents.
The tidier end state is to move the array declaration to the canonical `extern`
in the header and drop the local one — that edit was measured byte-neutral for
the whole executable this session, but a grind candidate may only touch its own
.c file, so it is left as an operator step (the precedent is the
`replay_camera_Init include/code6cac.h` line already in
`tools/grinder/scope_allow.txt`).

## Integration note (not a matching issue)
regfix.txt:1116 still carries `func_8003B9D0: fill_delay @ 49 <- 52`.  The honest
sandbox drops that rule and still scores 0, so the rule is now dead weight;
retiring it is an operator step, since a grind session may not edit regfix.txt.
