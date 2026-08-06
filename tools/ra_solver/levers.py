#!/usr/bin/env python3
"""ra_solver.levers — perturbation class -> candidate C-lever mapping.

The forward solvers answer "why did our allocation diverge".  The inverse
solver answers "which INPUT perturbation would flip us to target".  That
answer is still in compiler-internals space (refs, live span, birth order,
conflict edges, preferences, calls-crossed).  This module is the last hop:
perturbation class -> the project's C-lever catalog (.claude/rules/, indexed
by codegen-technique-index.md).

Three policy tiers, and the distinction is load-bearing — the inverse solver
is a DIAGNOSIS tool and must never propose a cheat:

  PLAIN       ordinary C restructuring; no special review beyond the normal bar.
  SANCTIONED  a narrow carve-out with STRICT prerequisites (documented lever
              exhaustion, FAKE annotation, layer-1+2 cheat-reviewer).  Emitted
              with its prerequisites attached, never as a first suggestion.
  FORBIDDEN   never emitted as a suggestion.  Listed only in the "do NOT reach
              for this" block, because these are exactly the constructs an
              agent reaches for when a perturbation looks unreachable in C.

Nothing here may be relaxed without the review discipline in
.claude/rules/review-discipline-before-commit.md.
"""

PLAIN = "PLAIN"
SANCTIONED = "SANCTIONED"
FORBIDDEN = "FORBIDDEN"

# --------------------------------------------------------------------------
# Perturbation classes.  Each is an INPUT of one of the three forward models.
# --------------------------------------------------------------------------
REFS_UP = "refs_up"                # reg_n_refs / qty refs increased
REFS_DOWN = "refs_down"            # reg_n_refs / qty refs decreased
LIVE_SHRINK = "live_shrink"        # reg_live_length / (death-birth) shortened
LIVE_EXTEND = "live_extend"        # ... lengthened
BIRTH_ORDER = "birth_order"        # allocno/qty index order (the tie-break)
CONFLICT_ADD = "conflict_add"      # a new conflict edge (ranges made to overlap)
CONFLICT_DROP = "conflict_drop"    # an edge removed (ranges separated)
PREF_ADD = "pref_add"              # a new hard-reg copy preference
PREF_REROUTE = "pref_reroute"      # the EXISTING preference points elsewhere
PREF_CLEAR = "pref_clear"          # the copy relationship removed
CALLS_CROSSED = "calls_crossed"    # allocno crosses (or stops crossing) a call
HARDREG_LIVE = "hardreg_live"      # a hard reg occupied across a span (local)
EXTRA_QTY = "extra_qty"            # one more competing quantity in the block
CLASS_CHANGE = "class_change"      # preferred register class differs
ALLOC_ORDER = "alloc_order"        # allocation order forced, no input explains it

