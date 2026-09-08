# Hypothesis ledger — note2pitch

## s1 (recon, 2026-09-08)
H1 — DECLARATION FIX (mandated hypothesis #1): D_800A26E4 as a merged member of
  g_alarm_active_sentinel. NOT NEEDED — measured: the standalone `extern u16 D_800A26E4[]`
  already gives sandbox 0 (see evidence OBJECT MODEL entry). Verdict: CONFIRMED that the
  existing standalone declaration is the correct object model; no merge.
H2 — psyz PsyQ 4.0 `note2pitch` body (vm_n2p.c) is the source, with `_svm_cur.note /
  tone_center / tone_shift` for the three byte loads. CONFIRMED: sandbox 0 on the first
  measurement, verify-oracle ok. Form banked as candidate.c.
Frontier: empty — function is at distance 0; awaiting driver byte re-verify + Judge.
