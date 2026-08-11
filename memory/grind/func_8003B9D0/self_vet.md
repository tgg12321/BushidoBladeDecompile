# SELF-VET — func_8003B9D0  (STALE / VOID — do not reuse)

The self-vet that stood here belonged to the sandbox-0 body whose D3 construct
(`p = (u8 *)&eda[0x226]; saved_44c = *(s16 *)p;`) the layer-1 cheat-reviewer
FAILED and the driver has since BANNED for this function under any spelling.
That body is preserved verbatim at
`rejected/p-staging-layer1-cheat-banned.c`; its vet is void and must NOT be
re-submitted or adapted.

The forensics session (2026-08-11) confirmed the FAIL on mechanism: the
construct's only effect is to keep an otherwise-dead address-computation insn
alive past `delete_dead_from_cse` (cse.c:8683) so that cse2 makes a different
`find_best_addr` decision, and `flow` deletes that insn again before any byte
is emitted (evidence.md / hypotheses.md H5).

CONSTRUCTS: none — the current `candidate.c` is the plain body (zero `__asm__`,
no derived-address staging) at honest floor 6.  A fresh self-vet must be
written from scratch by whichever session reaches distance 0 with a legal form.
