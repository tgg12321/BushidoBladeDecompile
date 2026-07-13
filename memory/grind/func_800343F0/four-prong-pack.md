# Four-prong evidence pack — header retype `D_80102787` u8 → s8

## >>> STATUS: CONTESTED — prong (a) FAILED on adversarial re-derivation. <<<
## >>> Layer-1 cheat-reviewer verdict: FAIL. Edits REVERTED; nothing committed. <<<
## >>> Escalated as a ruling-request (s2). Do NOT treat this pack as a clean pass. <<<
##
## The killer fact (verified this session, not inherited): `D_800A3140` — the s32
## global this byte flows into — has exactly ONE reader in the entire tree,
## `asm/funcs/calc_loc_mat_fw.s:1152`: `lw; nop; bnez`. A zero/nonzero test.
## 0x80..0xFF and -128..-1 are BOTH nonzero, so the sign extension is observed by
## NOTHING, program-wide. §(a) and §(b) below are retained as written for the
## record, but their "positive signed semantics" / "functionally necessary"
## arguments do NOT survive that fact. See §Adversarial re-derivation at the end.

Invoking `.claude/rules/header-type-correction-from-use-sites.md` (owner ruling
2026-07-13). Proposed edit: `include/code6cac.h:409` `extern u8 D_80102787;` →
`extern s8 D_80102787;`, plus removal of the Judge-flagged pointer-pun at
`src/code6cac_b.c:3952`. Session s2, 2026-07-13.

This pack is the mandatory prerequisite the rule's "Not a wand" section names:
*"Does NOT license unilateral edits to include/*.h during grind sessions unless
the four-prong evidence pack is on record."*

## (a) Codebase-wide use sites consistent with s8, ≥1 positively signed — PASS

`grep D_80102787` over `src/**/*.c` + `include/**/*.h` (run this session, not
inherited) returns exactly three lines:

| Site | Code | Under s8 |
|---|---|---|
| `include/code6cac.h:409` | the declaration itself | — |
| `src/code6cac_b.c:3952` | `s32 val_87 = D_80102787;` | the positively-signed site |
| `src/code6cac.c:1349` | `D_80102787 = 0;` | type-agnostic byte store (`sb $zero`) |

**Positive signed semantics (the clause that fails abusers):** site :3952 is a
read into a wider signed lvalue with a depended-upon sign extension — the value
flows straight into `D_800A3140`, declared `extern s32` (`include/code6cac.h:84`)
and written with a full-word `sw` (`asm/funcs/func_800343F0.s:26`). Target loads
it with a native `lb` (line 11).

**The mechanism-level proof that the declaration — not the use site — was wrong:**
the same function reads three *genuinely* u8 neighbours with the *identical*
statement shape plus an `(s8)` cast:

    lbu $a2, %lo(D_80102785)   ... sll $a2,24 ; sra $a2,24     (lines 5, 15-16)
    lbu $v1, %lo(D_80102786)   ... sll $v1,24 ; sra $v1,24     (lines 7, 17-18)
    lbu $v0, %lo(D_80102784)   ... sll $v0,24 ; sra $v0,24     (lines 9, 19-20)
    lb  $a3, %lo(D_80102787)   ... (no extend pair)            (line 11)

An `(s8)` cast applied to a `u8`-declared global **provably cannot emit `lb`** —
the three neighbours are the in-function control group showing exactly what that
spelling compiles to. Only a signed *declaration* produces the native `lb`.
The contrast is within one function, one compiler invocation, one statement
shape. The original declared 84/85/86 unsigned and 87 signed.

## (b) Old type required a functionally necessary cast, predating the session — PASS

Mechanical WITH/WITHOUT analysis under the OLD type (`u8`):

- WITHOUT cast: `s32 val_87 = D_80102787;` → zero-extend → value in `[0, 255]`.
- WITH the pun: `s32 val_87 = *(s8 *)&D_80102787;` → sign-extend → `[-128, 127]`.

These are **behaviorally different** for any stored byte ≥ 0x80: the destination
`D_800A3140` is a signed `s32`, so it would hold `+128..+255` instead of
`-128..-1`. Not warning-silencing, not stylistic, not defensive-neutral — it
changes runtime values on the target ABI. Functionally necessary. PASS.

**Timing (the git-blame collision check):** `git blame -L 3946,3953
src/code6cac_b.c` → the cast is commit `38101f31` (Trenton, **2026-03-28**), the
original decompilation of the function. The residual/audit session that flagged
it is 2026-07. The cast predates the residual-chasing by ~3.5 months, so it is
evidence of a mistyped header, not a coercion introduced to close a byte. PASS.

## (c) Single canonical extern edit — PASS

One line, `include/code6cac.h:409`, the ground-truth declaration. No
alias-rename (`asm("Sym")`), no pointer-cast pun, no local typedef override, no
macro-hidden coercion. The pun that WAS the lie is deleted, not relocated.

## (d) Casts eliminated at ALL use sites, no residual — PASS

