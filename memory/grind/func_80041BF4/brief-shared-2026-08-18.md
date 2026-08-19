# Shared findings across the 13 asmfix carriers (read first)

## 0. Scanner result: ALL 13 ARE TIER=LOW. No canonical-asm grant is available for any of them.

I ran `python tools/scan_hand_coded.py --single <func>` (static, reads `asm/funcs/*.s` only —
no compile, no subprocess, no writes; verified by reading the source) on all 13 plus the
in-progress func_80017848. **Every one returns `tier=LOW`, score 0-2/8, "no strong hand-coded
indicators".** Zero S1/S2/S6/S7/S8 hits anywhere in the set; the only signals firing are S3
(no spills) and S4 (front-loaded loads), both of which are the weak "tightness" tier and cannot
produce STRONG on their own.

| func | score | signals firing |
|---|---|---|
| func_8008C1E8 | 1/8 | S4 |
| func_8001B748 | 0/8 | — |
| func_8002D518 | 2/8 | S3, S4 |
| func_80060A68 | 1/8 | S4 |
| func_80041BF4 | 0/8 | — |
| func_800283D0 | 1/8 | S4 |
| func_800393C8 | 2/8 | S3, S4 |
| func_8002E6B0 | 1/8 | S4 |
| func_800335D8 | 0/8 | — |
| func_8003EB84 | 1/8 | S3 |
| func_80070C70 | 1/8 | S4 |
| func_8002CA8C | 0/8 | — |
| func_80056CB8 | 1/8 | S4 |

Consequence: the Tier-5 "scanner-first" instruction in `tmp/regfix_campaign_map.md` §5 is
DISCHARGED for this whole set. Under [[judge-sole-gate]] the canonical-asm grant path needs
STRONG (S1/S2/S6) evidence; there is none. **All 13 are pure-C problems. Open pure-C-first on
every single one.** Do not spend another session on scanner evidence for these functions.
(`engine/queue.json` already records `hand_coded_tier: LOW` for 7 of them, consistent with this.)

## 1. The 29 asmfix rules split into two classes with completely different retirement economics

| class | what the rule does | can it retire before the function reaches distance 0? | rules |
|---|---|---|---|
| **BLOB** | replaces a whole body or a body region with literal asm | **No** — the C must reach 0 over the blobbed span first | 23 |
| **LABEL-PATCH** | inserts a stable label / retargets one branch; the body is still compiled C | **Yes** — retire by fixing the cause of the label drift | 6 |

BLOB: func_8008C1E8(1), func_8001B748(1), func_800283D0(1), func_80060A68(2), func_800393C8(2),
func_8002E6B0(2), func_8003EB84(2), func_8002CA8C(2), func_80070C70(4), func_80056CB8(4),
func_80017848(2, in progress).
LABEL-PATCH: func_8002D518(2), func_80041BF4(2), func_800335D8(2).

**The 6 LABEL-PATCH rules are the campaign's cheapest asmfix retirements** and they do NOT require
the carrier to reach COMPLETED-C — they only require the label drift they compensate for to stop
happening. func_80041BF4 is the standout: its banked `candidate.c` already fixes the m2c artifact
that both of its rules exist to paper over (see its brief).

## 2. Anchor fragility decides whether incremental C progress is committable

`asmfix delete_between` deletes INCLUSIVELY from the start anchor. If a C improvement moves or
deletes the anchor instruction, the delete silently mis-fires and the body duplicates — the
failure mode that cost the func_800393C8 session (+452 bytes, caught only by the oracle rebuild;
commit 2f20c593).

| anchor kind | fragility | carriers |
|---|---|---|
| `replace_with_asmfile` — no anchor at all | **ROBUST** | 8008C1E8, 8001B748, 800283D0 |
| `.frame` / `.end <func>` — assembler directives emitted for every function regardless of body | **ROBUST** | 8002E6B0, 8003EB84, 8002CA8C (+80017848) |
| literal body instructions (`lh $8,0($4)`, `lhu $4,0($3)`, `lh $2,0($16)`, `beq $3,$21,.L#`) | **FRAGILE** | 800393C8 (proven broken), **80060A68 (same shape, never flagged)**, 800335D8 |
| cc1 label SLOT (`{lbl#N}`) plus a literal instruction | **FRAGILE (slot-ordinal)** | 80070C70, 80056CB8, 80041BF4 |

For the six ROBUST carriers, improved-but-incomplete C is oracle-safe and committable — exactly
what commit 2f20c593 did for E6B0/EB84/17848. **For the seven FRAGILE carriers, plan "re-anchor or
retire the blob IN THE SAME CHANGE" from the first edit**, or the session ends the way 393C8's did.
Slot-ordinal anchors additionally shift whenever the C adds or removes a label BEFORE the
referenced slot, so any loop-shape change to 70C70/56CB8/41BF4 renumbers `{lbl#N}`.

Failure modes differ and that matters for how loudly you find out: a broken `delete_between`
duplicates bytes silently (oracle-only detection), while a broken `insert_before` label leaves an
undefined label reference and fails at link (loud, cheap).

