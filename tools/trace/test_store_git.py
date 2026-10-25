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
        shutil.rmtree(self.root, onerror=remove_readonly)

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

    def commit_manifest(self):
        self.git("add", "--", self.manifest_path.relative_to(self.repo).as_posix())
        self.git("commit", "-m", "Publish only capture manifest")

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
        self.commit_manifest()
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
        self.assertTrue(json.loads(self.manifest_path.read_text(encoding="utf-8"))["outputs"][0]["stored"])
        self.assertEqual(self.command("verify")[0], 0)
        self.assertEqual(self.command("migrate", "--apply")[0], 0)
        self.assertFalse(self.trace.exists())
        self.assertEqual(self.command("migrate", "--apply")[0], 0)
        self.assertEqual(self.command("verify")[0], 0)

    def test_migration_sets_flag_before_delete_and_resumes(self):
        self.seal()
        self.git("add", "--", self.capture.relative_to(self.repo).as_posix())
        self.git("commit", "-m", "Local capture before migration")
        with patch.object(store, "move_verified", side_effect=OSError("simulated crash")):
            self.assertEqual(self.command("migrate", "--apply")[0], 1)
        self.assertTrue(self.trace.exists())
        self.assertTrue(json.loads(self.manifest_path.read_text(encoding="utf-8"))["outputs"][0]["stored"])
        self.assertEqual(self.command("verify")[0], 0)
        self.assertEqual(self.command("migrate", "--apply")[0], 0)
        self.assertFalse(self.trace.exists())
        self.assertEqual(self.command("migrate", "--apply")[0], 0)
        self.assertEqual(list(self.root.rglob(".capture-store.json")), [])


if __name__ == "__main__":
    unittest.main()
