# SELF-VET — func_80072E10
CONSTRUCTS: TU-local `typedef struct PolyG4Xy` (standard PsyQ POLY_G4 layout: tag, rgb+code, x0/y0, rgb+pad, x1/y1, rgb+pad, x2/y2, rgb+pad, x3/y3); struct-field stores `p->xN/yN = k`; casts `(GameObj *)p` at the four callee call sites because the existing callee prototypes take the placeholder `GameObj *` type; `*(PolyG4Xy **)((s32)arg0 + 0xC)` context-field read/write (same spelling as the pre-migration body).
## T1 semantic purpose: Every store writes a vertex coordinate the callee (SetPolyG4/AddPrim path) consumes; the typedef names the primitive's real layout. Nothing in the diff is byte-neutral filler; removing any store changes the target bytes.
## T2 human-programmer: Yes. A programmer filling a POLY_G4's four vertices writes `p->x0 = 0x50; p->y0 = 0x32;` etc.; this is the SDK-canonical idiom (setXY4). The cast-scalar control form is the unnatural one.
## T3 GCC-internals justification: The struct form is chosen because it is what the code means (a POLY_G4). sched.c true_dependence / MEM_IN_STRUCT_P (evidence.md s1) explains why the CONTROL diverged, i.e. why the prior empty-if cheat existed; it is not the reason a reader would write struct fields. No construct exists solely to steer a pass.
## T4 permuter/search provenance: Hand-derived from the asm (POLY_G4 stride 0x24, SetPolyG4 callee); no permuter used.
## T5 family check: No forbidden family. Not aggregate-merge (no splat D_x scalars merged; the struct types a heap primitive reached through a pointer). Not a FAKE family. Ordinary C.
## T6 naming-announces-intent: Names are tag/r/g/b/code/x/y/pad matching libgpu POLY_G4; `pad1..pad3` are the real POLY_G4 pad bytes after each rgb triple (unused in this function, as in the SDK struct). No coercion names.
SANCTIONED-FAMILY-CLAIMS: none
ANNOTATION-CONFORMANCE: n/a — no FAKE construct
