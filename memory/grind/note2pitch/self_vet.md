# SELF-VET — note2pitch
CONSTRUCTS: none
## T1 semantic purpose: No construct present. Every statement in the body computes a value consumed by the returned pitch (note offset, /8 fine step with clamp at 15, /12 octave split, table lookup, octave shift). Removing any of them changes the function's output.
## T2 human-programmer: The body is Sony's own note2pitch as recovered in psyz (PsyQ 4.0 libsnd/vm_n2p.c), adapted only in identifier names (D_800A26E4 for pitch_table, BB2's s32/u16 typedefs). A reader sees a straightforward note-to-SPU-pitch table lookup.
## T3 GCC-internals justification: None used. No reasoning about allocator, scheduler, DCE, or emission order was needed; the psyz body compiled to the target bytes on the first measurement.
## T4 permuter/search provenance: No permuter or automated search was run. The form is a version-correct library source seed (psyz sweep, memory/grind/note2pitch/psyz-sweep-2026-08-18.md row 7).
## T5 family check: No forbidden family matched. No volatile, no pins, no asm, no dead stores, no pad locals, no do-while wrap, no alias renames, no casts beyond the (s16) narrowing that the original C performs.
## T6 naming-announces-intent: Locals are octave/note/shiftVal/semitones/semitone/tableIndex/step/shift/pitch/noteIndex, all psyz names, all read after being written. No pad/dummy/unused/spill names.
SANCTIONED-FAMILY-CLAIMS: none
ANNOTATION-CONFORMANCE: n/a — no FAKE construct