After the flip the codebase carries **zero** compensating casts on this symbol:
`:3952` becomes a plain read; `code6cac.c:1349`'s zero-store needs no change
(`sb` is sign-agnostic). No site retains old-type behavior. The correction
*simplifies* — it does not rearrange where the coercion sits.

## Measurements (this session)

    sandbox func_800343F0 --disable all  → score 0   (49/49 insns, 0 rules)
    sandbox func_8001C444 --disable all  → score 0   (31/31 insns, 0 rules)  [the store site — unaffected]

## Adversarial re-derivation (s2) — why prongs (a)+(b) FAIL as argued above

An independent layer-1 `cheat-reviewer` traced `D_800A3140` (the destination)
instead of `D_80102787` (the source) — the step §(a) above did not take. Verified
independently by grep over `src/` AND `asm/`:

    writer:  func_800343F0                sw  $a3, %lo(D_800A3140)      (the only one)
    reader:  calc_loc_mat_fw.s:1152       lw  $v0, %gp_rel(D_800A3140)
             calc_loc_mat_fw.s:1153       nop
             calc_loc_mat_fw.s:1154       bnez $v0, .L8002BBD0          (the only one)

**The sole consumer is a zero/nonzero test.** So:

- **Prong (a) FAILS.** The rule requires ≥1 use site with signed-specific
  semantics that is *dead or wrong* under the OLD type. There is none: no
  negative-literal compare, no signed shift, no round-toward-zero idiom. My
  claimed "read into a wider signed lvalue with the implicit expectation of sign
  extension" is typographically one of the rule's listed shapes, but nothing
  downstream *expects* the sign extension — the expectation is vacuous. My only
  real evidence for signedness was the target's `lb`, i.e. the residual. The rule
  forecloses exactly this: *"the residual is not evidence of correctness, the
  use-site semantics are"* — and names it the **strongest abuse signal**.
- **Prong (b) FAILS.** "Functionally necessary" is defined as *removing the cast
  changes runtime behavior*. Removing the pun under `u8` changes the stored bit
  pattern (0x80..0xFF vs -128..-1) but **no observable behavior**, since the only
  reader tests nonzero-ness and both ranges are nonzero. Bit-pattern difference
  alone is trivially true of any signed/unsigned narrow load; it is not evidence.

Prongs (c) and (d) do hold, but all four are mandatory. **The flip is therefore
NOT self-approvable.** It was reverted; nothing was committed.

## The crux for the owner (why this is a ruling and not more grinding)

The reviewer's fallback — "treat it as ordinary pure-C distance and keep hunting
non-coercive C levers" — **is not actionable here, and that is the whole point.**
The set of pure-C spellings that emit `lb` for a global is exactly one: *read a
signed-typed global*. This is not a register-allocation plateau where more search
pays. The three `u8` neighbours in this very function are the control group: the
same statement shape with an `(s8)` cast compiles to `lbu; sll 24; sra 24`. A cast
cannot produce `lb`. So the options are exhaustive and both currently blocked:

  1. the pointer-pun  → a CHEAT (Judge-flagged; on main today), or
  2. the header retype → blocked by prong (a) as re-derived above.

There is no option 3. The function cannot reach COMPLETED-C until one of these is
resolved by policy.

**The substantive question:** does prong (a)'s positive-evidence clause admit
**compiler-proven original-declaration signedness** — the in-function `lb` vs
`lbu+sll+sra` contrast, which proves the ORIGINAL SOURCE declared this byte
signed — as qualifying evidence, when the sign happens never to be *observed*
downstream? Two things bear on it:

- **The `u8` in the header is an auto-typing artifact, not ground truth.** The
  whole run `D_80102784/85/86/87` is uniformly `u8` — the signature of a bulk
  splat/auto-typer guess over a byte range, not four deliberate declarations. The
  target bytes disagree with that guess for exactly one of the four.
- **Plain `char` is signed by default on this ABI.** The original programmer most
  plausibly wrote `char D_80102787;` (or `signed char`) and `unsigned char` for
  the other three. `lb` is what that compiles to. Under this reading the retype
  *recovers* the original declaration rather than coercing codegen — and a
  programmer is under no obligation to make a variable's sign load-bearing.
- **The rule's own sibling precedents are mechanism-derived.**
  [[bitfield-direction-divergence]] recovers bitfield order from `srl`-vs-`sra`
  codegen, and [[mmio-volatile-type-level]] types MMIO volatile from address
  range — neither requires downstream behavioral dependence. The rule cites both
  as "header truth" siblings. Prong (a), read strictly, is in tension with them.

If the answer is NO, the owner should say what disposition this function takes,
since options 1 and 2 are exhaustive.

## Known future obligation (NOT a prong-(d) residual)

Still-undecompiled `func_80034708` reads this global with `lbu`
(`asm/funcs/func_80034708.s:533`). When that function is matched it will need a
`(u8)` value cast at that site. This is not a residual C use site today (it is
asm, not C) and does not violate prong (d). It is the expected mirror image: one
of the two functions must carry a cast under *either* declaration, and the
in-function neighbour contrast above (§a) is what decides which declaration is
the true one. Recorded here so `func_80034708`'s future ledger does not read
its `lbu` as evidence against this retype.
