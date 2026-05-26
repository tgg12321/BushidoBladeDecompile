#!/usr/bin/env python3
"""Check one function plus its nearby caller/callee cluster."""

from __future__ import annotations

import argparse
import os
import tempfile
from collections import defaultdict, deque
from pathlib import Path

from func_tooling import (
    compare_functions_in_object,
    compile_c_file_to_object,
    find_sections_containing_addr,
    get_all_func_info,
    get_original_callgraph,
    infer_source_file,
    invert_callgraph,
    load_map_symbols_sorted,
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("func_name", help="Anchor function")
    parser.add_argument(
        "--depth",
        type=int,
        default=1,
        help="Caller/callee expansion depth",
    )
    parser.add_argument(
        "--callers-only",
        action="store_true",
        help="Only include callers of the anchor",
    )
    parser.add_argument(
        "--callees-only",
        action="store_true",
        help="Only include callees of the anchor",
    )
    parser.add_argument(
        "--object-neighbors",
        type=int,
        default=1,
        help="Also include this many previous/next symbols from the same .text object span",
    )
    return parser.parse_args()


def walk_neighbors(seed: str, graph: dict[str, set[str]], depth: int) -> set[str]:
    if depth <= 0:
        return set()
    visited: set[str] = set()
    queue = deque([(seed, 0)])
    while queue:
        node, dist = queue.popleft()
        if dist >= depth:
            continue
        for neighbor in sorted(graph.get(node, ())):
            if neighbor in visited or neighbor == seed:
                continue
            visited.add(neighbor)
            queue.append((neighbor, dist + 1))
    return visited


def get_object_neighbors(func_name: str, count: int, all_funcs) -> set[str]:
    if count <= 0:
        return set()
    info = all_funcs[func_name]
    sections = [
        section
        for section in find_sections_containing_addr(info.addr)
        if section.section == ".text"
    ]
    if not sections:
        return set()

    section = sections[0]
    in_section = [
        name
        for addr, name in load_map_symbols_sorted()
        if section.addr <= addr < section.end and name in all_funcs
    ]
    if func_name not in in_section:
        return set()

    idx = in_section.index(func_name)
    start = max(0, idx - count)
    end = min(len(in_section), idx + count + 1)
    return set(in_section[start:idx] + in_section[idx + 1 : end])


def main() -> int:
    args = parse_args()
    all_funcs = get_all_func_info()
    if args.func_name not in all_funcs:
        raise SystemExit(f"{args.func_name} not found in asm metadata")

    callgraph = get_original_callgraph()
    callers = invert_callgraph(callgraph)

    include_callers = not args.callees_only
    include_callees = not args.callers_only

    cluster = {args.func_name}
    caller_set = walk_neighbors(args.func_name, callers, args.depth) if include_callers else set()
    callee_set = walk_neighbors(args.func_name, callgraph, args.depth) if include_callees else set()
    object_neighbor_set = get_object_neighbors(args.func_name, args.object_neighbors, all_funcs)
    cluster.update(caller_set)
    cluster.update(callee_set)
    cluster.update(object_neighbor_set)

    print(f"Anchor: {args.func_name}")
    if caller_set:
        print("Callers:")
        for name in sorted(caller_set):
            print(f"  {name}")
    if callee_set:
        print("Callees:")
        for name in sorted(callee_set):
            print(f"  {name}")
    if object_neighbor_set:
        print("Object neighbors:")
        for name in sorted(object_neighbor_set, key=lambda item: all_funcs[item].addr):
            print(f"  {name}")

    by_source: dict[Path, list[str]] = defaultdict(list)
    asm_only: list[str] = []
    for name in sorted(cluster, key=lambda item: all_funcs[item].addr):
        source = infer_source_file(name)
        if source is None:
            asm_only.append(name)
            continue
        by_source[source].append(name)

    if asm_only:
        print("Asm-only / unresolved source:")
        for name in asm_only:
            print(f"  {name}")

    overall_ok = True
    for source, funcs in sorted(by_source.items(), key=lambda item: str(item[0])):
        print(f"Source: {source}")
        with tempfile.NamedTemporaryFile(suffix=".o", delete=False) as tmp:
            obj_path = tmp.name
        try:
            compile_c_file_to_object(source, obj_path)
            results = compare_functions_in_object(
                obj_path,
                requested_funcs=funcs,
                linked=True,
                all_funcs=all_funcs,
            )
        finally:
            if os.path.exists(obj_path):
                os.unlink(obj_path)

        for result in results:
            if result.matched:
                print(f"  {result.name}: OK")
            else:
                print(f"  {result.name}: MISMATCH ({result.diff_words} differing words)")
                for line in result.lines:
                    print(f"    {line}")
                overall_ok = False

    return 0 if overall_ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
