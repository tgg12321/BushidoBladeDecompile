# SELF-VET — func_80040D48

Diff under review: `src/text1a_pre.c`, the body of `func_80040D48` only (plus its
immediately-preceding typedef/extern prelude, which is unchanged from the committed
chassis except that the `register s32 a0_s7 asm("s7")` pin was deleted in s1). No
`regfix.txt` / `asmfix.txt` / `.claude/rules/` / `engine/` / `tools/` / `Makefile` /
`*.ld` touched. The function body contains no `__asm__` of any kind and no `volatile`
of any kind (`grep -n "__asm__\|register .*asm(" src/text1a_pre.c` returns exactly one
hit, line 752, inside the UNRELATED and untouched `func_80041188`; that is the
`cheat_asm_stripped: 2` the sandbox reports for the TU).

Honest measurement with this exact body in place, this session, on HEAD 51a9e464:
`& tools/wteng.ps1 main sandbox func_80040D48 --disable all` →
`"score": 0, "target_insns": 272, "build_insns": 272, "rules_dropped": 34`.
All 34 legacy regfix rules for this function are dropped by the sandbox and the
function still matches instruction-for-instruction; reaching COMPLETED-C retires them.

CONSTRUCTS: (1) `s32 ent` — the table entry read, null-checked, then cast to the
object base `s4`; (2) `s5` carries the `s4+0x2C` pointer and then the Copy8 loop's
source pointer; (3) `a2p` carries the Copy8 loop's list-push walker (`s4+0x10D4`) and
then the `s4+0x8B4` walker; (4) `s1` carries the `arg4` input pointer and then the
`func_800417D0` walker over `s3`; (5) the Copy8 loop spelled `copyloop:` … `goto
copyloop;` with a `copydone:` exit label; (6) the case-0 init loop spelled `initloop:`
… `if (s0 < 0x12) goto initloop;`; (7) one shared loop counter `s0` across the four
counted loops; (8) block-scoped locals `tbl`, `a4p`, `p`, `s0_fn`, `scaled`, `list`,
`list2`, `list3`, `list4`, `idx`; (9) the pre-existing `Copy8_40D48` struct typedef
used for the 32-byte structure copy (unchanged from the committed chassis).

## T1 semantic purpose
Every construct carries a real, consumed value or real control flow; none is
byte-identical-with-or-without.
- (1) `ent` is the `s32` element of `D_800A9A10[]`. It is the value the null check
  tests and the value that becomes the object base. Delete it and the function has
  nothing to test. (Note it is written once and read twice — test and cast — so it is
  NOT a "once-written once-read named intermediate"; it is an ordinary named value.)
- (2)(3)(4) Each of these variables holds a real pointer in BOTH of its roles and each
  role's value is dereferenced or stored. Removing either role removes real work:
  `*(s16 *)(s5 + 6) = 0` and the `Copy8` source load; the two `*listN = (s32)a2p`
  list pushes; the `arg4` field reads and the `func_800417D0(s1)` calls. There is no
  dead write and no unread value anywhere in this body.
- (5)(6) These ARE the loops. The label is the loop head and the `goto` is the back
  edge; remove either and the loop does not iterate.
- (7) `s0` is compared and incremented/decremented in each of the four loops it drives.
- (8) All block-scoped locals are written and read.
Nothing here pads, parks, pins, holds, discards or aliases anything.

## T2 human-programmer
- (1) The global is declared `extern s32 D_800A9A10[]`. Reading the element into an
  `s32`, checking it for null, and then casting it to the object pointer is exactly how
  a 1998 C programmer writes "look up the handle, bail if the slot is empty, work
  through it". The two names have two different TYPES and two different meanings
  (table entry vs object base); a reader does not ask "why is this here?".
- (2)(3)(4) Reusing one pointer local for two sequential walks is ordinary, and (3) is
  ordinary twice over: both of `a2p`'s roles are *the same job* — walking a 0x68-stride
  array pushing entries onto the `D_800A3820` list. (2) and (4) are the two a reviewer
  could reasonably query, and I am flagging them rather than burying them: `s5` holds
  the `s4+0x2C` pointer and later the Copy8 source; `s1` holds `arg4` and later the
  `s3` walker. The affirmative reason to believe these are the ORIGINAL's variables and
  not our invention is that the target binary's own registers do precisely this and
  nothing else: `$s5` is defined as `addiu $s5,$s4,0x2C`, used once at
  `sh $zero,0x6($s5)`, and then redefined by `lw $s5,0x40($a3)` inside the copy loop;
  `$s1` is loaded from the incoming stack slot, read through `0x6C..0x82($s1)`, and then
  redefined by `addu $s1,$s3,$zero` for the walk. One hard register serving two roles
  across a whole function is what a single C variable produces; two C variables produce
  two pseudos, and (measured, banked in `rejected/`) two different registers.
- (5)(6) A label plus a backward `goto` is a plain C loop. The COMPLETED sibling
  `func_80040CB8` in this same translation unit already ships exactly this idiom — its
  loop head is the label `loop:` at `src/text1a_pre.c:526` with the backward `goto loop;`
  at `src/text1a_pre.c:547` — and SOTN master ships genuine backward-goto loops in
  byte-matched PSX code, indexed at `docs/reference/sotn-construct-index.md:1015`
  (SOTN `src/dra/5F60C.c:579`, `loop_check_equip_id_1:`, goto from line 582) and
  `docs/reference/sotn-construct-index.md:1033` (SOTN `src/main/main.c:40`,
  `main_search_loop_1:`, goto from line 57). It is not a marker, not a pad, and
  not a dead label: `copydone:` is jumped to and `initloop:`/`copyloop:` are the loop
  heads.
