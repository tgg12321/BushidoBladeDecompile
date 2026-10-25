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


if __name__ == "__main__":
    unittest.main()
