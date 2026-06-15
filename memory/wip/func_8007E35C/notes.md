# func_8007E35C (display.c) — GTE gpf/gpl lerp wrapper, needs canonical-asm authorization

## TL;DR
Pure GTE/cop2 wrapper, NO C form. `canonical` gate = **ASM-PARTIAL, 11/24 insns
cop2** (gpf op 0x4B90003D, gpl op 0x4BA0003E, mtc2, mfc2). Already byte-matches
with its `__asm__(.word ...)` body (prior match commit cd95841e). The ONLY
remaining work is formal **canonical-asm authorization** (add to
`inline_asm_canonical.txt`) — a worker cannot make that call; flagged for
orchestrator/user. Same situation as func_8007F1A8 (also blocked this session).

## Why it has no C form
Body = lbu byte-pair from *a0 -> mtc2 a2/$t0/$t1 to GTE data regs $8/$9/$10 ->
nop (GTE delay) -> gpf 0 (general-purpose interpolation) -> mfc2 $v0,$31 (flag) ->
lbu byte-pair from *a1 -> mtc2 a3/$t0/$t1 to $8/$9/$10 -> li $t3,12 -> gpl 0 ->
lw $t5,0x10(sp) (5th arg `out`) -> mfc2 $t0/$t1 from $25/$26 (MAC results) ->
srav >>12 -> sb byte-pair to *out -> return v0 (the flag). Every compute insn is
a cop2 op with hardcoded GTE register numbers the compiler cannot reach from C;
the lbu/sb/srav/lw are mechanical byte packaging. The `register s32 v0 asm("v0")`
pin captures the mfc2 $31 flag for the return — standard for these GTE-flag
wrappers.

## Established auto-authorize family (8007Exxx gpf/gpl cluster)
Direct siblings, all matched the same `.word`/__asm__ way but likewise NOT yet in
the registry: func_8007E2D4 (gpf/gpl sf=0), func_8007E24C (gpf/gpl lerp),
func_8007E3BC (gpf/gpl byte lerp + srav>>12). All from the "structural-split
func_8007E1AC into 6 GTE helpers" commit (311c0c6d). func_8007E8AC (a sibling
mvmva wrapper) IS already authorized (inline_asm_canonical.txt:257).

## Ready-to-paste registry line (for the authorizer)
```
func_8007E35C  # GTE gpf/gpl byte-pair lerp wrapper (display.c): lbu pair from *a0 -> mtc2 a2/$t0/$t1 to $8/$9/$10 -> nop -> gpf 0 (interp) -> mfc2 $v0,$31 (flag) -> lbu pair from *a1 -> mtc2 a3/$t0/$t1 -> li $t3,12 -> gpl 0 -> lw out(arg5) from 0x10(sp) -> mfc2 $t0/$t1 from $25/$26 (MAC1-2) -> srav >>12 -> sb pair to *out -> return flag ($v0). Pure cop2 ops with hardcoded GTE register numbers + mechanical byte packaging, no C form; v0 pin captures the mfc2 $31 flag for the return. 8007Exxx gpf/gpl cluster, sibling of authorized func_8007E8AC. Auto-authorize per gte-wrapper-misroute-park.
```
(Twin func_8007E2D4 / func_8007E24C / func_8007E3BC need analogous lines — handle the cluster together.)

## Status
Source at HEAD, already byte-matches (oracle green). Card BLOCKED pending
canonical-asm authorization. Per CLAUDE.md this is an orchestrator AUTO-HANDLE
(GTE leaf wrapper), outside a worker's authority.
