# SOTN research: partial-use volatile delay-counter array (2026-07-10)

Pattern under review: `volatile s32 arr[N];` local where only `arr[0]` is
ever written/read; the remaining element(s) shape the stack frame. Proposed
for func_80082A14 (v_wait) and func_80078E58 (_Pad1) — candidates banked at
memory/closer/candidates/vwait_pad1_volatile_array.c, both measured
sandbox-0 AND full-oracle-SHA1-green in place this session.

## Evidence

1. **SOTN master, matched Sony source**: `src/main/psxsdk/libetc/vsync.c`
   `v_wait()` (static) declares `volatile s32 timeout[2];` and references
   ONLY `timeout[0]` (init `timeout[0] = max << 0xF;`, decrement
   `timeout[0]--`). `timeout[1]` is never written or read. This is a
   MATCHED function in sotn-decomp's psxsdk tree — Sony's own shipped
   source idiom for busy-wait delay counters, not a decomp fake.
2. **BB2 v_wait is the SAME Sony function** (LIBETC/VSYNC verbatim-linked
   module, census 2026-07-09): the identical C body reproduces BB2's bytes
   with zero fake constructs and eliminates HEAD's
   `asm volatile("" ::: "memory")` scheduling barrier (catalog cheat-asm).
3. **_Pad1** (LIBAPI/PAD verbatim Sony object): the same idiom with
   `volatile s32 i[3]` (frame 16) replaces HEAD's forbidden UNWRITTEN
   `s32 pad[2]` + volatile scalar. LIBAPI has no public C, but the module
   family (Sony 4.0 pad/vsync delay loops) and the emitted frame+volatile
   spin shape are the same authorship idiom.

## Why this is not self-ratifiable

`.claude/rules/dead-vars-local-array.md` (2026-07-01 carve-out) sanctions
only WRITTEN-never-read arrays; the unwritten-ELEMENT-inside-a-used-array
sub-case is not enumerated, and the rule's stated basis ("no SOTN analog in
matched code for unwritten forms") is CONTRADICTED by the vsync.c citation
above. Layer-1 cheat-reviewer (session 2026-07-10) FAILed both functions on
exactly this gap and directed an owner ruling (no-new-park-categories:
each exception clears its own SOTN evidence bar).

## Requested ruling

Sanction the narrow pattern: a `volatile` local ARRAY used as a delay/spin
counter where only element [0] is referenced, WHEN (a) the function is a
verbatim-linked Sony library object (census-proven), and (b) the reference
source (sotn psxsdk matched tree) ships the same shape for the same
function family. Annotation: provenance comment (already in the banked
candidates). If granted, both functions close honestly in one session.
