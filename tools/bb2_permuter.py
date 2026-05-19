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


def _build_aliasing_pass():
    """Construct the auto-aliasing randomization pass and register it.

    Returns the pass function, which the caller can append to
    decomp-permuter's RANDOMIZATION_PASSES list.
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

    return perm_bb2_insert_aliasing


def _patch_randomizer(pass_weight: float = 1.0) -> None:
    """Append our pass to decomp-permuter's RANDOMIZATION_PASSES at runtime."""
    from src import randomizer  # type: ignore

    bb2_pass = _build_aliasing_pass()

    # decomp-permuter checks that every pass has a docstring; we set it on
    # the inner function via _build_aliasing_pass already.
    if not bb2_pass.__doc__:
        raise RuntimeError("Internal: pass missing docstring")

    randomizer.RANDOMIZATION_PASSES.append(bb2_pass)

    # We need to also patch the weights table loader. The simplest way is to
    # monkey-patch the file load step or inject our weight into the dict the
    # Randomizer constructor receives. The permuter loads weights from a
    # toml file (default_weights.toml); we'll patch the load to inject ours.
    import toml  # type: ignore

    orig_load = toml.load

    def patched_load(*args, **kwargs):
        data = orig_load(*args, **kwargs)
        # The weights TOML has a `[base]` section that get_default_weights
        # uses as the base layer (merged with compiler-specific weights).
        # Inject our pass weight there so the Randomizer constructor finds it.
        if isinstance(data, dict) and "base" in data and isinstance(data["base"], dict):
            data["base"][bb2_pass.__name__] = pass_weight
        return data

    toml.load = patched_load


def main() -> int:
    """Patch the randomizer, then delegate to decomp-permuter's main()."""
    # Optional environment knob to tune our pass weight
    pass_weight = float(os.environ.get("BB2_PERMUTER_WEIGHT", "1.0"))
    _patch_randomizer(pass_weight=pass_weight)
    print(
        f"[bb2_permuter] auto-aliasing pass registered "
        f"(weight={pass_weight}, pin_set={len(BB2_PIN_REGS)} regs)"
    )

    from src.main import main as perm_main  # type: ignore
    return perm_main()


if __name__ == "__main__":
    sys.exit(main())
