# func_8002DE20 — Ruling 11 (D) proof for `cross_a` and `cross_b` (2026-09-26, manual slotE)

Ruling: .claude/rules/ordinary-c-judge-decidable.md § Ruling 11 (owner 2026-09-26). Neither
variable is admitted by Rulings 5/6/8/9/10 (evidence.md "Role analysis": Ruling 5 fails 1(a)
sign-test consumer and 1(b) different expressions; extension (A) writes not identical; 6 not a
record pointer; 8 vmNoiseOn only; 9 no struct-member consumer / no base+offset writes; 10 no
public original). So Ruling 11 governs both, each on its own.

## Bodies
- **Re-anchored 2026-09-26 (landing prep, owner Q11).** candidate.c now carries the header's
  verbatim `($12)` in the six gte_ldv0 / gte_stlvnl statements (was `0($12)`), plus comment-only
  edits. The C tokens are unchanged. The measurements below were taken on the `0($12)` text
  (candidate sha256 bf84246240a994ac..., twin c30e799a215c9ff3...). The two texts are
  byte-identical: the maspsx empty-offset fix emits the `($12)` line unchanged and GNU as
  assembles it as `0($12)` (islands.md; the all-src proof in tmp/func_8002DE20/maspsx_neutral.py).
  The (D)(1) dumps were RE-TAKEN on the new text: tags `final2` / `final2_pv`. Their .lreg
  register lines (md5 3f8ae2d3... / 691c2de5...) and all twelve xor maps are identical to
  `final` / `final_pv`, and ALLOCDBG is identical. The excerpts below therefore stand for both.
- **reuse spelling** = memory/grind/func_8002DE20/candidate.c (current sha256 8c342831edbb1613...),
  the landing body. `sandbox --disable all --keep-cheat-asm` **0/506, 0 hunks** (0($12) text).
  (Default strip: 10 today — the islands' `move`/`nop` statements are stripped until the
  engine: change pins these macros. With the scratch engine change every one of the 30 island
  statements is recognized: tmp/func_8002DE20/recog.py. Every row below keeps the islands
  identical, so `--keep-cheat-asm` isolates the C question.)
- **one-variable-per-value spelling** = r11/final_pv.c (current sha256 27d3e8539bcdf290...), generated
  by `python3 tools/gen_pv.py candidate.c r11/final_pv.c` (tools/ = copies of tmp/func_8002DE20/
  scripts). It differs from candidate.c ONLY in declarations and identifiers: the 23 shared
  values of cross_a/cross_b become `cross_aT` / `cross_bT` (T = test number), each declared at the
  innermost scope enclosing its single write (T1/T4/T7/T9/T11 at function scope, the rest at
  the top of the `if` block they are written in); the two shared declarations go away. Same
  statement list. **90/506** (4 source-level + 47 operand-only hunks).
- **Change vs the layer-2-failed body** (rejected/layer2-fail-cross-pair-inline-o-islands-0.c):
  (1) islands re-spelled as separate header statements (islands.md); `vin` dropped (the islands
  take `&obj->unkF8` as the macro argument); (2) cross_b's test-11 value split out into its own
  single-write local `cross_ab2`, because its ablation is byte-identical (see (D)(4)): the reuse
  is now limited to values whose sharing is measured necessary. Everything else unchanged.

## Values (Ruling 11 "value" = writes reaching a common read)
Every write is read by exactly one test, `(cross_a ^ cross_b) >= 0`, in the same C block; no
read can see two writes, so each write is its own value. `E x P` below = `E.vy*P.x - E.vx*P.y`
as spelled (cross of edge E with point P relative to the edge start); c = (cx,cy), p1 = (x1,y1),
p2 = (x2,y2), A/B = obj->unkA8/unkB8.

