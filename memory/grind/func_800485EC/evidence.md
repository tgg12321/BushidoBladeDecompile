# Evidence bank — func_800485EC

## [s1] 2026-08-27 — recon → candidate-ready (sandbox 0)

- **Function identity:** TIM-image header parser. Args: `(u32 *tim, TimHdr485
  *spr, s16 x, s16 y, u16 cx, u16 cy)` — 4 register + 2 stack args. Checks
  `*(u8*)tim == 0x10`, reads the flag word (`mode = flag & 7`, clut present =
  `flag & 8`), optionally parses the clut block (cx/cy from the stack args,
  cw/ch from the block, clutdata pointer, `clut = GetClut(cx, cy)`), then the
  pixel block (x/y from a2/a3, w/h, pixdata pointer, `tpage = GetTPage(mode,
  0, x & 0xFFC0, y & 0xFF00)`). Sole caller: func_80048530 (same TU, matched).
- **ABI proof of K&R/no-prototype linkage:** callee reads stack args 4/5 with
  `lhu` at +0x38/+0x3C (u16 narrow reads at the LE low half) while the caller
  sign-extends all four halfword values (`sll/sra 16`) and stores full words.
  An ANSI `u16` prototype would make the caller mask (andi) instead of
  sign-extend; an `s32` prototype would make the callee read with `lw`. Only
  a K&R definition (`s16 x, y; u16 cx, cy;`) + unprototyped `extern void
  func_800485EC();` reproduces both sides. Verified: caller sandbox == 0
  unchanged after the extern swap.
- **Struct layout (0x20 bytes):** s16 fields at 0x00 mode, 0x02 x, 0x04 y,
  0x06 w, 0x08 h, 0x0A cx, 0x0C cy, 0x0E cw, 0x10 ch; u16 0x12 tpage,
  0x14 clut; pointers 0x18 pixdata, 0x1C clutdata. Store-then-reload
  (`sh; lh`) of cx/cy/x/y/mode is the natural `spr->f = arg; ...
  call(spr->f)` pattern — no coercion needed.
- **GetClut/GetTPage decls:** must be visible as promoted-arg prototypes
  (`u32 GetClut(s32, s32)`, `u32 GetTPage(s32, s32, s32, s32)`, mirroring
  src/gpu.c definitions) so the `lh`-loaded s16 fields pass without masks.
  The TU's old `extern s32 GetClut(u16, u16);` at former line 5947 had ZERO
  call sites (grep-verified) and was retyped — codegen-neutral.
- **THE tail-block cse mechanism (dump-proven, tmp/grind/func_800485EC/s1/):**
  pre-cse RTL (.jump) has the pixel-block reads p-relative; .cse shows cse1
  rewriting them onto tim. `find_best_addr` → `fold_rtx` uses
  `lookup_as_function(op0, PLUS)` on the address's base reg: any fresh
  `p = tim + K` def in the same cse block as the reads, with tim's quantity
  still valid, gets its reads folded to tim-relative and the init+increment
  merged (67 insns, floor 7). The clut arm never suffers this because
  `tim = p + (bnum >> 2)` re-sets tim between the copy and the reads,
  invalidating the table entry.
- **The natural closer:** `tim += 2; p = tim;` — the live self-increment
  kills tim's old cse quantity (nothing left to fold onto) and the fresh
  copy `p` dies before the GetTPage call, so RA gives the walker caller-saved
  $v1 exactly as target. Every statement is live and consumed; no FAKE
  constructs anywhere in the body.
- **A dead-store form also measured 0 first** (`p = tim + 2; tim += 2;` with
  the increment dead) — immediately superseded by the natural equivalent
  above; never a candidate.
- **Byte verification:** cmp_bytes.py (s1 scratch): 68/68 words identical to
  asm/funcs/func_800485EC.s except the two jal relocation words and the
  internal `j` absolute target (relative offset 0xAC identical) — all
  link-resolved. sandbox --disable all == 0 (68/68); caller func_80048530
  sandbox == 0.
- **Scheduling notes that mattered:** the clut-block `bnum` must be a named
  intermediate read immediately after `p = tim` (source order copy → lw →
  sh cx/lh → sh cy/lh) — folding the load into `tim = p + (*p >> 2)` lets
  sched1 float it and costs 4 (11 → 7 when named).
- **Dump harness note:** tools/grinder/dump.ps1 printed pre-existing
  redefinition/conflict errors from LATER regions of text1b.c (its harness
  concatenation, not this diff); the real pipeline compile is clean and the
  per-function dumps for this function were produced correctly.
