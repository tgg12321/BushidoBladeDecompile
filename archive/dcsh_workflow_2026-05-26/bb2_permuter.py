#!/usr/bin/env python3
"""
bb2_permuter.py -- BB2-targeted wrapper around decomp-permuter.

Phase 1 deliverable: injects an auto-aliasing randomization pass into
decomp-permuter at runtime, then delegates to upstream's main(). The
new pass inserts SOTN-accepted inline-move-aliasing blocks at random
points in the AST:

    register T x_pinned asm("$N");
    __asm__ volatile("move %0, %1" : "=r"(x_pinned) : "r"(x));

Pin set: $t0-$t9 + $v0-$v1 + $s0-$s7 (20 registers — locked in plan).
Output: standard upstream output (permuter/<func>/output-*/ on improvement).
Scoring: mips-gcc only (cc1psx deprecated).

Usage:
    python3 tools/bb2_permuter.py permuter/<func>/ --stop-on-zero --best-only -j 6

Wraps `python3 tools/decomp-permuter/permuter.py` after monkey-patching
the randomizer's pass list.
"""
from __future__ import annotations

import os
import random as random_module
import sys
from pathlib import Path
from typing import List, Optional


# Make decomp-permuter importable
PROJECT_ROOT = Path(__file__).resolve().parent.parent
PERM_ROOT = PROJECT_ROOT / "tools" / "decomp-permuter"
if not PERM_ROOT.exists():
    sys.exit(f"ERROR: {PERM_ROOT} missing — set up decomp-permuter first")
sys.path.insert(0, str(PERM_ROOT))
sys.path.insert(0, str(PERM_ROOT / "src"))


# Pin set (per design decision: $t0-$t9, $v0-$v1, $s0-$s7)
BB2_PIN_REGS: List[str] = (
    [f"$t{i}" for i in range(10)]
    + ["$v0", "$v1"]
    + [f"$s{i}" for i in range(8)]
)


# GCC 2.7.2's cc1 wants numeric register names in `asm("$N")` constraints
# ($16 not $s0). Symbolic names are accepted by SN cc1 but our cc1 rejects
# them as "invalid register name". Map symbolic -> numeric before emitting.
_REG_NAME_TO_NUM = {
    "$v0": "$2", "$v1": "$3",
    "$a0": "$4", "$a1": "$5", "$a2": "$6", "$a3": "$7",
    "$t0": "$8", "$t1": "$9", "$t2": "$10", "$t3": "$11",
    "$t4": "$12", "$t5": "$13", "$t6": "$14", "$t7": "$15",
    "$s0": "$16", "$s1": "$17", "$s2": "$18", "$s3": "$19",
    "$s4": "$20", "$s5": "$21", "$s6": "$22", "$s7": "$23",
    "$t8": "$24", "$t9": "$25",
}


def _to_numeric_reg(pin: str) -> str:
    return _REG_NAME_TO_NUM.get(pin, pin)


# Phase 2 v1: target-aware register weighting. At wrapper init we read
# the permuter dir's target.s and count register frequencies. The pin
# pass uses these as weights for `random.choices` so the search biases
# toward registers target actually uses (typically ~5 distinct registers
# per function out of 20 candidates).
#
# Phase 2 v2: target-derived top-N register list (smaller fixed set,
# typically 5 entries vs 20). Used by the pin pass when picking a
# "preferred" register without dilution by long-tail noise.
#
# Phase 3: argument-pin destinations -- registers that are written via
# `addu $rD, $aN, $zero` patterns in target.s. These are very-high-signal
# pin initializations from function parameters; the n-th such instruction
# in target's instruction order maps to the n-th register-pinned local
# in source order. Empty list if target has no arg-pin moves.
#
# All populated by `_load_target_register_weights(permuter_dir)` at
# main() entry.
_TARGET_REG_WEIGHTS: List = []
_TARGET_TOP_REGS: List[str] = []
_TARGET_ARG_PIN_DSTS: List[str] = []


def _weighted_choice(weights: List, random_obj) -> str:
    """Pick a register from weights=[(reg, weight), ...] using the given
    Random instance. Mirrors random.choices behavior but works with a list
    of (item, weight) tuples and uses the random_obj for determinism with
    permuter seeds."""
    total = sum(w for _, w in weights)
    r = random_obj.random() * total
    cum = 0.0
    for reg, w in weights:
        cum += w
        if r < cum:
            return reg
    return weights[-1][0]


