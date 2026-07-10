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

## Proposal 3 — LIBCD cdread.c module state, per-member view (12 symbols) — 2026-07-10

**Symbols:** `D_800A14D4` (buf), `D_800A14D8` (p), `D_800A14DC` (mode),
`D_800A14E0` (size), `D_800A14E4` (cnt), `D_800A14E8` (t2), `D_800A14EC` (t1),
`D_800A14F0` (pos), `D_800A14F4` (cbsync), `D_800A14F8` (cbready),
`D_800A14FC` (cbdata), `D_800A1500` (tslmode).

**Class:** Ruling-4 (ground-truth codegen), plus these are the SAME Sony
object the operator already granted as the block symbol `D_800A14D0`
(allowlist line 32, "Sony's source declares it volatile", CDREAD.OBJ verbatim
census) — this proposal only extends the existing grant to the per-member
symbol names of the identical memory.

**Why the per-member names are needed (measured, cc1psx-confirmed):** the
volatile-STRUCT spelling la-materializes the first member access per BB
(3-insn `la;lw` — combine refuses address substitution into volatile mems;
cc1psx emits the identical la-form, so Sony's source did not spell these
sites through the struct). Sony's cdread.c compiles per-member volatile
globals: zero-offset macro-form accesses, order pinned, RMW re-reads. With
the per-member volatile decls the whole 263-word region
0x80082000..0x8008241C (puts + cb_read + cb_data) is BIT-EXACT vs the EXE
(memory/closer/candidates/cdread_triple_README.md; 0/263 via link_sim, no
masking). Non-volatile per-member decls collapse the ordering (scheduler
commutes reads) and the cnt--/pos++ re-read sequences.

**IRQ writer (carried over from the D_800A14D0 grant):** the CD-ready ISR
callback IS cb_read itself (D_80082050, installed via
cdrom_SetCallbackB(&D_80082050), src/system.c) — the block is mutated from
interrupt context and read by CdReadSync spin-waits.

**Blocked function:** tslTm2LoadImage_2 (= puts + the two cdread callback
statics under its splice extent; dist 261, 1 asmfix splice). Candidate:
memory/closer/candidates/cdread_triple.patch (byte-neutral saEft00Add
compensation included and verified — tmp/closer/head_cmp.sh).

## Proposal 4 — canonical-asm content edit: C114 header words into bios_CdRemove_A0 — 2026-07-10

**Not a volatile grant — a canonical-asm entry content change (owner-gated
per the Closer rails).**

**Blocked function:** func_80082D34 (LIBETC INTR module: trapIntr + setIntr +
stopIntr + restartIntr + memclr; dist 295, 1 asmfix splice). The five
functions are PROVEN 0/295 bit-exact in pure C
(memory/closer/candidates/intr_module_close.patch; prover
tmp/closer/intr_prove.sh -> 0/297 over the full 0x4A4 extent).

**Proposed edit:** prepend two data words to the already-authorized
`bios_CdRemove_A0` canonical `__asm__` block in src/ings2.c (included in the
banked patch):

    .word 0x15007350
    .word 0x0040809C

**Evidence:** ground truth tmp/libscan/psyq40/LIBAPI.LIB module C114 (parser
tmp/closer/psyq_lib.py): .text = 32 bytes; XDEF `_96_remove` at .text+0x8;
bytes +0x0..+0x7 are EXACTLY these two words (zero relocs); +0x8..+0x1F are
exactly the authorized trampoline (addiu $t2,0xA0 / jr $t2 / addiu $t1,0x72 /
3 nops). The words are Sony's hand-written C114 object header data occupying
0x800831D0..D7 in the EXE — the last 8 bytes of func_80082D34's splat extent,
not compiler-emittable from any C. Placing them at the head of the C114
canonical block is the honest attribution (same Sony object as the
trampoline).

**On sign-off:** apply intr_module_close.patch + strip asmfix.txt:58 in the
same commit (splice double-emits otherwise); full-build SHA1 is the gate.
No inline_asm_canonical.txt entry change needed (entry name unchanged).
