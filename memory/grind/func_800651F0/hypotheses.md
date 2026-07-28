# Hypothesis ledger — func_800651F0

## s1 (recon)
- H1 CONFIRMED: sibling-parallel transcription of COMPLETED-C func_8006517C (ap/bp
  walking pointers + p-reassign final) reaches sandbox 0. Probe: applied to src, sandbox
  --disable all → 0 (29/29). Admissibility unresolved (ruling-request).
- H2 KILLED: reviewer-clean direct final store `D_800F0D5C = *bp;` → 8. Load stays low
  + value in $v0; target hoists `lw $v1,0x8($v1)` above the flag stores.
- H3 KILLED (as a 0-form; kept as floor): t-reuse final load → 4. Fresh/reused temp
  pseudo gets $v0, target needs $v1 (base-register overwrite).
