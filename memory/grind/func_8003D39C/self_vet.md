# SELF-VET — func_8003D39C

CONSTRUCTS: (1) `typedef struct {...} Sprt8Prim;` + `extern Sprt8Prim D_800A3930[2][32];`
(a TU-local declaration replacing the splat placeholder `extern u32 D_800A3930;`);
(2) `p = &D_800A3930[D_800A3218][n];` (two-dimensional subscript);
(3) `((u8 *)p)[3] = 3;` (primitive length byte);
(4) `p->code = 0x74;` followed later by `*(u32 *)&p->r0 = (color >> 1) | 0x74000000;`
(the packed RGB+code word);
(5) `ot = (OTag *)D_800A374C; ((OTag *)p)->addr = ot->addr; ot->addr = (u32)p;` (addPrim);
(6) statement order: `p->x0 = x; p->y0 = y;` placed before the u0/v0/clut stores.
There is NO /* FAKE */ construct in the diff, no volatile, no inline asm, no dead local,
no unused variable, no pad, no self-assign, no do/while(0), no goto.

## T1 semantic purpose
Every construct has an observable effect on the function's output.
(1) The declaration gives D_800A3930 its actual object model: 0x800A3930 is a 1024-byte
region ending exactly at the next symbol D_800A3D30 (undefined_syms_auto.txt:279-280),
which is 2 x 32 x 16 bytes — a double-buffered array of 32 SPRT_8 primitives. Without a
type the subscript arithmetic cannot be written at all.
(2) Computes the address the function writes to; removing it removes the function.
(3) Writes the GPU primitive length field (byte 3 of the tag word) = 3 words; the hardware
requires it. Materialises as `sb v0,3(a0)`.
(4) Both stores materialise in the ORIGINAL bytes: `sb v0,7(a0)` (the code field) and
`sw v0,4(a0)` (the packed word that re-writes the same byte). The redundancy is the
original program's, not an artefact introduced to move bytes: the shipped executable
contains both instructions. Removing the `p->code` store deletes `sb v0,7(a0)` from the
output and the function stops matching because the ORIGINAL contains that instruction.
(5) Links the primitive into the ordering table (the standard PsyQ addPrim getaddr/setaddr
pair); it is the whole point of the function.
(6) A statement order. Semantically all six field stores are independent, so any order is
correct; this order is the one the original source used (see T2/T3).

## T2 human-programmer
Yes to all six. (1) is how anyone declares a double-buffered array of 32 sprite
primitives. (2) is the ordinary way to index it. (3)/(4)/(5) are the PsyQ SPRT_8 +
addPrim idioms and (3)/(5) are already used verbatim by the MATCHED, committed function
func_8003D330 four lines above this one in the same file (src/code6cac_c2.c:1002-1010).
(6) reads as "set the length/code, then the position, then the texture coordinates and
CLUT, then the colour" — the natural order for filling a sprite primitive, and if anything
more natural than the previous candidate's order. Nothing in the diff would make a reader
ask "why is this here?": every line writes a field the GPU consumes.

## T3 GCC-internals justification
No construct in the diff is justified by a GCC internal. The two edits that moved the
score (the 2D declaration, 15 -> 6, and the x0/y0 statement position, 6 -> 0) were found
by measurement and are both explainable purely at the program level: the array really is
two-dimensional, and the field stores really are order-independent. The session's
evidence.md DOES explain, after the fact, WHY the register allocator responds to the
statement order (allocno_compare in tools/gcc-2.7.2/global.c ranks by n_refs/live_length,
and moving a use earlier shortens a live range) — but that is a post-hoc explanation of an
ordinary-C choice, not the reason a reader would need for the code to make sense. Neither
construct is a lever bolted on to reach a register: both are the plain spelling of what
the function does.

## T4 permuter/search provenance
No permuter, no auto-search, no randomised spelling search was run this session. All four
measured variants were hand-written from a program-level hypothesis (the stride 0x200 =
32 x 16 implies a `[2][32]` array; the arg-copy seat rotation implies a live-range
ordering question among the three parameters). The final form is a normal C function that
would be written this way with no knowledge of the compiler.

## T5 family check
No construct matches any forbidden family, by analogy or otherwise. There is no register
pin, no `__asm__`, no scheduling barrier, no volatile of any kind, no alias rename, no
unused local, no local array, no dead parameter assign, no dead conditional store, no
empty-body if, no always-true wrapper, no dead goto, no DImode chain, no goto-end
accumulator, no declaration-order trick, no opaque constant variable, no width-cast
padding, no linker-script reorder. The declaration change is not the aggregate-merge
family either: it does not merge several splat per-word D_ scalars into one object — it is
a SINGLE splat symbol given its true array type, and the region it spans
(0x800A3930 + 0x400 = 0x800A3D30) terminates exactly at the next symbol, so no other
symbol is subsumed or renamed. The declaration is TU-local in exactly the place the
previous TU-local placeholder `extern u32 D_800A3930;` stood (src/code6cac_c2.c:1011),
which is the file's existing convention for this symbol.

## T6 naming-announces-intent
No name in the diff is `pad`, `dummy`, `unused`, `spill`, `slack`, `tail`, `buf` or any
coercion-announcing name. The identifiers are `Sprt8Prim`, `n`, `p`, `ot`, `x`, `y`, `ch`,
`color` and the PsyQ field names `tag/r0/g0/b0/code/x0/y0/u0/v0/clut`. Every declared
object is read: `n` (three uses), `p` (nine uses), `ot` (two uses), all four parameters.
There are no declarations whose only uses are discards, address-of, or nothing.

SANCTIONED-FAMILY-CLAIMS: none — the diff is ordinary C and claims no exception family.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.
