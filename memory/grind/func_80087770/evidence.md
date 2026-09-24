# func_80087770 evidence

Sony LIBSND vmanager `_SsVmSetSeqVol` (SOTN `SpuVmSetSeqVol` is a simplified
later build; ps2sdk libsnd2 `vm/vm_seq.c _SsVmSetSeqVol` has the full volume
chain, minus BB2's lack of the `_snd_vmask` / vab-id checks). Canonical route C
(hand-coded tier LOW). Target 323 insns, frame 0x48 (24 bytes untouched locals —
ordinary phantom slots, reproduced with zero dead declarations).

## 2026-09-24 Codex session (interrupted)
Recovered the semantic body into src/main.c (uncommitted) and changed the
placeholder prototype. Measured 172 (325/323). Banked as the starting candidate;
the prototype change landed separately as `decl:` 3dd91162a (byte-neutral).

## 2026-09-24 manual session (Claude) — closed 172 -> 0
Each step read off `sandbox --diff`:
- 172 -> 133: pan branches test `pan < 0x40` first (target `beqz`), pan held
  in a `u8` (shorten_compare gives `sltiu`; `0x7F - pan` stays int so the
  program/voice pan stages divide SIGNED — target magic 0x82082083).
- 133 -> 96: vmNoiseOn-shaped stage 1 — `u32 voll_t/volr_t` pre-pan values,
  `u16 left/right` panned values (the u16 destination is what lets combine
  drop the sign correction / turn sra into srl on the `right` arms).
- 96 -> 57: tail squares computed into left/right before the stores.
- 57 -> 12: store order left, right, dirty. The loop.c dump showed
  `&_svm_sreg_buf+2` hoisted (life 9, savings 2 after CSE related the +0/+2
  constants); adjacent stores shorten the life below the threshold, so the
  address folds into the `%hi/%lo(at)` stores as in the target, and the frame
  becomes 0x48 with s0-s5.
- 12 -> 10: mono fold compare spelled `right > left` (operand order).
- 10 -> 0: the remaining hunks were local-alloc only (quotient temp a1 /
  quotient a0 / scaled a1 / volr v1). Declaration order is inert (32/32
  permutations measured 9). The target seats the first-stage quotient in the
  same register as the final left volume and `vol_factor / 0x3F01` in the same
  register as the final right volume, i.e. those values live in the
  voll_t / volr_t pseudos. Landed spelling: both channels start from the shared
  base (`voll_t = base; volr_t = voll_t * pg * tn / 0x3F01;
  voll_t = voll_t * pg * tn / 0x3F01; voll_t *= m_voll / 0x7F ...`) — every
  write is real and derived from the variable's own previous value (Ruling 4
  compound splits); no dead write, no carrier of an unrelated role.

### Layer-2 review 1 — FAIL (pan cascade carrier), resolved
The first landing reused one `u8 pan` local for the tone, program and voice pan
stages — vmNoiseOn's shape, which Ruling 8 admits for vmNoiseOn ONLY (a pan
cascade elsewhere needs its own owner ruling). Both spellings the reviewer asked
for were measured with the rest of the body unchanged:
- (a) each stage reads its field directly, no local: 0 (323/323)
  (tmp/f87770/pa.c);
- (b) three once-written locals `tone_pan` / `prog_pan` / `voice_pan`: 0
  (323/323) (tmp/f87770/pb.c).
The carrier was never load-bearing. Landed (b). Because (a) also scores 0, the
three locals are readability only, not RA-purposed named intermediates. The
rejected reuse body is banked as rejected/pan-reuse-carrier.c. The header
comment's SOTN claim was also corrected: SOTN SpuVmSetSeqVol is the same API in
a different build with no volume chain.

Measured and rejected on the way (tmp/f87770/, sandbox scores): single
`vol_factor` expression 10; `vol_factor` split in place 9; `VagAtr *tone`
pointer 20; fresh `master`/`scaled` locals 15/4/10; volr_t-held quotient 9;
fully symmetric `voll_t = volr_t = base` / duplicated-base forms 9/46;
`voll_t` holding the quotient then reassigned from volr_t (w2) = 0 but a
two-role carrier under Ruling 5 — not used.