def _load_target_register_weights(permuter_dir: Path):
    """Parse permuter_dir/target.s and return three artifacts:

      weighted_list: [(reg, weight), ...] for random.choices over the full
        pin set (smoothing factor +1) -- Phase 2 v1
      top_regs: [reg, ...] -- the top-N registers by frequency (N=5),
        filtered to BB2_PIN_REGS, ordered most-used first -- Phase 2 v2
      arg_pin_dsts: [reg, ...] -- destinations of `addu $rD, $aN, $0`
        instructions in target.s (parameter-to-register pin inits), in
        instruction order, filtered to BB2_PIN_REGS -- Phase 3
    """
    target_s = permuter_dir / "target.s"
    if not target_s.exists():
        return [(r, 1.0) for r in BB2_PIN_REGS], [], []

    import re
    from collections import Counter

    text = target_s.read_text(encoding="utf-8", errors="ignore")
    found = re.findall(r"\$(t[0-9]|s[0-7]|v[01]|a[0-3])\b", text)
    counter = Counter(found)

    weights: List = []
    for pin in BB2_PIN_REGS:
        name = pin.lstrip("$")
        weights.append((pin, float(counter.get(name, 0)) + 1.0))

    # Top-N by frequency
    bb2_set = set(BB2_PIN_REGS)
    top_regs = []
    for name, _count in counter.most_common():
        pin = f"${name}"
        if pin in bb2_set:
            top_regs.append(pin)
        if len(top_regs) >= 5:
            break

    # Phase 3: parse the chain of register-pin initializations from
    # target.s. We collect `addu $rD, $rS, $zero` instructions whose source
    # is either an argument register ($aN) OR a previously-discovered pin
    # destination -- this captures both direct arg pins
    # (`register T x asm("$s0") = arg0` -> `addu $s0, $a0, $0`) and
    # aliasing-chain pins (`register T y asm("$s2") = x` -> `addu $s2,
    # $s0, $0`) in declaration order.
    #
    # Bounded to the prologue: we only scan the first PROLOGUE_INSN_LIMIT
    # instruction-bearing lines, since pin inits are always in the prologue
    # (a `addu $rD, $a0, $0` deep in the function body is a re-pinning, not
    # an aliasing pin -- the func_8002D320 case in the regression suite).
    addu_zero_re = re.compile(
        r"\baddu\s+\$(\w+)\s*,\s*\$(\w+)\s*,\s*\$(0|zero)\b"
    )
    # Any MIPS-style instruction line begins with optional comment then
    # indent then mnemonic. We use a permissive "is this an instruction
    # line" detector to count prologue instructions.
    insn_re = re.compile(r"^\s*(?:/\*.*?\*/\s*)?[a-z]")
    arg_regs = {"a0", "a1", "a2", "a3"}
    arg_pin_dsts: List[str] = []
    pinned_names = set()

    PROLOGUE_INSN_LIMIT = 25
    insn_count = 0
    for line in text.split("\n"):
        if not insn_re.search(line):
            continue
        insn_count += 1
        if insn_count > PROLOGUE_INSN_LIMIT:
            break
        m = addu_zero_re.search(line)
        if not m:
            continue
        dst_name = m.group(1)
        src_name = m.group(2)
        if src_name not in arg_regs and src_name not in pinned_names:
            continue
        pin = f"${dst_name}"
        if pin not in bb2_set:
            continue
        if dst_name in pinned_names:
            continue
        arg_pin_dsts.append(pin)
        pinned_names.add(dst_name)
        if len(arg_pin_dsts) >= 8:
            break

    return weights, top_regs, arg_pin_dsts


