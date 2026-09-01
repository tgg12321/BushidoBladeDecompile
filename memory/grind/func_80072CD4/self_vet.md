# SELF-VET — func_80072CD4

CONSTRUCTS: (1) a file-scope `POLY_G4` typedef reusing verbatim the struct shape already on
main at src/code6cac_b2_pre.c:148-158; (2) plain struct-field assignments through
`((POLY_G4 *)arg1)->field = <literal>` for every colour byte the function writes. No local
variable of any kind, no holder, no annotation, no barrier, no cast beyond the single
`(POLY_G4 *)` view of the incoming `GameObj *`, no `__asm__`, no `volatile`, no `goto`.
Each control-flow path assigns each field it owns exactly once.

## T1 semantic purpose
Every assignment in the body materialises a colour byte that the target function's own bytes
store. Independently checked against asm/funcs/func_80072CD4.s: all 24 field values in the
body correspond one-for-one to target stores, and both inner paths really do store 0xFC to
offsets +0x04 and +0x0C. There is no construct in the diff whose removal would leave the
observable behaviour unchanged — delete any one assignment and a vertex colour byte is left
unwritten. The `POLY_G4` typedef is the primitive's actual memory layout; it carries the
field offsets and nothing else.

## T2 human-programmer
Yes. The specification is "set the four vertex colours of a gouraud quad, using one of two
palettes for the first two vertices depending on a flag, and a third palette entirely when
arg0 >= 4". Writing each branch as a complete, self-contained set of vertex colours is the
natural way to express that: a reader of the `if` sees the whole palette for that case in one
place, instead of two thirds of it here and one third of it after the join. Nothing in the
body prompts "why is this here?" — there is no name, no variable, no comment, no statement
that does not paint a pixel. The neighbouring already-COMPLETED-C function in this same file,
func_80072BC4 (src/text1b.c:6023-6029), is written in exactly this style: it duplicates
`*(u8 *)(arg1 + 0x1D) = 0xC3;` into both arms of its inner if/else while writing +0x1C and
+0x1E after the join.

## T3 GCC-internals justification
No. The justification offered here is the program's own semantics — per-path complete vertex
palettes — and it stands without reference to any compiler pass. I am deliberately NOT
offering a pass-level mechanism as the reason the construct is present, and the construct
does not depend on one being true. (Earlier sessions of this function did argue a
scheduler/cross-jump mechanism for a *different*, holder-carrying body; that body is dead,
remains banned, and is not what is being landed. The form here has no holder and no
compiler-behaviour argument attached to it.)

## T4 permuter/search provenance
No. The body was not produced by decomp-permuter or by any automated search; it is a
hand-written expression of the function's palette semantics using the project's existing
POLY_G4 declaration. It is not "necessary only because a search found it", and it does not
survive by evading a detector: it declares no variable and contains no construct a detector
would have anything to say about.

## T5 family check
No forbidden family applies, and no sanctioned exception family is being claimed. Walking the
catalog: no register-asm pin, no `__asm__` of any kind, no scheduling barrier, no volatile in
any spelling, no unused local array or frame coercion, no dead-param assign, no dead
conditional store, no `if (1)` / `do{}while(0)` wrapping, no dead-goto label pad, no DImode
chain, no accumulator+shared-label exit, no alias rename, no opaque constant variable, no
width cast, no linker-script reorder. There is no local, dead, or discarded value anywhere in
the body, which is what most of that catalog is made of. The one construct a reviewer will
look hardest at — the same 0xFC appearing in both inner arms — is not a duplicated *dead*
statement: r0, g0 and r1 are each written on both paths as part of that path's complete RGB
triple, alongside b0/g1/b1 which genuinely differ between the paths. Nothing is
cherry-picked; the whole triple pair moves together, which is why this is a semantic
partition of the palette rather than a lever aimed at one store.

## T6 naming-announces-intent
No name in the diff announces coercion intent. The only identifiers introduced are the
`POLY_G4` tag and its member names (`tag`, `r0/g0/b0/code`, `x0/y0`, `r1/g1/b1/pad1`, …),
copied character-for-character from the declaration already on main at
src/code6cac_b2_pre.c:148-158, which in turn matches the PsyQ libgpu primitive layout. The
`padN` / `code` members are the hardware primitive's real reserved bytes, not agent-invented
padding, and the body never reads or writes them. No `dummy`, `spill`, `slack`, `_pad`,
`tail`, or `unused` symbol exists in the diff.

SANCTIONED-FAMILY-CLAIMS: none — this is ordinary C. No exception family is claimed and none
is needed. (The duplicated-statement-into-arms family is explicitly NOT claimed: its
prerequisites, including the /* FAKE */ annotation, do not attach to statements that are
semantically purposeful on every path.)

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.
