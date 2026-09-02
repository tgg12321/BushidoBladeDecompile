# SELF-VET — func_8003D52C
CONSTRUCTS: none (ordinary C: address-form va_start macro identical to src/main.c:1707; one pointer local `p` reused across two sequential loops; literal character constants; while loops with break)
## T1 semantic purpose: every statement is required by the function's behaviour (segment splitting on '%', sprintf per segment, character rendering with ' ', '
', '~cNNN' colour escapes, column/row wrap). The va_start macro is the file's own varargs plumbing (now the same form main.c uses). No construct is byte-neutral filler.
## T2 human-programmer: reusing a single scratch pointer for two consecutive scans of different buffers is routine C; `&last + 1` is the classic pre-stdarg va_start idiom and is what this project's main.c already uses. Nothing invites "why is this here?".
## T3 GCC-internals justification: none required for any construct. The evidence file names put_var_into_stack / global.c only to EXPLAIN the measurements; the C stands on program logic alone (a varargs printf-lite that needs a scratch pointer).
## T4 permuter/search provenance: no permuter used. Both changes were reasoned from the target asm (home-slot store, $s1 shared by both loops) and measured directly.
## T5 family check: no forbidden family touched — no pins (the retired body's asm("s1")/asm("s3") pins were removed), no __asm__, no volatile, no dead stores, no pads, no empty bodies, no constant holders (pct/seg_start/buf_ptr holders from the retired body were dropped and measured byte-neutral).
## T6 naming-announces-intent: locals are buf, seg, ap, cur_arg, seen_pct, p, ch, row, d1..d3 — all named for their role and all read.
SANCTIONED-FAMILY-CLAIMS: none
ANNOTATION-CONFORMANCE: n/a — no FAKE construct
