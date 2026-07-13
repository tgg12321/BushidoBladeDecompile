# Naming Proposals -- High confidence

**High confidence**: deterministic pattern (BIOS jumptable, raw syscall, data-as-code, recognizable PsyQ stdlib idiom) OR a Kengo `name-unique` match with diff <=1 instruction. These are the lowest-risk renames; review and apply in small batches with SHA1 verification.

**Risk**: BIOS A0/B0/C0 lookups are deterministic from the trampoline shape; PsyQ memcpy/memset proposals are address-suffixed to avoid name collisions if multiple wrappers exist for the same primitive.

**Workflow**: copy the proposed name into `named_syms.txt`, run `make setup && make`, verify SHA1 unchanged, commit.

Total High (pending): **0**

_All 10 high-confidence proposals have been resolved as of the 2026-07-13 audit
(`proposals_audit_2026-07-13.md`): 7 applied as-proposed, 3 applied under a different
name (see the audit's APPLIED-DIFFERENTLY section). Resolved rows are catalogued in
`proposals_resolved.csv`. No high-confidence proposals remain pending._
