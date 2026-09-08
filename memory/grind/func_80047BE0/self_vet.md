# SELF-VET — func_80047BE0
CONSTRUCTS: none (plain C: two `extern SVECTOR D_800EF0D8[17]; extern SVECTOR D_800EF168[17];` declarations replacing the old `extern s16` scalar declarations, the RotTransPers3 prototype corrected to take `SVECTOR *` arguments, while-loops with explicit counters, post-increment pointer stores, and one `(s16 *)base` cast at the func_8004BCC0 call to match that function's existing prototype)
## T1 semantic purpose: every statement is load-bearing program logic — the vertex fill (vx = wave sample - 0xFA0, vy = 0, vz = ramp from -0x2EE0 by 0x7D0), the six RotTransPers3 + ReadSZfifo3 projections writing 18 sxy words and 18 sz halfwords into scratchpad, and the func_8004BCC0 chaining call on every channel after the first. No statement is byte-neutral if removed.
## T2 human-programmer: yes — this is a straightforward "fill 17 SVECTORs, project them in triples, store the results" routine; a PsyQ programmer writes exactly this with SVECTOR and RotTransPers3. Nothing prompts a "why is this here?".
## T3 GCC-internals justification: none needed or used. The only tuning was source statement ORDER (`i = 0` before `src = D_800EF59C`; `j = 0` before `z = -0x2EE0`), which is ordinary C statement ordering, not a named GCC-pass mechanism.
## T4 permuter/search provenance: no permuter run; the body was hand-written from the asm read in this session.
## T5 family check: no forbidden family matched. No register pins, no asm, no barriers, no volatile, no dead locals, no dead stores, no pads, no `if (1)`, no goto chassis, no do-while(0), no aliases. The retired-chassis body's `if (1) { ... }` wrap was NOT carried over (banked under rejected/).
## T6 naming-announces-intent: no coercion names; the locals are sxy0..sxy2, pflag, flag, sz0..sz2, s7val, i, j, src, base, v, dst32, dst16, z.
SANCTIONED-FAMILY-CLAIMS: none
ANNOTATION-CONFORMANCE: n/a — no FAKE construct
