"""Capture store resolver and reader regression tests; no emulator required."""

import hashlib
import argparse
import contextlib
import io
import json
import os
from pathlib import Path
import sys
import shutil
import unittest
import uuid
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parent))
import matrix
import gdbtrace
import knockout
import runkit
from tools.trace import store
from tools.trace.scan import events, collision_grid


class StoreTest(unittest.TestCase):
    def setUp(self):
        scratch = Path(__file__).resolve().parents[2] / "tmp"
        scratch.mkdir(exist_ok=True)
        # mkdir's default permissions work in Windows restricted sandboxes;
        # tempfile's explicit 0700 directories do not on this host.
        self.root = scratch / ("store-test-" + uuid.uuid4().hex)
        self.root.mkdir()
        self.addCleanup(shutil.rmtree, self.root)
        self.capture = self.root / "repo" / "sample"
        self.capture.mkdir(parents=True)
        root_patch = patch.object(store, "REPO_ROOT", self.root / "repo")
        root_patch.start()
        self.addCleanup(root_patch.stop)
        self.external = self.root / "store" / "sample"
        self.external.mkdir(parents=True)
        env = patch.dict(os.environ, {"BB2_CAPTURE_STORE": str(self.external.parent)})
        env.start()
        self.addCleanup(env.stop)
        self.raw = b'{"ev":"start","time":1,"spec":{"watch":[{"at":"x"}]}}\n'
        self.rec = dict(path=r"C:\old\sample\capture.jsonl",
                        sha1=hashlib.sha1(self.raw).hexdigest(), size=len(self.raw))
        self.manifest = dict(q124_complete=True, outputs=[self.rec])
        (self.capture / "capture.manifest.json").write_text(
            json.dumps(self.manifest), encoding="utf-8")
        (self.external / "capture.jsonl").write_bytes(self.raw)

    def test_local_takes_precedence_without_verification(self):
        path = self.capture / "capture.jsonl"
        path.write_bytes(b"local historical behavior")
        self.assertEqual(store.resolve(self.capture, path.name), path)

    def test_store_hit_verified(self):
        self.assertEqual(store.resolve(self.capture, "capture.jsonl"),
                         self.external / "capture.jsonl")
        with store.open_capture_file(self.capture, "capture.jsonl") as stream:
            self.assertEqual(stream.read(), self.raw.decode("utf-8"))

    def test_sha1_mismatch_refused(self):
        (self.external / "capture.jsonl").write_bytes(b"x" * len(self.raw))
        with self.assertRaisesRegex(ValueError, "SHA1/size mismatch"):
            store.resolve(self.capture, "capture.jsonl")

    def test_size_mismatch_refused(self):
        self.rec["size"] += 1
        with self.assertRaisesRegex(ValueError, "SHA1/size mismatch"):
            store.resolve(self.capture, "capture.jsonl", self.manifest)

    def test_missing_names_both_paths_and_environment(self):
        with self.assertRaises(store.StoreMissing) as caught:
            store.resolve(self.capture, "capture.mcd")
        message = str(caught.exception)
        for value in (self.capture / "capture.mcd", self.external / "capture.mcd",
                      "BB2_CAPTURE_STORE"):
            self.assertIn(str(value), message)

    def test_environment_override(self):
        self.assertEqual(store.store_root(), self.external.parent)

    def test_linked_worktree_shares_main_store(self):
        main = self.root / "main"
        common = main / ".git"
        with patch.dict(os.environ, {}, clear=True), patch.object(
                store.subprocess, "run", return_value=argparse.Namespace(stdout=str(common))) as git:
            self.assertEqual(store.store_root(),
                             main.parent / "bb2-captures")
            self.assertEqual(git.call_args[0][0][1:3], ["-C", str(store.REPO_ROOT)])

    def test_default_without_git(self):
        with patch.dict(os.environ, {}, clear=True), patch.object(
                store.subprocess, "run", side_effect=FileNotFoundError), patch.object(
                store, "_warned_git_fallback", False), contextlib.redirect_stderr(io.StringIO()) as errors:
            self.assertEqual(store.store_root(), store.REPO_ROOT.parent / "bb2-captures")
            self.assertEqual(store.store_root(), store.REPO_ROOT.parent / "bb2-captures")
        self.assertEqual(errors.getvalue().count("warning:"), 1)
        self.assertIn("git rev-parse --git-common-dir failed", errors.getvalue())

    def test_bulk_names(self):
        for name in ("capture.jsonl", "capture.a.png", "capture.script.log", "capture.mc1.mcd"):
            self.assertTrue(store.is_bulk(name), name)
        for name in ("spec.json", "capture.manifest.json", "phases.tsv", "capture.script.lua"):
            self.assertFalse(store.is_bulk(name), name)

    def test_no_record_refuses_store_file(self):
        with self.assertRaises(store.StoreUnverified):
            store.resolve(self.capture, "capture.jsonl", {})

    def test_no_manifest_refuses_store_file(self):
        (self.capture / "capture.manifest.json").unlink()
        with self.assertRaises(store.StoreUnverified):
            store.resolve(self.capture, "capture.jsonl")

    def test_incomplete_record_refused(self):
        for key in ("size", "sha1"):
            rec = dict(self.rec)
            del rec[key]
            with self.subTest(key=key), self.assertRaises(store.StoreUnverified):
                store.resolve(self.capture, "capture.jsonl", dict(outputs=[rec]))

    def test_repo_relative_keys_do_not_collide(self):
        for family in ("one", "two"):
            capture = store.REPO_ROOT / "docs/naming/runtime" / family / "baseline"
            capture.mkdir(parents=True)
            external = self.external.parent / capture.relative_to(store.REPO_ROOT)
            external.mkdir(parents=True)
            (external / "capture.jsonl").write_bytes(self.raw)
            self.assertEqual(store.resolve(capture, "capture.jsonl", self.manifest),
                             external / "capture.jsonl")

    def test_invalid_directory_and_names(self):
        with self.assertRaisesRegex(store.StoreMissing, "capture.jsonl"):
            store.resolve(self.root, "capture.jsonl")
        for name in ("", ".", "..", "a/b", "a\\b", "C:escape"):
            with self.subTest(name=name), self.assertRaises(store.StoreUnverified):
                store.resolve(self.capture, name)

    def test_existing_local_outside_repo_is_read_without_store_lookup(self):
        outside = self.root / "outside"
        outside.mkdir()
        path = outside / "capture.jsonl"
        path.write_bytes(self.raw)
        with patch.object(store, "store_root", side_effect=AssertionError("store lookup")):
            self.assertEqual(store.resolve(outside, path.name), path)
            with store.open_capture_file(outside, path.name) as stream:
                self.assertEqual(stream.read(), self.raw.decode("utf-8"))
        with self.assertRaises(store.StoreMissing):
            store.resolve(outside, path.name, allow_local=False)

    def test_windows_override_under_wsl(self):
        self.assertEqual(store.environment_path(r"C:\Capture Store\bulk", False),
                         Path("/mnt/c/Capture Store/bulk"))
        for path in (r"C:relative", r"\\server\share", r"relative\path"):
            with self.subTest(path=path), self.assertRaisesRegex(
                    store.StoreUnverified, "BB2_CAPTURE_STORE"):
                store.environment_path(path, False)

    def test_matrix_reads_store_only_capture(self):
        result = matrix.load_capture(self.capture)
        self.assertEqual(result["kind"], "events")

    def test_scanner_fallback_and_index_precedence(self):
        manifest_path = self.capture / "capture.manifest.json"
        self.assertEqual(events.capture_bytes({}, manifest_path, self.manifest), self.raw)
        blobs = {self.capture / "capture.jsonl": b"indexed bytes"}
        self.assertEqual(events.capture_bytes(blobs, manifest_path, self.manifest), b"indexed bytes")

    def test_index_missing_capture_never_reads_working_file(self):
        (self.capture / "capture.jsonl").write_bytes(self.raw)
        (self.external / "capture.jsonl").unlink()
        with self.assertRaises(store.StoreMissing):
            events.capture_bytes({}, self.capture / "capture.manifest.json", self.manifest)

    def test_matrix_passes_loaded_manifest(self):
        with patch.object(store, "_manifest", side_effect=AssertionError("re-globbed manifest")):
            self.assertEqual(matrix.load_capture(self.capture)["kind"], "events")

    def test_one_store_module_identity(self):
        for module in (gdbtrace, knockout, runkit, matrix, collision_grid):
            self.assertIs(module.store, store)
        with patch.object(store, "resolve", side_effect=store.StoreMissing("missing")):
            with self.assertRaises(store.StoreMissing):
                events.capture_bytes({}, self.capture / "capture.manifest.json", self.manifest)

    def test_unrelated_malformed_manifest_and_missing_path_ignored(self):
        (self.capture / "aaa.manifest.json").write_text("invalid", encoding="utf-8")
        manifest = dict(outputs=[{}, self.rec])
        (self.capture / "capture.manifest.json").write_text(json.dumps(manifest), encoding="utf-8")
        self.assertEqual(store.resolve(self.capture, "capture.jsonl"), self.external / "capture.jsonl")

    def test_own_manifest_preferred(self):
        other = dict(outputs=[dict(self.rec, sha1="0" * 40)])
        (self.capture / "aaa.manifest.json").write_text(json.dumps(other), encoding="utf-8")
        self.assertEqual(store.resolve(self.capture, "capture.jsonl"), self.external / "capture.jsonl")

    def test_malformed_own_manifest_refused(self):
        (self.capture / "capture.manifest.json").write_text("invalid", encoding="utf-8")
        with self.assertRaises(store.StoreUnverified):
            store.resolve(self.capture, "capture.jsonl")

    def test_check_log_reports_corrupt_log_and_png(self):
        script = dict(end_frame=10, phases=[dict(frame=0, name="boot")],
                      inputs=[], screenshots=[dict(frame=3, name="a")])
        log = b"0 phase boot\n3 shot a\n"
        rec = dict(path="capture.script.log", sha1=hashlib.sha1(log).hexdigest(), size=len(log))
        png = dict(path="capture.a.png", sha1=hashlib.sha1(b"image").hexdigest(), size=5)
        (self.capture / "capture.manifest.json").write_text(
            json.dumps(dict(outputs=[rec, png])), encoding="utf-8")
        (self.external / "capture.script.log").write_bytes(b"corrupt")
        self.assertIn("mismatch", ";".join(runkit.check_log(script, str(self.capture / "capture.jsonl"))))
        (self.capture / "capture.script.log").write_bytes(log)
        (self.external / "capture.a.png").write_bytes(b"corrupt")
        self.assertIn("mismatch", ";".join(runkit.check_log(script, str(self.capture / "capture.jsonl"))))

    def test_capture_state_reports_corruption(self):
        (self.external / "capture.jsonl").write_bytes(b"corrupt")
        called, leak, reasons = knockout.capture_state(self.capture, "f")
        self.assertFalse(called)
        self.assertIn("mismatch", ";".join(reasons))

    def knockout_fixture(self):
        from test_knockout import KnockoutTest
        fixture = KnockoutTest(methodName="runTest")
        fixture.setUp()
        self.addCleanup(fixture.doCleanups)
        self.assertEqual(fixture.batch(funcs=("one",))[0], 0)
        return fixture

    def offload_corrupt(self, path):
        external = store.store_root() / path.relative_to(store.REPO_ROOT)
        external.parent.mkdir(parents=True, exist_ok=True)
        external.write_bytes(b"corrupt")
        path.unlink()

    def test_compare_reports_corrupt_outputs_screens_and_trace(self):
        with patch.object(store, "REPO_ROOT", Path(__file__).resolve().parents[2]):
            fixture = self.knockout_fixture()
            for name in ("capture.knockout.100.png", "capture.jsonl"):
                with self.subTest(name=name):
                    self.offload_corrupt(fixture.out / "one" / name)
                    text = io.StringIO()
                    with contextlib.redirect_stdout(text):
                        rc = gdbtrace.compare(argparse.Namespace(
                            base=str(fixture.out / "baseline/capture.jsonl"),
                            other=str(fixture.out / "one/capture.jsonl"),
                            base_spec=str(fixture.out / "baseline/spec.json"),
                            other_spec=str(fixture.out / "one/spec.json"),
                            context=None, out=None))
                    self.assertEqual(rc, 1)
                    self.assertIn("mismatch", text.getvalue())
                    self.assertIn("NOT COMPARABLE", text.getvalue())
                    self.assertNotIn(str(fixture.root), text.getvalue())

    def test_knockout_summarize_reports_corruption(self):
        with patch.object(store, "REPO_ROOT", Path(__file__).resolve().parents[2]):
            fixture = self.knockout_fixture()
            self.offload_corrupt(fixture.out / "one/capture.knockout.100.png")
            self.offload_corrupt(fixture.out / "one/capture.knockout.live.png")
            self.assertEqual(knockout.summarize(
                fixture.out, ["one"], {"one": None}, [100, 120], "fake",
                {"baseline": 0, "one": 0}), 1)
            text = (fixture.out / "knockout.tsv").read_text(encoding="utf-8")
            self.assertIn("mismatch", text)
            notes = (fixture.out / "knockout.md").read_text(encoding="utf-8")
            self.assertIn("StoreMismatch", notes)
            self.assertNotIn(str(fixture.root), notes)

    def test_missing_screenshot_preserves_comparability(self):
        with patch.object(store, "REPO_ROOT", Path(__file__).resolve().parents[2]):
            fixture = self.knockout_fixture()
            shot = fixture.out / "one/capture.knockout.100.png"
            shot.unlink()
            path = fixture.out / "one/capture.manifest.json"
            manifest = json.loads(path.read_text(encoding="utf-8"))
            manifest["outputs"] = [r for r in manifest["outputs"]
                                   if Path(r["path"]).name != shot.name]
            path.write_text(json.dumps(manifest), encoding="utf-8")
            text = io.StringIO()
            with contextlib.redirect_stdout(text):
                rc = gdbtrace.compare(argparse.Namespace(
                    base=str(fixture.out / "baseline/capture.jsonl"),
                    other=str(fixture.out / "one/capture.jsonl"),
                    base_spec=str(fixture.out / "baseline/spec.json"),
                    other_spec=str(fixture.out / "one/spec.json"), context=None, out=None))
            self.assertEqual(rc, 0)
            self.assertIn("| knockout.100 (frame 100) | missing | - |", text.getvalue())
            self.assertNotIn("NOT COMPARABLE", text.getvalue())

    def test_convert_shots_reports_store_error_by_name(self):
        rec = dict(path="capture.script.log", sha1=hashlib.sha1(b"log").hexdigest(), size=3)
        (self.capture / "capture.manifest.json").write_text(
            json.dumps(dict(outputs=[rec])), encoding="utf-8")
        (self.external / "capture.script.log").write_bytes(b"corrupt")
        with contextlib.redirect_stderr(io.StringIO()) as errors:
            self.assertEqual(runkit.convert_shots(str(self.capture / "capture.jsonl")), [])
        self.assertIn("StoreMismatch", errors.getvalue())
        self.assertNotIn(str(self.root), errors.getvalue())

    def test_knockout_recorded_absence_reports_corruption(self):
        with patch.object(store, "REPO_ROOT", Path(__file__).resolve().parents[2]):
            fixture = self.knockout_fixture()
            # A recorded missing output, replaced with an unverified store file,
            # must be a problem rather than silently accepted as still absent.
            path = fixture.out / "one/capture.manifest.json"
            manifest = json.loads(path.read_text(encoding="utf-8"))
            target = fixture.out / "one/capture.knockout.100.png"
            for rec in manifest["outputs"]:
                if Path(rec["path"]).name == target.name:
                    rec.clear()
                    rec.update(path=str(target), missing=True)
            path.write_text(json.dumps(manifest), encoding="utf-8")
            self.offload_corrupt(target)
            with contextlib.redirect_stderr(io.StringIO()) as errors:
                self.assertEqual(knockout.check(fixture.out), 1)
            self.assertIn("requires SHA1", errors.getvalue())

    def test_unrecorded_local_diff_png_keeps_working(self):
        path = self.capture / "compare.a.diff.png"
        path.write_bytes(b"local")
        self.assertEqual(store.resolve(self.capture, path.name), path)

    def test_reader_rejects_store_corruption(self):
        (self.external / "capture.jsonl").write_bytes(self.raw + b"\n")
        with self.assertRaisesRegex(ValueError, "SHA1/size mismatch"):
            matrix.load_capture(self.capture)

    def test_custom_stem_png_finds_its_manifest(self):
        rec = dict(path="custom.shot.png", sha1=hashlib.sha1(b"expected").hexdigest(), size=8)
        (self.capture / "custom.manifest.json").write_text(
            json.dumps(dict(outputs=[rec])), encoding="utf-8")
        (self.external / "custom.shot.png").write_bytes(b"tampered")
        with self.assertRaisesRegex(ValueError, "SHA1/size mismatch"):
            store.resolve(self.capture, "custom.shot.png")


if __name__ == "__main__":
    unittest.main()