def _build_passes():
    """Construct the BB2 randomization passes and return them.

    Two passes are built:
      perm_bb2_add_pin       — annotate an existing local Decl with a
                                register asm() pin (no statement-count
                                change; smallest possible mutation)
      perm_bb2_insert_aliasing — insert a new register-pinned local + an
                                inline-move asm and redirect one downstream
                                read (statement-count + 2; bigger mutation)

    Returns a list of (pass_fn, default_weight) tuples.
    """
    from perm_pycparser import c_ast as ca  # type: ignore
    from src import ast_util  # type: ignore
    from src.randomizer import (  # type: ignore
        RandomizationFailure,
        Region,
        ensure,
        get_insertion_points,
    )

    def perm_bb2_insert_aliasing(
        fn: "ca.FuncDef",
        ast: "ca.FileAST",
        indices: "Indices",
        region: "Region",
        random: "random_module.Random",
    ) -> None:
        """BB2: Insert a SOTN-accepted inline-move-aliasing block at a random
        position AND redirect one subsequent read of the source variable to
        the pinned alias. The block has the shape:
            register T pin asm("$N");
            __asm__ volatile("move %0, %1" : "=r"(pin) : "r"(source));
            ... (next read of source) ...  ->  ... (now reads pin) ...
        where T is the source variable's declared type, $N is from the
        configured pin set, and `source` is an existing in-scope variable.

        Replacing a downstream read is critical: without it, GCC's DCE
        eliminates the pinned local (since it's never used) and the asm
        becomes a useless side-effect. With one downstream use redirected,
        GCC must materialize the register pin, producing useful bytes."""
        # Pick an insertion point
        cands = get_insertion_points(fn, region)
        ensure(cands)

        # Walk the function to find candidate source-variable declarations
        # (scalar / pointer types only) and remember their actual types so we
        # generate a compatible pinned local.
        candidate_decls: List["ca.Decl"] = []

        class _Collect(ca.NodeVisitor):
            def visit_Decl(self, node: "ca.Decl") -> None:  # noqa: N802
                if not node.name:
                    return self.generic_visit(node)
                t = node.type
                # Accept TypeDecl (scalar) or PtrDecl (pointer) only
                if isinstance(t, (ca.ArrayDecl,)):
                    return self.generic_visit(node)
                if isinstance(t, ca.TypeDecl) and isinstance(
                    getattr(t, "type", None), (ca.Struct, ca.Union)
                ):
                    return self.generic_visit(node)
                candidate_decls.append(node)
                self.generic_visit(node)

        _Collect().visit(fn)
        ensure(candidate_decls)

        # Choose source var + pin register. Pin reg uses the target-aware
        # weighted distribution if available (Phase 2), falling back to
        # uniform over BB2_PIN_REGS.
        src_decl = random.choice(candidate_decls)
        src_var = src_decl.name
        if _TARGET_REG_WEIGHTS:
            pin_reg = _weighted_choice(_TARGET_REG_WEIGHTS, random)
        else:
            pin_reg = random.choice(BB2_PIN_REGS)
        pin_name = f"_bb2_pin_{random.randint(1, 10**6):06d}"

        # Pick the insertion point AFTER we've identified candidate src vars
        # (any cand point is fine; we'll later find a downstream read).
        tob, toi, _, _ = random.choice(cands)

        # Build the AST for the aliasing block.
        # 1. The pinned local with the SAME TYPE as the source decl, so the
        #    asm operand constraint binds cleanly. We deep-copy src_decl's
        #    type tree and rename it.
        from copy import deepcopy
        pin_type = deepcopy(src_decl.type)
        # Rename the declname at the leaf (TypeDecl) so this decl's name is
        # pin_name, not src_var.
        leaf = pin_type
        while True:
            if isinstance(leaf, ca.TypeDecl):
                leaf.declname = pin_name
                break
            if hasattr(leaf, "type") and leaf.type is not None:
                leaf = leaf.type
            else:
                break

        pin_decl = ca.Decl(
            name=pin_name,
            quals=[],
            align=None,
            storage=["register"],
            funcspec=[],
            gcc_attributes=[],
            type=pin_type,
            init=None,
            bitsize=None,
            asmlabel=ca.Constant(
                type="string", value=f'"{_to_numeric_reg(pin_reg)}"'
            ),
        )

        # 2. The asm move
        asm_node = ca.Asm(
            quals=["volatile"],
            asm=ca.Constant(type="string", value='"move %0, %1"'),
            output_operands=[
                ca.AsmOperand(
                    symbolic_name=None,
                    constraint=ca.Constant(type="string", value='"=r"'),
                    expr=ca.ID(name=pin_name),
                ),
            ],
            input_operands=[
                ca.AsmOperand(
                    symbolic_name=None,
                    constraint=ca.Constant(type="string", value='"r"'),
                    expr=ca.ID(name=src_var),
                ),
            ],
            clobbers=[],
            gotos=[],
        )

        # 3. Find a downstream read of src_var (i.e. an ID node referencing
        #    src_var that appears AFTER the insertion point in textual / AST
        #    order). Replace ONE of them with a reference to pin_name. This
        #    keeps the pinned local live so GCC can't DCE it.
        downstream_reads: List["ca.ID"] = []

        # Use ast_util.compute_node_indices (already in `indices`) to compare
        # positions. Walk the function body collecting ID nodes whose name is
        # src_var and whose index is after the chosen insertion position.
        # The insertion position's index is approximated by the index of the
        # statement currently at `tob[toi]` (if any).
        try:
            anchor_stmt = ast_util.get_block_stmts(tob, False)[toi]
            anchor_idx = indices.starts.get(anchor_stmt, 0)
        except (IndexError, AttributeError):
            anchor_idx = 0

        class _FindReads(ca.NodeVisitor):
            def visit_ID(self, node: "ca.ID") -> None:  # noqa: N802
                if node.name == src_var:
                    nidx = indices.starts.get(node, -1)
                    if nidx > anchor_idx:
                        downstream_reads.append(node)
                self.generic_visit(node)

        _FindReads().visit(fn)

        if downstream_reads:
            # Replace the first downstream read with pin_name. We don't need
            # to mutate the AST structurally -- ca.ID is a simple node whose
            # name attribute we can overwrite. (The code generator reads
            # node.name when emitting.)
            chosen_read = random.choice(downstream_reads)
            chosen_read.name = pin_name

        # 4. Insert the aliasing-block statements (decl, then asm) at the
        #    chosen insertion point. ast_util.insert_statement inserts BEFORE
        #    the given index, so insert asm first, then decl (decl ends up
        #    above asm in the final output).
        ast_util.insert_statement(tob, toi, asm_node)
        ast_util.insert_statement(tob, toi, pin_decl)

    def perm_bb2_add_pin(
        fn: "ca.FuncDef",
        ast: "ca.FileAST",
        indices: "Indices",
        region: "Region",
        random: "random_module.Random",
    ) -> None:
        """BB2: Annotate an existing local Decl with a register-asm pin (no
        new statements -- only the asmlabel attribute on the existing decl
        changes).

        Selection strategy (Phase 2 v2):
        1. With 60% probability, use POSITIONAL PAIRING: pair the n-th local
           Decl with the n-th most-used target register. This is the most
           "guess-the-original-source" mode -- if target's developer pinned
           their first local to their preferred-first register, we land it.
        2. With 30% probability, use target-frequency-weighted random reg
           on a random Decl (Phase 2 v1 behavior, broader exploration).
        3. With 10% probability, fully random (covers edge cases the target
           weighting under-explores).
        """
        # Collect candidate decls in declaration order
        candidates: List["ca.Decl"] = []

        class _Collect(ca.NodeVisitor):
            def visit_Decl(self, node: "ca.Decl") -> None:  # noqa: N802
                if not node.name:
                    return self.generic_visit(node)
                t = node.type
                if isinstance(t, ca.ArrayDecl):
                    return self.generic_visit(node)
                if isinstance(t, ca.TypeDecl) and isinstance(
                    getattr(t, "type", None), (ca.Struct, ca.Union)
                ):
                    return self.generic_visit(node)
                if node.asmlabel is not None:
                    return self.generic_visit(node)
                if node.storage and any(
                    s in ("extern", "static") for s in node.storage
                ):
                    return self.generic_visit(node)
                candidates.append(node)
                self.generic_visit(node)

        _Collect().visit(fn)
        ensure(candidates)

        # Choose selection strategy. Probability budget (when full target
        # info is available):
        #   50% — Phase 3 arg-pin mapping: n-th Decl ↔ n-th arg-pin-dst.
        #         High signal when target has clear arg→reg pin inits.
        #   20% — Phase 2 v2 positional pairing using top-frequency regs.
        #         Fallback when arg-pin signal is short or absent.
        #   20% — Phase 2 v1 target-weighted random.
        #   10% — Phase 1 uniform random (escape valve).
        r = random.random()
        target = None
        pin_reg = None

        if r < 0.50 and _TARGET_ARG_PIN_DSTS and candidates:
            # Strategy: arg-pin mapping. The n-th in-scope Decl is paired
            # with the n-th destination from target's arg-pin moves.
            max_pair = min(len(candidates), len(_TARGET_ARG_PIN_DSTS))
            idx = random.randrange(max_pair)
            target = candidates[idx]
            pin_reg = _TARGET_ARG_PIN_DSTS[idx]
        elif r < 0.70 and _TARGET_TOP_REGS and candidates:
            # Strategy: top-frequency positional pairing
            max_pair = min(len(candidates), len(_TARGET_TOP_REGS))
            idx = random.randrange(max_pair)
            target = candidates[idx]
            pin_reg = _TARGET_TOP_REGS[idx]
        elif r < 0.90 and _TARGET_REG_WEIGHTS:
            # Strategy: target-weighted random reg, random Decl
            target = random.choice(candidates)
            pin_reg = _weighted_choice(_TARGET_REG_WEIGHTS, random)
        else:
            # Strategy: uniform random (Phase 1 baseline)
            target = random.choice(candidates)
            pin_reg = random.choice(BB2_PIN_REGS)

        pin_numeric = _to_numeric_reg(pin_reg)
        target.asmlabel = ca.Constant(
            type="string", value=f'"{pin_numeric}"'
        )
        if "register" not in target.storage:
            target.storage = ["register"] + list(target.storage)

    def perm_bb2_type_qualifier(
        fn: "ca.FuncDef",
        ast: "ca.FileAST",
        indices: "Indices",
        region: "Region",
        random: "random_module.Random",
    ) -> None:
        """BB2: Toggle a type qualifier on an existing Decl. SOTN-accepted
        mutation that produced 2 matches in BB2 backlog runs:
        func_80086014 via `register volatile unsigned int`, and
        func_8007D9C4 via `extern volatile int *`.

        Operates on:
          * Local Decls that already have a `register asm(...)` pin
            (volatile-upgrade pattern -- func_80086014's match)
          * File-scope `extern T *X` decls (extern-type-mutation pattern --
            func_8007D9C4's match)

        Mutations:
          * toggle_volatile (the productive mutation in both observed matches)
          * change_signedness (less productive but cheap)

        Kept narrowly scoped so it doesn't dilute the search budget on
        functions where add_pin is the productive pass.
        """
        # Two-category candidate selection:
        # (A) function-local Decls that ALREADY have a register pin
        # (B) file-scope extern Decls
        cat_a: List["ca.Decl"] = []
        cat_b: List["ca.Decl"] = []

        class _CollectLocals(ca.NodeVisitor):
            def visit_Decl(self, node: "ca.Decl") -> None:  # noqa: N802
                if not node.name:
                    return self.generic_visit(node)
                t = node.type
                if isinstance(t, ca.TypeDecl) and isinstance(
                    getattr(t, "type", None), (ca.Struct, ca.Union, ca.Enum)
                ):
                    return self.generic_visit(node)
                if isinstance(t, ca.FuncDecl):
                    return self.generic_visit(node)
                # Only pinned locals (asmlabel present)
                if node.asmlabel is not None:
                    cat_a.append(node)
                self.generic_visit(node)

        class _CollectExterns(ca.NodeVisitor):
            def visit_Decl(self, node: "ca.Decl") -> None:  # noqa: N802
                if not node.name:
                    return self.generic_visit(node)
                t = node.type
                if isinstance(t, ca.FuncDecl):
                    return self.generic_visit(node)
                if isinstance(t, ca.TypeDecl) and isinstance(
                    getattr(t, "type", None), (ca.Struct, ca.Union, ca.Enum)
                ):
                    return self.generic_visit(node)
                if node.storage and "extern" in node.storage:
                    cat_b.append(node)
                self.generic_visit(node)

        _CollectLocals().visit(fn)
        _CollectExterns().visit(ast)
        ensure(cat_a or cat_b)

        # 50/50 between the two categories (when both populated). If only
        # one is populated, use that.
        if cat_a and cat_b:
            pool = cat_a if random.random() < 0.5 else cat_b
        else:
            pool = cat_a or cat_b
        target = random.choice(pool)

        # Most productive mutation is volatile-toggle; signedness is
        # rarer but occasionally useful for `signed int` vs `int` cases.
        mutation = "toggle_volatile" if random.random() < 0.75 else "change_signedness"

        # Use TypeDecl.quals for qualifiers
        type_decl = target.type
        # Walk through PtrDecl/ArrayDecl wrappers to find the TypeDecl
        inner = type_decl
        while isinstance(inner, (ca.PtrDecl, ca.ArrayDecl)):
            inner = inner.type
        if not isinstance(inner, ca.TypeDecl):
            raise RandomizationFailure()

        if mutation == "toggle_volatile":
            # PtrDecl carries its own quals; if the type is `T *`, prefer
            # toggling on the PtrDecl (so we get `T * volatile` or
            # `volatile T *`-style).
            host = type_decl if isinstance(type_decl, ca.PtrDecl) else inner
            quals = list(host.quals or [])
            if "volatile" in quals:
                quals.remove("volatile")
            else:
                quals.append("volatile")
            host.quals = quals
            # Also force the Decl-level quals to align (pycparser sometimes
            # tracks them separately)
            decl_quals = list(target.quals or [])
            if "volatile" in decl_quals and mutation == "toggle_volatile":
                decl_quals.remove("volatile") if "volatile" not in quals else None
            if "volatile" in quals and "volatile" not in decl_quals:
                decl_quals.append("volatile")
            target.quals = decl_quals
        elif mutation == "toggle_register":
            storage = list(target.storage or [])
            if "register" in storage:
                storage.remove("register")
            else:
                storage.append("register")
            target.storage = storage
        elif mutation == "change_signedness":
            # Find a plain IdentifierType and try to toggle its signedness.
            id_type = getattr(inner, "type", None)
            if not isinstance(id_type, ca.IdentifierType):
                raise RandomizationFailure()
            names = list(id_type.names)
            if "signed" in names:
                names = [n if n != "signed" else "unsigned" for n in names]
            elif "unsigned" in names:
                names = [n for n in names if n != "unsigned"]
                if not any(n in names for n in ("int", "char", "short", "long")):
                    names.append("int")
            else:
                # Add `unsigned` to a plain `int`-ish base
                if any(n in names for n in ("int", "char", "short", "long")):
                    names = ["unsigned"] + names
                else:
                    raise RandomizationFailure()
            id_type.names = names

    def perm_bb2_scheduler_perturb(
        fn: "ca.FuncDef",
        ast: "ca.FileAST",
        indices: "Indices",
        region: "Region",
        random: "random_module.Random",
    ) -> None:
        """BB2: Wrap a SHORT range of adjacent statements in
        `do { ... } while (0)`. Targets the score-60 plateau cases
        where the regfix rule is a single-pair reorder; coalescing
        two adjacent statements into one schedule unit often lets
        GCC produce the target's instruction order.

        Differs from upstream perm_ins_block in two ways:
          1. Range size is bounded to 2-3 statements (the productive
             size for plateau-60 cases; longer ranges rarely match
             and just dilute the search).
          2. Always uses do-while (the productive form observed in
             func_80077894's match -- if(1) wrapping doesn't have the
             same scheduler effect because GCC's optimizer can elide
             the conditional check).
        """
        from src import ast_util  # type: ignore

        # Collect all block-containing statements
        blocks: List["Block"] = []

        def rec(block):
            blocks.append(block)
            for stmt in ast_util.get_block_stmts(block, False):
                ast_util.for_nested_blocks(stmt, rec)

        rec(fn.body)
        ensure(blocks)
        block = random.choice(blocks)
        stmts = ast_util.get_block_stmts(block, True)
        # Skip leading Decls (can't wrap them in do-while safely)
        decl_count = 0
        for s in stmts:
            if isinstance(s, (ca.Decl, ca.Pragma)):
                decl_count += 1
            else:
                break
        # Need at least 2 non-decl statements to pick a 2-stmt range
        if len(stmts) - decl_count < 2:
            raise RandomizationFailure()
        # Pick a starting index from non-decl region; pick range size 2 or 3
        max_start = len(stmts) - 2
        lo = random.randrange(decl_count, max_start + 1)
        range_size = 2 if random.random() < 0.7 else 3
        hi = min(lo + range_size, len(stmts))
        if hi - lo < 2:
            raise RandomizationFailure()
        # Wrap [lo, hi) in do { ... } while (0)
        if not all(region.contains_node(n) for n in stmts[lo:hi]):
            raise RandomizationFailure()
        new_block = ca.Compound(block_items=stmts[lo:hi])
        cond = ca.Constant(type="int", value="0")
        stmts[lo:hi] = [
            ca.Pragma("_permuter sameline start"),
            ca.DoWhile(cond=cond, stmt=new_block),
            ca.Pragma("_permuter sameline end"),
        ]

    def perm_bb2_strength_reduce(
        fn: "ca.FuncDef",
        ast: "ca.FileAST",
        indices: "Indices",
        region: "Region",
        random: "random_module.Random",
    ) -> None:
        """BB2: SOTN strength-reduce-defeat mutations. Find arithmetic
        with a power-of-2 constant operand and rewrite into an
        equivalent form, e.g.:

            x * 4   <->  x << 2  <->  2 * (2 * x)
            x / 4   <->  x >> 2
            x % 4   <->  x & 3   (for unsigned / non-negative)

        Targets the mario_getMarioVoiceData_8005BE84 match pattern --
        `(arg_save * 4)` -> `(2 * (2 * arg_save))` shifted GCC's
        strength-reduction enough to match. That mutation was found by
        random luck in perm_general; this pass tries it directly.

        We pick ONE candidate BinaryOp per iteration and apply ONE
        rewrite (randomly chosen from the productive set).
        """
        candidates: List["ca.BinaryOp"] = []

        class _Collect(ca.NodeVisitor):
            def visit_BinaryOp(self, node: "ca.BinaryOp") -> None:  # noqa: N802
                if node.op not in ("*", "/", "%", "<<", ">>"):
                    return self.generic_visit(node)
                # Need at least one constant operand
                for operand in (node.left, node.right):
                    if isinstance(operand, ca.Constant) and operand.type == "int":
                        try:
                            val = int(operand.value, 0)  # handles 0x / 0o / decimal
                        except (ValueError, TypeError):
                            continue
                        # Power of 2 with small log
                        if val > 0 and (val & (val - 1)) == 0 and val <= 65536:
                            candidates.append(node)
                            break
                self.generic_visit(node)

        _Collect().visit(fn)
        ensure(candidates)
        target = random.choice(candidates)

        # Determine which operand is the constant and which is the var
        if isinstance(target.left, ca.Constant):
            const_op = target.left
            var_op = target.right
        else:
            const_op = target.right
            var_op = target.left
        try:
            const_val = int(const_op.value, 0)
        except (ValueError, TypeError):
            raise RandomizationFailure()
        log2 = const_val.bit_length() - 1  # since power of 2

        # Pick a mutation that's valid for this op
        choices: list[str] = []
        if target.op == "*":
            # x * 4 -> x << 2  OR  x * 4 -> 2 * (2 * x)
            if log2 >= 1:
                choices.append("mul_to_shift")
            if log2 == 2:
                choices.append("mul_to_chain2")  # x*4 -> 2*(2*x)
            if log2 == 3:
                choices.append("mul_to_chain3")  # x*8 -> 2*(2*(2*x))
        elif target.op == "/":
            if log2 >= 1:
                choices.append("div_to_shift")
        elif target.op == "%":
            if log2 >= 1:
                choices.append("mod_to_mask")
        elif target.op == "<<":
            # x << 2 -> x * 4
            choices.append("shift_to_mul")
        elif target.op == ">>":
            # x >> 2 -> x / 4 (only safe for unsigned, but we try anyway --
            # the build will fail if signedness matters and the auditor
            # will reject; net effect is the mutation just doesn't help)
            choices.append("shift_to_div")
        if not choices:
            raise RandomizationFailure()
        mutation = random.choice(choices)

        def _make_const(v: int) -> "ca.Constant":
            return ca.Constant(type="int", value=str(v))

        if mutation == "mul_to_shift":
            new_node = ca.BinaryOp("<<", var_op, _make_const(log2))
        elif mutation == "mul_to_chain2":
            # x * 4 -> 2 * (2 * x)
            inner = ca.BinaryOp("*", _make_const(2), var_op)
            new_node = ca.BinaryOp("*", _make_const(2), inner)
        elif mutation == "mul_to_chain3":
            # x * 8 -> 2 * (2 * (2 * x))
            inner = ca.BinaryOp("*", _make_const(2), var_op)
            inner2 = ca.BinaryOp("*", _make_const(2), inner)
            new_node = ca.BinaryOp("*", _make_const(2), inner2)
        elif mutation == "div_to_shift":
            new_node = ca.BinaryOp(">>", var_op, _make_const(log2))
        elif mutation == "mod_to_mask":
            new_node = ca.BinaryOp("&", var_op, _make_const(const_val - 1))
        elif mutation == "shift_to_mul":
            new_node = ca.BinaryOp("*", var_op, _make_const(1 << const_val))
        elif mutation == "shift_to_div":
            new_node = ca.BinaryOp("/", var_op, _make_const(1 << const_val))
        else:
            raise RandomizationFailure()

        # In-place mutate the target node so the AST keeps its parent links
        target.op = new_node.op
        target.left = new_node.left
        target.right = new_node.right

    # Default weights tuned for "lightest most useful, heavier as escape":
    #   perm_bb2_add_pin: 10.0 — small mutation, statement-count-preserving,
    #     comparable to upstream perm_temp_for_expr=100 / perm_ins_block=10
    #   perm_bb2_scheduler_perturb: 3.0 — focused 2-3 stmt do-while wrap;
    #     produced func_80077894's match. Heavier than type_qualifier
    #     because it targets the largest plateau-cause category.
    #   perm_bb2_type_qualifier: 2.0 — narrow scope (pinned locals + externs);
    #     produces matches directly when a Decl's type was the obstacle.
    #     Tuned low to avoid diluting add_pin's search budget.
    #   perm_bb2_insert_aliasing: 0.5 — heavier, slower per iter, less likely
    #     to land usefully. Disabled by default; set BB2_PERMUTER_HEAVY=1 to
    #     enable, or BB2_PERMUTER_WEIGHT to scale both up together.
    enable_heavy = os.environ.get("BB2_PERMUTER_HEAVY", "0") == "1"
    return [
        (perm_bb2_add_pin, 10.0),
        (perm_bb2_scheduler_perturb, 3.0),
        (perm_bb2_strength_reduce, 2.5),
        (perm_bb2_type_qualifier, 2.0),
        (perm_bb2_insert_aliasing, 0.5 if enable_heavy else 0.0),
    ]