## 3. Distance is NOT comparable across this set — three are measured against scaffolding

`sandbox --disable all` strips cheat-asm, so the number is honest for whatever C is in the tree.
But the C in the tree is not always an attempt at the function:

- **func_8002CA8C (d177): the C body is an EMPTY STUB** —
  `void func_8002CA8C(u8*, s32, s32) { (void)a0; (void)a1; (void)a2; }` at
  `src/code6cac_b.c:1020`. Its 177 is "no C written", against a 179-insn target. The real starting
  work is writing the body, not closing a gap.
- **func_80056CB8 (d188): the loop body is deliberate SCAFFOLDING** (`src/text1b.c:1763-1777`) — a
  `__asm__ volatile("" : : "r"(var_fp) : "$16".."$21","memory")` clobber plus zero-stores whose own
  comment reads "Stub keeps GCC saving callee-saves and allocating struct stack slots". The 188 is
  against that scaffold.
- **func_800393C8 (d57): the queue number is stale.** The banked candidate measures **26**
  (`memory/wip/func_800393C8/candidate.c`); the C in tree still carries the old register pins and
  the fabricated `if ((u32)&frame_pad[0] == 0)` frame hack.

The other ten have real, complete C bodies and their distances mean what they say.

## 4. The code6cac_b trio (func_800283D0 / func_8002E6B0 / func_8002CA8C): shared ORIGIN, not shared cause

Origin dates from `git log -S` against `asmfix.txt`:

- func_8002E6B0 = **pad_main_control**, blobbed in `6a49a1c3` **2026-04-17** — the oldest asmfix blob in the file.
- func_8002CA8C blobbed in `bbce2bad` **2026-04-27** ("Match func_8002CA8C").
- func_800283D0 = **saTan2KabutoWareMove**, blobbed **2026-04-27**.

2026-04-27 is the zero-stub-completion date. **All three are pre-engine, "match by any means"-era
artifacts that happen to share a TU** — there is no shared codegen phenomenon. Their documented
residuals are unrelated: 283D0 is a `lhu`/`andi` combine fold plus a prologue s-register rotation;
E6B0 is one centroid scheduling decision driving a caller-saved rename cascade; CA8C has no C at
all. **One scanner pass does not answer for all three** (I ran all three: LOW/LOW/LOW), and neither
will one C lever. Treat them as three independent functions.

The genuine cross-carrier cluster is elsewhere, and it is a POLICY cluster rather than a TU one:
**func_8002CA8C, func_80070C70, func_80056CB8, func_80060A68 and func_80017848 were all lifted to
asm between 2026-05-09 and 2026-05-12** under the "asmfix-slice clause" of
`memory/feedback_voluntary_stop_forbidden.md` — a policy that permitted "structurally-correct C +
register-allocation plateau" to be lifted to asm after roughly 120 minutes of effort. **That memory
file no longer exists in the repo**, and the clause is superseded by the completion standard and
[[asmfix-all-debt-end-state]] (owner ruling 2026-08-06: every asmfix entry is debt). Their comments
in `asmfix.txt` still cite it. Nobody should read those comments as "already judged unclosable" —
the judgement was made under a rule that no longer exists, on a 120-minute budget, before the
engine, the RA solver, the scheduler solver and the LICM instrument existed.

## 5. Real cross-carrier codegen cluster: the prologue arg-home / callee-save-order wall

func_8008C1E8 and func_800283D0 have the SAME documented residual — target saves one callee-saved
register out of the normal order and homes an argument into it immediately at entry, while GCC
sinks the copy past the first test, cascading an s-register rotation through the body. Both WIP
notes name it, and both name it as the known-hard class ([[no-new-park-categories]], the
func_8007C2A0/C4B8 twins). func_80041BF4's banked residual (29) is the same family expressed as a
3-cycle s2/s3/s4 rotation, and is documented as modelled-exhausted with every route closed.

**If any session ever cracks the arg-home ordering, re-measure all three immediately.**
Conversely, do not OPEN any of the three by attacking the rotation — it is measured dead on 41BF4
and unmoved on 8C1E8/283D0.

## 6. Two unsanctioned pad locals are sitting in this set

`volatile s32 _frame_pad[2]` at `src/text1b.c:3333` (func_80060A68) and `s32 frame_pad[2]` plus
`if ((u32)&frame_pad[0] == 0) { D_800A3209 = D_800A3209; }` at `src/code6cac_c_mid.c:1270/1278`
(func_800393C8). The dead-pad carve-outs in [[dead-vars-local-array]] require a frame-math proof, a
range annotation, exhaustion and dual review; the 2026-08-17/18 pad rulings are explicitly scoped
to func_8003D9A0 / func_8003DBE4 / func_8003CF84 and do not generalize. Neither of these carries
the required apparatus. Treat both as debt to delete during the strike, not as a lever. (The 393C8
banked candidate already deletes its copy.)
