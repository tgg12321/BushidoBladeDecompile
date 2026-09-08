# SELF-VET — func_8003F6D8
CONSTRUCTS: none (two typedef'd structs describing the record layout; two nested counted for-loops; a per-iteration local `off = i * 0xD0 + 8` that is consumed by the address computation; no FAKE, no asm, no volatile, no dead stores, no pads)
## T1 semantic purpose: every statement is consumed. `off` feeds the record address; `in`, `obj` feed the two calls; the structs give the field offsets. Removing any changes behavior or does not compile.
## T2 human-programmer: a programmer walking a variable-length header with 0xD0-byte records would naturally compute the record offset from the loop index; the struct typedefs are how the field accesses are spelled. Nothing a reader would ask "why is this here?" about.
## T3 GCC-internals justification: the ledger explains WHY form C matches (loop.c giv reduction), but the construct itself is ordinary indexed addressing; program logic fully explains it. No GCC internal is the mechanism of any construct's presence.
## T4 permuter/search provenance: hand-written from the asm layout; no permuter used.
## T5 family check: no forbidden family matched; no sanctioned-exception family claimed. Plain C.
## T6 naming-announces-intent: names are `i`, `j`, `off`, `in`, `obj`, `rec`, struct field names `count/objs/pairs/unk78/quads`. `unk78` is an unknown-purpose real field spanning 0x78-0x84 (never accessed here), not a pad announcing coercion.
SANCTIONED-FAMILY-CLAIMS: none
ANNOTATION-CONFORMANCE: n/a — no FAKE construct