| test T | lines (candidate.c, verbatim-($12) text) | group | cross_a value aT | cross_b value bT |
|---|---|---|---|---|
| 1 | 162-164 | p1 inside | A x c | A x p1 |
| 2 | 165-167 | | B x c | B x p1 |
| 3 | 168-170 | | (B-A) x (c-A) | (B-A) x (p1-A) |
| 4 | 176-178 | p2 inside | A x c | A x p2 |
| 5 | 179-181 | | B x c | B x p2 |
| 6 | 182-184 | | (B-A) x (c-A) | (B-A) x (p2-A) |
| 7 | 190-192 | edge 0-A | A x p1 | A x p2 |
| 8 | 193-195 | | (p2-p1) x (A-p1) | (p2-p1) x (0-p1) |
| 9 | 200-202 | edge 0-B | B x p1 | B x p2 |
| 10 | 203-205 | | (p2-p1) x (B-p1) | (p2-p1) x (0-p1) |
| 11 | 210-212 | edge A-B | (B-A) x (p1-A) | (own local `cross_ab2`: (B-A) x (p2-A)) |
| 12 | 213-215 | | (p2-p1) x (A-p1) | (p2-p1) x (B-p1) |

cross_a: 12 values; cross_b: 11 values. Each is a mult/sub computation whose instructions are
in the target (e.g. T1: 0x8002E2BC..0x8002E2F8; T4 recomputes A.vy*cx at 0x8002E3A8).

## Prong walk (A)-(H)
- **(A)** Both are locals of this function, declared once at function scope — the innermost
  scope enclosing all their writes (they are written at top level and inside nested `if`
  blocks). No other declaration moved. No `&cross_a` / `&cross_b` anywhere.
