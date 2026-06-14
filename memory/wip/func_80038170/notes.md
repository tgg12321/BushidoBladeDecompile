# func_80038170 (code6cac_c_mid.c) — BLOCKED: phantom 8-byte frame local

## TL;DR
Same class as func_8001924C (read that WIP too). HEAD byte-matches the full
build ONLY via a frame-coercion cheat: `s32 dummy0, dummy1;` +
`__asm__ volatile("" :: "m"(dummy0), "m"(dummy1));` — the `"m"` memory operands
force two unused 8-byte stack slots (a [[dead-vars-local-array]] scalar variant).
HEAD also carries `register asm("a3")`/`asm("a1")` pins + a regfix
`reorder 10,11,13,12,9 @ 9-13`. The honest pure-C distance (cheats stripped) is
**12, and ALL 12 diffs are a single +8-byte stack-frame-size delta** — the
141-insn body is otherwise byte-identical to target.

## The gap (precise)
clean build (no dummy/pins/reorder): `addiu sp,sp,-48`, saves at 24/28/32/36/40.
target:                              `addiu sp,sp,-56`, saves at 32/36/40/44/48,
with a 16-byte local hole at sp+16..31 that the target NEVER accesses (the only
sp-relative insns in the whole body are the 5 reg saves + 5 restores). Our clean
build reserves only 8 of those local bytes; the target reserves 16. The 8-byte
delta is a phantom local with no body reference.

## Why not pure-C-closable
- The 8 extra bytes are an unused stack hole the target reserves but never reads
  or writes — no callee takes its address, no spill, not alignment (48 is already
  8-aligned). No semantic local is recoverable from the byte-identical body.
- `dummy0/dummy1` + `"m"` is the FORBIDDEN frame-coercion cheat (scalar
  address/"m" variant of dead-vars-local-array). A named guess is the same cheat.
- The pins (a3=i, a1=mask) and the reorder rule are separate cheats, but the diff
  shows the BODY matches without them — they're not what's blocking; the frame
  size is. (They were likely added together historically.)

## HEAD relies on the cheat
Canonical build/src/code6cac_c_mid.o = `-56` (matches target) — via the dummy
`"m"` slots. Oracle is green at HEAD but the match is a frame cheat; this function
is INCOMPLETE.

## Resume guidance
1. Confirm: strip pins + dummy + reorder → sandbox 12 (pure frame-size diff).
2. Recover the semantic local if possible: this function builds a structured
   output buffer `out` from several D_8008Fxxx tables — check whether the original
   used a small stack scratch (struct/array) the compiler fully DCE'd. The body
   shows none, but a caller/data-layout view might.
3. Otherwise canonical-asm / park (unrecoverable phantom local). Surface for user
   authorization. On the dead-vars-local-array re-evaluation list.

Source at HEAD (oracle green via the dummy cheat). Card BLOCKED.
