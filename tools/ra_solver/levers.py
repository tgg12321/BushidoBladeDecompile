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

# --- pre-RA (cse.c / combine.c / loop.c — the RTL the earlier passes build) -
CSE_MERGE = "cse_merge"            # one materialisation where we have two
CSE_SPLIT = "cse_split"            # two where we have one (CSE defeated)
RTL_SHAPE = "rtl_shape"            # the insn stream itself differs; no
                                   # downstream perturbation can reach it

# --- reload (reload1.c spill loop / retry_global_alloc) classes ------------
RELOAD_FORBIDDEN = "reload_forbidden"   # the `losers` / forbidden_regs set
RELOAD_PRESSURE = "reload_pressure"     # whether the pseudo is spilled at all

# --- scheduler (sched.c list scheduler) classes ----------------------------
DEP_ADD_TRUE = "dep_add_true"      # a true/data LOG_LINK edge added
DEP_ADD_ANTI = "dep_add_anti"      # an anti/output edge added
DEP_DROP = "dep_drop"              # an edge removed
LUID_ORDER = "luid_order"          # INSN_LUID order = source statement order
INSN_COST = "insn_cost"            # INSN_COST = instruction selection
SCHED_UNIT = "sched_unit"          # function unit (hazard side of selection)

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
    # ---- pre-RA (CSE / combine) -----------------------------------------
    CSE_MERGE: [
        ("store-const-reload-cse", PLAIN,
         "target materialises a value ONCE where we materialise it twice: "
         "re-read the GLOBAL (or keep reading the one lvalue) instead of "
         "caching a local, so cse.c sees one expression and keeps one "
         "register copy."),
        ("(single named intermediate)", PLAIN,
         "give the value ONE name and use it in both places, rather than "
         "writing the constant/expression out twice.  Two textual occurrences "
         "of the same constant are two RTL sets unless CSE unifies them."),
        ("named-local-fake-exception", SANCTIONED,
         "a constant-holder local, LAST RESORT: FAKE-annotated, documented "
         "lever exhaustion, layer-1+2 review.  Only after the honest single-"
         "naming above has been tried and measured."),
        ("staged-value-reused-variable", SANCTIONED,
         "stage the value through an EXISTING currently-dead local; live code "
         "only, zero dead stores, FAKE-annotated + exhaustion documented."),
    ],
    CSE_SPLIT: [
        ("split-read-defeats-hoist", PLAIN,
         "duplicate the read into the branch arms so CSE cannot unify them "
         "(SOTN-sanctioned)."),
        ("duplicated-statement-into-arms", SANCTIONED,
         "a REAL statement duplicated into 2+ arms; byte-neutrality verified, "
         "exhaustion documented, FAKE annotation, layer-1/2 review."),
        ("defeat-combine-symbol-fold", PLAIN,
         "pre-compute a displaced pointer so combine cannot fold the "
         "displacement into the addressing mode."),
    ],
    RTL_SHAPE: [
        ("(upstream of every model)", PLAIN,
         "The instruction MULTISET differs, so the residual is not an "
         "allocation or a schedule: it is the RTL the front end and the early "
         "optimisers built.  No perturbation of RA or scheduler inputs can "
         "reach it — those models permute and rename a FIXED set of insns.  "
         "Fix the expression the C computes, then re-derive."),
    ],
    # ---- reload ---------------------------------------------------------
    RELOAD_FORBIDDEN: [
        ("(register pressure at the reload point)", PLAIN,
         "forbidden_regs is reload's `losers` argument — the registers reload "
         "spilled to satisfy this insn's max_needs.  Which registers land there "
         "follows from how many values are simultaneously live AT THAT INSN, so "
         "the lever is pressure: fewer live values across the point (split a "
         "long-lived variable, sink a computation past it, or reduce the "
         "operand count of the insn that needed the reload)."),
        ("(operand shape)", PLAIN,
         "max_needs is decided by find_reloads from the INSN's operands — a "
         "memory operand needing an address register, a constant needing "
         "materialisation.  Changing the expression changes the need."),
    ],
    RELOAD_PRESSURE: [
        ("(avoid the spill entirely)", PLAIN,
         "the cleanest answer to a retry is usually not to steer it but to "
         "prevent it: if the pseudo is not evicted, the initial global_alloc "
         "assignment stands and the pre-reload model (simulate.py) governs.  "
         "Reduce live values across the spill point."),
    ],
    # ---- scheduler ------------------------------------------------------
    # NOTE the asymmetry with the RA classes.  A dependence edge is the one
    # perturbation whose most OBVIOUS spelling is a cheat: `volatile`, a
    # memory clobber, or an `__asm__` barrier all create edges directly.  Those
    # are FORBIDDEN, so the mapping routes edge atoms exclusively to
    # restructures that make the dependence REAL — the data genuinely flows, or
    # the two accesses genuinely alias.
    DEP_ADD_TRUE: [
        ("(value flow)", PLAIN,
         "make the dependence real: the second statement must CONSUME what the "
         "first produces.  Name the intermediate and use it, instead of "
         "recomputing or reading the source twice."),
        ("walking-pointer-serializes-parallel-loads", PLAIN,
         "walk the array with post-increment pointers (`*ap++`); the pointer "
         "dependence serialises otherwise-parallel loads, which is the "
         "legitimate way to stop the scheduler stealing later loads into delay "
         "slots.  Explicitly the SANCTIONED alternative to the memory-clobber "
         "barrier and the per-load register pin."),
        ("store-before-jal", PLAIN,
         "ordering recipe for a value saved across a call site."),
        ("defer-store-past-later-compute-into-jal-delay", PLAIN,
         "hoist the stored value into a local and move the store statement "
         "AFTER a later independent compute, so the sw schedules into the "
         "jal delay slot."),
    ],
    DEP_ADD_ANTI: [
        ("(genuine aliasing)", PLAIN,
         "an anti/output edge means two accesses touch the SAME location. The "
         "honest producers are: access both through the same pointer/lvalue, or "
         "read and write the same object rather than two separate ones.  If the "
         "accesses genuinely cannot alias, this edge is not reachable in C and "
         "the answer is a different vector."),
        ("legitimate-volatile-interrupt-touched", SANCTIONED,
         "the ONLY volatile route, and it is a narrow gate, not a lever: the "
         "global must be genuinely IRQ-touched (two-prong test) or a type-level "
         "MMIO register (0x1F801000-0x1F802FFF).  sched.c's read_dependence "
         "needs BOTH reads volatile.  Qualify the SYMBOL first; never add "
         "volatile because an edge was wanted."),
    ],
    DEP_DROP: [
        ("(break the aliasing)", PLAIN,
         "split the value, or access the two locations through distinct objects "
         "so the compiler can prove independence."),
        ("split-read-defeats-hoist", PLAIN,
         "duplicate the read into the arms so the shared edge disappears."),
        ("loop-exit-work-inside-loop-sched-fence", PLAIN,
         "move the loop's exit work INSIDE the loop (`if (cond) continue; tail; "
         "break;`) so post-loop inits stop being hoisted above a tail store."),
    ],
    LUID_ORDER: [
        ("(statement order)", PLAIN,
         "INSN_LUID is source order and is the scheduler's final tie-break — "
         "the cheapest atom to spell.  Move the generating statement."),
        ("switch-break-shared-return-sched-hoist", PLAIN,
         "`break;` + one shared trailing `return 0;` instead of per-case "
         "returns, so sched1 stops hoisting the v0-set into a load-delay slot."),
        ("loop-note-fixes-delay-slot-steal", PLAIN,
         "write the loop as a real `while`/`do` rather than relying on a "
         "barrier to block a delay-slot steal."),
        ("hoist-call-arg-local-flips-jal-delay", PLAIN,
         "hoist the late-loaded call argument into a local declared FIRST in "
         "the block."),
    ],
    INSN_COST: [
        ("(instruction selection)", PLAIN,
         "INSN_COST is a property of the INSTRUCTION CHOSEN, not a knob: a load "
         "is 2, a move 1, a multiply 12.  Changing it means computing the value "
         "a different way (shift vs multiply, cached local vs re-load).  If the "
         "target's instruction is the same as ours, this atom is not spellable "
         "and the vector is fiction."),
    ],
    SCHED_UNIT: [
        ("(instruction selection)", PLAIN,
         "same as INSN_COST seen from the hazard side — the function unit "
         "follows from the opcode.  Only reachable by changing the operation."),
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
    DEP_ADD_ANTI: [
        "The obvious spellings of a new anti/output edge — `volatile`, a "
        "`memory` clobber, an `__asm__` scheduling barrier — are ALL forbidden "
        "families. An anti-edge vector is only real if the two accesses "
        "genuinely alias; otherwise report the vector as unspellable rather "
        "than reaching for a barrier.",
    ],
    DEP_ADD_TRUE: [
        "Per walking-pointer-serializes-parallel-loads: memory-clobber barriers "
        "and per-load register pins were the ORIGINAL (rejected) fix for this "
        "shape; the pointer-walk restructure is what replaced them.",
    ],
    INSN_COST: [
        "INSN_COST / unit atoms are only spellable when target's instruction "
        "actually differs from ours. Check the two streams before believing a "
        "cost vector — otherwise it is describing a change C cannot make.",
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
