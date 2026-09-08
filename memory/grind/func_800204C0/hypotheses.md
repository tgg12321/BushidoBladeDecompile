# Hypothesis ledger — func_800204C0

## s1 (2026-09-08, recon)
- H1 CONFIRMED: the func_800203B4-granted island spelling (move $12,%0 macro bodies) reproduces
  all 25 island insns byte-exact here (same three $t4 preambles, same 0x4A486012 command).
- H2 CONFIRMED: head `*p += 1; if ((*p & 7) == 2)` (memory read-modify-write + re-read) yields the
  `move $v1,$v0` delay-slot copy AND the 8-byte phantom frame slot (vars=32). Sandbox 0.
- H3 KILLED (instance): head `cnt = *p + 1; *p = cnt; if ((cnt & 7) == 2)` with an s32 cnt local —
  sandbox 10 (vars=24, no copy). rejected/head-local-cnt-frame24.c.
- Open: none on codegen — residual is 0. The remaining step is the operator registry row
  (tools/grinder/owner_cluster_grants.txt) per the func_80019310 precedent; not a session surface.
