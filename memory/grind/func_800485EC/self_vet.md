# SELF-VET — func_800485EC

CONSTRUCTS: K&R-style definition with narrow params (s16 x, y; u16 cx, cy) + unprototyped `extern void func_800485EC();`, local typedef TimHdr485 (0x20-byte header struct), named intermediate `u32 bnum` (once-written once-read clut block size), tail sequence `tim += 2; p = tim;` (live param increment + live fresh pointer copy), retyped zero-call-site TU decl `extern u32 GetClut(s32, s32);` (was `extern s32 GetClut(u16, u16);`), new decls `extern u32 GetTPage(s32, s32, s32, s32);` / `extern u32 GetClut(s32, s32);`

## T1 semantic purpose
Every construct has observable semantic purpose. K&R def + unprototyped extern: defines the actual narrow parameter types the ABI evidence shows (callee lhu of u16 stack args; caller default-promotion sign-extension) — removing it changes behavior of argument passing semantics, not just bytes. TimHdr485: the output structure the function fills; every field written. bnum: real value (clut block byte size), consumed in `tim = p + (bnum >> 2)`. `tim += 2`: advances tim past the pixel block's bnum/dxdy words; its value is immediately consumed by `p = tim`. `p = tim`: the working pointer whose post-increment value is stored to spr->pixdata. GetClut/GetTPage decl types: promoted-arg prototypes matching the real definitions in src/gpu.c. No statement in the diff is dead; the compiled body contains no dead stores.

## T2 human-programmer
Yes for all. A 1998 PsyQ programmer parsing a TIM header naturally writes "advance the stream pointer past the section header, take a working pointer, read w/h, store the data pointer" — the clut arm and pixel tail are the same idiom twice. K&R definitions are period-typical. A reader asks "why is this here?" about nothing: there is no construct whose only explanation is codegen.

## T3 GCC-internals justification
The final form needs no GCC-internals justification: `tim += 2; p = tim;` is ordinary stream-walking code (an equally natural spelling of `p = tim + 2`), chosen among natural equivalents by measurement. The cse1 find_best_addr analysis in evidence.md explains why OTHER natural spellings missed, not why this construct exists — the construct's explanation is program logic (advance stream, take walker).

## T4 permuter/search provenance
No auto-search was used. All forms were hand-derived from the target asm and the pass dumps; the closing form was reached by mechanism analysis and 6 measured probes, all recorded in evidence.md/hypotheses.md and rejected/.

## T5 family check
No forbidden family matches: no register pins, no inline asm, no volatile, no dead stores/self-assigns (the once-measured dead-store variant was discarded in favor of the natural form and is recorded in evidence.md as superseded, never proposed), no unused locals/arrays/pads, no alias renames, no do-while(0), no duplicated statements (the dup-into-arms probe was rejected on measurement and is banked in rejected/). The named intermediate `bnum` is once-written/once-read with a real consumed value and is byte-neutral ordinary C — it needs no family claim, but it also satisfies the named-intermediate shape if the reviewer wishes to classify it.

## T6 naming-announces-intent
Names are semantic: tim (TIM stream pointer), spr (sprite header out-param), bnum (TIM section byte-count field's PsyQ name), p (walker), flag, x/y/cx/cy (coordinates), TimHdr485 fields mode/x/y/w/h/cx/cy/cw/ch/tpage/clut/pixdata/clutdata. No pad/dummy/unused/spill-class names.

SANCTIONED-FAMILY-CLAIMS: none

ANNOTATION-CONFORMANCE: n/a — no FAKE construct
