# Fresh adversarial checkpoint review, 2026-09-28

Reviewer: independent Codex subagent `/root/review_c21c`; did not author s6.
Reviewed: the s6 candidate pulled at main e6feac99c, now preserved verbatim in
`rejected/s6-col-carrier-37.c`. Default-FAIL posture; read assembly, types and
current rule text independently. **Verdict: FAIL. Not a completion review.**

The reviewer returned these blockers (summarized without changing the verdict):

- `col`: Ruling 11 defines a value through writes reaching a common read.
  All consumers here stay inside their arms; zero and 0x80 do not reach a
  common read. C(3) fails, including the current Q20 amendment. The second
  bar also reassigns the value already held on every feasible incoming path
  because `i` is unchanged; B(2) fails. A real runtime choice with common
  consumers is a different candidate and needs measurement/review.
- `cells`: `(s32)s.header + 0xC` violates R9(b)'s explicit no-cast shape.
  Header/cell layout is supported by the reader, but the candidate still
  needs complete R9 receipts; sibling precedent is not admission.
- `i`: counter/level reuse still lacks the full R11(D) necessity proof and
  required alternatives. Measured allocator effects alone are insufficient.
  The single-letter name and missing declaration-site annotation also fail E/F.
- `mode`: potential named-local exception member, but missing site annotation
  and complete documented lever exhaustion/pass evidence.

Additional reviewer requests:

- Ablate the WHOLE `next`/`k` cluster to `rec[row]` and `1-row` together.
- Split the tile-loop and gauge-loop record-pointer roles and measure.
- Review context argument typing against actual heterogeneous pointer fields;
  represent the descriptor's known color bytes explicitly.
- Rectangle/color record and POLY_G4 layouts agree with inspected accesses;
  no concrete fabricated-padding defect found there.

Author response: s7 removes `col`, the whole `next`/`k` cluster, and tile/gauge
`rec` reuse; types header/cells as byte pointers; exposes descriptor color bytes.
It measures 41/622 with two chained color assignments. See evidence.md and
probes/s7 for individual and combined measurements. **The response is not a
review PASS.** i/cells/mode admission evidence and the binary match remain open.
This review cannot transfer to a later body or be cited as a completion approval.