- (7) One counter for four sequential loops is what the sibling `func_80040B44`
  (COMPLETED-C, `68065f31`) does and what any human does.

## T3 GCC-internals justification
The DISCOVERY path for constructs (1), (4), (6) went through GCC internals and this
session's ledger records that reasoning in full (loop.c `combine_givs` /
`strength_reduce`'s "not worth while" test; global.c `allocno_compare`; flow.c's
loop-depth ref weighting). Discovery is not justification, so, explicitly:

No construct in this diff REQUIRES a GCC internal to explain why it is in the program.
Each is justified by program logic alone (T1/T2). What the internals explain is only
WHICH of several behaviourally identical, equally ordinary spellings the 1998 author
used — they are evidence about the original source, not a mechanism the construct
exploits. `do { } while` versus `label: … goto label;` are two ordinary ways to write
the same counted loop; the dumps told me which one produced the shipped bytes. One
variable used twice versus two variables used once are two ordinary ways to write the
same walk; the target's register file told me which one the original had.

The cheat signal this test exists to catch — a construct that is inert as program text
but load-bearing as codegen — is absent: there is no statement in this body whose
removal leaves the function's behaviour unchanged. The diff is also strictly SIMPLER
than the chassis it replaces: it deletes the `register s32 a0_s7 asm("s7")` pin, deletes
one local (`s1p`), and retires 34 regfix rules. A coercion pass adds scaffolding; this
one removed it.

## T4 permuter/search provenance
Zero permuter runs, zero auto-search, zero randomized mutation across all three
sessions. Every form was hand-derived from the target disassembly plus cc1 `-da` dumps
read directly (`tmp/grind/func_80040D48/dumps/`, regenerated this session against the
current body) and from the GCC 2.7.2 source in `tools/gcc-2.7.2/loop.c`,
`config/mips/mips.c` and `mips.h`. Every candidate was a written hypothesis with a
predicted outcome, measured one at a time with `sandbox --disable all`; the ones that
failed are banked in `rejected/` with their measured floor and the reason. Nothing in
the diff survives because a detector missed it — the diff contains no construct any
detector targets (no asm, no pins, no volatile, no dead stores, no unused locals, no
`(void)`/`&local` discards, no pads).

## T5 family check
Checked construct-by-construct against the forbidden-family catalog: no register-asm
pins (one was DELETED); no hardcoded-`$N` asm injection (no asm at all); no
lost-codegen regfix insert (34 rules dropped, none added); no scheduling barrier; no
INLINE_MOVE_ALIASING; no volatile coercion in any spelling (the keyword does not appear
in the body); no unused/written-never-read local array; no `&local` or `(void)` discard;
no dead-param-assign; no dead-conditional-store; no empty-body `if`; no `if (1)` wrap;
no dead-goto label-pad (every label in the body is a live branch target and every `goto`
is a taken control-flow edge); no DImode chain; no goto-end accumulator; no
param-local-alias declaration-order trick; no `s32 one = 1;` opaque constant holder; no
lowercase `asm()`; no `asm("sym")` alias rename; no redundant width casts; no `bb2.ld`
reorder; no combine-foldable chain-extender (s1 measured that family DEAD here — cse1
folds same-EBB derived-base sums, banked in
`rejected/derived-base-anchor-ref-cse-folded.c` — and it is not in this diff).

Nor does the diff claim any SANCTIONED family, because it does not need one. The
closest questions and their answers:
- The three variable reuses are not the "borrow a currently-dead local to STAGE an
  extra value" construct that `staged-value-reused-variable.md` governs: that family
  adds a statement carrying a value the variable's own role does not need. Here no
  statement is added — each variable simply has two sequential lifetimes, both of them
  its own real work, exactly as the sibling `func_80040B44`'s shared counter `i` was
  accepted (`68065f31`, self_vet "SANCTIONED-FAMILY-CLAIMS: none"). They are equally
  not `defeat-licm-hoist-var-reuse.md`, whose scope is a loop-invariant that LICM
  hoists; nothing here is hoisted and no invariant is involved.
- The two goto-spelled loops are not the sanctioned `do { } while (0)` carve-out and do
  not lean on it; they are not a wrap of anything, they are the loops themselves.
- `ent` is not a "named intermediate" in the 6-prong sense: it is read twice, and its
  value is not a staged copy of another variable but the table element itself.
If a reviewer holds that any of these does belong to a family, the honest reading is
that it belongs to NONE — I am not invoking a carve-out to license anything, I am
asserting that this diff needs no carve-out. If the reviewer disagrees on (2)/(4)
specifically, that is a classification question worth a ruling; it is stated openly
above rather than glossed.

## T6 naming-announces-intent
No name in the body is `pad`, `_pad`, `dummy`, `unused`, `spill`, `sp_*`, `_buf`,
`tail`, `slack` or `_frame_pad`. The locals are register-derived (`s0`…`s5`, `a2p`,
`a3p`, `a4p`, `s0_fn`) — the project's normal convention for a function whose symbols
are not yet recovered, inherited from the committed chassis and m2c — plus `ent`,
`tbl`, `p`, `scaled`, `idx`, `list`/`list2`/`list3`/`list4`. Every one of them is both
written and read; none has address-of taken; none is discarded. `copyloop`,
`copydone`, `initloop` name what they are (a loop head, a loop exit, a loop head).

SANCTIONED-FAMILY-CLAIMS: none

ANNOTATION-CONFORMANCE: n/a — no FAKE construct
  No `/* FAKE */` construct is present and none is required: no sanctioned family is
  claimed, no dead code exists, no coercion is present, and every value in the body is
  computed because the program needs it.