- **(B)(1)** Every write is read by its test before the next write (table). **(B)(2)** — the
  point for the reviewer: two writes re-store, on ONE incoming path each, the value the
  variable held on entry along that path: a4 (T4, `A x c`) when T1's test failed (a1 = `A x c`,
  no write in between on that path), and b7 (T7, `A x p2`) when T4's test failed (b4 =
  `A x p2`). On every other incoming path a2/a3 (resp. b5/b6) intervene, so the variable does
  NOT hold that value at the statement in general; deleting either write changes the program's
  result on those paths. The statement is the source's recomputation, present identically in
  the per-value spelling ((C)(2)) and in the target (T4's mults at 0x8002E3A0..). Ruling 5
  2(c)'s banned case is an unconditional re-load of an unchanged lvalue (func_80060A68); here
  the textual program has intervening writes (a2/a3, b5/b6) between the two statements. This
  reading is the author's; layer-2 decides it.
  **Restructuring attempt (2026-09-26):** swapping test 4's roles (`cross_b = A x c;
  cross_a = A x p2;`) removes BOTH path-wise re-stores. Values were checked on every incoming
  path of tests 4-12, and no write then stores a held value. Measured in all four
  statement/operand orders (tmp/func_8002DE20/mk_swap.py, keep-asm): sw1 48, sw2 **4**,
  sw3 51, sw4 **4**. sw2's four hunks are operand-only. The target keeps test 4's `A x c` in
  $a0 and `A x p2` in $v1, i.e. `A x c` sits in the pseudo that held test 1's `A x c` (cross_a,
  ALLOCDBG hardreg=4). Splitting a4 or b7 into its own local costs 9 / 11 (ablations). So the
  bytes show the original rewrote `A x c` into the variable that held it on the test-1-fail
  path; no spelling that avoids the re-store reaches the target. Whether (B)(2) reads "already
  holds" path-wise (fail) or as a redundant, removable write (this write is neither) is not
  settled by the text. Flagged to the orchestrator for the rules draft (2026-09-26).
- **(C)(1)** r11/final_pv.c is recorded. **(C)(2)** same statement list, only declarations and
  identifiers differ (by construction of gen_pv.py; `diff candidate.c r11/final_pv.c` shows
  only declaration lines, renamed identifiers, and the dropped shared declarations/annotation).
  **(C)(3)** every value is an arithmetic computation in the target's bytes; no constant, no copy.
- **(D)** below.
- **(E)** (ii): every value of each variable is a 2D cross product of an edge vector with a
  point-minus-edge-start vector; `cross_a` / `cross_b` state exactly that kind and nothing more
  (the `_a`/`_b` suffix = first / second point of the pair, true of every write).
- **(F)** the declaration comment (candidate.c:46-50) says they hold one value per same-side
  test, names the kind, cites Ruling 11 and this file.
- **(G)** needs a fresh layer-2 (not done: the island admission is pending the owner).
- **(H)** other constructs: struct view, mid_i, return form, z nudge were cleared by the
  2026-09-26 layer-2; `cross_ab2` is a single-write local; the islands are islands.md's question.

## (D)(1) Dumps and command lines
Command (WSL, repo root): `bash tmp/func_8002DE20/dump.sh <tag> <body.c>` (copy: tools/dump.sh).
TU = `git show HEAD:src/code6cac_b.c` (blob 8467c186fee7..., unchanged since 40adc7d6c
2026-09-25) with the `INCLUDE_ASM("asm/funcs", func_8002DE20);` line replaced by the body —
exactly the landing splice; cpp with engine.buildconfig CPP_DEFS; then tools/rtl_track/dump.py
(cc1 `-da`, engine.buildconfig CC_FLAGS `-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1
-mno-abicalls -fno-builtin -w -mel -msoft-float`) for .flow/.lreg/.greg, and the instrumented
tools/gcc-2.7.2/cc1 with `BB2_ALLOC_DEBUG=1` for the global.c order. Outputs:
tmp/func_8002DE20/rtl/<tag>.{c,i,flow,lreg,greg,alloc} (scratch; the excerpts below are the record).
- tag `final` = candidate.c as of 72c3b4d41 (`0($12)` text; spliced TU tmp/func_8002DE20/rtl/final.c).
- tag `final_pv` = r11/final_pv.c as of 72c3b4d41.
- tags `final2` / `final2_pv` = the current candidate.c (verbatim `($12)`) and its re-derived
  twin. The excerpts below were taken from `final`/`final_pv`; `final2`/`final2_pv` are
  identical in every excerpted line (Bodies, above).
Pseudo map: from the xor insns' operands (source `(cross_aT ^ cross_bT)`, operand 1 = a, 2 = b)
and the .lreg register lines. `final`: 91 = cross_a, 92 = cross_b, 93 = cross_ab2.
`final_pv`: function-scope a1/a4/a7/a9/a11 = 91..95, b1/b4/b7/b9/b11 = 96..100 (declaration
order); block-scope a2/b2 = 429/430, a3/b3 = 440/441, a5/b5 = 476/477, a6/b6 = 487/488,
a8/b8 = 523/524, a10/b10 = 549/550, a12/b12 = 591/592.

Reuse spelling, tag `final`, .lreg verbatim:
```
Register 91 used 24 times across 35 insns; dies in 12 places; GR_REGS or none.
Register 92 used 22 times across 23 insns; dies in 11 places; GR_REGS or none.
Register 93 used 2 times across 2 insns in block 35; GR_REGS or none.
Register 421 used 2 times across 2 insns in block 21; GR_REGS or none.
Register 571 used 2 times across 2 insns in block 35; GR_REGS or none.

(insn 841 828 843 (set (reg:SI 421)
        (xor:SI (reg/v:SI 91)
            (reg/v:SI 92))) 99 {xorsi3} (insn_list 828 (insn_list 839 (nil)))
    (expr_list:REG_DEAD (reg/v:SI 91)
        (expr_list:REG_DEAD (reg/v:SI 92)
            (nil))))

(insn 1271 1269 1273 (set (reg:SI 571)
        (xor:SI (reg/v:SI 91)
            (reg/v:SI 93))) 99 {xorsi3} (insn_list 1242 (insn_list 1269 (nil)))
    (expr_list:REG_DEAD (reg/v:SI 91)
        (expr_list:REG_DEAD (reg/v:SI 93)
            (nil))))
```
.greg dispositions (excerpt, verbatim pairs): `91 in 4  92 in 3  93 in 3  421 in 2  430 in 2
571 in 3  588 in 2`. ALLOCDBG (verbatim):
```
ALLOCDBG func=func_8002DE20 ord=0 pseudo=92 hardreg=3 nrefs=22 livelen=23 pri=38260
ALLOCDBG func=func_8002DE20 ord=2 pseudo=91 hardreg=4 nrefs=24 livelen=35 pri=27428
```
All twelve tests (xormap.py over final.lreg/.greg): tests 1-10 and 12 `xor <out>(v0) <- 91(a0)
^ 92(v1)`; test 11 `xor 571(v1) <- 91(a0) ^ 93(v1)`. Target (asm/funcs): `xor v0,a0,v1` at all
tests except test 11 (0x8002E640) `xor v1,a0,v1`. Identical.

One-variable-per-value spelling, tag `final_pv`, .lreg verbatim (T1, T11 shown; every one of the
24 per-value pseudos has an `in block N` line: 91/96 block 21, 429/430 22, 440/441 23, 92/97 25,
476/477 26, 487/488 27, 93/98 29, 523/524 30, 94/99 32, 549/550 33, 95/100 35, 591/592 36):
```
Register 91 used 2 times across 3 insns in block 21; GR_REGS or none.
Register 96 used 2 times across 2 insns in block 21; GR_REGS or none.
Register 428 used 2 times across 2 insns in block 21; GR_REGS or none.
Register 95 used 2 times across 3 insns in block 35; GR_REGS or none.
Register 100 used 2 times across 2 insns in block 35; GR_REGS or none.
Register 590 used 2 times across 2 insns in block 35; GR_REGS or none.

(insn 841 839 843 (set (reg:SI 428)
        (xor:SI (reg/v:SI 91)
            (reg/v:SI 96))) 99 {xorsi3} (insn_list 828 (insn_list 839 (nil)))
    (expr_list:REG_DEAD (reg/v:SI 91)
        (expr_list:REG_DEAD (reg/v:SI 96)
            (nil))))

(insn 1283 1281 1285 (set (reg:SI 590)
        (xor:SI (reg/v:SI 95)
            (reg/v:SI 100))) 99 {xorsi3} (insn_list 1254 (insn_list 1281 (nil)))
    (expr_list:REG_DEAD (reg/v:SI 95)
        (expr_list:REG_DEAD (reg/v:SI 100)
            (nil))))
```
.greg dispositions (excerpt, verbatim pairs): `91 in 2  96 in 3  428 in 2  429 in 2  430 in 3
439 in 2  95 in 3  100 in 4  590 in 3`. All twelve tests: output register == operand-1
register (`xor v0 <- (v0) ^ (v1)`, test 11 `xor v1 <- (v1) ^ (a0)`) — never the target's.

## (D)(2) Mechanism
local-alloc (tools/gcc-2.7.2/local-alloc.c):
- `local_alloc` :470-478 gives a pseudo a local quantity (`reg_qty = -2`) only if it is used
  in ONE basic block (`reg_basic_block[i] >= 0`) and dies exactly once (`reg_n_deaths[i] == 1`);
  otherwise `reg_qty = -1` and only global.c sees it.
- `block_alloc` :1180-1290: xorsi3 has no operand that must match operand 0, so for the xor
  insn it calls `combine_regs(operand i, output)` for operand 1, then operand 2.
- `combine_regs` :1824-1827: "If UREG is a pseudo-register that hasn't already been assigned a
  quantity number, it means that it is not local to this block or dies more than once. In either
  event, we can't do anything with it." -> return 0; otherwise :1905-1922 ties the (local, not yet
  allocated) output into UREG's quantity when UREG dies in this insn (REG_DEAD) — the output
  then shares UREG's hard register.
Reuse: 91/92 are used in 12/11 blocks and die in 12/11 places (`dies in 12 places`, no `in
block`), so reg_qty = -1; combine_regs refuses both operands and the xor output is allocated on
its own ($v0, the target's register); 91/92 go to global.c (ALLOCDBG ord 0/2: $v1, $a0 =
the target's). At test 11 operand 2 is the single-block `cross_ab2` (93), so the output ties
to it ($v1) — the target's `xor v1,a0,v1`.
Per-value: every value pseudo is `in block N` with one death -> reg_qty = -2 -> operand 1 dies
at the xor -> the output is tied to operand 1: `xor rX,rX,rY` at every test.

## (D)(3) Necessity (every one-variable-per-value spelling)
Property of the reuse spelling: the xor operands of tests 1-10 and 12 are pseudos that are NOT
local-alloc quantities (used in several blocks / dying several times).
In ANY one-variable-per-value spelling, the variable of value T is written by one statement and
read by one expression, test T's `^`. By (C)(2) the statement list is the reuse body's, in which
that write is immediately followed by the test with no control flow between (the write is
straight-line `*`/`-` arithmetic, no division or `&&`/`?:`), and the target fixes the same thing:
each test's products lie between the previous conditional branch and the xor with no label. So
the variable's pseudo is referenced in exactly one basic block and dies exactly once (at the xor,
its only read) — whatever its declaration order, scope, integer type, or name. (A non-register
spelling — `volatile`, `static`, address-taken, a global — puts it in memory, which the target's
register-only code rules out.) By local-alloc.c :470-478 it is a local quantity; the xor output is
local too (set at the xor, read only by the branch in the same block: `Register 428 ... in block
21`); so block_alloc/combine_regs (:1905-1922) ties the output to the first operand that is a
dying local — operand 1 whenever operand 1 is a per-value variable, else operand 2. Hence in
every per-value spelling the xor output equals an operand's register at every test, while the
target's tests 1-10 and 12 have `xor v0,a0,v1` (output distinct from both operands). Operand
order in the source cannot help: whichever operand comes first, it ties. And splitting only ONE
operand of a test (the ablations) still ties to it. That is necessity BECAUSE each value has its
own variable: only a variable that also carries another test's value is multi-block.
Test 11's second operand is the exception the target itself shows (`xor v1,a0,v1`: tied to
operand 2) — which is why that one value is NOT shared in candidate.c.

## (D)(4) Measured alternatives (`sandbox --disable all --keep-cheat-asm`, /506; islands identical)
Chassis Y = candidate.c (code-identical: y_cand.c tokens == candidate.c tokens); chassis X = the
same body with cross_b's test-11 value also shared (r11_cand.c). Ablation bodies =
`gen_pv.py <chassis> --only <vT>` (Y ablations regenerated from candidate.c are token-identical
to the measured files).
| spelling | score |
|---|---|
| reuse, chassis Y = candidate.c | **0** (0 hunks) |
| reuse, chassis X (b11 shared too) | 0 (0 hunks) |
| full one-variable-per-value (r11/final_pv.c; == X's twin r11_pv.c up to one name) | **90** (4 src + 47 op) |
| Y ablation, cross_a value alone: a1 / a2 / a3 / a4 / a5 / a6 | 2 / 7 / 14 / 9 / 29 / 29 |
| Y ablation, cross_a: a7 / a8 / a9 / a10 / a11 / a12 | 9 / 14 / 9 / 14 / 72 / 14 |
| Y ablation, cross_b: b1 / b2 / b3 / b4 / b5 / b6 | 26 / 2 / 2 / 2 / 7 / 2 |
| Y ablation, cross_b: b7 / b8 / b9 / b10 / b12 | 11 / 2 / 11 / 2 / 2 |
| X ablation a1..a12 | 2 7 14 9 29 29 9 14 9 14 99 14 |
| X ablation b1..b12 | 26 2 2 2 7 2 11 2 11 2 **0** 2 |
| structural: no variables, products inline in each test (s_inline.c) | 90 |
| structural: `static inline s32 cross2(ex,ey,px,py)` helper, per-value locals (s_helper_pv.c) | 90 |
| structural: helper calls inline in each test, no variables (s_helper_inline.c) | 90 |
| structural: helper + the shared pair (s_helper_shared.c) | 0 |
| structural: the pair as `s32 side[2]` (tmp s_sides_array.c) | 265 (578 insns: stack array) |
| older chassis (joined islands): one pair per group, block scope | 54 |
| older chassis: fresh pairs, only the final group shares | 72 |
Every Y ablation is > 0: each of the 23 shared values is individually necessary. X's b11 = 0 is
why candidate.c splits that value out.

Permuter from the one-variable-per-value body: see the "Permuter" section appended below.

## (D)(4) Permuter from the one-variable-per-value body
Workspace tmp/func_8002DE20/perm_pv, built by tmp/func_8002DE20/mkperm.py from r11/final_pv.c's
X-twin r11_pv.c (identical code up to the name cross_b11/cross_ab2): minimal TU, asm statements as
`#pragma _permuter b64literal`, target.o from asm/funcs with mvmva -> .word. Launched with
`tools/permuter_campaign.py launch --label r11-per-value-90 -j 2` 2026-09-26T21:01Z, stopped
after 1323 s / 11,748 iterations (harvest --stop). Permuter base 1080 -> best 385 (34 finds;
last novel sub-500 find at 1046 s).
What the finds do (diff.txt of each output dir):
- 385, 400, 420, 450, 860, 980, 990-2: stage a sub-expression through ANOTHER test's
  function-scope per-value local (`cross_b1 = obj->unkB8.vx * cy;` in test 5; `cross_b1 =
  obj->unkA8.vx` before tests 11/12; `cross_a11 = cross_a6`; `cross_a11 = obj->unkB8.vx`) — i.e.
  they re-create a multi-block, multi-value variable, the same mechanism as the reuse; 990-1
  and 1020-2 borrow `cross_a11` / `max_z` the same way. Banned class (Ruling 1 carriers), not a lever.
- 430 / 435 / 730: carrier-free — test 1's cross_a written inline in the condition (730 also a
  `inline_fn` returning obj->unkB8.vx). Measured in the sandbox on the real TU: the 430 edit on
  r11/final_pv.c = **107** (508 insns), worse than 90 — the minimal-TU permuter score does not
  transfer.
