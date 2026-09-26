# 0x80046954: `snd_SeNullCallback` (C/link name) vs `empty_stub` (census). Verifier verdict

**Verdict: REFUTE.** A RESET of `snd_SeNullCallback` on reset-contradicted grounds is not warranted.
- The `NullCallback` part is accurate.
- The `snd_Se` part is unsupported. It is not contradicted. Slot 9 is never touched by live code,
  so what it holds cannot be known.

**Better single name: the census name `empty_stub`.** It is fully proven. The link spelling should
be `func_80046954`, with `empty_stub_80046954` kept as the alias. This follows the
0x800457D4 precedent. Moving src to the census name is a housekeeping sync of a name the census
never adopted. It is not a contradiction RESET, and the record must not claim that "snd/Se" was
shown false.

All facts below were re-derived from the raw words of `disc/SLUS_006.63` using my own scripts in
this directory (`refs.py`, `ctx.py`, `lookups.py`, `callers878.py`, `c3af40.py`; output in
`refs_out.txt`). I did not rely on any earlier verifier's text.

## Chain

1. **Body.** `asm/funcs/func_80046954.s` is `jr ra; nop`, 2 insns, words `03E00008 00000000`.
   The preceding function (0x80046934) ends in `jr ra; nop`, so nothing falls through into it.
2. **Every reference to 0x80046954 in the EXE.** I scanned all 151,040 words for four forms:
   `jal` (0x0C011A55), `j`, a 32-bit data word 0x80046954, and `lui 0x8004` paired with an
   `addiu`/`ori`/`addi` of lo16 0x6954 in the same register within 12 insns.
   - Exactly one reference exists: `lui a1,0x8004` @8004697C + `addiu a1,a1,0x6954` @80046980,
     inside func_8004695C.
   - That pair loads a1 for `jal func_80045694` @80046984, with a0 = 9 in the delay slot.
3. **func_80045694(id, cb)** scans the 10 heap records at D_800EED10 (16-byte stride, s16 id at +0)
   for `id == a0`. When it finds one, it stores a1 at record +0xC (D_800EED1C). So 0x80046954 is
   passed as heap slot 9's callback.
4. **How the callback is invoked.** func_80045294 moves heap blocks: DrawSync, then func_800520B8
   moves the blocks. For each following record it adds the delta to the base, loads rec+0xC, and
   **skips the call when the value is 0** (`beqz v0` @8004536C). Otherwise it calls
   `jalr v0` @80045380 with a0 = the record's s16 id and a1 = the delta.
   - func_800455AC (alloc) initialises rec+0xC to 0 (`sw zero,0xC(v1)` @800455E4).
   - A no-op function installed there therefore has the same effect as no callback.
   - **"NullCallback" (a no-op callback) is therefore accurate.** It is a relocation callback, not a
     "sample finished" callback as `docs/engine/sound.md:53-56` says. That doc prose is
     contradicted. The name does not say when the callback fires, so the name is not contradicted.
