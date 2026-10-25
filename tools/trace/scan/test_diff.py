#!/usr/bin/env python3
"""Synthetic differential tables, confined to repository scratch space."""

import contextlib
import csv
import hashlib
import io
import json
import os
from pathlib import Path
import shutil
import sys
import unittest
from unittest.mock import patch
import uuid

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[3]))
from tools.trace.scan import diff


class DiffTest(unittest.TestCase):
    def setUp(self):
        self.root = diff.REPO_ROOT / "tmp" / ("diff-test-" + uuid.uuid4().hex)
        self.root.mkdir(parents=True)
        self.addCleanup(self.cleanup)
        root_patch = patch.object(diff, "REPO_ROOT", self.root)
        root_patch.start()
        self.addCleanup(root_patch.stop)
        self.matrix = self.root / "docs/naming/runtime/matrix"
        self.scenarios = [
            ("a", "one", "menu:top,mode:x,stage:a"),
            ("a2", "one", "menu:top,mode:x,stage:a"),
            ("b", "two", "menu:top,mode:x,stage:b"),
            ("c", "three", "menu:top,mode:y,stage:a,stage:c")]
        self.cells = ["p:1+", "p:2", "p:3", "."]
        self.tables()
        for capture, scenario, _ in self.scenarios:
            self.phase_table(capture, [("choose", scenario), ("p", ""), ("q", ""),
                                       ("fight", ""), ("bout2", "")])
            self.provenance(capture)
        # Matrix-rule unit probes intentionally supply arbitrary windows/inputs.
        # Integration probes below disable this seam and use real hashed scripts.
        self.recorded_patch = patch.object(diff, "recorded_phases", lambda path, phases: (phases, {}))
        self.recorded_patch.start()
        self.addCleanup(self.recorded_patch.stop)

    def write_json(self, path, value):
        path.write_bytes(json.dumps(value).encode("utf-8"))

    def provenance(self, capture):
        manifest = dict(start=dict(kind="power-on: emulator held at the reset vector until armed",
                                   memcard1=dict(sha1_at_start="a" * 40),
                                   memcard2=dict(sha1_at_start="b" * 40)),
                        bios=dict(sha1="c" * 40),
                        tool={"gdbtrace.py": dict(sha1="2" * 40), "runkit.py": dict(sha1="3" * 40)},
                        build=dict(exe=dict(sha1="4" * 40), elf=dict(sha1="5" * 40)),
                        disc=dict(cue=dict(sha1="d" * 40), tracks=[dict(sha1="e" * 40)]),
                        emulator=dict(exe=dict(sha1="f" * 40), main=dict(sha1="1" * 40),
                                      version="test-version", headless=True))
        path = self.matrix.parent / capture
        self.write_json(path / "capture.manifest.json", manifest)
        self.write_json(path / "spec.json", {})
        return manifest

    def cleanup(self):
        expected = Path(__file__).resolve().parents[3] / "tmp"
        if self.root.resolve().parent != expected or not self.root.name.startswith("diff-test-"):
            raise RuntimeError("fixture cleanup escaped workspace scratch directory")
        shutil.rmtree(self.root)

    def write_table(self, path, header, rows):
        if path.name == "phases.tsv" and "last_frame" not in header:
            rows = [list(r) for r in rows]
            first = header.index("first_frame")
            # Older synthetic probes specify only starts; add complete windows
            # and a later phase so they exercise their intended input checks.
            rows = [r + [rows[i + 1][first] - 1 if i + 1 < len(rows) else r[first] + 9]
                    for i, r in enumerate(rows)]
            header = list(header) + ["last_frame"]
            tail = ["tail", rows[-1][first] + 10, "", rows[-1][first] + 19]
            rows.append(tail)
        path.parent.mkdir(parents=True, exist_ok=True)
        with path.open("w", encoding="utf-8", newline="") as stream:
            writer = csv.writer(stream, delimiter="\t", lineterminator="\n")
            writer.writerow(header)
            writer.writerows(rows)

    def tables(self):
        self.write_table(self.matrix / "scenarios.tsv", ["capture", "scenario", "covers"],
                         self.scenarios)
        self.write_table(self.matrix / "coverage.tsv",
                         ["addr", "capture_name", "other_capture_names", "no_name", "captures_hit"]
                         + [r[0] for r in self.scenarios],
                         [["0x80010000", "recorded", "", "1", "3"] + self.cells])

    def phase_table(self, capture, phases):
        phases = list(phases) + [("tail", "")]
        self.write_table(self.matrix.parent / capture / "phases.tsv",
                         ["phase", "first_frame", "last_frame", "screen", "inputs", "screenshots"],
                         [(phase, i * 10, i * 10 + 9, "-",
                             str(i * 10) + ":" + self.choice(inputs)
                             if inputs and ":" not in inputs else inputs, "-")
                          for i, (phase, inputs) in enumerate(phases)])

    def choice(self, value):
        return {"one": "LEFT", "two": "RIGHT", "three": "DOWN", "four": "UP"}.get(value, value)

    def rows(self):
        self.tables()
        content, sets = diff.render(self.matrix)
        self.sets = {r["set_id"]: r for r in
                     csv.DictReader(io.StringIO(sets), delimiter="\t")}
        rows = list(csv.DictReader(io.StringIO(content), delimiter="\t"))
        for row in rows:
            if row["kind"] == "only_x":
                self.assertEqual(row["inconclusive"], "1")
                self.assertIs(json.loads(row["reason"])["capture_level_only"], True)
        return rows

    def reasons(self, row):
        reasons = json.loads(row["reason"])
        if row["kind"] == "only_x":
            self.assertIs(reasons.pop("capture_level_only"), True)
        return reasons

    def members(self, set_id):
        return json.loads(self.sets[set_id]["members"])

    def only(self, tag):
        return next(r for r in self.rows() if r["kind"] == "only_x" and r["tag"] == tag)

    def test_only_x_hit_is_capture_level_only(self):
        row = self.only("mode:x")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(json.loads(row["reason"]), {"capture_level_only": True})
        self.assertEqual(json.loads(row["hit_captures"]), ["a", "a2", "b"])
        self.assertEqual(json.loads(row["hit_scenarios"]), ["one", "two"])
        self.assertEqual(self.members(row["zero_outside"]), ["c"])
        self.assertEqual(self.members(row["axis_values_covered"]), ["x", "y"])
        self.assertEqual((row["capture_name"], row["no_name"], row["axis"]),
                         ("recorded", "1", "mode"))

    def test_stub_scenario_row_adds_no_axis_values(self):
        def covered(extra_kind):
            self.tables()
            self.write_table(self.matrix / "scenarios.tsv", ["capture", "scenario", "covers", "kind"],
                             [list(r) + ["hitmap"] for r in self.scenarios]
                             + [["s", "four", "menu:top,mode:z,stage:a", extra_kind]])
            content, sets = diff.render(self.matrix)
            self.sets = {r["set_id"]: r for r in csv.DictReader(io.StringIO(sets), delimiter="\t")}
            row = next(r for r in csv.DictReader(io.StringIO(content), delimiter="\t")
                       if r["kind"] == "only_x" and r["tag"] == "mode:x")
            return self.members(row["axis_values_covered"])
        self.assertEqual(covered("hitmap,stub"), ["x", "y"])
        self.assertEqual(covered("hitmap"), ["x", "y", "z"])

    def test_hit_outside_no_row(self):
        self.cells[-1] = "p:1"
        self.assertFalse(any(r["tag"] == "mode:x" for r in self.rows()))

    def test_only_x_never_conclusive_for_latent_probes(self):
        original = list(self.scenarios)
        for probe in ("untagged_hit", "mode_pick", "stage_later_inputs"):
            with self.subTest(probe=probe):
                self.scenarios = list(original)
                self.cells = ["p:1", "p:2", "p:3", "."]
                tag = "mode:x"
                if probe == "untagged_hit":
                    self.scenarios[2] = ("b", "two", "menu:top,mode:x")
                elif probe == "mode_pick":
                    self.cells[:3] = ["choose:1"] * 3
                else:
                    self.scenarios = [(c, s, "menu:top,mode:x,stage:" +
                                       ("c" if c == "c" else "a"))
                                      for c, s, _ in original]
                    self.cells[:3] = ["fight:1"] * 3
                    self.phase_table("c", [("choose", "three"), ("p", ""), ("q", ""),
                                           ("fight", "33:CROSS"), ("bout2", "")])
                    tag = "stage:a"
                row = self.only(tag)
                self.assertEqual(row["inconclusive"], "1")
                self.assertIs(json.loads(row["reason"])["capture_level_only"], True)

    def test_partial_outside_is_not_zero(self):
        self.cells[-1] = "p:?"
        row = self.only("mode:x")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row), {"outside_partial": ["c"]})
        self.assertEqual(self.members(row["zero_outside"]), [])

    def test_untargeted_outside_with_an_armed_outside(self):
        self.scenarios.append(("d", "four", "mode:z"))
        self.cells.append("-")
        row = self.only("mode:x")
        self.assertEqual(self.reasons(row), {"outside_not_targeted": ["d"]})
        self.assertEqual(self.members(row["zero_outside"]), ["c"])
        self.assertEqual(self.members(row["axis_values_covered"]), ["x", "y", "z"])

    def test_requires_armed_outside(self):
        self.cells[-1] = "-"
        self.assertFalse(any(r["tag"] == "mode:x" for r in self.rows()))

    def test_unknown_stage_on_zero_capture(self):
        self.scenarios = [("a", "one", "menu:top,mode:x,stage:a"),
                          ("a2", "one", "menu:top,mode:x,stage:a"),
                          ("b", "two", "menu:top,mode:x,stage:a"),
                          ("c", "three", "menu:top,mode:x")]
        row = self.only("stage:a")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row), {"unknown_on_axis": ["stage"]})

    def test_unknown_other_axis_on_zero_capture(self):
        self.scenarios[-1] = ("c", "three", "menu:top,mode:y")
        row = self.only("mode:x")
        self.assertEqual(row["inconclusive"], "1")
        reasons = self.reasons(row)
        self.assertEqual(reasons["unknown_on_other_axis"], ["stage"])
        self.assertEqual(reasons["confounded_with_axis"]["stage"],
                         {"hit": ["stage:a", "stage:b"], "zero": []})

    def test_mode_confounded_with_stage(self):
        self.scenarios[-1] = ("c", "three", "menu:top,mode:y,stage:c")
        row = self.only("mode:x")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row), {"confounded_with_axis": {
            "stage": {"hit": ["stage:a", "stage:b"], "zero": ["stage:c"]}}})

    def test_mode_confounded_with_menu(self):
        self.scenarios[-1] = ("c", "three", "menu:option,mode:y,stage:a")
        row = self.only("mode:x")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row), {"confounded_with_axis": {
            "menu": {"hit": ["menu:top"], "zero": ["menu:option"]}},
            "confounded_with_tag": ["menu:top"]})

    def test_shared_menu_tag_confounds_despite_overlapping_values(self):
        self.scenarios[:3] = [(c, s, tags + ",menu:sel") for c, s, tags in self.scenarios[:3]]
        row = self.only("mode:x")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row), {"confounded_with_tag": ["menu:sel"]})

    def test_within_capture_axis_non_matching_hit_phase(self):
        self.scenarios[-1] = ("c", "three", "menu:other,mode:y,stage:c")
        # Covers includes top, but every hit is in a fight, not a top-menu phase.
        self.cells[:3] = ["fight:1", "bout2:2", "fight:3"]
        row = self.only("menu:top")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row), {"within_capture_axis": "menu"})

    def test_multi_value_capture_is_inconclusive(self):
        self.scenarios[:2] = [(c, s, tags + ",mode:z") for c, s, tags in self.scenarios[:2]]
        row = self.only("mode:x")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row), {"multi_value_capture": ["a", "a2"]})

    def test_single_scenario_one_inconclusive_row(self):
        self.cells[2] = "."
        rows = [r for r in self.rows() if r["kind"] == "only_x"]
        self.assertEqual(len(rows), 1)
        row = rows[0]
        self.assertEqual((row["tag"], row["axis"], row["inconclusive"]), ("-", "-", "1"))
        self.assertEqual(self.reasons(row), {"single_hit_scenario": ["one"]})
        self.assertEqual(json.loads(row["hit_scenarios"]), ["one"])
        self.assertEqual(json.loads(row["hit_captures"]), ["a", "a2"])
        self.assertEqual(self.members(row["zero_outside"]), ["b", "c"])
        self.assertEqual(row["axis_values_covered"], "")
        # Even a lone scenario with no armed outside emits its one summary.
        self.cells[2:] = ["-", "-"]
        rows = [r for r in self.rows() if r["kind"] == "only_x"]
        self.assertEqual(len(rows), 1)
        self.assertEqual(self.members(rows[0]["zero_outside"]), [])

    def test_one_tag_pair_both_directions(self):
        self.cells[2:] = [".", "fight:1"]
        pairs = [r for r in self.rows() if r["kind"] == "pair"]
        self.assertEqual(len(pairs), 1)
        row = pairs[0]
        self.assertEqual((row["tag"], row["other_tag"], row["inconclusive"]),
                         ("stage:a", "stage:b", "0"))
        self.assertEqual(json.loads(row["hit_captures"]), ["a", "a2"])
        self.assertEqual(self.members(row["zero_captures"]), ["b"])
        self.cells = [".", ".", "p:4", "fight:1"]
        pairs = [r for r in self.rows() if r["kind"] == "pair"]
        self.assertEqual(len(pairs), 1)
        self.assertEqual((pairs[0]["tag"], pairs[0]["other_tag"]), ("stage:b", "stage:a"))
        self.assertEqual(self.members(pairs[0]["zero_captures"]), ["a", "a2"])

    def test_pair_single_scenario_uses_whole_matrix_hits(self):
        self.cells[2] = "."
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row), {"single_hit_scenario": ["one"],
                                                   "hit_tag_zero_elsewhere": ["c"]})
        self.cells[-1] = "fight:1"
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(row["inconclusive"], "0")

    def test_pair_hit_in_divergence_phase(self):
        self.cells = ["choose:1", "choose:2", ".", "fight:1"]
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row), {"hit_at_or_before_divergence": {
            "a": ["choose"], "a2": ["choose"]}})

    def test_pair_hits_before_and_after_divergence(self):
        for capture, scenario, _ in self.scenarios:
            self.phase_table(capture, [("t_load", ""), ("choose", scenario), ("fight", "")])
        self.cells = ["t_load:1;fight:1", "fight:2", ".", "fight:1"]
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(self.reasons(row), {
            "hit_at_or_before_divergence": {"a": ["t_load"]}})
        self.cells[0] = "fight:1"
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(row["inconclusive"], "0")

    def test_pair_phase_lists_differ(self):
        self.cells[2:] = [".", "fight:1"]
        self.phase_table("b", [("choose", "two"), ("different", "")])
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(row["inconclusive"], "1")
        reason = self.reasons(row)["phase_lists_differ"]
        self.assertEqual(reason["b"], ["choose", "different", "tail"])
        self.assertEqual(reason["a"], ["choose", "p", "q", "fight", "bout2", "tail"])

    def test_pair_relative_inputs_after_divergence(self):
        self.cells = ["fight:1", "fight:2", ".", "fight:1"]
        for capture, scenario, _ in self.scenarios:
            self.phase_table(capture, [("choose", scenario), ("wait", "12:CROSS"),
                                       ("fight", "23:SQUARE,25:CIRCLE")])
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(row["inconclusive"], "0")
        # A difference in a phase before the last hit invalidates the pair.
        self.phase_table("b", [("choose", "two"), ("wait", "13:CROSS"),
                               ("fight", "23:SQUARE,25:CIRCLE")])
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(self.reasons(row), {
            "inputs_differ_after_divergence": ["wait"]})
        # A difference in the hit phase also invalidates it, including order.
        self.phase_table("b", [("choose", "two"), ("wait", "12:CROSS"),
                               ("fight", "25:CIRCLE,23:SQUARE")])
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(self.reasons(row), {
            "inputs_differ_after_divergence": ["fight"]})

    def test_pair_shifted_absolute_frames(self):
        self.cells = ["fight:1", "fight:2", ".", "fight:1"]
        for capture, scenario, _ in self.scenarios:
            shift = 100 if capture == "b" else 0
            self.write_table(self.matrix.parent / capture / "phases.tsv",
                             ["phase", "first_frame", "inputs"],
                             [("boot", shift, str(shift + 2) + ":START"),
                              ("choose", shift + 10, str(shift + 11) + ":" + self.choice(scenario)),
                              ("fight", shift + 20, str(shift + 23) + ":SQUARE")])
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row), {"run_start_differs": {"a": 0, "a2": 0, "b": 100}})

    def focused_pair(self, variant):
        self.cells = ["load:1", "load:2", ".", "fight:1"]
        for capture, _, _ in self.scenarios:
            zero = capture == "b"
            pick_end = 49 if variant == "divergence_short" and zero else 99
            load_start = pick_end + 1
            pick = "1:RIGHT" if zero else "1:LEFT"
            load = ""
            if variant == "pad":
                load = str(load_start + 20) + (":CROSS@2" if zero else ":CROSS")
            elif variant == "hold":
                load = str(load_start + 20) + (":CROSS/30" if zero else ":CROSS/6")
            elif variant in ("last_equal", "last_differ"):
                load = str(load_start + 99) + (":SQUARE" if zero and variant == "last_differ"
                                              else ":CROSS")
            elif variant == "unknown":
                pick = "fight inputs continue"
            phases = [("pick", 0, pick_end, pick),
                      ("load", load_start, load_start + 99, load)]
            if variant != "run_end":
                phases.append(("tail", load_start + 100, load_start + 199, ""))
            self.write_table(self.matrix.parent / capture / "phases.tsv",
                             ["phase", "first_frame", "last_frame", "inputs"], phases)
        return next(r for r in self.rows() if r["kind"] == "pair")

    def test_pair_zero_run_must_continue_after_hit_window(self):
        row = self.focused_pair("run_end")
        self.assertEqual(self.reasons(row), {"zero_side_shorter": ["load"]})
        self.assertEqual(row["inconclusive"], "1")

    def test_pair_checks_length_of_divergence_phase(self):
        row = self.focused_pair("divergence_short")
        self.assertEqual(self.reasons(row), {"zero_side_shorter": ["pick"]})

    def test_pair_pad_identity_matters(self):
        row = self.focused_pair("pad")
        self.assertEqual(self.reasons(row), {"inputs_differ_after_divergence": ["load"]})

    def test_pair_clipped_hold_duration_matters(self):
        row = self.focused_pair("hold")
        self.assertEqual(self.reasons(row), {"inputs_differ_after_divergence": ["load"]})

    def test_pair_last_frame_press_is_included(self):
        row = self.focused_pair("last_differ")
        self.assertEqual(self.reasons(row), {"inputs_differ_after_divergence": ["load"]})

    def test_pair_last_frame_press_is_valid(self):
        row = self.focused_pair("last_equal")
        self.assertEqual(row["inconclusive"], "0")
        self.assertEqual(self.reasons(row), {})

    def test_pair_unknown_divergence_never_matches(self):
        row = self.focused_pair("unknown")
        self.assertNotIn("hit_at_or_before_divergence", self.reasons(row))

    def test_pair_unparseable_scan_includes_phase_zero(self):
        row = self.focused_pair("unknown")
        self.assertEqual(self.reasons(row).get("inputs_unparseable"), ["pick"])
        self.assertEqual(row["inconclusive"], "1")

    def test_pair_timing_before_divergence(self):
        self.cells = ["load:1", "load:2", ".", "fight:1"]
        for capture, _, _ in self.scenarios:
            pick = 5000 if capture == "b" else 10
            button = "RIGHT" if capture == "b" else "LEFT"
            self.write_table(self.matrix.parent / capture / "phases.tsv",
                             ["phase", "first_frame", "last_frame", "inputs"],
                             [("boot", 0, pick - 1, "2:START"),
                              ("pick", pick, pick + 9, str(pick + 1) + ":" + button),
                              ("load", pick + 10, pick + 19, ""),
                              ("tail", pick + 20, pick + 29, "")])
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row), {"timing_differs_before_divergence": ["boot"]})

    def test_pair_default_hold_spills_into_next_phase(self):
        self.cells = ["load:1", "load:2", ".", "fight:1"]
        for capture, _, _ in self.scenarios:
            button = "DOWN" if capture == "b" else "LEFT"
            self.write_table(self.matrix.parent / capture / "phases.tsv",
                             ["phase", "first_frame", "last_frame", "inputs"],
                             [("pick", 100, 199, "198:" + button),
                              ("load", 200, 299, ""), ("tail", 300, 399, "")])
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row), {"inputs_differ_after_divergence": ["load"]})

    def test_pair_gaps_before_and_after_divergence(self):
        self.cells = ["p2:1", "p2:2", ".", "fight:1"]
        for gap, gap_phase in ((200, "p1"), (800, "p2")):
            with self.subTest(gap_before=gap_phase):
                for capture, _, _ in self.scenarios:
                    spans = []
                    for i in range(4):
                        shift = gap if capture == "b" and i >= int(gap_phase[1:]) else 0
                        first = i * 100 + shift
                        inputs = (str(first + 1) + ":" +
                                  ("RIGHT" if capture == "b" else "LEFT")) if i == 1 else ""
                        spans.append(("p" + str(i), first, first + 99, inputs))
                    self.write_table(self.matrix.parent / capture / "phases.tsv",
                                     ["phase", "first_frame", "last_frame", "inputs"], spans)
                row = next(r for r in self.rows() if r["kind"] == "pair")
                self.assertEqual(row["inconclusive"], "1")
                self.assertEqual(self.reasons(row), {"phase_gap": {"b": [gap_phase]}})

    def test_pair_equivalent_hold_and_pad_spellings(self):
        self.cells = ["load:1", "load:2", ".", "fight:1"]
        for capture, _, _ in self.scenarios:
            zero = capture == "b"
            self.write_table(self.matrix.parent / capture / "phases.tsv",
                             ["phase", "first_frame", "last_frame", "inputs"],
                             [("boot", 0, 9, "8:CROSS/6@2" if zero else "8:CROSS(2P)"),
                              ("pick", 10, 19, "11:RIGHT" if zero else "11:LEFT"),
                              ("load", 20, 29, "22:CROSS/6" if zero else "22:CROSS"),
                              ("tail", 30, 39, "")])
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(row["inconclusive"], "0")
        self.assertEqual(self.reasons(row), {})

    def test_pair_unparseable_inputs_before_divergence(self):
        self.cells = ["load:1", "load:2", ".", "fight:1"]
        probes = [("fight inputs continue", "last fight input 25850"),
                  ("fight inputs continue", "2:START"),
                  ("fight inputs continue", "fight inputs continue")]
        for hit_raw, zero_raw in probes:
            with self.subTest(hit=hit_raw, zero=zero_raw):
                for capture, scenario, _ in self.scenarios:
                    self.write_table(self.matrix.parent / capture / "phases.tsv",
                                     ["phase", "first_frame", "inputs"],
                                     [("p0", 0, zero_raw if capture == "b" else hit_raw),
                                      ("choose", 10, "11:" + self.choice(scenario)),
                                      ("load", 20, "22:CROSS")])
                row = next(r for r in self.rows() if r["kind"] == "pair")
                self.assertEqual(row["inconclusive"], "1")
                self.assertEqual(self.reasons(row).get("inputs_unparseable"), ["p0"])

    def window_pair(self, probe):
        self.cells = ["load:1", "load:2", ".", "fight:1"]
        if probe in ("held", "outside"):
            self.cells[:2] = ["fight:1", "fight:2"]
        for capture, _, _ in self.scenarios:
            zero = capture == "b"
            pick_input = "150:DOWN" if zero else "150:LEFT"
            if probe == "held" and not zero:
                pick_input = "150:LEFT/5000"
            if probe == "outside" and not zero:
                pick_input = "150:LEFT,650:SQUARE"
            phases = [("pick", 100, 199, pick_input),
                      ("load", 200, 200 if probe == "short" and zero else 299, ""),
                      ("fight", 300, 9999, ""), ("tail", 10000, 10010, "")]
            if probe == "short":
                # Same phase names; zero run ends immediately in load.
                phases = phases[:2]
                if not zero:
                    phases[-1] = ("load", 200, 9999, "")
            self.write_table(self.matrix.parent / capture / "phases.tsv",
                             ["phase", "first_frame", "last_frame", "inputs"], phases)
        return next(r for r in self.rows() if r["kind"] == "pair")

    def test_pair_truncated_zero_window(self):
        row = self.window_pair("short")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row)["zero_side_shorter"], ["load"])

    def test_pair_hold_carries_beyond_divergence(self):
        row = self.window_pair("held")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row)["inputs_differ_after_divergence"], ["fight", "load"])

    def test_pair_press_outside_listed_window(self):
        row = self.window_pair("outside")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row)["inputs_unparseable"], ["fight", "pick"])

    def test_pair_controlled_windows_remain_conclusive(self):
        row = self.window_pair("controlled")
        self.assertEqual(row["inconclusive"], "0")
        self.assertEqual(self.reasons(row), {})

    def test_pair_boundary_press_edges(self):
        self.cells = ["load:1", "load:2", ".", "fight:1"]
        for variant in ("carried", "adjacent"):
            with self.subTest(edge=variant):
                for capture, _, _ in self.scenarios:
                    pick = "2:RIGHT" if capture == "b" else (
                        "8:LEFT" if variant == "carried" else "8:LEFT/2")
                    load = "10:LEFT/4" if capture == "b" or variant == "adjacent" else ""
                    self.write_table(self.matrix.parent / capture / "phases.tsv",
                                     ["phase", "first_frame", "last_frame", "inputs"],
                                     [("choose", 0, 9, pick), ("load", 10, 19, load),
                                      ("tail", 20, 29, "")])
                row = next(r for r in self.rows() if r["kind"] == "pair")
                self.assertEqual(self.reasons(row), {"inputs_differ_after_divergence": ["load"]})

    def test_pair_overlapping_phases_are_not_contiguous(self):
        self.cells = ["load:1", "load:2", ".", "fight:1"]
        for capture, _, _ in self.scenarios:
            start = 90 if capture == "b" else 100
            self.write_table(self.matrix.parent / capture / "phases.tsv",
                             ["phase", "first_frame", "last_frame", "inputs"],
                             [("choose", 0, 99, "1:RIGHT" if capture == "b" else "1:LEFT"),
                              ("load", start, start + 99, ""), ("tail", start + 100, start + 199, "")])
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(self.reasons(row), {"phase_gap": {"b": ["load"]}})

    def test_pair_hit_in_unlisted_phase(self):
        self.cells = ["p:1;missing:1", "p:2", ".", "fight:1"]
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(self.reasons(row), {"hit_at_or_before_divergence": {"a": ["missing"]}})

    def test_pair_zero_hold_is_unparseable(self):
        self.cells = ["load:1", "load:2", ".", "fight:1"]
        for capture, _, _ in self.scenarios:
            self.write_table(self.matrix.parent / capture / "phases.tsv",
                             ["phase", "first_frame", "last_frame", "inputs"],
                             [("choose", 0, 99, "1:RIGHT" if capture == "b" else "1:LEFT"),
                              ("load", 100, 199, "120:CROSS/0"), ("tail", 200, 299, "")])
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(self.reasons(row).get("inputs_unparseable"), ["load"])
        self.assertEqual(row["inconclusive"], "1")

    def test_pair_cross_axis_difference_is_inconclusive(self):
        self.scenarios[:3] = [("a", "one", "menu:top,mode:x"),
                              ("a2", "one", "menu:top,mode:x"),
                              ("b", "two", "menu:top,stage:b")]
        self.cells = ["p:1", "p:2", ".", "fight:1"]
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(self.reasons(row), {"within_capture_axis": ["mode", "stage"]})

    def test_pair_checks_through_last_of_multiple_hit_phases(self):
        self.cells = ["p:1;fight:1", "p:2;fight:1", ".", "fight:1"]
        self.phase_table("b", [("choose", "two"), ("p", ""), ("q", "21:CROSS"),
                               ("fight", ""), ("bout2", "")])
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(self.reasons(row), {"inputs_differ_after_divergence": ["q"]})

    def test_pair_ignores_inputs_after_last_hit(self):
        self.cells = ["p:1", "p:2", ".", "fight:1"]
        self.phase_table("b", [("choose", "two"), ("p", ""), ("q", "21:CROSS"),
                               ("fight", ""), ("bout2", "")])
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(row["inconclusive"], "0")

    def test_pair_zero_tag_hit_elsewhere(self):
        self.cells = ["p:1", "p:2", ".", "fight:1"]
        self.scenarios[-1] = ("c", "three", "mode:y,stage:b")
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row), {"zero_tag_hit_elsewhere": ["c"]})

    def test_pair_start_state_fields_must_match(self):
        self.cells = ["p:1", "p:2", ".", "fight:1"]
        probes = [("start.kind", "savestate"),
                  ("start.memcard1.sha1_at_start", "9" * 40),
                  ("start.memcard2.sha1_at_start", "9" * 40),
                  ("bios.sha1", "9" * 40), ("disc.cue.sha1", "9" * 40),
                  ("emulator.exe.sha1", "9" * 40), ("emulator.main.sha1", "9" * 40),
                  ("emulator.version", "another-version"), ("emulator.headless", False)]
        for field, value in probes:
            with self.subTest(field=field):
                manifest = self.provenance("b")
                parent = manifest
                keys = field.split(".")
                for key in keys[:-1]:
                    parent = parent[key]
                parent[keys[-1]] = value
                self.write_json(self.matrix.parent / "b/capture.manifest.json", manifest)
                row = next(r for r in self.rows() if r["kind"] == "pair")
                self.assertEqual(row["inconclusive"], "1")
                self.assertIn(value, self.reasons(row)["start_state_differs"][field])
        manifest = self.provenance("b")
        manifest["disc"]["tracks"][0]["sha1"] = "9" * 40
        self.write_json(self.matrix.parent / "b/capture.manifest.json", manifest)
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertIn(["9" * 40], self.reasons(row)["start_state_differs"]["disc.tracks.sha1"])

    def test_pair_start_state_unknown_when_missing(self):
        self.cells = ["p:1", "p:2", ".", "fight:1"]
        for missing in ("capture.manifest.json", "spec.json"):
            with self.subTest(missing=missing):
                self.provenance("b")
                (self.matrix.parent / "b" / missing).unlink()
                row = next(r for r in self.rows() if r["kind"] == "pair")
                self.assertEqual(row["inconclusive"], "1")
                self.assertIn("b:" + missing, self.reasons(row)["start_state_unknown"])
        manifest = self.provenance("b")
        del manifest["bios"]["sha1"]
        self.write_json(self.matrix.parent / "b/capture.manifest.json", manifest)
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertIn("b:bios.sha1", self.reasons(row)["start_state_unknown"])

    def test_pair_older_manifest_without_headless_means_windowed(self):
        self.cells = ["p:1", "p:2", ".", "fight:1"]
        for capture in ("a", "a2", "b"):
            manifest = self.provenance(capture)
            if capture == "b":
                del manifest["emulator"]["headless"]
            else:
                manifest["emulator"]["headless"] = False
            self.write_json(self.matrix.parent / capture / "capture.manifest.json", manifest)
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(row["inconclusive"], "0")
        self.provenance("a")
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(self.reasons(row)["start_state_differs"]["emulator.headless"], [False, True])

    def test_pair_unsupported_savestate_record_is_not_accepted(self):
        self.cells = ["p:1", "p:2", ".", "fight:1"]
        for capture in ("a", "a2", "b"):
            manifest = self.provenance(capture)
            manifest["start"]["kind"] = "savestate"
            manifest["start"]["saved_image"] = {"sha1": ("8" if capture == "b" else "7") * 40}
            self.write_json(self.matrix.parent / capture / "capture.manifest.json", manifest)
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(row["inconclusive"], "1")
        self.assertIn("b:start (unsupported kind)", self.reasons(row)["start_state_unknown"])
        self.assertIn("start.record", self.reasons(row)["start_state_differs"])

    def test_pair_spec_altering_state(self):
        self.cells = ["p:1", "p:2", ".", "fight:1"]
        for spec in ({"pokes": [{}]}, {"stubs": ["target"]},
                     {"script": {"savestate_load": "unsupported"}}):
            with self.subTest(spec=spec):
                self.write_json(self.matrix.parent / "b/spec.json", spec)
                row = next(r for r in self.rows() if r["kind"] == "pair")
                self.assertEqual(row["inconclusive"], "1")
                self.assertEqual(self.reasons(row)["spec_alters_state"], ["b"])

    def verified_specs(self):
        self.recorded_patch.stop()
        self.cells = ["load:1", "load:2", ".", "fight:1"]
        for capture, scenario, _ in self.scenarios:
            self.phase_table(capture, [("choose", "1:" + self.choice(scenario)), ("load", "")])
            script = dict(end_frame=30,
                          phases=[dict(frame=0, name="choose"), dict(frame=10, name="load"),
                                  dict(frame=20, name="tail")],
                          inputs=[dict(frame=1, press=[self.choice(scenario)], hold=6)])
            self.hashed_spec(capture, dict(script=script))

    def hashed_spec(self, capture, spec):
        path = self.matrix.parent / capture
        self.write_json(path / "spec.json", spec)
        manifest = json.loads((path / "capture.manifest.json").read_text())
        manifest["spec"] = dict(sha1=hashlib.sha1((path / "spec.json").read_bytes()).hexdigest())
        self.write_json(path / "capture.manifest.json", manifest)

    def test_verified_script_and_tsv_agree(self):
        self.verified_specs()
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(row["inconclusive"], "0")

    def test_pair_uses_same_inputs_tool_and_build_hashes(self):
        self.cells = ["p:1", "p:2", ".", "fight:1"]
        for group, name in (("tool", "gdbtrace.py"), ("tool", "runkit.py"),
                            ("build", "exe"), ("build", "elf")):
            with self.subTest(group=group, name=name):
                manifest = self.provenance("b")
                manifest[group][name]["sha1"] = "9" * 40
                self.write_json(self.matrix.parent / "b/capture.manifest.json", manifest)
                row = next(r for r in self.rows() if r["kind"] == "pair")
                self.assertEqual(row["inconclusive"], "1")
                self.assertIn("9" * 40, self.reasons(row)["start_state_differs"][group + "." + name + ".sha1"])

    def test_spec_without_hold_uses_runkit_default_four(self):
        self.verified_specs()
        for capture, scenario, _ in self.scenarios:
            spec = json.loads((self.matrix.parent / capture / "spec.json").read_text())
            del spec["script"]["inputs"][0]["hold"]
            self.hashed_spec(capture, spec)
            self.phase_table(capture, [("choose", "1:" + self.choice(scenario) + "/4"), ("load", "")])
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(row["inconclusive"], "0")
        spans, reasons = diff.recorded_phases(self.matrix.parent / "a", None)
        self.assertEqual(spans[0]["_relative_inputs"][0][3], 4)

    def test_spec_press_omitted_from_tsv(self):
        self.verified_specs()
        spec = json.loads((self.matrix.parent / "b/spec.json").read_text())
        spec["script"]["inputs"].append(dict(frame=12, press=["START"], hold=6))
        self.hashed_spec("b", spec)
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row)["phases_tsv_differs_from_spec"], {"b": ["load"]})
        self.assertEqual(self.reasons(row)["inputs_differ_after_divergence"], ["load"])

    def test_spec_press_misphased_in_tsv(self):
        self.verified_specs()
        spec = json.loads((self.matrix.parent / "b/spec.json").read_text())
        spec["script"]["inputs"].append(dict(frame=12, press=["CROSS"], hold=6))
        self.hashed_spec("b", spec)
        self.phase_table("b", [("choose", "1:RIGHT,12:CROSS"), ("load", "")])
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row)["phases_tsv_differs_from_spec"], {"b": ["choose", "load"]})

    def test_spec_sha1_mismatch_and_missing_spec(self):
        self.verified_specs()
        path = self.matrix.parent / "b/spec.json"
        path.write_bytes(path.read_bytes() + b" ")
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row)["spec_unverified"], ["b"])
        path.unlink()
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(self.reasons(row)["spec_unverified"], ["b"])

    def test_pair_hit_in_untagged_capture(self):
        self.cells = ["p:1", "p:2", ".", "fight:1"]
        self.scenarios[-1] = ("c", "three", "mode:y")
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row), {"hit_in_untagged_capture": ["c"]})

    def test_pair_hit_side_not_reproduced(self):
        self.cells = ["p:1", ".", ".", "fight:1"]
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row), {"hit_side_not_reproduced": ["a2"]})

    def test_pair_hit_tag_zero_elsewhere(self):
        self.cells = ["p:1", "p:2", ".", "."]
        self.scenarios.append(("d", "four", "mode:y,stage:d"))
        self.cells.append("fight:1")
        self.phase_table("d", [("choose", "four"), ("fight", "")])
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row), {"hit_tag_zero_elsewhere": ["c"]})

    def test_missing_phases_is_inconclusive(self):
        self.cells[2:] = [".", "fight:1"]
        (self.matrix.parent / "b/phases.tsv").unlink()
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row), {"phases_missing": ["b"]})
        self.cells = ["p:1", "p:2", "p:3", "."]
        row = self.only("mode:x")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row), {"phases_missing": ["b"]})

    def test_identical_inputs_have_no_divergence(self):
        self.cells[2:] = [".", "fight:1"]
        self.phase_table("b", [("choose", "one"), ("p", ""), ("q", ""),
                                ("fight", ""), ("bout2", "")])
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row), {"hit_at_or_before_divergence": {
            "a": ["p"], "a2": ["p"]}})

    def test_stage_only_x_divergence(self):
        self.scenarios = [(c, s, "menu:top,mode:x,stage:" + ("c" if c == "c" else "a"))
                          for c, s, _ in self.scenarios]
        self.cells = ["choose:1", "p:2", "p:3", "."]
        row = self.only("stage:a")
        self.assertEqual(self.reasons(row), {
            "hit_at_or_before_divergence": {"a": ["choose"]}})
        self.cells[0] = "p:1"
        row = self.only("stage:a")
        self.assertEqual(row["inconclusive"], "1")
        # No matching zero phase list cannot establish stage evidence.
        self.cells[0] = "choose:1"
        self.phase_table("c", [("different", "")])
        row = self.only("stage:a")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row), {
            "no_comparable_zero_capture": ["a", "a2", "b"]})

    def test_stage_only_x_uses_relative_inputs(self):
        self.scenarios = [(c, s, "menu:top,mode:x,stage:" + ("c" if c == "c" else "a"))
                          for c, s, _ in self.scenarios]
        self.cells = ["choose:1", "choose:1", "choose:1", "."]
        for capture, _, _ in self.scenarios:
            shift = 100 if capture == "c" else 0
            self.write_table(self.matrix.parent / capture / "phases.tsv",
                             ["phase", "first_frame", "inputs"],
                             [("boot", shift, str(shift + 2) + ":START"),
                              ("choose", shift + 10, str(shift + 11) + ":CROSS"),
                              ("fight", shift + 20, str(shift + 23) + ":" +
                               ("SQUARE" if capture == "c" else "CIRCLE"))])
        row = self.only("stage:a")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row), {"hit_at_or_before_divergence": {
            "a": ["choose"], "a2": ["choose"], "b": ["choose"]}})

    def test_coverage_columns_follow_captures_hit(self):
        original = diff.render(self.matrix)
        path = self.matrix / "coverage.tsv"
        header, rows = diff.read_table(path, ["captures_hit"])
        header.insert(header.index("captures_hit"), "extra_metadata")
        self.write_table(path, header, [[row.get(c, "metadata") for c in header] for row in rows])
        self.assertEqual(diff.render(self.matrix), original)
        header.remove("captures_hit")
        self.write_table(path, header, [[row.get(c, "metadata") for c in header] for row in rows])
        with self.assertRaisesRegex(ValueError, "missing columns"):
            diff.render(self.matrix)

    def test_shared_hit_phase_grammar(self):
        self.cells = ["p:1+;q:?;fight:2", "p:3", ".", "fight:1"]
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(json.loads(row["hit_captures"]), ["a", "a2"])
        self.assertEqual(self.reasons(row), {"hit_side_partial": ["a"]})

    def test_pair_within_capture_axis_is_inconclusive(self):
        self.scenarios[:3] = [("a", "one", "menu:top,mode:x"),
                              ("a2", "one", "menu:top,mode:x"),
                              ("b", "two", "menu:option,mode:x")]
        self.cells = ["fight:1", "fight:2", ".", "fight:3"]
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row), {"within_capture_axis": "menu"})

    def test_two_tag_difference_no_pair(self):
        self.cells[2] = "."
        self.scenarios[2] = ("b", "two", "menu:top,mode:z,stage:b")
        self.assertFalse(any(r["kind"] == "pair" for r in self.rows()))

    def test_pair_requires_all_zero_captures(self):
        self.cells = [".", "p:?", "p:4", "."]
        self.assertFalse(any(r["kind"] == "pair" for r in self.rows()))
        self.cells[1] = "-"
        self.assertFalse(any(r["kind"] == "pair" for r in self.rows()))
        self.cells = ["p:1;q:?", "-", ".", "."]
        row = next(r for r in self.rows() if r["kind"] == "pair")
        self.assertEqual(row["inconclusive"], "1")
        self.assertEqual(self.reasons(row),
                         {"hit_side_partial": ["a"], "hit_side_not_targeted": ["a2"],
                          "single_hit_scenario": ["one"], "hit_tag_zero_elsewhere": ["c"]})

    def test_build_check_corruption_and_repo_relative_paths(self):
        old_cwd = Path.cwd()
        try:
            os.chdir(self.root)
            for command in ["build", "check"]:
                self.assertEqual(diff.main([command]), 0)
            out = self.root / "docs/naming/runtime/evidence/only_x.tsv"
            sets_out = out.with_name("only_x_sets.tsv")
            originals = {p: p.read_bytes() for p in (out, sets_out)}
            self.assertEqual(diff.main(["build"]), 0)
            for path, original in originals.items():
                self.assertNotIn(b"\r", original)
                self.assertEqual(path.read_bytes(), original)
                # One changed byte in either table must fail check.
                path.write_bytes(original[:-1] + b"!")
                with contextlib.redirect_stderr(io.StringIO()) as stderr:
                    self.assertEqual(diff.main(["check"]), 1)
                self.assertIn("diff table differs", stderr.getvalue())
                path.write_bytes(original)
            self.assertEqual(diff.main(["check"]), 0)
        finally:
            os.chdir(old_cwd)

    def test_set_table_round_trip(self):
        self.scenarios[-1] = ("c", "three", "menu:other,mode:y,stage:c")
        rows = self.rows()
        used = set()
        for row in rows:
            for column, kind in diff.SET_COLUMNS.items():
                set_id = row[column]
                if not set_id:
                    continue
                used.add(set_id)
                entry = self.sets[set_id]
                self.assertEqual(entry["kind"], kind)
                members = self.members(set_id)
                self.assertEqual(members, sorted(set(members)))
                self.assertEqual(diff.compact(members), entry["members"])
                self.assertEqual(hashlib.sha1(entry["members"].encode("utf-8")).hexdigest()[:12],
                                 set_id)
        self.assertEqual(used, set(self.sets))
        self.assertEqual(len({r["members"] for r in self.sets.values()}), len(self.sets))
        # Different only-X tags share the same outside captures list.
        tags = {r["tag"]: r for r in rows if r["kind"] == "only_x"}
        self.assertEqual(tags["mode:x"]["zero_outside"], tags["menu:top"]["zero_outside"])

    def test_covered_tags_present_on_every_row(self):
        self.cells[2] = "."
        rows = self.rows()
        self.assertEqual({r["kind"] for r in rows}, {"only_x", "pair"})
        expected = sorted({tag for _, _, tags in self.scenarios for tag in tags.split(",")})
        self.assertEqual(len({r["covered_tags"] for r in rows}), 1)
        for row in rows:
            self.assertEqual(self.sets[row["covered_tags"]]["kind"], "tags")
            self.assertEqual(self.members(row["covered_tags"]), expected)
        # A scenario-table entry without a matrix capture is not in covered_tags.
        self.write_table(self.matrix / "scenarios.tsv", ["capture", "scenario", "covers"],
                         self.scenarios + [("extra", "extra", "menu:uncovered")])
        _, sets = diff.render(self.matrix)
        tag_set = next(r for r in csv.DictReader(io.StringIO(sets), delimiter="\t")
                       if r["kind"] == "tags")
        self.assertEqual(json.loads(tag_set["members"]), expected)

    def test_missing_and_unused_sets_fail_check(self):
        self.assertEqual(diff.main(["build"]), 0)
        path = self.root / "docs/naming/runtime/evidence/only_x_sets.tsv"
        original = path.read_bytes()
        # Missing referenced set; a missing table must fail too.
        path.write_bytes(original.split(b"\n", 1)[0] + b"\n")
        with contextlib.redirect_stderr(io.StringIO()):
            self.assertEqual(diff.main(["check"]), 1)
        path.unlink()
        with contextlib.redirect_stderr(io.StringIO()):
            self.assertEqual(diff.main(["check"]), 1)
        members = diff.compact(["unused"])
        set_id = hashlib.sha1(members.encode("utf-8")).hexdigest()[:12]
        entry = diff.table(diff.SET_HEADER, [dict(set_id=set_id, kind="captures", members=members)])
        path.write_bytes(original + entry.split("\n", 1)[1].encode("utf-8"))
        with contextlib.redirect_stderr(io.StringIO()):
            self.assertEqual(diff.main(["check"]), 1)
        path.write_bytes(original)
        self.assertEqual(diff.main(["check"]), 0)

    def test_determinism_and_order(self):
        rows = self.rows()
        self.assertEqual(rows, sorted(rows, key=lambda r: tuple(r[c] for c in diff.HEADER)))
        original = diff.render(self.matrix)
        self.scenarios.reverse()
        self.cells.reverse()
        self.tables()
        self.assertEqual(diff.render(self.matrix), original)

    def test_invalid_inputs(self):
        self.cells[0] = "p:0"
        self.tables()
        with self.assertRaises(ValueError):
            diff.render(self.matrix)
        self.cells[0] = "p:1"
        self.scenarios[1] = ("a2", "one", "mode:z")
        self.tables()
        with self.assertRaises(ValueError):
            diff.render(self.matrix)


if __name__ == "__main__":
    unittest.main(verbosity=2)
