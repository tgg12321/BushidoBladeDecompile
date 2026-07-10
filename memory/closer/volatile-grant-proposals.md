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

## §2 — _spu_RQ pending-key family (SpuSetKey close, 2026-07-10 session 5)

Symbols (Sony names from PsyQ 4.0 LIBSPU.LIB S_SK relocs, psyq_lib.py):

| symbol | Sony name | proposed decl |
|---|---|---|
| D_800F7420 | `_spu_RQ` | `extern volatile u16 D_800F7420[4];` (MERGED — subsumes D_800F7424; S_SK relocs prove one object, addends 0/2/4/6) |
| D_800A28A0 | `_spu_RQmask` | `extern volatile s32 D_800A28A0;` (already volatile at HEAD, ungranted debt) |
| D_800A2CD4 | `_spu_env` | `extern volatile s32 D_800A2CD4;` (already volatile at HEAD, ungranted debt) |
| D_800A289C | `_spu_RQvoice` | `extern volatile s32 D_800A289C;` (NEW volatile) |

- **Use-site shape:** SpuSetKey (func_8008AAD4) does ordered
  pending-queue stores + re-read-after-test clears (`if (RQ[2]&m) RQ[2]&=~m` —
  target re-loads between test and clear); volatile is load-bearing for the
  strict store order and the re-reads (sandbox-stripped build collapses them:
  score 63).
- **IRQ/consumer writer:** the queued requests are flushed by the SPU event
  processor `coli_HitPauseKatana_2` @0x80089A48 (asm: RMWs D_800A28A0
  clear-bits at 0x80089B3C/0x80089C04/0x80089CC0, gated on `_spu_env & 1` at
  0x80089A50) — the flush runs from the SsSeqCalledTbyT/VSync tick path, i.e.
  asynchronous to SpuSetKey callers. Same Sony-declared-volatile class as the
  granted D_800A2D14 (_spu_transferCallback) precedent.
- **Candidate:** memory/closer/candidates/spusetkey_v40_proven.c — standalone
  masked-identical incl. registers; residual diffs are merged-symbol reloc
  addends only (byte-identical after link).
- **Also needed to land:** replace the two split externs in src/main.c with the
  merged `D_800F7420[4]`; respell `D_800F7424[i]` → `D_800F7420[2+i]`
  (main.c is the only referencing TU; undefined_syms_auto.txt entry can stay).
