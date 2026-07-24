# Evidence bank — func_80017FA0

## s1 (recon, 2026-07-24) — FLOOR 13 -> 2. WIP "no pure-C fold" claim FALSIFIED.

- [s1] Baseline honest floor = 13 (sandbox --disable all on HEAD cheat-form; build 59 vs target 61 insns). Old WIP candidate.c (indexed `((volatile s32*)0x1F800000)[idx]` form) = 16, WORSE. NB the queue/WIP floor of 13 was HEAD's cheat-form honest distance, not a pure-C candidate.

- [s1] The 13 diffs decompose (full objdump alignment, tmp/grind/func_80017FA0/s1/build_disasm.txt): the inner double-loop (21 insns), outer tail (11), loop-setup (4), blez region — all BYTE-IDENTICAL to target already. The 13 differ split into: (a) 0x1F800060 fixed store = 3-insn `lui;ori;sw 0(r)` vs target 2-insn folded `lui;sw 0x60(r)` [3 diffs, bottom, isolated — no reg cascade]; (b) 0x1F8000B8 fixed store same 3-vs-2 fold [top]; (c) v0<->v1 register cascade on temp/ptr[0]/sll [5 diffs] — DOWNSTREAM of (b): the early delay-slot lui materializing the B8 address in v0 forces temp into v1; (d) the empty 8-byte frame [2 diffs].

- [s1] DECISIVE fold mechanism (probe tmp/grind/func_80017FA0/s1/foldprobe.*, cc1 -O2 -G0 -funsigned-char -mcpu=3000 then maspsx 2.34 + as + objdump): a raw `*(volatile s32*)0xCONST = x` (P1) synthesizes the address into a reg (`lui;ori`) then `sw x,0(reg)` = 3 insns NO fold. Routing the SAME store through a pointer-typed lvalue — local pointer var `scr[idx]` (P4) OR struct-field cast (P5) — makes cc1 emit `sw x,<const>` which the assembler folds to `lui at,0x1f80; sw x,0x60(at)` = 2-insn folded = TARGET bytes. P2/P3 (extern symbol) also fold but leave R_MIPS_HI16/LO16 relocs (needs a linker symbol; unnecessary).

- [s1] APPLIED to real fn: replaced the two fixed writes `*(volatile s32*)0x1F8000B8` / `0x1F800060` with `scr[0x2E]` / `scr[0x18]` where `volatile s32 *scr = (volatile s32*)0x1F800000;` (removed the two cheat asm barriers). sandbox --disable all = 2 (build 60 vs target 61). Closed 11 of 13 diffs — BOTH folds AND the entire v0<->v1 cascade AND one delay slot — in one pure-C edit. Saved to memory/grind/func_80017FA0/candidate.c. src reverted clean (not candidate-ready).

- [s1] canonical-asm signal NEGATIVE: scan_hand_coded --single = tier=LOW score=1/8 (only S3 no-spills fires; ordinary GCC-scheduled fn). So the 8-byte frame came from a C-source local decl, NOT hand-written asm.

