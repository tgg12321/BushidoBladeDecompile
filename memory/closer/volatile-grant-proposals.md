# Volatile-grant proposals (owner-gated; audited between sessions)

Per the Closer hard rails: agents NEVER edit volatile_extern_allowlist.txt.
Each entry below carries the full two-prong evidence for an `extern volatile`
grant. The operator audits and appends ratified entries to the allowlist.

## Proposal 1 — LIBCOMB SIO async-transfer state (5 symbols) — 2026-07-10

**Symbols:** `D_800F1B00`, `D_800F1B04` (SioAnsyncRead buf/len),
`D_800F1AF0`, `D_800F1AF4`, `D_800F1AF8` (SioAnsyncWrite buf/len/CTS-state)

**Blocked functions:** func_8008BEA4 (SioAnsyncRead), func_8008C184
(SioAnsyncWrite) — both rule-free, both currently matched only via the
prohibited `if ((flag && flag) && flag) { do { } while (0); }` wrapper.
Proven candidate: memory/closer/candidates/sio_ansync_pair_volatile.c
(standalone masked-identical vs build/src/main.o, registers included).

**Prong (a) — IRQ writer (file:line-class citations):**
The SIO interrupt service path is `HandleSio` (Sony static, 0x8008C9F4,
inside asm/funcs/func_8008C464.s — census: LIBCOMB/COMB module, verbatim
Sony PsyQ 4.0 object; symbol map tmp/libscan/symbols.txt). Write/mutate
sites in asm/funcs/func_8008C464.s:
- D_800F1B04: `sw $zero` @ 0x8008CA54 (line 403-404); cleared @ 0x8008C880
- D_800F1B00: read-advance-store @ 0x8008CBEC-0x8008CC00 (lines 512-517);
  cleared @ 0x8008CE5C (line 673)
- D_800F1AF0: read-advance-store @ 0x8008CD18-0x8008CD2C (lines 590-595)
- D_800F1AF4: `sw $zero` @ 0x8008C840 (lines 261-262)
- D_800F1AF8: written in the same handler window (CTS/DSR latch)
The handler is hooked into the kernel interrupt chain by AddCOMB
(0x8008BE04, census XDEF) — the standard LIBCOMB SysEnqIntRP-style hook.

**Prong (b) — use-site shape:**
- SioAnsyncRead/Write store len, buf, THEN set the busy flag and THEN
  enable the SIO IRQ (`ctrl |= 0x800 / 0x400`) — target keeps the stores
  strictly source-ordered (sw a1 B04 before sw a0 B00 before the ctrl
  load); our scheduler otherwise reorders them (measured diff-4, the a1
  store drifts past the ctrl load). Ordered-stores-before-IRQ-enable is
  the textbook produced-by-volatile shape and is required for
  correctness (enabling the IRQ before the state is complete races the
  handler).
- The Syncro siblings (cpu_side_move_dir_3, SetPacketData) poll the same
  words in loops (`loop_flag[1] += 1; loop_flag[2] -= 1;` re-read across
  iterations) — IRQ-mutated-loop-bound shape.

**Ground truth:** whole COMB module is a verbatim-linked Sony PsyQ 4.0
object (memory/closer/psyq-library-census.md, 0x8008BE04-0x8008D050).
Sony's libcomb declares its async state volatile (no public C source, but
the emitted strictly-source-ordered stores across all four entry points
are only reachable with volatile under GCC 2.7.2 — measured this session:
plain/non-volatile forms reorder deterministically).

**Sandbox note for the auditor:** the sandbox strips un-granted volatile
(cheat-invisible), so these candidates score 4 in-sandbox until granted;
the standalone pipeline proof (tmp/closer/diffsa.sh) is the evidence.