def _build_aliasing_pass():
    """Backwards-compatible alias for the single-pass return shape used in
    the original Phase 1 MVP. Returns just the heavier pass for callers
    that don't want both."""
    passes = _build_passes()
    # Return the insert_aliasing pass (matches the old single-return shape)
    for fn, _w in passes:
        if fn.__name__ == "perm_bb2_insert_aliasing":
            return fn
    return passes[0][0]


def _patch_randomizer(pass_weight_scale: float = 1.0) -> None:
    """Append our passes to decomp-permuter's RANDOMIZATION_PASSES list and
    inject their default weights into the loaded TOML.

    pass_weight_scale: multiplier applied to each pass's default weight (set
    to 0 to register but disable; set to 1.0 for defaults; set to >1 to bias
    the search toward bb2 passes).
    """
    from src import randomizer  # type: ignore

    bb2_passes = _build_passes()

    # Validate each pass before registering
    for pass_fn, _w in bb2_passes:
        if not pass_fn.__doc__:
            raise RuntimeError(f"Internal: {pass_fn.__name__} missing docstring")

    pass_weight_map = {
        pass_fn.__name__: weight * pass_weight_scale
        for pass_fn, weight in bb2_passes
    }

    for pass_fn, _w in bb2_passes:
        randomizer.RANDOMIZATION_PASSES.append(pass_fn)

    # Monkey-patch toml.load so the weights TOML loaded by upstream contains
    # entries for our passes (the Randomizer.__init__ check otherwise fails).
    import toml  # type: ignore

    orig_load = toml.load

    def patched_load(*args, **kwargs):
        data = orig_load(*args, **kwargs)
        if isinstance(data, dict) and "base" in data and isinstance(data["base"], dict):
            for name, weight in pass_weight_map.items():
                data["base"][name] = weight
        return data

    toml.load = patched_load
    return pass_weight_map


