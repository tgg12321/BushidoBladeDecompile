"""Real Git publication regressions; all Git writes stay in a private temp repo."""
import contextlib
import io
import json
import os
from pathlib import Path
import shutil
import stat
import subprocess
import sys
import tempfile
import unittest
import uuid
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from tools.trace import store


class PublicationGitTest(unittest.TestCase):
    def setUp(self):
        self.root = Path(tempfile.gettempdir()).resolve() / ("store-git-" + uuid.uuid4().hex)
        self.root.mkdir()
        self.addCleanup(self.cleanup)
        self.repo = self.root / "repo"
        self.capture = self.repo / "docs/naming/runtime/sample"
        self.capture.mkdir(parents=True)
        self.manifest_path = self.capture / "capture.manifest.json"
        self.trace = self.capture / "capture.jsonl"
        self.external = self.root / "store"
        self.stored_trace = self.external / "docs/naming/runtime/sample/capture.jsonl"
        self.git_env = {k: v for k, v in os.environ.items() if not k.startswith("GIT_")}
        self.git_env.update(GIT_CONFIG_NOSYSTEM="1", GIT_CONFIG_GLOBAL=os.devnull)
        self.git("init", "--initial-branch=main")
        self.git("commit", "--allow-empty", "-m", "Private test fixture")
        for replacement in (patch.object(store, "REPO_ROOT", self.repo),
                            patch.object(store, "LEGACY_BASE", self.git("rev-parse", "HEAD").strip()),
                            patch.object(store, "MIGRATION_COMMIT", store.MIGRATION_COMMIT),
                            patch.dict(os.environ, {"BB2_CAPTURE_STORE": str(self.external),
                                                    "BB2_CAPTURE_STORE_WRITE": "1"})):
            replacement.start()
            self.addCleanup(replacement.stop)

    def cleanup(self):
        if (self.root.resolve().parent != Path(tempfile.gettempdir()).resolve()
                or not self.root.name.startswith("store-git-")):
            raise RuntimeError("Git test cleanup escaped its private temp directory")
        def remove_readonly(function, path, error):
            if not Path(path).resolve().is_relative_to(self.root.resolve()):
                raise RuntimeError("Git fixture cleanup escaped its private directory")
            os.chmod(path, stat.S_IREAD | stat.S_IWRITE)
            function(path)
        shutil.rmtree(self.root, **{"onexc" if sys.version_info >= (3, 12) else "onerror": remove_readonly})

    def git(self, *args):
        # The task explicitly authorizes real commits in this throwaway repo.
        if self.repo.resolve() != self.root.resolve() / "repo":
            raise RuntimeError("Git fixture escaped its private repo")
        return subprocess.run(
            ["git", "-C", str(self.repo), "-c", "core.autocrlf=false",
             "-c", "core.hooksPath=" + str(self.root / "disabled-hooks"),
             "-c", "commit.gpgSign=false", "-c", "user.name=Trace Tests",
             "-c", "user.email=trace-tests@example.invalid", *args],
            check=True, capture_output=True, text=True, encoding="utf-8", env=self.git_env).stdout

    def seal(self):
        self.trace.write_bytes(b'{"ev":"start"}\n')
        manifest = dict(outputs=[store.file_record(self.trace)])
        store.write_manifest(self.manifest_path, manifest)
        return manifest

    def commit_manifest(self, legacy=False):
        self.git("add", "--", self.manifest_path.relative_to(self.repo).as_posix())
        self.git("commit", "-m", "Publish only capture manifest")
        if legacy:
            store.LEGACY_BASE = self.git("rev-parse", "HEAD").strip()
            self.git("commit", "--allow-empty", "-m", "Module 2 boundary fixture")

    def command(self, *args):
        with contextlib.redirect_stdout(io.StringIO()) as output:
            code = store.main([*args, "--root", str(self.capture)])
        return code, output.getvalue()

    def test_manifest_only_commit_remembers_publication_without_receipts(self):
        manifest = self.seal()
        store.offload(self.capture, manifest)
        self.assertTrue(manifest["outputs"][0]["stored"])
        self.assertFalse(self.trace.exists())
        self.commit_manifest()
        tracked = self.git("ls-files").splitlines()
        self.assertEqual(tracked, [self.manifest_path.relative_to(self.repo).as_posix()])
        self.assertEqual(list(self.root.rglob(".capture-store.json")), [])
        committed = json.loads(self.git("show", "HEAD:" + tracked[0]))
        self.assertIs(committed["outputs"][0]["stored"], True)
        self.stored_trace.write_bytes(b"corrupt store copy")
        code, output = self.command("verify")
        self.assertEqual(code, 1)
        self.assertIn("LOST ", output)
        self.stored_trace.unlink()
        code, output = self.command("verify")
        self.assertEqual(code, 1)
        self.assertIn("LOST ", output)
        self.assertIn("never_published=0", output)

    def test_legacy_unpublished_record_succeeds_without_flag(self):
        self.seal()
        self.trace.unlink()
        self.commit_manifest(legacy=True)
        code, output = self.command("verify")
        self.assertEqual(code, 0)
        self.assertIn("NEVER PUBLISHED ", output)
        self.assertIn("never_published=1", output)

    def test_writer_crash_after_flag_before_move_verifies_and_resumes(self):
        manifest = self.seal()
        self.commit_manifest()
        with patch.object(store, "move_verified", side_effect=OSError("simulated crash")):
            with self.assertRaisesRegex(OSError, "simulated crash"):
                store.offload(self.capture, manifest)
        self.assertTrue(self.trace.exists())
        self.assertNotIn("stored", json.loads(self.manifest_path.read_text(encoding="utf-8"))["outputs"][0])
        self.assertEqual(self.command("verify")[0], 0)
        self.assertEqual(self.command("migrate", "--apply")[0], 0)
        self.assertFalse(self.trace.exists())
        self.assertEqual(self.command("migrate", "--apply")[0], 0)
        self.commit_manifest()
        self.assertEqual(self.command("verify")[0], 0)

    def test_migration_sets_flag_before_delete_and_resumes(self):
        self.seal()
        self.git("add", "--", self.capture.relative_to(self.repo).as_posix())
        self.git("commit", "-m", "Local capture before migration")
        with patch.object(store, "move_verified", side_effect=OSError("simulated crash")):
            self.assertEqual(self.command("migrate", "--apply")[0], 1)
        self.assertTrue(self.trace.exists())
        self.assertNotIn("stored", json.loads(self.manifest_path.read_text(encoding="utf-8"))["outputs"][0])
        self.assertEqual(self.command("verify")[0], 0)
        self.assertEqual(self.command("migrate", "--apply")[0], 0)
        self.assertFalse(self.trace.exists())
        self.assertEqual(self.command("migrate", "--apply")[0], 0)
        self.assertEqual(list(self.root.rglob(".capture-store.json")), [])

    def test_removed_working_flag_cannot_hide_committed_loss(self):
        manifest = self.seal()
        store.offload(self.capture, manifest)
        self.commit_manifest()
        del manifest["outputs"][0]["stored"]
        store.write_manifest(self.manifest_path, manifest)
        self.stored_trace.unlink()
        code, output = self.command("verify")
        self.assertEqual(code, 1)
        self.assertIn("MANIFEST DIFFERS", output)
        self.assertIn("LOST ", output)
        self.assertIn("never_published=0", output)
        self.manifest_path.unlink()
        self.assertIn("LOST ", self.command("verify")[1])

    def test_commit_gate_uses_index_and_rejects_new_unflagged_local_only(self):
        manifest = self.seal()
        self.git("add", "--", self.manifest_path.relative_to(self.repo).as_posix())
        self.assertEqual(self.command("check-commit")[0], 1)
        # An unstaged edit cannot hide the staged output.
        store.write_manifest(self.manifest_path, dict(outputs=[]))
        self.assertEqual(self.command("check-commit")[0], 1)
        store.write_manifest(self.manifest_path, manifest)
        store.offload(self.capture, manifest)
        self.assertEqual(self.command("check-commit")[0], 0)
        self.stored_trace.write_bytes(b"bad")
        self.assertEqual(self.command("check-commit")[0], 1)

    def test_commit_gate_legacy_and_tracked_bulk(self):
        self.seal()
        self.commit_manifest(legacy=True)
        self.trace.unlink()
        self.assertEqual(self.command("check-commit")[0], 0)
        self.assertIn("legacy_never_published=1", self.command("check-commit")[1])
        self.trace.write_bytes(b'{"ev":"start"}\n')
        self.git("add", "--", self.trace.relative_to(self.repo).as_posix())
        self.assertIn("legacy_never_published=0", self.command("check-commit")[1])

    def test_bulk_blob_refused_after_migration_commit(self):
        manifest = self.seal()
        old_shot = self.capture / "old.png"
        old_shot.write_bytes(b"old")
        self.git("add", "--", self.capture.relative_to(self.repo).as_posix())
        self.git("commit", "-m", "Pre-migration capture with bulk in git")
        before = self.git("rev-parse", "HEAD").strip()
        self.git("rm", "--cached", "-q", "--", self.trace.relative_to(self.repo).as_posix())
        store.offload(self.capture, manifest)
        self.git("add", "--", self.manifest_path.relative_to(self.repo).as_posix())
        self.git("commit", "-m", "Migration fixture")
        store.MIGRATION_COMMIT = self.git("rev-parse", "HEAD").strip()
        # The pre-migration commit is still judged by the old rules.
        self.assertEqual(self.command("check-commit", "--rev", before)[0], 0)
        # A carried-over bulk entry and its deletion stay allowed.
        self.git("commit", "--allow-empty", "-m", "Post-migration commit")
        self.assertEqual(self.command("check-commit", "--rev", "HEAD")[0], 0)
        self.git("rm", "--cached", "-q", "--", old_shot.relative_to(self.repo).as_posix())
        self.assertEqual(self.command("check-commit")[0], 0)
        self.git("commit", "-m", "Post-migration deletion")
        self.assertEqual(self.command("check-commit", "--rev", "HEAD")[0], 0)
        # A recorded, matching bulk blob is refused once the parent is migrated.
        self.trace.write_bytes(self.stored_trace.read_bytes())
        self.git("add", "-f", "--", self.trace.relative_to(self.repo).as_posix())
        code, output = self.command("check-commit")
        self.assertEqual(code, 1, output)
        self.assertIn("bulk blob added to git after the store migration", output)
        self.git("commit", "-m", "Lane replay re-adds bulk")
        self.assertEqual(self.command("check-commit", "--rev", "HEAD")[0], 1)
        self.git("reset", "-q", "--soft", "HEAD^")
        self.git("rm", "--cached", "-q", "--", self.trace.relative_to(self.repo).as_posix())
        self.trace.unlink()
        self.assertEqual(self.command("check-commit")[0], 0)
        # An unrecorded bulk file is refused too.
        stray = self.capture / "shot.png"
        stray.write_bytes(b"png")
        self.git("add", "-f", "--", stray.relative_to(self.repo).as_posix())
        self.assertEqual(self.command("check-commit")[0], 1)
        self.git("rm", "--cached", "-q", "--", stray.relative_to(self.repo).as_posix())
        # So is a case-variant bulk name.
        upper = self.capture / "Shot.PNG"
        upper.write_bytes(b"png")
        self.git("add", "-f", "--", upper.relative_to(self.repo).as_posix())
        self.assertEqual(self.command("check-commit")[0], 1)

    def test_new_manifest_only_commit_is_loss_even_without_flag(self):
        self.seal()
        self.trace.unlink()
        self.commit_manifest()
        self.assertEqual(self.command("verify")[0], 1)
        self.assertIn("LOST ", self.command("verify")[1])

    def test_migrate_refuses_nonobject_head_cleanly(self):
        self.seal()
        self.manifest_path.write_text("[]\n", encoding="utf-8")
        self.commit_manifest()
        self.seal()
        code, output = self.command("migrate")
        self.assertEqual(code, 1)
        self.assertIn("HEAD manifest must be an object", output)

    def assert_both_refuse(self):
        self.assertEqual(self.command("check-commit")[0], 1)
        code, output = self.command("verify")
        self.assertEqual(code, 1)
        self.assertIn("LOST ", output)

    def test_changed_legacy_record_cannot_borrow_manifest_history(self):
        self.seal()
        self.commit_manifest(legacy=True)
        self.trace.write_bytes(b"new generation")
        store.write_manifest(self.manifest_path, dict(outputs=[store.file_record(self.trace)]))
        self.trace.unlink()
        self.commit_manifest()
        self.assert_both_refuse()

    def test_legacy_record_cache_is_per_manifest_not_per_output(self):
        manifest = self.seal()
        extra = self.capture / "capture.script.log"
        extra.write_bytes(b"old log")
        manifest["outputs"].append(store.file_record(extra))
        store.write_manifest(self.manifest_path, manifest)
        self.trace.unlink()
        extra.unlink()
        self.commit_manifest(legacy=True)
        # Metadata changes after the boundary do not invalidate unchanged records.
        manifest["note"] = "later metadata"
        store.write_manifest(self.manifest_path, manifest)
        self.commit_manifest()
        for command in ("check-commit", "verify"):
            with patch.object(store, "git_output", wraps=store.git_output) as git:
                code, output = self.command(command)
            self.assertEqual(code, 0)
            self.assertIn("published=2", output)
            logs = [call.args for call in git.call_args_list if call.args[0] == "log"]
            self.assertEqual(len(logs), 2)  # Directory history and manifest ancestry.

    def test_changed_legacy_path_is_not_legacy(self):
        manifest = self.seal()
        self.trace.unlink()
        self.commit_manifest(legacy=True)
        manifest["outputs"][0]["path"] = "other/capture.jsonl"
        store.write_manifest(self.manifest_path, manifest)
        self.commit_manifest()
        self.assert_both_refuse()

    def test_new_output_in_legacy_manifest_is_not_legacy(self):
        manifest = self.seal()
        self.trace.unlink()
        self.commit_manifest(legacy=True)
        new = self.capture / "capture.new.png"
        new.write_bytes(b"new output")
        manifest["outputs"].append(store.file_record(new))
        new.unlink()
        store.write_manifest(self.manifest_path, manifest)
        self.commit_manifest()
        self.assert_both_refuse()
        self.assertIn("never_published=1", self.command("verify")[1])

    def test_backdated_new_manifest_cannot_become_legacy(self):
        self.seal()
        self.trace.unlink()
        with patch.dict(self.git_env, {"GIT_COMMITTER_DATE": "2000-01-01T00:00:00Z"}):
            self.commit_manifest()
        self.assert_both_refuse()

    def test_recorded_absence_is_skipped_in_both_commands(self):
        store.write_manifest(self.manifest_path, dict(outputs=[
            dict(path="capture.jsonl", missing=True),
            dict(path="capture.absent.png", missing=True)]))
        self.commit_manifest()
        for command in ("check-commit", "verify"):
            code, output = self.command(command)
            self.assertEqual(code, 0)
            self.assertNotIn("LOST ", output)
            self.assertNotIn("NEVER PUBLISHED ", output)
        # missing:true with a digest is not a recorded absence exemption.
        manifest = self.seal()
        manifest["outputs"][0]["missing"] = True
        store.write_manifest(self.manifest_path, manifest)
        self.trace.unlink()
        self.commit_manifest()
        self.assert_both_refuse()

    def test_index_blob_digest_and_size_ignore_working_bytes(self):
        manifest = self.seal()
        self.git("add", "--", self.capture.relative_to(self.repo).as_posix())
        self.trace.write_bytes(b"working edit must not matter")
        self.assertEqual(self.command("check-commit")[0], 0)
        self.git("add", "--", self.trace.relative_to(self.repo).as_posix())
        self.trace.write_bytes(b'{"ev":"start"}\n')
        code, output = self.command("check-commit")
        self.assertEqual(code, 1)
        self.assertIn("SHA1/size mismatch", output)
        # A correct digest cannot conceal a wrong size.
        manifest["outputs"][0] = store.file_record(self.trace)
        manifest["outputs"][0]["size"] += 1
        store.write_manifest(self.manifest_path, manifest)
        self.git("add", "--", self.capture.relative_to(self.repo).as_posix())
        self.assertEqual(self.command("check-commit")[0], 1)

    def test_missing_record_refuses_local_index_and_store(self):
        manifest = dict(outputs=[dict(path="capture.jsonl", missing=True)])
        store.write_manifest(self.manifest_path, manifest)
        self.commit_manifest()
        self.trace.write_bytes(b"unexpected local file")
        self.assertIn("missing record has existing", self.command("check-commit")[1])
        self.git("add", "--", self.trace.relative_to(self.repo).as_posix())
        self.trace.unlink()
        self.assertEqual(self.command("check-commit")[0], 1)
        self.git("rm", "--cached", "--", self.trace.relative_to(self.repo).as_posix())
        self.stored_trace.parent.mkdir(parents=True)
        self.stored_trace.write_bytes(b"unexpected store file")
        self.assertEqual(self.command("check-commit")[0], 1)
        self.stored_trace.unlink()
        self.assertEqual(self.command("check-commit")[0], 0)

    def test_record_removal_requires_index_and_store_removal(self):
        manifest = self.seal()
        self.git("add", "--", self.capture.relative_to(self.repo).as_posix())
        self.git("commit", "-m", "Capture with tracked output")
        store.write_manifest(self.manifest_path, dict(outputs=[]))
        self.git("add", "--", self.manifest_path.relative_to(self.repo).as_posix())
        code, output = self.command("check-commit")
        self.assertEqual(code, 1)
        self.assertIn("removed output record capture.jsonl still indexed or stored", output)
        self.git("rm", "--", self.trace.relative_to(self.repo).as_posix())
        self.stored_trace.parent.mkdir(parents=True)
        self.stored_trace.write_bytes(b"still published")
        self.assertEqual(self.command("check-commit")[0], 1)
        self.stored_trace.unlink()
        code, output = self.command("check-commit")
        self.assertEqual(code, 0)
        self.assertIn("REMOVED ", output)
        self.assertIn("full removal", output)

    def test_rerecord_updates_existing_path_with_verified_generation(self):
        manifest = self.seal()
        store.offload(self.capture, manifest)
        self.commit_manifest()
        self.trace.write_bytes(b"new generation")
        manifest = dict(outputs=[store.file_record(self.trace)])
        store.offload(self.capture, manifest, manifest_path=self.manifest_path)
        self.assertIn("store_name", manifest["outputs"][0])
        self.git("add", "--", self.manifest_path.relative_to(self.repo).as_posix())
        self.assertEqual(self.command("check-commit")[0], 0)

    def test_deleted_manifest_cannot_hide_store_generation(self):
        manifest = self.seal()
        store.offload(self.capture, manifest)
        self.commit_manifest()
        self.git("rm", "--", self.manifest_path.relative_to(self.repo).as_posix())
        self.assertEqual(self.command("check-commit")[0], 1)
        self.stored_trace.unlink()
        self.assertIn("REMOVED ", self.command("check-commit")[1])
        self.assertEqual(self.command("check-commit")[0], 0)

    def test_rev_judges_commit_not_index_or_working_tree(self):
        self.seal()
        self.git("add", "--", self.capture.relative_to(self.repo).as_posix())
        self.git("commit", "-m", "Valid capture")
        valid = self.git("rev-parse", "HEAD").strip()
        self.trace.write_bytes(b"bad indexed content")
        self.git("add", "--", self.trace.relative_to(self.repo).as_posix())
        self.git("commit", "-m", "Broken capture")
        invalid = self.git("rev-parse", "HEAD").strip()
        self.seal()
        self.git("add", "--", self.capture.relative_to(self.repo).as_posix())
        self.assertEqual(self.command("check-commit")[0], 0)
        self.assertEqual(self.command("check-commit", "--rev", valid)[0], 0)
        self.assertEqual(self.command("check-commit", "--rev", invalid)[0], 1)

    def test_legacy_walk_retains_deleted_bulk_history(self):
        self.seal()
        self.commit_manifest(legacy=True)
        self.git("add", "--", self.trace.relative_to(self.repo).as_posix())
        self.git("commit", "-m", "Once published")
        self.git("rm", "--", self.trace.relative_to(self.repo).as_posix())
        self.git("commit", "-m", "Bulk removed")
        self.assert_both_refuse()

    def test_legacy_walk_keeps_manifest_paths_separate_and_handles_spaces(self):
        self.seal()
        self.trace.unlink()
        self.commit_manifest(legacy=True)
        other = self.capture / "other manifest.manifest.json"
        self.git("mv", "--", self.manifest_path.relative_to(self.repo).as_posix(),
                 other.relative_to(self.repo).as_posix())
        self.git("commit", "-m", "Renamed manifest cannot borrow age")
        self.assert_both_refuse()

    def test_shallow_history_refuses_legacy_exemption(self):
        self.seal()
        self.trace.unlink()
        self.commit_manifest(legacy=True)
        real = store.git_output
        def shallow(*args):
            return "true" if args == ("rev-parse", "--is-shallow-repository") else real(*args)
        with patch.object(store, "git_output", side_effect=shallow):
            self.assertEqual(self.command("check-commit")[0], 1)
            self.assertIn("requires complete Git history", self.command("check-commit")[1])

    def test_rollback_git_failure_preserves_publication_error(self):
        manifest = self.seal()
        with patch.object(store, "move_verified", side_effect=OSError("original publish error")), \
                patch.object(store, "head_blob", side_effect=store.StoreUnverified("git failed")):
            with self.assertRaises(store.StoreUnverified) as raised:
                store.offload(self.capture, manifest)
        self.assertIn("original publish error", str(raised.exception))
        self.assertIn("git failed", str(raised.exception))
        self.assertIsInstance(raised.exception.__cause__, OSError)
        self.assertTrue(self.trace.exists())

    def test_migration_rollback_error_reports_original(self):
        self.seal()
        self.commit_manifest()
        with patch.object(store, "move_verified", side_effect=OSError("original migration error")), \
                patch.object(store, "rollback_stored", side_effect=store.StoreUnverified("git failed")):
            code, output = self.command("migrate", "--apply")
        self.assertEqual(code, 1)
        self.assertIn("original migration error", output)
        self.assertIn("git failed", output)
        self.assertTrue(self.trace.exists())


    def test_rerecord_absolute_paths_from_other_worktrees_and_slashes(self):
        manifest = self.seal()
        manifest["outputs"][0]["path"] = r"C:\old\ln-trace\capture.jsonl"
        store.write_manifest(self.manifest_path, manifest)
        self.git("add", "--", self.capture.relative_to(self.repo).as_posix())
        self.git("commit", "-m", "Old writer absolute path")
        for name in ("D:/other/ln-trace-a/capture.jsonl", r"E:\main\tmp\capture.jsonl"):
            manifest["outputs"][0]["path"] = name
            store.write_manifest(self.manifest_path, manifest)
            self.git("add", "--", self.manifest_path.relative_to(self.repo).as_posix())
            code, output = self.command("check-commit")
            self.assertEqual(code, 0, output)
            self.assertNotIn("removed output", output)
            self.git("commit", "-m", "Re-record from another worktree")

    def test_missing_cannot_forge_store_name_or_hide_any_generation(self):
        manifest = self.seal()
        record = manifest["outputs"][0]
        record["store_name"] = "capture.jsonl.sha1-" + record["sha1"]
        target = store.store_target(self.trace, [record])
        target.parent.mkdir(parents=True)
        target.write_bytes(self.trace.read_bytes())
        self.trace.unlink()
        store.write_manifest(self.manifest_path, manifest)
        self.commit_manifest()
        missing = dict(path="capture.jsonl", missing=True,
                       store_name="capture.jsonl.sha1-" + "0" * 40)
        for forged in (True, False):
            if not forged:
                missing.pop("store_name")
            store.write_manifest(self.manifest_path, dict(outputs=[missing]))
            self.git("add", "--", self.manifest_path.relative_to(self.repo).as_posix())
            code, output = self.command("check-commit")
            self.assertEqual(code, 1, output)
            self.assertIn("missing record", output)
        # Still refuse a generation not mentioned by the parent manifest.
        target.rename(target.with_name("capture.jsonl.sha1-" + "1" * 40))
        self.assertEqual(self.command("check-commit")[0], 1)
        target.with_name("capture.jsonl.sha1-" + "1" * 40).unlink()
        self.stored_trace.write_bytes(b"plain key")
        self.assertEqual(self.command("check-commit")[0], 1)
        self.stored_trace.unlink()
        self.assertEqual(self.command("check-commit")[0], 0)

    def test_rev_missing_ignores_later_tree_and_leftover_working_file(self):
        store.write_manifest(self.manifest_path,
                             dict(outputs=[dict(path="capture.jsonl", missing=True)]))
        self.commit_manifest()
        absent = self.git("rev-parse", "HEAD").strip()
        self.seal()
        self.git("add", "--", self.capture.relative_to(self.repo).as_posix())
        self.git("commit", "-m", "Later commit adds capture")
        code, output = self.command("check-commit", "--rev", absent)
        self.assertEqual(code, 0, output)
        self.assertIn("outputs=0 refused=0", output)

    def test_case_variant_runtime_root_is_refused_in_index_and_commit(self):
        self.seal()
        blob = self.git("hash-object", "-w", str(self.manifest_path)).strip()
        key = "Docs/naming/runtime/sample/capture.manifest.json"
        self.git("update-index", "--add", "--cacheinfo", "100644," + blob + "," + key)
        code, output = self.command("check-commit")
        self.assertEqual(code, 1, output)
        self.assertIn("case-variant runtime root", output)
        self.git("commit", "-m", "Case variant fixture")
        self.assertIn("case-variant runtime root", self.command("check-commit", "--rev", "HEAD")[1])

    def test_windows_alias_output_name_is_refused_in_both_commands(self):
        manifest = self.seal()
        self.commit_manifest()
        self.assertNotIn("Windows alias output name", self.command("check-commit")[1])
        for name in ("capture.jsonl.", "capture.jsonl ", "capture.jsonl:x", "CAPTUR~1.JSO"):
            with self.subTest(name=name):
                rec = dict(manifest["outputs"][0], path="C:\\old\\sample\\" + name)
                store.write_manifest(self.manifest_path, dict(outputs=[rec]))
                self.git("add", "--", self.manifest_path.relative_to(self.repo).as_posix())
                code, output = self.command("check-commit")
                self.assertEqual(code, 1, output)
                self.assertIn("Windows alias output name", output)
                self.git("commit", "-m", "Alias name fixture")
                code, output = self.command("verify")
                self.assertEqual(code, 1, output)
                self.assertIn("MALFORMED", output)
                self.assertIn("Windows alias output name", output)

    def test_verify_reports_nonregular_entry_and_continues(self):
        self.seal()
        self.git("add", "--", self.capture.relative_to(self.repo).as_posix())
        blob = self.git("hash-object", "-w", str(self.trace)).strip()
        key = self.capture.relative_to(self.repo).as_posix() + "/link"
        self.git("update-index", "--add", "--cacheinfo", "120000," + blob + "," + key)
        self.git("commit", "-m", "Nonregular entry fixture")
        code, output = self.command("verify")
        self.assertEqual(code, 1, output)
        self.assertIn("expected regular file", output)
        self.assertIn("verify: files=1", output)

    def test_gate_resolve_call_count_does_not_scale_with_records(self):
        manifest = self.seal()
        manifest["outputs"] *= 200
        store.write_manifest(self.manifest_path, manifest)
        self.git("add", "--", self.capture.relative_to(self.repo).as_posix())
        with patch.object(Path, "resolve", side_effect=AssertionError("per-record resolve")):
            code, output = self.command("check-commit")
        self.assertEqual(code, 0, output)
        self.assertIn("outputs=200 refused=0", output)

    def test_lane_dispatch_reads_real_git_unicode_and_case_variant_paths(self):
        from types import SimpleNamespace
        from tools.lanes import lanes
        base = self.git("rev-parse", "HEAD").strip()
        directory = self.capture / "é"
        directory.mkdir()
        note = directory / "note.txt"
        note.write_text("input", encoding="utf-8")
        self.git("add", "--", self.capture.relative_to(self.repo).as_posix())
        self.git("commit", "-m", "Unicode capture input")
        blob = self.git("hash-object", "-w", str(note)).strip()
        self.git("update-index", "--add", "--cacheinfo",
                 "100644," + blob + ",Docs/naming/runtime/é.txt")
        self.git("commit", "-m", "Case variant capture input")
        ctx = SimpleNamespace(cfg={"input_checks": [{"cmd": "python store.py check-commit",
                                  "inputs": ["docs/naming/runtime/"]}]})
        with patch.object(lanes, "git", side_effect=lambda *args, **kwargs: self.git(*args)), \
                patch.object(lanes, "run_shell", return_value=0) as run:
            lanes.input_checks(ctx, self.repo, base, io.StringIO())
        self.assertEqual(run.call_count, 2)

    def test_bulk_digest_streams_large_blob_in_bounded_chunks(self):
        data = b"x" * (3 * 1024 * 1024 + 7)
        self.trace.write_bytes(data)
        blob = self.git("hash-object", "-w", str(self.trace)).strip()
        real = subprocess.Popen
        reads = []
        class Reader:
            def __init__(self, wrapped):
                self.wrapped = wrapped
            def read(self, size):
                reads.append(size)
                return self.wrapped.read(size)
            def readline(self):
                return self.wrapped.readline()
            def close(self):
                return self.wrapped.close()
        def process(*args, **kwargs):
            result = real(*args, **kwargs)
            result.stdout = Reader(result.stdout)
            return result
        with patch.object(store.subprocess, "Popen", side_effect=process):
            digests = store.git_blob_digests([blob])
        self.assertEqual(digests[blob], (store.hashlib.sha1(data).hexdigest(), len(data)))
        self.assertGreater(len(reads), 3)
        self.assertLessEqual(max(reads), 1024 * 1024)

    def test_publish_failure_preserves_identical_committed_flag(self):
        manifest = self.seal()
        manifest["outputs"][0]["stored"] = True
        store.write_manifest(self.manifest_path, manifest)
        self.commit_manifest()
        with patch.object(store, "move_verified", side_effect=OSError("publish failed")):
            with self.assertRaises(OSError):
                store.offload(self.capture, manifest)
            self.assertTrue(json.loads(self.manifest_path.read_text(encoding="utf-8"))["outputs"][0]["stored"])
            self.assertEqual(self.command("migrate", "--apply")[0], 1)
            self.assertTrue(json.loads(self.manifest_path.read_text(encoding="utf-8"))["outputs"][0]["stored"])


class RepositoryHistoryTest(unittest.TestCase):
    def test_historical_rerecord_d01de4456(self):
        # Read-only regression on the actual recorded absolute worktree paths.
        probe = subprocess.run(["git", "-C", str(store.REPO_ROOT), "rev-parse",
                                "--verify", "d01de4456^{commit}"], capture_output=True)
        if probe.returncode:
            self.skipTest("historical regression commit not available")
        external = Path(tempfile.gettempdir()) / ("store-history-" + uuid.uuid4().hex)
        external.mkdir()
        try:
            with patch.dict(os.environ, {"BB2_CAPTURE_STORE": str(external)}), \
                    contextlib.redirect_stdout(io.StringIO()) as output:
                code = store.main(["check-commit", "--rev", "d01de4456"])
            self.assertEqual(code, 0, output.getvalue())
            self.assertIn("outputs=2577 refused=0", output.getvalue())
        finally:
            shutil.rmtree(external)


if __name__ == "__main__":
    unittest.main()