# --------------------------------------------------------------------------
# class -> [(slug, tier, how)]
# `slug` is a .claude/rules/<slug>.md file unless noted.
# --------------------------------------------------------------------------
LEVERS = {
    REFS_UP: [
        ("duplicated-statement-into-arms", SANCTIONED,
         "duplicate a REAL statement into 2+ control-flow arms; the proven "
         "byte-free ref-lift for global-RA priority walls (motion_SetMotion). "
         "Prereqs: byte-neutrality verified + documented exhaustion + FAKE "
         "annotation + layer-1/2 review."),
        ("split-read-defeats-hoist", PLAIN,
         "duplicate a shared read into the branch arms instead of hoisting it "
         "(SOTN-sanctioned); each arm's copy is a real reference."),
        ("(algorithmic)", PLAIN,
         "the value genuinely has more uses in the original: look for a "
         "recomputation the current C caches, or a field read the C hoisted."),
    ],
    REFS_DOWN: [
        ("hoist-shared-arm-computation-defeats-copy-pref", PLAIN,
         "hoist a duplicated per-arm expression out into ONE post-if/else "
         "statement; jump2 duplicates it back at codegen, so the bytes survive "
         "while the pseudo loses its per-arm references."),
        ("defeat-licm-hoist-var-reuse", PLAIN,
         "reuse one variable so two values share references."),
    ],
    LIVE_SHRINK: [
        ("(variable identity)", PLAIN,
         "SPLIT the variable: give the short-lived use its own local so its "
         "range stops at the last real use.  Catalog law: conflict edges = "
         "variable identity (split removes, reuse adds)."),
        ("staged-value-reused-variable", SANCTIONED,
         "stage the value through an EXISTING currently-dead local; live code "
         "only, zero dead stores, FAKE-annotated + exhaustion documented."),
    ],
    LIVE_EXTEND: [
        ("(variable identity)", PLAIN,
         "MERGE: reuse one variable for both values so the range spans both, "
         "or move the last use later in the statement order."),
        ("defeat-licm-hoist-var-reuse", PLAIN,
         "reuse a scratch variable across the region you need covered."),
    ],
    BIRTH_ORDER: [
        ("(declaration/statement order)", PLAIN,
         "allocno ties break by pseudo number = FIRST-DEFINITION order.  Move "
         "the definition earlier/later, or introduce the named intermediate "
         "sooner (SOTN's `randy` chain is the same mechanism)."),
        ("split-init-accumulation-sanctioned", PLAIN,
         "`v = a; v += b;` instead of `v = a + b;` changes which pseudo is "
         "born first (memory/feedback, user-sanctioned 2026-06-13)."),
    ],
    CONFLICT_ADD: [
        ("(variable identity)", PLAIN,
         "make the two values genuinely coexist: reuse one variable across "
         "both regions, or move a use so the ranges really overlap."),
        ("call-return-if-result-reuse-v0", PLAIN,
         "when the needed overlap is with a call's return value, initialise "
         "the result var FROM the call return."),
    ],
    CONFLICT_DROP: [
        ("(variable identity)", PLAIN,
         "SPLIT the variable so the ranges no longer overlap — the "
         "camera_set_zoom lever 1 shape (pre-clamp temps separated from the "
         "call-crossing finals; 48 -> 19 on its own)."),
        ("drop-param-alias-local", PLAIN,
         "drop a param->local alias so the two stop coexisting."),
    ],
    PREF_ADD: [
        ("call-return-if-result-reuse-v0", PLAIN,
         "init the value from a call return so it acquires the $v0 copy pref."),
        ("restore-discarded-return-displaces-v0", PLAIN,
         "capture a return value the current C discards."),
        ("(argument flow)", PLAIN,
         "feed the value into (or take it from) the argument register you want "
         "— the $a0 pref PROPAGATES through REG_DEAD copy merges, stopping "
         "only at conflicts (global.c:851)."),
    ],
    PREF_REROUTE: [
        ("(preference reroute)", PLAIN,
         "CATALOG LAW: preferences must be REROUTED, not added — find_reg takes "
         "the LOWEST preferred reg, so an added preference above the existing "
         "one is inert.  Change WHICH copy the value participates in."),
        ("exit-path-return-set-cse-join", PLAIN,
         "set the return value in each exit path rather than at the join."),
        ("hoist-shared-arm-computation-defeats-copy-pref", PLAIN,
         "hoist the shared arm expression so the pseudo stops inheriting the "
         "arg-copy pref via expand_preferences."),
    ],
    PREF_CLEAR: [
        ("drop-param-alias-local", PLAIN,
         "remove the copy relationship (drop the alias / stop round-tripping "
         "the value through the argument or return register)."),
    ],
    CALLS_CROSSED: [
        ("(call-crossing restructure)", PLAIN,
         "move the computation to the other side of the call, or split the "
         "variable so only the part that must survive the call crosses it.  "
         "calls_crossed==0 excludes only FIXED regs; >0 excludes all "
         "CALL_USED, which is the whole caller-saved/callee-saved decision."),
    ],
    HARDREG_LIVE: [
        ("(argument liveness)", PLAIN,
         "local-alloc's free set is `fixed | regs_live_at[birth..death)`.  The "
         "honest producer is a real argument or return value genuinely live "
         "across that span — i.e. a call placed inside the range, or a "
         "parameter still used after it."),
        ("store-before-jal", PLAIN,
         "ordering recipe for a value saved across a call site."),
    ],
    EXTRA_QTY: [
        ("(one more real temp)", PLAIN,
         "an additional named intermediate whose span overlaps the region and "
         "whose qty priority beats the incumbent.  It must be a value the "
         "function genuinely computes — a declared-but-unused local is a "
         "FORBIDDEN frame/RA coercion (dead-vars-local-array)."),
        ("named-local-fake-exception", SANCTIONED,
         "constant-holder / dead SCALAR local biasing RA — LAST RESORT, "
         "FAKE-annotated, exhaustion documented, arrays still forbidden."),
    ],
    CLASS_CHANGE: [
        ("(idiom shape)", PLAIN,
         "register class is decided by the RTL idiom, not by spelling: MD_REGS "
         "($hi/$lo) come from mult/div and GCC's divide-by-constant `mulhi`.  "
         "Change the arithmetic the function performs, or accept it."),
    ],
    ALLOC_ORDER: [
        ("(OUT OF MODEL)", PLAIN,
         "No modelled input explains this flip: the order had to be forced "
         "directly.  That means the mechanism is outside the current model — "
         "the local-alloc SUGGESTED-REGISTER pass (qty_phys_copy_sugg / "
         "qty_phys_sugg, reported-not-scored today), qty_size for DImode, or "
         "reload's spill-retry.  Extend the instrumentation before spending "
         "another spelling search."),
    ],
}

