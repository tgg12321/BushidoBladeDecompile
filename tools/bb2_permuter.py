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

        # Choose source var + pin register
        src_decl = random.choice(candidate_decls)
        src_var = src_decl.name
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
            asmlabel=ca.Constant(type="string", value=f'"{pin_reg}"'),
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
        changes). The mutation is statement-count-preserving and minimal:
        the search space is (n_decls * n_pin_regs), typically <= 200 per
        function. Useful for exploring 'what if THIS local was pinned to
        $tN' without growing the function."""
        # Collect candidate decls: scalar / pointer types, no existing pin
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
                # Skip if already pinned (don't overwrite existing pins)
                if node.asmlabel is not None:
                    return self.generic_visit(node)
                # Skip if not a "register" storage already (we add register)
                # Actually we add storage too -- but only if the existing
                # decl has no incompatible storage class.
                if node.storage and any(
                    s in ("extern", "static") for s in node.storage
                ):
                    return self.generic_visit(node)
                candidates.append(node)
                self.generic_visit(node)

        _Collect().visit(fn)
        ensure(candidates)

        target = random.choice(candidates)
        pin_reg = random.choice(BB2_PIN_REGS)
        target.asmlabel = ca.Constant(
            type="string", value=f'"{pin_reg}"'
        )
        if "register" not in target.storage:
            target.storage = ["register"] + list(target.storage)

    # Default weights tuned for "lightest most useful, heavier as escape":
    #   perm_bb2_add_pin: 10.0 — small mutation, statement-count-preserving,
    #     comparable to upstream perm_temp_for_expr=100 / perm_ins_block=10
    #   perm_bb2_insert_aliasing: 0.5 — heavier, slower per iter, less likely
    #     to land usefully. Disabled by default; set BB2_PERMUTER_HEAVY=1 to
    #     enable, or BB2_PERMUTER_WEIGHT to scale both up together.
    enable_heavy = os.environ.get("BB2_PERMUTER_HEAVY", "0") == "1"
    return [
        (perm_bb2_add_pin, 10.0),
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

    from src.main import main as perm_main  # type: ignore
    return perm_main()


if __name__ == "__main__":
    sys.exit(main())