- [s1] RESIDUAL (distance 2) = the empty 8-byte leaf frame: target `addiu sp,-8` (in the beqz delay slot) ... `addiu sp,8`, ZERO frame stores. Frame-trigger probe (tmp/grind/func_80017FA0/s1/frameprobe.*): F0 no-local=vars0; F1 dead SCALAR=vars0 (scalar->reg/DCE); F2 dead 8-byte array `s32 buf[2]` unused=vars8 with ZERO sw/lw to $sp (EXACTLY target's residual); F3 written array=vars8 but emits sw stores target lacks. => the ONLY pure-C producer of target's zero-store 8-byte frame is a dead <=8-byte local ARRAY = forbidden dead-vars-local-array. WRITTEN-array carve-out inapplicable (target has ZERO frame stores). Same class + shape-of-argument as sibling AddTbpOfst_80047EE8 (owner-escalated, awaiting ruling) and endgame-lock-disposition.

- WIP rejected_form: {'form': 'idx_base: all scratchpad writes via ((volatile s32*)0x1F800000)[idx], early `return`', 'score': 33, 'reason': 'LICM-hoists the inner-loop lui 0x1f80 out of the loop (target re-materializes it each iteration); also drops the frame. Worse than hybrid.'}

- WIP rejected_form: {'form': 'HEAD committed form (6 regfix rules + asm barriers)', 'score': 0, 'reason': 'byte-matches but ONLY via cheat-regfix (scratchpad fold subst/delete) + cheat-asm (_frame dead-frame barrier, val coercion barrier). inline-asm-policy + scratchpad-gte ARCHIVED forbid it.'}

- == imported from memory/wip notes.md ==
# func_80017FA0 (code6cac.c) — WIP / BLOCKED (canonical-asm candidate)

## TL;DR
HEAD byte-matches but ONLY via **6 regfix subst/delete rules** + **asm
barriers** (a `_frame` dead-frame coercion and a `val` `"0"(val)` barrier). Both
are cheats (inline-asm-policy expanded catalog; the ARCHIVED [[scratchpad-gte]]
rule forbids this exact recipe). Honest pure-C floor measured at **16** (full
build, rules neutralized) — and the residual 16 is **two irreducible non-pure-C
gaps**, so this is a **canonical-asm authorization** function, not pure-C-closable.

## What matches in pure C (the inner loop — the bulk)
Keep the inner scratchpad writes as **full-constant addresses**
`*(volatile s32 *)(0x1F800064 + sp_inner) = ...`. That makes GCC re-materialize
`lui 0x1f80` every iteration, which is what the target does (the target does NOT
LICM-hoist the scratchpad base out of the loop). The whole inner double-loop then
matches byte-for-byte. See candidate.c.

## The two irreducible gaps (why it's not pure-C)
1. **Constant-store-address fold.** The two FIXED writes `0x1F8000B8` (=`ptr[0]<<7`)
   and `0x1F800060` (=`a0[3][1]`): target emits `lui at,0x1f80; sw v0,offset(at)`
   (2 insns, low 16 bits folded into the sw displacement). GCC 2.7.2 emits
   `lui r,0x1f80; ori r,r,0xb8; sw v0,0(r)` (3 insns, no fold). `((volatile s32*)
   0x1F800000)[idx]` folds *inconsistently* (folded in the all-index variant,
   unfolded in the hybrid — it's register-pressure-sensitive, not reliable). The 6
   regfix `subst`/`delete` rules manufacture the fold. No pure-C idiom reliably
   produces it (confirmed across this + the cpu_check_tubazeri scratchpad family).
2. **Unused 8-byte frame.** Target does `addiu sp,sp,-8` … `addiu sp,sp,8` with
   **nothing stored to the frame**. GCC emits no frame for this body. HEAD forces
   it with `s32 _frame; asm volatile("" :: "m"(_frame))` — a dead-var + barrier,
   forbidden by the expanded catalog.

Both are hallmarks of hand-written asm (or a different cc1 fold behavior).

## Endpoint
Per [[scratchpad-gte]] path (b): authorize as **canonical-asm** (user decision —
outside worker scope). Blocked on the board with this reason. 24 prior commits
(2026-03/04) never reached a clean pure-C match — consistent with no pure-C path.


- [s1] Baseline honest floor 13 (HEAD cheat-form, sandbox --disable all); old WIP candidate.c indexed form scored 16 (worse).

- [s1] The inner double-loop (21 insns), outer tail (11), loop-setup (4) already byte-match target; all 13 diffs were the 2 fixed-store folds + the fold-induced v0/v1 cascade + the 8-byte frame.

- [s1] Pure-C pointer-lvalue store (scr[idx], volatile s32 *scr = (volatile s32*)0x1F800000) reproduces the target's 2-insn folded lui;sw disp form for BOTH fixed writes; sandbox --disable all = 2, no rules, no cheat asm.

- [s1] Sole residual (distance 2) is the empty 8-byte frame; only a dead <=8-byte local array reproduces it (F2), which is forbidden dead-vars-local-array with no applicable carve-out.

- [s1] canonical-asm signal NEGATIVE (scan_hand_coded tier=LOW 1/8) — ordinary compiled C, so a C source produced the frame; not a hand-asm authorization case.

- [s1] Same zero-store phantom-frame residual class as sibling AddTbpOfst_80047EE8 (OWNER-ESCALATION filed 2026-07-20, awaiting ruling) and InitHiraRmd_80047FBC.