# --------------------------------------------------------------------------
# Never suggested.  Present so the report can say so explicitly: these are the
# constructs an agent reaches for when the honest lever looks unreachable.
# --------------------------------------------------------------------------
FORBIDDEN_FAMILIES = [
    ("register-asm-pins",
     "`register T x asm(\"$N\")` — DIAGNOSTIC-ONLY, never committable."),
    ("inline-asm-injection",
     "hardcoded-`$N` single-instruction `__asm__`, and `asm(\"Sym\")` alias "
     "renames — the same cheat as a regfix rule, in a different file."),
    ("lost-codegen-insert-cheat",
     "`insert_after \"addu $rN,$0,$zero\"` regfix to restore what const-prop "
     "ate — and regfix/asmfix rules generally: the engine's sandbox strips "
     "them before scoring, so they cannot even help."),
    ("dead-vars-local-array",
     "unused arrays / `(void)&local` frame coercion.  (The WRITTEN-never-read "
     "array carve-out applies only when the TARGET BYTES contain the stores.)"),
    ("legitimate-volatile-interrupt-touched",
     "volatile as a codegen coercion.  Only the IRQ-touched two-prong "
     "carve-out and type-level MMIO (0x1F801000-0x1F802FFF) qualify."),
    ("or-tree-shape-shift / strength-reduce-defeat / dead-branch-scheduling / "
     "goto-end-prologue-delay-slot",
     "archived FORBIDDEN tombstones — do not revive."),
]

# --------------------------------------------------------------------------
# Measured facts that kill plausible-looking lever choices.  Emitted with the
# matching class so the same negative is not re-derived.
# --------------------------------------------------------------------------
MEASURED_NEGATIVES = {
    REFS_UP: [
        "Dead stores are INERT for a reg_n_refs lift — flow.c deletes them "
        "BEFORE the references are counted (duplicated-statement-into-arms, "
        "measured).  A dead store is both a cheat AND ineffective here.",
    ],
    PREF_ADD: [
        "An ADDED preference numerically above the existing one is inert: "
        "find_reg takes the lowest preferred register.  Reroute instead.",
    ],
    BIRTH_ORDER: [
        "camera_set_zoom variant B (2026-08-05): producing the target's "
        "emission order in a 4-insn block did NOT flip the local-alloc "
        "assignment — final stream order is not necessarily alloc-time order.",
    ],
    ALLOC_ORDER: [
        "For a fully-pinned block (every insn accounted for in target), refs / "
        "span / birth are all constrained by the instruction stream; there may "
        "be no free parameter left to move.",
    ],
}


def explain(cls):
    """-> (candidates, negatives) for a perturbation class."""
    return LEVERS.get(cls, []), MEASURED_NEGATIVES.get(cls, [])


def format_report(classes, indent="  "):
    """Human-readable lever block for the set of classes in a solution."""
    out = []
    for cls in classes:
        cands, negs = explain(cls)
        out.append(f"{indent}{cls}:")
        for slug, tier, how in cands:
            tag = "" if tier == PLAIN else f"  [{tier}]"
            out.append(f"{indent}  - {slug}{tag}")
            for line in _wrap(how, 72):
                out.append(f"{indent}      {line}")
        for n in negs:
            for i, line in enumerate(_wrap(n, 70)):
                out.append(f"{indent}  {'! ' if i == 0 else '    '}{line}")
    return "\n".join(out)


def forbidden_block(indent="  "):
    out = [f"{indent}NEVER (these do not become available because the honest "
           f"lever is hard):"]
    for slug, why in FORBIDDEN_FAMILIES:
        out.append(f"{indent}  x {slug}")
        for line in _wrap(why, 70):
            out.append(f"{indent}      {line}")
    return "\n".join(out)


def _wrap(text, width):
    words, line, out = text.split(), "", []
    for w in words:
        if line and len(line) + 1 + len(w) > width:
            out.append(line)
            line = w
        else:
            line = f"{line} {w}".strip()
    if line:
        out.append(line)
    return out
