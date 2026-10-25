"""Emulator-free writer, provenance and migration checks."""
import argparse
import contextlib
import io
import json
import hashlib
import os
from pathlib import Path
import shutil
import sys
import tempfile
import unittest
import uuid
import errno
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parent))
import gdbtrace
import runkit
from tools.trace import store


class WriterTest(unittest.TestCase):
    def setUp(self):
        # Ordinary mkdir preserves sandbox access on Windows (0700 does not).
        self.root = Path(tempfile.gettempdir()) / ("store-writer-" + uuid.uuid4().hex)
        self.root.mkdir()
        self.addCleanup(shutil.rmtree, self.root)
        self.repo = self.root / "repo"
        self.capture = self.repo / "docs/naming/runtime/capture"
        self.capture.mkdir(parents=True)
        self.external = self.root / "store"
        self.stored = self.external / "docs/naming/runtime/capture"
        self.head = {}
        self.head_contents = {}
        for replacement in (patch.object(store, "REPO_ROOT", self.repo),
                            patch.dict(os.environ, {
                                "BB2_CAPTURE_STORE": str(self.external),
                                "BB2_CAPTURE_STORE_WRITE": "1"}),
                            patch.object(store, "head_blob", side_effect=lambda p: self.head.get(str(Path(p).resolve()))),
                            patch.object(store, "git_output", side_effect=self.git_read),
                            patch.object(store, "ever_committed", return_value=False),
                            patch.object(store, "git_blobs", side_effect=lambda objects: {
                                obj: next(data for path, data in self.head_contents.items()
                                          if self.head[path] == obj) for obj in objects})):
            replacement.start()
            self.addCleanup(replacement.stop)

    @staticmethod
    def blob(path):
        data = Path(path).read_bytes()
        return hashlib.sha1(b"blob " + str(len(data)).encode("ascii") + b"\0" + data).hexdigest()

    def git_read(self, *args):
        if args[0] == "hash-object":
            return self.blob(args[-1])
        if args[0] == "ls-files":
            return ""
        if args[0] == "rev-parse":
            return "false"
        if args[0] == "log":
            return ""
        if args[0] == "ls-tree" and "-z" in args:
            return "".join("100644 blob " + blob + "\t" +
                           Path(path).relative_to(self.repo).as_posix() + "\0"
                           for path, blob in self.head.items())
        if args[0] == "ls-tree":
            return "\n".join(Path(p).relative_to(self.repo).as_posix()
                             for p in self.head if p.endswith(".manifest.json"))
        if args[0] == "show":
            return self.head_contents[str((self.repo / args[1][len("HEAD:"):]).resolve())].decode("utf-8")
        raise AssertionError(args)

    def commit_snapshot(self, *paths):
        # Simulate immutable HEAD blobs without any git writes or test commits.
        self.head.update({str(Path(p).resolve()): self.blob(p) for p in paths})
        self.head_contents.update({str(Path(p).resolve()): Path(p).read_bytes() for p in paths})

    def capture_file(self, name="capture.jsonl", data=b"trace"):
        path = self.capture / name
        path.write_bytes(data)
        return path

    def seal(self, paths):
        manifest = dict(outputs=[store.file_record(p) for p in paths])
        store.write_manifest(self.capture / "capture.manifest.json", manifest)
        return manifest

    def quiet(self, function, *args, **kwargs):
        with contextlib.redirect_stdout(io.StringIO()):
            return function(*args, **kwargs)

    def test_finish_manifest_offloads_bulk_only_and_readers_resolve(self):
        trace = self.capture_file()
        png = self.capture_file("capture.shot.png", b"image")
        card = self.capture_file("capture.mc1.mcd", b"card")
        spec = self.capture_file("spec.json", b"{}")
        args = argparse.Namespace(out=str(trace), lua=False, keep_emulator=False)
        with patch.object(runkit, "convert_shots", return_value=[str(png)]):
            runkit.finish_manifest(args, {}, None, dict(outcome="ok"))
        manifest = json.loads((self.capture / "capture.manifest.json").read_text(encoding="utf-8"))
        for path in (trace, png, card):
            self.assertFalse(path.exists())
            resolved = store.resolve(self.capture, path.name)
            store.verify(resolved, store._records(manifest, path.name)[0])
        self.assertTrue(spec.exists())

    def test_all_bulk_types_and_nonbulk(self):
        paths = [self.capture_file(n) for n in (
            "capture.jsonl", "capture.a.png", "capture.script.log",
            "capture.mc1.mcd", "spec.json", "capture.script.lua")]
        store.offload(self.capture, self.seal(paths))
        for path in paths:
            self.assertEqual(path.exists(), not store.is_bulk(path.name))

    def test_disabled_writing_keeps_local(self):
        path = self.capture_file()
        with patch.dict(os.environ, {"BB2_CAPTURE_STORE_WRITE": "0"}):
            store.offload(self.capture, self.seal([path]))
        self.assertTrue(path.exists())
        self.assertFalse(self.external.exists())

    def test_scratch_run_stays_local_then_copy_to_runtime_offloads(self):
        scratch = self.repo / "tmp/trace-gt/run"
        scratch.mkdir(parents=True)
        path = scratch / "capture.jsonl"
        path.write_bytes(b"trace")
        manifest = dict(outputs=[store.file_record(path)])
        store.offload(scratch, manifest)
        self.assertTrue(path.exists())
        self.assertFalse(self.external.exists())
        shutil.copyfile(path, self.capture / path.name)
        store.write_manifest(self.capture / "capture.manifest.json", manifest)
        store.offload(self.capture, manifest)
        self.assertFalse((self.capture / path.name).exists())
        self.assertEqual((self.stored / path.name).read_bytes(), b"trace")

    def test_keep_emulator_cards_stay_local(self):
        trace = self.capture_file()
        card = self.capture_file("capture.mc1.mcd", b"card")
        args = argparse.Namespace(out=str(trace), lua=False, keep_emulator=True)
        with patch.object(runkit, "convert_shots", return_value=[]):
            runkit.finish_manifest(args, {}, None, dict(outcome="ok"))
        self.assertTrue(card.exists())
        self.assertFalse(trace.exists())

    def test_offload_failure_reports_manifest_written_and_local_files(self):
        path = self.capture_file()
        store.offload(self.capture, self.seal([path]))
        path.write_bytes(b"new trace")
        args = argparse.Namespace(out=str(path), lua=False, keep_emulator=False)
        with patch.object(runkit, "convert_shots", return_value=[]), patch.object(store, "move_verified", side_effect=OSError("interrupted")), contextlib.redirect_stderr(io.StringIO()) as errors:
            manifest_path = runkit.finish_manifest(args, {}, None, dict(outcome="ok"))
        self.assertTrue(Path(manifest_path).exists())
        self.assertTrue(path.exists())
        self.assertIn("manifest written; offload failed", errors.getvalue())
        self.assertIn("stayed local: capture.jsonl", errors.getvalue())

    def test_outside_repo_unchanged(self):
        path = self.root / "outside" / "capture.jsonl"
        path.parent.mkdir()
        path.write_bytes(b"trace")
        with patch.object(store, "store_root", side_effect=AssertionError):
            store.offload(path.parent, dict(outputs=[store.file_record(path)]))
        self.assertTrue(path.exists())

    def test_writer_rerecords_different_generation_preserving_main(self):
        path = self.capture_file()
        main_manifest = self.seal([path])
        store.offload(self.capture, main_manifest)
        path.write_bytes(b"new capture")
        new_manifest = self.seal([path])
        store.offload(self.capture, new_manifest)
        self.assertFalse(path.exists())
        self.assertIn("store_name", new_manifest["outputs"][0])
        self.assertEqual(store.resolve(self.capture, path.name).read_bytes(), b"new capture")
        self.assertEqual(store.resolve(self.capture, path.name, main_manifest,
                                       allow_local=False).read_bytes(), b"trace")

    def test_generation_conflict_refuses_before_flag(self):
        path = self.capture_file()
        store.offload(self.capture, self.seal([path]))
        path.write_bytes(b"new capture")
        manifest = self.seal([path])
        generation = self.stored / (path.name + ".sha1-" + manifest["outputs"][0]["sha1"])
        generation.write_bytes(b"conflicting bytes")
        with self.assertRaises(store.StoreMismatch):
            store.offload(self.capture, manifest)
        saved = json.loads((self.capture / "capture.manifest.json").read_text(encoding="utf-8"))
        self.assertNotIn("stored", saved["outputs"][0])
        self.assertEqual(path.read_bytes(), b"new capture")
        self.assertEqual(generation.read_bytes(), b"conflicting bytes")

    def test_generation_crash_records_new_key_before_move(self):
        path = self.capture_file()
        store.offload(self.capture, self.seal([path]))
        path.write_bytes(b"new capture")
        manifest = self.seal([path])
        with patch.object(store, "move_verified", side_effect=OSError("crash")):
            self.assertFalse(store.offload_report(self.capture, manifest))
        saved = json.loads((self.capture / "capture.manifest.json").read_text(encoding="utf-8"))
        self.assertNotIn("stored", saved["outputs"][0])
        self.assertIn("store_name", saved["outputs"][0])
        with self.assertRaises(store.StoreMissing):
            store.resolve(self.capture, path.name, saved, allow_local=False)
        store.offload(self.capture, saved)
        self.assertEqual(store.resolve(self.capture, path.name).read_bytes(), b"new capture")

    def test_link_unsupported_uses_exclusive_verified_copy(self):
        for code in (errno.EPERM, errno.EXDEV, errno.EOPNOTSUPP):
            path = self.capture_file("capture.%s.png" % code)
            with patch.object(store.os, "link", side_effect=OSError(code, "unsupported")):
                store.offload(self.capture, self.seal([path]))
            self.assertFalse(path.exists())
            self.assertEqual((self.stored / path.name).read_bytes(), b"trace")

    def test_copy_fallback_race_never_overwrites(self):
        path = self.capture_file()
        manifest = self.seal([path])
        target = self.stored / path.name
        def race(source, destination):
            Path(destination).write_bytes(b"other lane")
            raise OSError(errno.EXDEV, "unsupported")
        with patch.object(store.os, "link", side_effect=race):
            with self.assertRaises(store.StoreMismatch):
                store.offload(self.capture, manifest)
        self.assertEqual(target.read_bytes(), b"other lane")
        self.assertTrue(path.exists())
        saved = json.loads((self.capture / "capture.manifest.json").read_text(encoding="utf-8"))
        self.assertNotIn("stored", saved["outputs"][0])

    def test_copy_fallback_partial_error_removes_only_own_target(self):
        path = self.capture_file()
        manifest = self.seal([path])
        def partial(source, target):
            target.write(b"partial")
            raise OSError("copy interrupted")
        with patch.object(store.os, "link", side_effect=OSError(errno.EXDEV, "unsupported")), patch.object(store.shutil, "copyfileobj", side_effect=partial):
            with self.assertRaisesRegex(OSError, "copy interrupted"):
                store.offload(self.capture, manifest)
        self.assertFalse((self.stored / path.name).exists())
        self.assertTrue(path.exists())
        saved = json.loads((self.capture / "capture.manifest.json").read_text(encoding="utf-8"))
        self.assertNotIn("stored", saved["outputs"][0])
        store.offload(self.capture, saved)
        self.assertFalse(path.exists())

    def test_copy_fallback_verification_failure_removes_own_target(self):
        path = self.capture_file()
        manifest = self.seal([path])
        def corrupt(source, target):
            target.write(b"wrong")
        with patch.object(store.os, "link", side_effect=OSError(errno.EPERM, "unsupported")), patch.object(store.shutil, "copyfileobj", side_effect=corrupt):
            with self.assertRaises(store.StoreMismatch):
                store.offload(self.capture, manifest)
        self.assertFalse((self.stored / path.name).exists())
        self.assertTrue(path.exists())

    def test_migrate_refuses_disagreement_in_second_owner(self):
        path = self.capture_file()
        self.seal([path])
        store.write_manifest(self.capture / "other.manifest.json",
                             dict(outputs=[dict(store.file_record(path), sha1="0" * 40)]))
        self.assertEqual(self.quiet(store.migrate, self.repo, apply=True), 1)
        self.assertTrue(path.exists())
        self.assertFalse(self.external.exists())

    def test_manifest_replace_failure_cleans_temp(self):
        path = self.capture / "capture.manifest.json"
        with patch.object(store.os, "replace", side_effect=PermissionError("reader")):
            with self.assertRaises(PermissionError):
                store.write_manifest(path, dict(outputs=[]))
        self.assertEqual(list(self.capture.glob(".manifest-*")), [])

    def test_compare_manifest_write_failure_is_reported(self):
        import test_knockout
        fixture = test_knockout.KnockoutTest(methodName="runTest")
        fixture.setUp()
        self.addCleanup(fixture.doCleanups)
        self.assertEqual(fixture.batch(funcs=("one",))[0], 0)
        other = fixture.out / "one"
        with patch.object(store.os, "replace", side_effect=PermissionError("reader")), contextlib.redirect_stderr(io.StringIO()) as errors:
            code = gdbtrace.compare(argparse.Namespace(
                base=str(fixture.out / "baseline/capture.jsonl"),
                other=str(other / "capture.jsonl"),
                base_spec=str(fixture.out / "baseline/spec.json"),
                other_spec=str(other / "spec.json"), context=None,
                out=str(other / "compare.md")))
        self.assertEqual(code, 1)
        self.assertIn("manifest write failed", errors.getvalue())
        self.assertTrue((other / "compare.md").exists())
        self.assertEqual(list(other.glob(".manifest-*")), [])

    def test_knockout_rejects_uninventoried_obsolete_receipt(self):
        import test_knockout
        fixture = test_knockout.KnockoutTest(methodName="runTest")
        fixture.setUp()
        self.addCleanup(fixture.doCleanups)
        self.assertEqual(fixture.batch(funcs=("one",))[0], 0)
        (fixture.out / ".capture-store.json").write_text("{}", encoding="utf-8")
        with contextlib.redirect_stderr(io.StringIO()) as errors:
            self.assertEqual(test_knockout.k.check(fixture.out), 1)
        self.assertIn("unrecorded file:", errors.getvalue())

    def test_generation_name_cannot_escape_store(self):
        path = self.capture_file()
        manifest = self.seal([path])
        manifest["outputs"][0]["store_name"] = "../../outside"
        with self.assertRaises(store.StoreUnverified):
            store.resolve(self.capture, path.name, manifest, allow_local=False)

    def legacy_compare(self):
        source = self.capture / "capture.menu.png"
        runkit.png(str(source), 2, 1, b"\xff\0\0\0\0\0")
        self.seal([source])
        baseline = self.capture.parent / "baseline"
        baseline.mkdir()
        base_png = baseline / source.name
        runkit.png(str(base_png), 2, 1, bytes(6))
        base_manifest = baseline / "capture.manifest.json"
        store.write_manifest(base_manifest, dict(outputs=[store.file_record(base_png)]))
        report = self.capture / "compare.md"
        report.write_text(
            "- baseline: `../baseline/capture.manifest.json`\n"
            "- stubbed: `capture.manifest.json`\n| menu (frame 3) | 1 of 2 | - |\n",
            encoding="utf-8")
        diff = self.capture / "compare.menu.diff.png"
        gdbtrace.diff_shot(str(base_png), str(source), str(diff))
        self.commit_snapshot(source, base_png, report, diff, base_manifest,
                             self.capture / "capture.manifest.json")
        return diff

    def test_backfill_check_apply_and_idempotence(self):
        diff = self.legacy_compare()
        self.assertEqual(self.quiet(store.backfill_records, self.repo, check=True), 1)
        owner = self.capture / "compare.manifest.json"
        self.assertFalse(owner.exists())
        self.assertEqual(self.quiet(store.backfill_records, self.repo), 0)
        before = owner.read_bytes()
        self.assertEqual(self.quiet(store.backfill_records, self.repo), 0)
        self.assertEqual(owner.read_bytes(), before)
        self.assertEqual(self.quiet(store.backfill_records, self.repo, check=True), 0)
        self.assertEqual(store._manifest(self.capture, diff.name)["outputs"],
                         [store.file_record(diff)])
        summary = json.loads(owner.read_text(encoding="utf-8"))
        self.assertEqual(summary["base"], "../baseline/capture.jsonl")
        self.assertEqual(summary["other"], "capture.jsonl")
        self.assertEqual(summary["version"], 1)

    def test_backfill_refuses_unknown_bulk(self):
        self.capture_file("unknown.png")
        self.assertEqual(self.quiet(store.backfill_records, self.repo), 1)
        self.assertFalse(list(self.repo.rglob("*.manifest.json")))

    def test_backfill_requires_report_and_source(self):
        self.legacy_compare()
        (self.capture / "compare.md").write_text("unrelated", encoding="utf-8")
        self.assertEqual(self.quiet(store.backfill_records, self.repo), 1)
        self.assertFalse((self.capture / "compare.manifest.json").exists())

    def test_backfill_refuses_untracked_or_tampered_evidence(self):
        diff = self.legacy_compare()
        for path in (diff, self.capture / "compare.md", self.capture / "capture.manifest.json",
                     self.capture.parent / "baseline/capture.manifest.json"):
            original = path.read_bytes()
            with self.subTest(path=path):
                path.write_bytes(original + b" ")
                self.assertEqual(self.quiet(store.backfill_records, self.repo), 1)
                self.assertFalse((self.capture / "compare.manifest.json").exists())
                path.write_bytes(original)
        del self.head[str(diff.resolve())]
        self.assertEqual(self.quiet(store.backfill_records, self.repo), 1)

    def test_backfill_requires_exact_regeneration_even_if_diff_is_committed(self):
        diff = self.legacy_compare()
        diff.write_bytes(b"not a true diff")
        self.commit_snapshot(diff)
        self.assertEqual(self.quiet(store.backfill_records, self.repo), 1)
        self.assertFalse((self.capture / "compare.manifest.json").exists())

    def test_backfill_refuses_corrupt_source_screenshot(self):
        self.legacy_compare()
        (self.capture.parent / "baseline/capture.menu.png").write_bytes(b"tampered")
        self.assertEqual(self.quiet(store.backfill_records, self.repo), 1)

    def test_migrate_dry_run_apply_idempotent_and_verify(self):
        path = self.capture_file()
        self.seal([path])
        self.assertEqual(self.quiet(store.migrate, self.repo), 0)
        self.assertTrue(path.exists())
        self.assertFalse(self.external.exists())
        self.assertEqual(self.quiet(store.migrate, self.repo, apply=True), 0)
        self.assertFalse(path.exists())
        self.assertEqual(store.resolve(self.capture, path.name).read_bytes(), b"trace")
        self.assertEqual(self.quiet(store.migrate, self.repo, apply=True), 0)
        self.assertEqual(self.quiet(store.verify_tree, self.repo), 0)

    def test_migrate_accepts_existing_verified_copy(self):
        path = self.capture_file()
        self.seal([path])
        target = self.stored / path.name
        target.parent.mkdir(parents=True)
        shutil.copyfile(path, target)
        self.assertEqual(self.quiet(store.migrate, self.repo, apply=True), 0)
        self.assertFalse(path.exists())

    def test_migrate_refuses_unrecorded_and_mismatching(self):
        path = self.capture_file()
        self.assertEqual(self.quiet(store.migrate, self.repo, apply=True), 1)
        self.seal([path])
        path.write_bytes(b"changed")
        self.assertEqual(self.quiet(store.migrate, self.repo, apply=True), 1)
        self.assertTrue(path.exists())

    def test_migrate_refuses_changed_tracked_bulk_or_manifest(self):
        path = self.capture_file()
        manifest_path = self.capture / "capture.manifest.json"
        self.seal([path])
        self.commit_snapshot(path, manifest_path)
        # A self-consistent new manifest cannot vouch for modified HEAD bytes.
        path.write_bytes(b"tampered")
        self.seal([path])
        self.assertEqual(self.quiet(store.migrate, self.repo, apply=True), 1)
        self.assertTrue(path.exists())
        path.write_bytes(b"trace")
        self.seal([path])
        manifest_path.write_bytes(manifest_path.read_bytes() + b" ")
        self.assertEqual(self.quiet(store.migrate, self.repo, apply=True), 1)
        self.assertTrue(path.exists())

    def test_migrate_corrupt_copy_keeps_local(self):
        path = self.capture_file()
        self.seal([path])
        def corrupt(source, target, records):
            Path(target).write_bytes(b"corrupt")
        with patch.object(store, "copy_recorded", side_effect=corrupt):
            self.assertEqual(self.quiet(store.migrate, self.repo, apply=True), 1)
        self.assertTrue(path.exists())
        self.assertEqual(list(self.external.rglob(".capture-*")), [])
        self.assertFalse((self.stored / path.name).exists())

    def test_migrate_corrupt_existing_store_keeps_local(self):
        path = self.capture_file()
        self.seal([path])
        target = self.stored / path.name
        target.parent.mkdir(parents=True)
        target.write_bytes(b"corrupt")
        self.assertEqual(self.quiet(store.migrate, self.repo, apply=True), 1)
        self.assertEqual(path.read_bytes(), b"trace")

    def test_migrate_publish_failure_keeps_local(self):
        path = self.capture_file()
        self.seal([path])
        with patch.object(store.os, "link", side_effect=OSError("interrupted")):
            self.assertEqual(self.quiet(store.migrate, self.repo, apply=True), 1)
        self.assertTrue(path.exists())

    def test_store_cannot_delete_source_when_override_is_repo(self):
        path = self.capture_file()
        self.seal([path])
        with patch.dict(os.environ, {"BB2_CAPTURE_STORE": str(self.repo)}):
            self.assertEqual(self.quiet(store.migrate, self.repo, apply=True), 1)
        self.assertEqual(path.read_bytes(), b"trace")

    def test_verify_detects_local_and_store_corruption(self):
        path = self.capture_file()
        manifest = self.seal([path])
        path.write_bytes(b"corrupt")
        self.assertEqual(self.quiet(store.verify_tree, self.repo), 1)
        path.write_bytes(b"trace")
        store.offload(self.capture, manifest)
        (self.stored / path.name).write_bytes(b"corrupt")
        self.assertEqual(self.quiet(store.verify_tree, self.repo), 1)

    def test_verify_never_published_versus_lost(self):
        path = self.capture_file()
        manifest = self.seal([path])
        path.unlink()
        self.assertEqual(self.quiet(store.verify_tree, self.repo), 0)
        with patch.object(store.LegacyRecords, "was_committed", return_value=True):
            self.assertEqual(self.quiet(store.verify_tree, self.repo), 1)
        path.write_bytes(b"trace")
        store.offload(self.capture, manifest)
        (self.stored / path.name).unlink()
        self.assertEqual(self.quiet(store.verify_tree, self.repo), 1)

    def test_verify_rejects_malformed_records(self):
        manifest_path = self.capture / "capture.manifest.json"
        for malformed in (dict(outputs={}), dict(outputs=None), dict(outputs=[None]),
                          dict(outputs=[{}]), dict(outputs=[dict(path="capture.jsonl", sha1="x")])):
            with self.subTest(manifest=malformed):
                store.write_manifest(manifest_path, malformed)
                self.assertEqual(self.quiet(store.verify_tree, self.repo), 1)

    def test_pool_events_reads_verified_store_with_manifest(self):
        import test_pool
        path = self.capture_file(data=b'{"ev":"start"}\n')
        manifest = self.seal([path])
        store.offload(self.capture, manifest)
        self.assertEqual(test_pool.events(str(path), manifest), [dict(ev="start")])
        (self.stored / path.name).write_bytes(b"bad")
        with self.assertRaises(store.StoreMismatch):
            test_pool.events(str(path), manifest)

    def test_stored_flags_preserve_matrix_data_and_refresh_manifest_hashes(self):
        import test_matrix
        fixture = test_matrix.MatrixTest(methodName="runTest")
        fixture.setUp()
        self.addCleanup(fixture.doCleanups)
        manifest_path = fixture.a / "capture.manifest.json"
        trace = fixture.a / "capture.jsonl"
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        manifest["outputs"] = [store.file_record(trace)]
        store.write_manifest(manifest_path, manifest)
        before = test_matrix.matrix.render(fixture.paths, fixture.registry)
        self.assertEqual(fixture.cli("build"), (0, ""))
        with patch.object(store, "REPO_ROOT", fixture.root):
            store.mark_stored(manifest_path, manifest, [trace.name])
            store.move_verified(trace, manifest["outputs"])
            after = test_matrix.matrix.render(fixture.paths, fixture.registry)
            for name in ("coverage.tsv", "values.tsv", "polls.tsv"):
                self.assertEqual(after[name], before[name])
            digest = hashlib.sha1(manifest_path.read_bytes()).hexdigest()
            self.assertIn("capture.manifest.json\t" + digest, after["sources.tsv"])
            self.assertIn(digest, after["scenarios.tsv"])
            self.assertNotEqual(after["sources.tsv"], before["sources.tsv"])
            self.assertEqual(fixture.cli("check")[0], 1)
            self.assertEqual(fixture.cli("build"), (0, ""))
            self.assertEqual(fixture.cli("check"), (0, ""))

    def test_capture_report_cannot_overwrite_capture_manifest(self):
        path = self.capture / "capture.manifest.json"
        path.write_bytes(b"keep")
        with self.assertRaisesRegex(store.StoreUnverified, "overwrite"):
            gdbtrace.compare(argparse.Namespace(out=str(self.capture / "capture.md")))
        self.assertEqual(path.read_bytes(), b"keep")

    def test_compare_writes_records_and_offloads_diff(self):
        import test_knockout
        fixture = test_knockout.KnockoutTest(methodName="runTest")
        fixture.setUp()
        self.addCleanup(fixture.doCleanups)
        fixture.out = fixture.root / "docs/naming/runtime/out"
        self.assertEqual(fixture.batch(funcs=("one",))[0], 0)
        other = fixture.out / "one"
        with patch.object(store, "REPO_ROOT", fixture.root), patch.object(
                gdbtrace, "diff_shot", side_effect=self.fake_diff):
            self.assertEqual(gdbtrace.compare(argparse.Namespace(
                base=str(fixture.out / "baseline/capture.jsonl"),
                other=str(other / "capture.jsonl"),
                base_spec=str(fixture.out / "baseline/spec.json"),
                other_spec=str(other / "spec.json"), context=None,
                out=str(other / "compare.md"))), 0)
            summary = json.loads((other / "compare.manifest.json").read_text(encoding="utf-8"))
            self.assertTrue(summary["outputs"])
            self.assertEqual(summary["base"], "../baseline/capture.jsonl")
            self.assertEqual(summary["other"], "capture.jsonl")
            self.assertEqual(summary["version"], 1)
            for rec in summary["outputs"]:
                self.assertFalse((other / rec["path"]).exists())
                store.verify(store.resolve(other, rec["path"]), rec)
            args = argparse.Namespace(
                base=str(fixture.out / "baseline/capture.jsonl"),
                other=str(other / "capture.jsonl"),
                base_spec=str(fixture.out / "baseline/spec.json"),
                other_spec=str(other / "spec.json"), context=None,
                out=str(other / "compare.md"))
            # A zero-change rerun retains both the earlier immutable image and
            # its verified provenance, for local and migrated readers alike.
            with patch.object(gdbtrace, "diff_shot", return_value=((0, 2, None), None)):
                self.assertEqual(gdbtrace.compare(args), 0)
            current = json.loads((other / "compare.manifest.json").read_text(encoding="utf-8"))
            self.assertEqual(current["outputs"], summary["outputs"])
            for rec in current["outputs"]:
                store.verify(store.resolve(other, rec["path"], current), rec)
            # A changed diff cannot overwrite a different lane's stored image.
            def different_diff(a, b, output):
                Path(output).write_bytes(b"different generation")
                return (1, 2, (0, 0, 0, 0)), None
            with patch.object(gdbtrace, "diff_shot", side_effect=different_diff), contextlib.redirect_stderr(io.StringIO()):
                self.assertEqual(gdbtrace.compare(args), 0)
            for rec in summary["outputs"]:
                store.verify(store.resolve(other, rec["path"], summary, allow_local=False), rec)

    def test_compare_zero_change_keeps_local_image_and_record(self):
        import test_knockout
        fixture = test_knockout.KnockoutTest(methodName="runTest")
        fixture.setUp()
        self.addCleanup(fixture.doCleanups)
        self.assertEqual(fixture.batch(funcs=("one",))[0], 0)
        other = fixture.out / "one"
        args = argparse.Namespace(
            base=str(fixture.out / "baseline/capture.jsonl"),
            other=str(other / "capture.jsonl"),
            base_spec=str(fixture.out / "baseline/spec.json"),
            other_spec=str(other / "spec.json"), context=None,
            out=str(other / "compare.md"))
        with patch.object(gdbtrace, "diff_shot", side_effect=self.fake_diff):
            self.assertEqual(gdbtrace.compare(args), 0)
        before = json.loads((other / "compare.manifest.json").read_text(encoding="utf-8"))
        with patch.object(gdbtrace, "diff_shot", return_value=((0, 2, None), None)):
            self.assertEqual(gdbtrace.compare(args), 0)
        after = json.loads((other / "compare.manifest.json").read_text(encoding="utf-8"))
        self.assertEqual(after, before)
        for rec in after["outputs"]:
            store.verify(other / rec["path"], rec)

    def test_knockout_summarizes_and_checks_store_only_runs(self):
        import test_knockout
        fixture = test_knockout.KnockoutTest(methodName="runTest")
        fixture.setUp()
        self.addCleanup(fixture.doCleanups)
        fixture.out = fixture.root / "docs/naming/runtime/out"
        original_run = fixture.fake_run

        def stored_run(directory, spec, passthrough):
            result = original_run(directory, spec, passthrough)
            manifest = json.loads((directory / "capture.manifest.json").read_text(encoding="utf-8"))
            store.offload(directory, manifest)
            return result

        with patch.object(store, "REPO_ROOT", fixture.root), patch.object(
                fixture, "fake_run", side_effect=stored_run):
            self.assertEqual(fixture.batch(funcs=("one",))[0], 0)
            self.assertEqual(test_knockout.k.check(fixture.out), 0)
            self.assertFalse(list(fixture.out.rglob("*.png")))
            self.assertFalse(list(fixture.out.rglob("capture.jsonl")))

    @staticmethod
    def fake_diff(a, b, output):
        if output:
            Path(output).write_bytes(b"diff")
        return (1, 2, (0, 0, 0, 0)), None


if __name__ == "__main__":
    unittest.main()
