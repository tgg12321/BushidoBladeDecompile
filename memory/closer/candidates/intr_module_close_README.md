# intr_module_close.patch — func_80082D34 (LIBETC INTR module) 0/297 bit-exact

Session 12 (2026-07-10). Replaces the func_80082D34 placeholder in src/ings2.c
with the full Sony LIBETC intr.c v1.76 module tail: trapIntr (func_80082D34) +
static setIntr + static stopIntr + static restartIntr + memclr (func_800831A4),
AND adds the two LIBAPI C114 object header words to the bios_CdRemove_A0
canonical asm block.

## Proof (re-runnable)

    bash tools/wsl.sh 'bash tmp/closer/intr_prove.sh'
    -> == .text region 297 words vs EXE: 0 differing     (0x80082D34 + 0x4A4)

- 295/295 words of the five compiled functions bit-exact vs disc/SLUS_006.63
  (relocation-resolving link_sim, zero masking).
- +2 words = the C114 header words (see below).
- TU head-parity: per-chunk cc1-output diff HEAD-vs-patched = label
  renumbering only; no other ings2.c function's emission changes.

## The one non-obvious lever

Sony's trapIntr computes the interrupt mask as an ASSIGNMENT-IN-CONDITION:
`while ((mask = (enabled & *i_stat) & *i_mask) != 0)` — this produces the
target's compute-into-$v0 + `beqz $v0` + `addu $s0,$v0` delay-slot copy at
both the loop entry and the rotated bottom test (a separate
`mask = ...; while (mask)` spelling allocates straight into $s0 and loses
the copy; measured 21 differing words).

## BLOCKED ON (owner): canonical-asm block edit

The last 8 bytes of the splice extent (0x800831D0/D4 = 0x15007350 0x0040809C)
are NOT compiler-emittable. Ground truth (tmp/libscan/psyq40/LIBAPI.LIB,
module C114, via tmp/closer/psyq_lib.py): C114 .text is 32 bytes; XDEF
_96_remove at .text+0x8; bytes +0..+7 are exactly these two words (zero
relocs); +0x8..+0x1F is exactly the already-authorized bios_CdRemove_A0
trampoline (addiu $t2,0xA0; jr $t2; addiu $t1,0x72; nop;nop;nop). I.e. the
two words are Sony's hand-written C114 object header data — same module as
the authorized trampoline. The patch prepends them as `.word` lines (with a
provenance comment) inside the bios_CdRemove_A0 canonical block. This is a
content change to an authorized canonical-asm entry => owner-gated; proposal
filed in memory/closer/volatile-grant-proposals.md.

## Replay mechanics (driver/next session, after owner sign-off)

1. `git apply memory/closer/candidates/intr_module_close.patch`
2. Strip asmfix.txt:58 (`func_80082D34: replace_with_asmfile ...`) in the SAME
   commit — the splice double-emits against the new C + the extended block;
   the rules-on tree is unbuildable until the rule is gone (DispStuff /
   tslTm2LoadImage_2 precedent).
3. Full build -> SHA1 == oracle is the gate; sandbox on func_80082D34 reads
   an extent-split artifact until the reference rebuild (statics get local F
   symbols; self-heals — S_SCA precedent).

## Cheat-policy notes (for the reviewer)

- Volatile surface: `*(volatile u16 *)g_sys_irq_counter` casts (committed
  precedent in startIntr, MMIO computed-address class) + the existing
  HEAD decls `extern volatile u16 *D_800A2604` / `extern volatile s32
  *D_800A260C` (MMIO pointer statics, type-level). No new grants.
- No FAKE constructs, no dead code, no do-while wraps. Pure SOTN intr.c
  transcription; the only deviation from SOTN v1.73 is the
  assignment-in-condition mask loop (matches the v1.76 object) and
  BB2's 11-slot handlers table (same as SOTN).
- Statics are unreferenced in the TU (vtable holds raw addresses in
  7D920.data.s) — GCC 2.7.2 still emits them (SpuRGetAllKeysStatus
  precedent).
