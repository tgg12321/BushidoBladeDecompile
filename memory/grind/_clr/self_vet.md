# SELF-VET — _clr
CONSTRUCTS: aggregate-merge declaration `typedef struct GpuDrEnv { u32 tag; u32 code[15]; } GpuDrEnv; extern GpuDrEnv D_800F1858;` (include/gpu.h) replacing splat's per-word D_800F1858..D_800F1888; 12 alias-suffixed rows in undefined_syms_auto.txt; ordinary-C body (psyz `_clr` transplant); two prototypes `void _cwc(u32); u32 _param(u32);`; the existing `extern volatile s32 *D_8009BF48;` hoisted above _clr; `GpuRect` typedef moved above _clr unchanged
## T1 semantic purpose: every store in the body writes a GPU packet word the DMA consumes; the clamps bound the rect to VRAM; the aggregate is the object the original code addresses by base+offset (`&code[8]` in $a2, stored through). No construct is byte-neutral filler; nothing is dead.
## T2 human-programmer: yes — it is Sony's own source shape (psyz sys.c:706-741, SOTN sys.c:567 spell the same body); a DR_ENV struct is how PsyQ declares this buffer.
## T3 GCC-internals justification: none needed — the aggregate is justified by the program's object model (one packet buffer, address-taken member), not by an allocator/scheduler mechanism. The per-word kill is explained by object identity, not by a lever.
## T4 permuter/search provenance: no permuter, no search; hand transplant from a version-correct library decomp.
## T5 family check: the only non-ordinary element is the per-word -> aggregate merge, a FROZEN sanctioned family (below). No pins, asm, volatile additions, dead stores, pads, do-while wraps, or casts beyond `*(s32 *)&rect->x` (the packet-word read Sony's source uses; ordinary C).
## T6 naming-announces-intent: names are `GpuDrEnv`, `tag`, `code`, `ptr`, `rect`, `color` — semantic; no pad/dummy/spill names.
SANCTIONED-FAMILY-CLAIMS:
  FAMILY: per-word splat symbol -> aggregate merge
  SCOPE: "two or more splat-invented `D_<addr>` scalars may be replaced by a single aggregate declaration."
  PRECEDENT: `.claude/rules/no-new-park-categories.md:238`
  PRECEDENT: `include/gpu.h:81`
  PRECEDENT: `cc2caa23a`
  PRONGS: (a) base+offset evidence in the ORIGINAL code, predating this session: asm/funcs/_clr.s materialises 0x800F187C (`lui $a2,%hi(D_800F187C); addiu $a2,%lo(D_800F187C)`), ANDs it into the tag word stored at D_800F1858, then stores 0x03FFFFFF through `$a2` (`sw $a3,0x0($a2)`) — one object reached through a base register; flagged independently by the 2026-08-18 psyz transplant index (memory/grind/_clr/psyz-transplant-index-2026-08-18.md:73). (b) declared as PsyQ's DR_ENV shape (tag + command words), no magic strides. (c) complete: no C names D_800F185C..D_800F1888 after the patch (grep src/ include/); the 12 rows stay in undefined_syms_auto.txt ONLY because asm/funcs/_clr.s still references them, each suffixed `/* alias of D_800F1858+0xN; retire with _clr (...) */` per the 2026-09-03 amendment. (d) header-canonical in include/gpu.h beside the landed GpuQueueItem `_que` merge. (e) no other C consumer exists (xref C:- for all 13); full-tree `verify-oracle --rebuild --allow-dirty` SHA1 == oracle.
ANNOTATION-CONFORMANCE: n/a — no FAKE construct (the aggregate-merge family mandates no annotation)