5. **Slot 9 is dead.**
   - The three functions 0x80046934 (alloc 9), 0x8004695C (commit 9 + set cb) and 0x800469A0
     (resize 9) each have **zero references**: no jal, no j, no data word, no lui/addiu pair.
   - I enumerated every call to the heap API from raw words, together with the a0 value at each site:
     - alloc func_800455AC: slots {player a0, player a0+3, 6, 7, 8 (s4=8), **9 only @8004693C**, 10}.
     - commit func_80045600 and set-cb func_80045694: the same set. Slot 9 appears only in 0x8004695C.
     - resize func_80045510: {a0+3, 7, 8, **9 only @800469AC**}.
     - free func_800453E0: {6, 7, 8, 10, player slots}. Nothing ever frees slot 9.
     - lookups func_8004574C / func_800457A0: {8, 6, 7, the player slot, and the callback's own id}.
       No site uses 9.
   - **Player slots.** func_80045878 is reached only through func_80040510 (single jal @80040520),
     which passes a0 straight through. func_80040510 has 4 call sites:
     - 8001CAFC: a0 = 1.
     - 8001DE54 and 8001DEB8: a0 = s0, looping s0 from 0 while `slti s0,2`.
     - 8003AFE0: a0 = s0 = the a0 of func_8003AF40. All 12 of its jal sites pass a0 = 0 or 1.
       The unclear one @8003C110 gets a0 = 0 on every incoming path.

     So the player slots are {0,1} and {3,4}, and never 9.
   - Record ids are written only by func_800451D0 (init to -1), func_800455AC (alloc) and
     func_800453E0 (compaction shift).
   - **Result: no live code allocates, reads, fills or frees slot 9, and no code associates an
     NDATA id with it.** Unlike slot 8 (func_800467B8 → NDATA 0x4D+) and slot 10 (func_800469C4),
     slot 9 has no loader. 0x80046954 is never installed at runtime and never executed.
6. **snd / Se.**
   - The contents of slot 9 cannot be known: there is no file set to read.
   - I did not run an NDATA magic scan. With no loader, any choice of ids would be arbitrary, so a
     hit or a miss would say nothing about slot 9. Scanning would only look like evidence.
   - The arena is not sound-free: slot 6 and the player slots carry VAB banks, as the second sweep
     re-established. So "an SE bank in this heap" is not impossible either.
   - What remains is the legacy renamer's pattern guess across the 0x800467B8-0x80046AA0 cluster.
     For slot 8 that guess ("Bgm") was shown false (model files). That shows the method is
     unreliable, but it is not a contradiction for slot 9.
   - I did not re-derive the "SE bank home 0x8010DB00" claim and I do not rely on it.
   - Kengo: `kengo_matches.csv:512` is size-only-ambiguous. It is no evidence.
   - **Unsupported, not contradicted. Under the owner's bar that rules out a reset-contradicted RESET.**

## Which single name
- `empty_stub` is computation-restatement, verified in sweep-2026-09-25 (verify/misc.csv) and
  consistent with this re-read. It claims nothing the body does not show.
- `snd_SeNullCallback` is half-proven. `NullCallback` is structural fact, taken from caller context.
  `snd_Se` is an unprovable semantic claim.
- The census never adopted the C name. `LEGACY_RENAMER_AUDIT.md:59` says "Adopted as comment";
  `named_syms.txt` keeps it only in a comment.
- **Recommendation:** keep `empty_stub` as the census name. Bring src in line with the census as a
  sync, not a RESET, following the 0x800457D4 precedent: src defines `func_800457D4`, and
  named_syms carries the alias `empty_stub_800457D4`.
- Do not coin a middle name like "heap slot-9 null callback". "Slot 9 callback" is caller context,
  and no admitted naming class covers it.
- **If the owner's process allows a C link name to be retired only through a verified
  contradiction RESET,** this row stays a recorded desync (hold). It must not be applied as a
  RESET that claims "Se contradicted".

## Apply hazards (for the manual lane)
- `src/sound.c:180` holds the definition and `src/sound.c:186` the use in `func_8004695C`. Rename
  both to `func_80046954`.
- Never use bare `empty_stub` as a link name, because 0x800457D4 also has census name `empty_stub`.
- If `empty_stub_80046954` were used as the C name, it would clash with the `named_syms.txt`
  assignment of the same name (linked with `-T named_syms.txt`). Use `func_80046954` in src.
- `asm/funcs/func_8004695C.s:10-11` uses `%hi/%lo(snd_SeNullCallback)`. func_8004695C is C in
  sound.c, but any tool that assembles the .s (for example sandbox or target references) would
  lose the symbol. Update it to `func_80046954`.
- `tools/rename_funcs.py:22` is the legacy map `func_80046954 -> snd_SeNullCallback`. Re-running it
  would bring the name back.
- `docs/engine/sound.md:45,53-56` describes channel 9 as "Load an SE sample" and the callbacks as
  "invoked when the underlying sample finishes". Both are wrong about the mechanism: these are
  heap-slot relocation callbacks. This is prose only, and the doc should be corrected.
- `named_syms.txt:309` has the comment "a null SE callback table slot". It is a comment only.
- Name-keyed gate lists: none contain `snd_SeNullCallback` (git grep outside docs/naming: only the
  paths above). This is a matched C function, so the oracle guards the edit. The rename is
  codegen-neutral in principle, but it must still pass the build SHA1.