So no carrier-free find reaches the target; every large gain is a cross-block carrier.

## (D)(3) addendum — per-value spellings plus SANCTIONED-family constructs (brief point 10)
Question: can a one-variable-per-value spelling reach the reuse's property (each xor operand a
non-local pseudo) through a sanctioned construct — FAKE dead store, self-assignment,
combine-foldable chain-extender, or a pointer alias — at zero bytes? Bodies r11/ext_<m>.c,
generated by tools/gen_ext.py from r11/final_pv.c: at the top of the block each test guards (a
DIFFERENT basic block from the value's write and xor) every test's two per-value locals get:

| m | construct | sandbox keep-asm | per-value pseudos local / xors tied (dump) |
|---|---|---|---|
| dead | `P = 0; Q = 0; /* FAKE */` dead stores | 102 (502 insns) | 24/24 local, 12/12 tied |
| self | `P = P; Q = Q; /* FAKE */` | 90 | 24/24, 12/12 |
| chain | `P = P + 1 - 1; ...` | 90 | 24/24, 12/12 |
| use | live use: next write `... + (P - P) + (Q - Q)` | 90 | 24/24, 12/12 |
| usevar | live use: `... + (P ^ P) + (Q ^ Q)` | 90 | 24/24, 12/12 |
| alias | `alias_a1 = &cross_a1; /* FAKE */`, test 1 reads `*alias_a1` | 179 (514 insns, `sw v0,0(sp)`: stack slot) | — |

Dumps: `bash tmp/func_8002DE20/dump.sh ext_<m> tmp/func_8002DE20/ext_<m>.c`; tools/passrefs.py
counts each pseudo's references per pass (91 = a1, 96 = b1, 429/430 = a2/b2):
- self / chain / use / usevar: 2 refs in `.rtl` (the write and the xor) through `.lreg` — the
  extra reference never reaches RTL. fold-const cancels `P - P`, `P ^ P`, `P + 1 - 1` and the
  self-assignment at tree level, before expand.
- dead: the store is in RTL through `.cse2` (`(set (reg/v:SI 91) (const_int 0))`, insn 848)
  and gone in `.flow`. flow.c propagate_block :1479-1493 turns a dead insn into
  NOTE_INSN_DELETED on the final pass BEFORE its registers are marked, and the block tracking
  that sets REG_BLOCK_GLOBAL (:2066-2075 sets, :2502-2511 uses) runs only for insns that
  survive. So `.lreg` keeps `Register 91 used 2 times across 3 insns in block 21`. (The dead
  store still perturbs earlier passes: 502 insns.)
- alias: `&P` puts P in the stack frame; the target keeps every value in registers with an
  8-byte frame (only $s0 saved), so this is excluded by the bytes.

By mechanism (author's argument): a reference can make P multi-block for local_alloc only if
it survives flow's final pass, i.e. it is LIVE. A live use of P in another block Y costs bytes
unless something folds it after flow counts it. combine is the only fold after flow, and it
works within one block along LOG_LINKS. In Y, P is only an input, so eliminating it needs an
algebraic cancellation of P against P, or against a known copy of P. fold-const does the
same-decl case at tree level, as measured. CSE (pre-flow, the same simplify_* routines,
extended basic blocks) does the copy case whenever the copy is visible in Y's extended block.
Where the copy is NOT visible (a join block), combine cannot see the equality either, so the
`subu`/`xor` survives and costs bytes. No zero-byte sanctioned construct therefore changes the
locality that forces the tie.