def main() -> int:
    """Patch the randomizer, then delegate to decomp-permuter's main()."""
    # Optional environment knob to scale our pass weights
    pass_weight_scale = float(os.environ.get("BB2_PERMUTER_WEIGHT", "1.0"))
    weights = _patch_randomizer(pass_weight_scale=pass_weight_scale)
    print(
        f"[bb2_permuter] {len(weights)} bb2 pass(es) registered "
        f"(weight_scale={pass_weight_scale}, pin_set={len(BB2_PIN_REGS)} regs)"
    )
    for name, w in weights.items():
        print(f"  {name}: weight={w}")

    # Phase 2-3: target-aware register selection. Look for the permuter
    # dir in argv (first non-flag arg) and load target.s register info.
    global _TARGET_REG_WEIGHTS, _TARGET_TOP_REGS, _TARGET_ARG_PIN_DSTS
    permuter_dir = None
    for arg in sys.argv[1:]:
        if not arg.startswith("-") and Path(arg).is_dir():
            permuter_dir = Path(arg)
            break
    if permuter_dir is not None:
        (
            _TARGET_REG_WEIGHTS,
            _TARGET_TOP_REGS,
            _TARGET_ARG_PIN_DSTS,
        ) = _load_target_register_weights(permuter_dir)
        if _TARGET_TOP_REGS:
            top_str = ", ".join(_TARGET_TOP_REGS)
            print(f"[bb2_permuter] target.s top-{len(_TARGET_TOP_REGS)} regs: {top_str}")
        if _TARGET_ARG_PIN_DSTS:
            pin_str = " -> ".join(_TARGET_ARG_PIN_DSTS)
            print(f"[bb2_permuter] target.s arg-pin dsts (in order): {pin_str}")
        elif not _TARGET_TOP_REGS:
            print(f"[bb2_permuter] target.s not found or uninformative; uniform reg weights")

    from src.main import main as perm_main  # type: ignore
    return perm_main()


if __name__ == "__main__":
    sys.exit(main())
