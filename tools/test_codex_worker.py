#!/usr/bin/env python3
"""Regression tests for tools/codex_worker.py's layer-2 recording and land gate, on a throwaway repo
(never the real tree; the engine call is mocked, no Codex run, nothing pushed).

  python tools/test_codex_worker.py
"""
from __future__ import annotations

import datetime as dt
import hashlib
import io
import json
import os
import shutil
import stat
import subprocess
import sys
import tempfile
import unittest
from contextlib import ExitStack, redirect_stderr, redirect_stdout
from pathlib import Path
from types import SimpleNamespace
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import codex_worker as cw  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
REAL_L2MOD = cw.layer2_module

REAL_RECORD = cw.engine_layer2_record   # Repo tests mock it

FUNC = "comb_ExchangePad"
H1, H2 = "799ab313f506448f", "0123456789abcdef"


def _rm_readonly(fn, path, _exc):
    os.chmod(path, stat.S_IWRITE)
    fn(path)


def rec(verdict, h=H1, reviewer="r1"):
    return json.dumps({"func": FUNC, "addr": "0x80028708", "verdict": verdict, "body_hash": h,
                       "reviewer": reviewer}) + "\n"


class WindowsArgv(unittest.TestCase):
    def test_hand_computed_windows_arguments(self):
        cases = [
            (r'"C:\Program Files\codex.exe" exec', [r'C:\Program Files\codex.exe', 'exec']),
            (r'codex.exe a\\b d"e f"g h', ['codex.exe', r'a\\b', 'de fg', 'h']),
            (r'codex.exe a\"b c', ['codex.exe', 'a"b', 'c']),
            (r'codex.exe a\\"b c" d', ['codex.exe', 'a\\b c', 'd']),
            (r'codex.exe a\\\"b c', ['codex.exe', 'a\\"b', 'c']),
            (r'codex.exe a\\\\"b c"', ['codex.exe', 'a\\\\b c']),
            (r'codex.exe "a""b" c', ['codex.exe', 'a"b', 'c']),
            (r'codex.exe "" tail\\', ['codex.exe', '', 'tail\\\\']),
            (r'"C:\bin\" exec', ['C:\\bin\\', 'exec']),
            (r'codex.exe -c sandbox_mode=\"read-only\"',
             ['codex.exe', '-c', 'sandbox_mode="read-only"']),
        ]
        for command, expected in cases:
            with self.subTest(command=command):
                self.assertEqual(cw.windows_argv(command), expected)

    def test_ambiguous_commands_refused(self):
        for command in (' codex.exe exec', 'codex.exe "unfinished', '"codex.exe', 'codex.exe\0',
                        'co"dex.exe" exec', '"codex.exe"suffix exec'):
            with self.subTest(command=command), self.assertRaises(ValueError):
                cw.windows_argv(command)

    def test_config_strings_and_bare_words(self):
        for value in ('sandbox_mode = read-only', 'sandbox_mode="read-only"',
                      r'sandbox_mode="read-\u006fnly"', r'sandbox_mode="read-\U0000006fnly"'):
            with self.subTest(value=value):
                self.assertEqual(cw.research_config(value), ('sandbox_mode', 'read-only'))


class FailureOutput(unittest.TestCase):
    def test_refusal_tests_capture_both_streams(self):
        names = (
            "CodexIdle.test_research_config_writable_sandbox_refused",
            "CodexIdle.test_no_codex_running_still_refuses_research",
            "CodexIdle.test_idle_still_checks_processes_for_scratch",
            "PassAfterVeto.test_review_older_than_fail_refused",
            "LoadVerdict.test_refusals",
        )
        for name in names:
            with self.subTest(test=name):
                out, err = io.StringIO(), io.StringIO()
                suite = unittest.defaultTestLoader.loadTestsFromName(f"{__name__}.{name}")
                with redirect_stdout(out), redirect_stderr(err):
                    result = unittest.TestResult()
                    suite.run(result)
                self.assertTrue(result.wasSuccessful(), result.errors + result.failures)
                self.assertNotIn("codex_worker: REFUSED", out.getvalue() + err.getvalue())
                self.assertNotIn("codex_worker: FAILED", out.getvalue() + err.getvalue())

    def test_old_python_reports_version_before_importing_tomllib(self):
        script = '''
import builtins, runpy, sys
original_import = builtins.__import__
def without_tomllib(name, *args, **kwargs):
    if name == "tomllib":
        raise ImportError("tomllib unavailable on old Python")
    return original_import(name, *args, **kwargs)
builtins.__import__ = without_tomllib
sys.version_info = (3, 10)
runpy.run_path(sys.argv[1], run_name="__main__")
'''
        result = subprocess.run([sys.executable, "-c", script, str(Path(cw.__file__))],
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 1)
        self.assertIn("codex_worker: needs Python 3.12+", result.stderr)
        self.assertNotIn("ImportError", result.stderr)

    def capture_exit(self, action):
        out, err = io.StringIO(), io.StringIO()
        with redirect_stdout(out), redirect_stderr(err), \
                mock.patch.object(out, "flush", wraps=out.flush) as out_flush, \
                mock.patch.object(err, "flush", wraps=err.flush) as err_flush:
            with self.assertRaises(SystemExit) as caught:
                action()
            return caught.exception.code, out.getvalue(), err.getvalue(), \
                out_flush.call_count, err_flush.call_count

    def test_refuse_writes_both_streams_and_flushes(self):
        code, out, err, out_flush, err_flush = self.capture_exit(
            lambda: cw.refuse("a Codex run is live"))
        self.assertEqual(code, 2)
        self.assertEqual(err, "codex_worker: REFUSED -- a Codex run is live\n")
        self.assertEqual(out, "codex_worker: REFUSED (exit 2): a Codex run is live\n")
        self.assertGreater(out_flush, 0)
        self.assertGreater(err_flush, 0)

    def test_fail_writes_both_streams_and_preserves_code(self):
        code, out, err, out_flush, err_flush = self.capture_exit(
            lambda: cw.fail("specific failure", 5))
        self.assertEqual(code, 5)
        self.assertEqual(err, "codex_worker: specific failure\n")
        self.assertEqual(out, "codex_worker: FAILED (exit 5): specific failure\n")
        self.assertGreater(out_flush, 0)
        self.assertGreater(err_flush, 0)

    def test_runtime_error_uses_fail(self):
        with mock.patch.object(sys, "argv", ["codex_worker", "land", "item"]), \
                mock.patch.object(cw, "cmd_land", side_effect=RuntimeError("git failed")):
            code, out, err, _, _ = self.capture_exit(cw.main)
        self.assertEqual(code, 2)
        self.assertEqual(err, "codex_worker: git failed\n")
        self.assertEqual(out, "codex_worker: FAILED (exit 2): git failed\n")

    def test_child_return_code_passthrough_has_no_failure_label(self):
        main, scratch, trusted = Path("main"), Path("scratch"), Path("trusted")
        with mock.patch.object(sys, "argv", ["codex_worker", "shadow", "item", "make"]), \
                mock.patch.object(cw, "main_checkout", return_value=main), \
                mock.patch.object(cw, "check_engine_args"), \
                mock.patch.object(cw, "layout", return_value=(main, scratch, trusted, "branch")), \
                mock.patch.object(cw, "verify_trusted"), \
                mock.patch.object(cw, "harvest", return_value=("base", "tree12345", [])), \
                mock.patch.object(cw, "sync_trusted_to_tree"), \
                mock.patch.object(cw.subprocess, "run", return_value=SimpleNamespace(returncode=7)):
            code, out, err, _, _ = self.capture_exit(cw.main)
        self.assertEqual(code, 7)
        self.assertIn("building scratch tree", out)
        self.assertNotIn("REFUSED", out + err)
        self.assertNotIn("FAILED", out + err)

    def test_success_prints_nothing_new(self):
        out, err = io.StringIO(), io.StringIO()
        with mock.patch.object(sys, "argv", ["codex_worker", "land", "item"]), \
                mock.patch.object(cw, "cmd_land", side_effect=lambda a: print("success")), \
                redirect_stdout(out), redirect_stderr(err):
            cw.main()
        self.assertEqual(out.getvalue(), "success\n")
        self.assertEqual(err.getvalue(), "")


class CodexIdle(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="cw-idle-")
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        # Pure metadata paths: the test itself may live inside a forbidden codex-* tree.
        self.main = Path(self.root.anchor) / "cw-idle-fixture" / "main"
        self.scratch = self.main.parent / "bb2-worktrees" / "codex-item"
        self.snap = self.main.parent / "bb2-worktrees" / "codexr-research"
        self.meta = {"run_id": "research-1", "sandbox": "read-only", "cwd": str(self.snap)}
        self.meta_file = self.root / "bridge" / "runs" / "research-1" / "meta.json"
        self.meta_file.parent.mkdir(parents=True)
        self.meta_file.write_text(json.dumps(self.meta), encoding="utf-8")
        self.status = {"bridge_slot": {"busy": True, "holder": {"run_id": "research-1", "codex_pid": 41}},
                       "processes": [{"pid": 41, "kind": "headless-task", "cmd": self.exec_cmd()}]}
        self.config = self.root / "codex-home" / "config.toml"
        self.config.parent.mkdir()
        self.env = mock.patch.dict(os.environ, CODEX_BRIDGE_HOME=str(self.root / "bridge"),
                                   CODEX_HOME=str(self.config.parent))
        self.env.start()
        self.addCleanup(self.env.stop)

    def exec_cmd(self, sandbox="read-only", cwd=None, extra=""):
        # The bridge's own exec command line (codex_bridge.py run).
        output = subprocess.list2cmdline([str(self.meta_file.parent / "last.md")])
        return (f'codex.exe exec --json -o {output} -m gpt -c model_reasoning_effort="medium" '
                f'-c sandbox_mode="{sandbox}" -c approval_policy="never" {extra}'
                f'-C {cwd or self.snap} -')

    def check(self, allowed):
        results = [SimpleNamespace(returncode=0, stdout=json.dumps(self.status)),
                   SimpleNamespace(returncode=0, stdout="unrelated process")]
        with mock.patch.object(cw, "pin_file", return_value=self.root / "pin"), \
                mock.patch.object(cw.subprocess, "run", side_effect=results), \
                redirect_stdout(io.StringIO()), redirect_stderr(io.StringIO()):
            if allowed:
                cw.codex_idle(self.main, self.scratch)
            else:
                with self.assertRaises(SystemExit) as err:
                    cw.codex_idle(self.main, self.scratch)
                self.assertEqual(err.exception.code, 2)

    def test_readonly_research_exec_attributed(self):
        self.check(True)

    def test_research_metadata_environment_overrides_refused(self):
        for key in ("env", "environment", "env_overrides", "environment_overrides", "CODEX_HOME",
                    "codex_home"):
            with self.subTest(key=key):
                self.meta_file.write_text(json.dumps(dict(self.meta, **{key: {}})), encoding="utf-8")
                self.check(False)

    def test_research_metadata_must_be_dict(self):
        # A string-keyed mapping is not necessarily a dict. Use one which
        # satisfies every old access, so exception handling cannot hide this guard.
        from collections import UserDict
        with mock.patch.object(cw.json, "loads", return_value=UserDict(self.meta)):
            self.assertFalse(cw.readonly_research_running(
                self.main, self.scratch, self.status, self.status["processes"]))

    def test_research_metadata_keys_must_be_strings(self):
        class NonStringKey:
            def lower(self):
                return "unrelated"

        meta = dict(self.meta)
        meta[NonStringKey()] = None
        with mock.patch.object(cw.json, "loads", return_value=meta):
            self.assertFalse(cw.readonly_research_running(
                self.main, self.scratch, self.status, self.status["processes"]))

    def test_research_config_missing_safe(self):
        self.check(True)

    def test_research_config_clean_safe(self):
        for text in ('', 'model = "gpt"\n', 'sandbox_mode = "read-only"\n',
                     '[profiles.research]\nmodel = "gpt"\n'):
            with self.subTest(text=text):
                self.config.write_text(text, encoding="utf-8")
                self.check(True)

    def test_research_config_writable_sandbox_refused(self):
        self.config.write_text('sandbox_mode = "workspace-write"\n', encoding="utf-8")
        self.check(False)

    def test_research_config_profile_selection_refused(self):
        self.config.write_text('profile = "full"\n', encoding="utf-8")
        self.check(False)

    def test_research_config_profiles_must_be_table(self):
        for value in ('"text"', '[]', '[{model = "gpt"}]', '42', 'true'):
            with self.subTest(value=value):
                self.config.write_text(f'profiles = {value}\n', encoding="utf-8")
                self.assertFalse(cw.research_user_config_safe())
                self.check(False)

    def test_research_config_profile_sandbox_refused(self):
        for mode in ("workspace-write", "read-only"):
            with self.subTest(mode=mode):
                self.config.write_text(f'[profiles.research]\nsandbox_mode = "{mode}"\n',
                                       encoding="utf-8")
                self.check(False)

    def test_research_config_unparsable_refused(self):
        self.config.write_text('model = [', encoding="utf-8")
        self.check(False)

    def test_research_config_unreadable_refused(self):
        with mock.patch.object(Path, "read_text", side_effect=PermissionError):
            self.assertFalse(cw.research_user_config_safe())

    def test_research_config_default_home(self):
        with mock.patch.dict(os.environ, CODEX_HOME=""), \
                mock.patch.object(Path, "home", return_value=self.root):
            config = self.root / ".codex" / "config.toml"
            config.parent.mkdir()
            config.write_text('profile = "full"\n', encoding="utf-8")
            self.check(False)

    def test_readonly_research_real_bridge_command_line(self):
        # start_task's argv, serialized as Popen does on Windows (TOML quotes
        # are escaped). Exercise spaces in executable, output and cwd paths,
        # for every effort accepted by the proof (not a live models cache).
        self.snap = self.main.parent / "space here" / "bb2-worktrees" / self.snap.name
        self.main = self.snap.parent.parent / "main"
        self.scratch = self.snap.parent / "codex-item"
        self.meta["cwd"] = str(self.snap)
        self.meta_file = self.root / "bridge with spaces" / "runs" / "research-1" / "meta.json"
        self.meta_file.parent.mkdir(parents=True)
        self.meta_file.write_text(json.dumps(self.meta), encoding="utf-8")
        with mock.patch.dict(os.environ, CODEX_BRIDGE_HOME=str(self.meta_file.parents[2])):
            for effort in ("low", "medium", "high", "xhigh", "max", "ultra"):
                with self.subTest(effort=effort):
                    common = ["--json", "-o", str(self.meta_file.parent / "last.md"),
                              "-m", "gpt-6.1-sol", "-c", f'model_reasoning_effort="{effort}"',
                              "-c", 'sandbox_mode="read-only"', "-c", 'approval_policy="never"']
                    cmd = [r"C:\Program Files\Codex\codex.exe", "exec"] + common + ["-C", str(self.snap), "-"]
                    self.status["processes"][0]["cmd"] = subprocess.list2cmdline(cmd)
                    self.check(True)

    def test_readonly_research_exec_without_directory_refused(self):
        self.status["processes"][0]["cmd"] = self.exec_cmd().replace(f"-C {self.snap} ", "")
        self.check(False)

    def test_readonly_research_unknown_options_and_positionals_refused(self):
        for extra in ("--unknown-option", "unexpected-positional"):
            with self.subTest(extra=extra):
                self.status["processes"][0]["cmd"] = self.exec_cmd(extra=extra + " ")
                self.check(False)

    def test_readonly_research_parseable_unknown_config_keys_refused(self):
        for setting in ("profile=full", "model_provider=oss"):
            with self.subTest(setting=setting):
                self.assertEqual(cw.research_config(setting), tuple(setting.split("=")))
                self.status["processes"][0]["cmd"] = self.exec_cmd(extra=f"-c {setting} ")
                self.check(False)

    def test_readonly_research_requires_exec_subcommand(self):
        for subcommand in ("--yolo", "resume"):
            with self.subTest(subcommand=subcommand):
                self.status["processes"][0]["cmd"] = self.exec_cmd().replace(
                    "codex.exe exec ", f"codex.exe {subcommand} ")
                self.check(False)

    def test_readonly_research_executable_basename(self):
        for executable, allowed in (
                ("codex", True), ("CODEX.EXE", True),
                (r"C:\Program Files\Codex\CoDeX.ExE", True),
                ("C:/Program Files/Codex/CODEX", True),
                ("other.exe", False), (r"C:\codex.exe\other.exe", False),
                ("C:/codex/other.exe", False), ("codex.exe.bak", False)):
            with self.subTest(executable=executable):
                self.status["processes"][0]["cmd"] = self.exec_cmd().replace(
                    "codex.exe", subprocess.list2cmdline([executable]), 1)
                self.check(allowed)

    def test_readonly_research_literal_quote_bypasses_refused(self):
        for override in ('-s danger-full-access', f'--add-dir {self.main}', f'-C {self.main}'):
            with self.subTest(override=override):
                self.status["processes"][0]["cmd"] = (
                    r'codex.exe exec -m \"gpt ' + override
                    + r' -c x=\"y -c sandbox_mode=read-only -C ' + str(self.snap) + ' -')
                self.check(False)

    def test_readonly_research_bridge_values_pinned(self):
        base = self.exec_cmd()
        output = subprocess.list2cmdline([str(self.meta_file.parent / "last.md")])
        replacements = [
            (f'-o {output}', '-o elsewhere.md'),
            ('approval_policy="never"', 'approval_policy=on-request'),
            ('model_reasoning_effort="medium"', 'model_reasoning_effort=unknown'),
            ('-m gpt', r'-m \"gpt\"'),
            ('-m gpt', '-m "gpt odd"'),
            ('-m gpt', '-m -gpt'),
        ]
        for old, new in replacements:
            with self.subTest(new=new):
                self.status["processes"][0]["cmd"] = base.replace(old, new)
                self.check(False)

    def test_readonly_research_config_syntax_refused(self):
        for setting in ("'read-only'", '"read-only"junk', 'read-only=extra',
                        '"read-only"=extra', '["read-only"]', '"read-only', '"read-only\\q"'):
            with self.subTest(setting=setting):
                override = subprocess.list2cmdline(['sandbox_mode=' + setting])
                self.status["processes"][0]["cmd"] = self.exec_cmd().replace(
                    '-c sandbox_mode="read-only"', '-c ' + override)
                self.check(False)

    def test_readonly_research_config_spellings(self):
        for flag in ("-c ", "-c", "-c=", "--config ", "--config="):
            for assignment in ("sandbox_mode=read-only", "sandbox_mode= read-only", "sandbox_mode = read-only",
                               "sandbox_mode =read-only"):
                with self.subTest(flag=flag, assignment=assignment):
                    self.status["processes"][0]["cmd"] = self.exec_cmd().replace(
                        '-c sandbox_mode="read-only"', f'{flag}"{assignment}"')
                    self.check(True)

    def test_readonly_research_sandbox_config_overrides_refused(self):
        for flag in ("-c ", "-c", "-c=", "--config ", "--config="):
            for assignment in ("sandbox_mode=workspace-write", "sandbox_mode= workspace-write", "sandbox_mode = workspace-write",
                               "sandbox_mode =workspace-write", "sandbox_mode=read-only"):
                with self.subTest(flag=flag, assignment=assignment):
                    self.status["processes"][0]["cmd"] = self.exec_cmd(
                        extra=f'{flag}"{assignment}" ')
                    self.check(False)

    def test_readonly_research_unquoted_spaced_overrides_refused(self):
        for setting in ("sandbox_mode = workspace-write", "sandbox_mode =workspace-write",
                        "sandbox_mode= workspace-write"):
            with self.subTest(setting=setting):
                self.status["processes"][0]["cmd"] = self.exec_cmd(extra=f"-c {setting} ")
                self.check(False)

    def test_readonly_research_option_values_are_not_evidence(self):
        for option in ("-o", "-m"):
            with self.subTest(option=option):
                self.status["processes"][0]["cmd"] = (
                    f"codex.exe exec {option} -csandbox_mode=read-only -C {self.snap} -")
                self.check(False)

    def test_readonly_research_short_option_case_is_significant(self):
        for replacement in ('-Csandbox_mode="read-only"', '-c SANDBOX_MODE="read-only"'):
            with self.subTest(replacement=replacement):
                self.status["processes"][0]["cmd"] = self.exec_cmd().replace(
                    '-c sandbox_mode="read-only"', replacement)
                self.check(False)

    def test_readonly_research_other_sandbox_configs_refused(self):
        for key in ("sandbox_workspace_write", "sandbox_workspace_write.writable_roots",
                    "sandbox_permissions", "writable_roots", "permissions"):
            for flag in ("-c ", "-c", "--config ", "--config="):
                with self.subTest(key=key, flag=flag):
                    self.status["processes"][0]["cmd"] = self.exec_cmd(
                        extra=f'{flag}"{key} = []" ')
                    self.check(False)

    def test_readonly_research_option_overrides_refused(self):
        for option in ("-s workspace-write", "-sworkspace-write", "-s=workspace-write",
                       "--sandbox workspace-write", "--sandbox=workspace-write",
                       "-p NAME", "-pNAME", "-p=NAME", "--profile NAME", "--profile=NAME"):
            with self.subTest(option=option):
                self.status["processes"][0]["cmd"] = self.exec_cmd(extra=option + " ")
                self.check(False)

    def test_readonly_research_directory_spellings(self):
        for directory in (f'-C "{self.snap}"', f'-C"{self.snap}"',
                          f'--cd "{self.snap}"', f'--cd="{self.snap}"'):
            with self.subTest(directory=directory):
                self.status["processes"][0]["cmd"] = self.exec_cmd().replace(
                    f"-C {self.snap}", directory)
                self.check(True)
                for other in (self.scratch, f"{self.snap}-2", f"{self.snap} other"):
                    with self.subTest(other=str(other)):
                        override = directory.replace(str(self.snap), str(other))
                        self.status["processes"][0]["cmd"] = self.exec_cmd(extra=override + " ")
                        self.check(False)
                        self.status["processes"][0]["cmd"] = self.exec_cmd() + " " + override
                        self.check(False)

    def test_readonly_research_resume_without_directory_refused(self):
        self.status["processes"][0]["cmd"] = self.exec_cmd().replace(
            "exec ", "exec resume ").replace(f"-C {self.snap} -", "thread-id -")
        self.check(False)

    def test_readonly_research_second_process_attributed(self):
        self.status["processes"].append({"pid": 42, "kind": "headless-task",
                                         "cmd": self.exec_cmd().replace(
                                             str(self.snap), str(self.snap).replace("/", "\\").upper())})
        self.check(True)

    def test_no_listed_process_or_unlisted_holder_refused(self):
        procs = self.status["processes"]
        self.status["processes"] = []
        self.check(False)
        self.status["processes"] = [dict(procs[0], pid=42)]
        self.check(False)
        self.status["processes"] = procs
        for pid in (None, "41", 0, True):
            with self.subTest(pid=pid):
                self.status["bridge_slot"]["holder"]["codex_pid"] = pid
                self.check(False)

    def test_holder_pid_alone_not_exempt(self):
        self.status["processes"][0]["cmd"] = "codex exec"
        self.check(False)

    def test_command_line_must_show_readonly_exec_of_snapshot(self):
        for cmd in (self.exec_cmd(sandbox="workspace-write"),
                    self.exec_cmd(cwd=self.scratch, extra=f"--add-dir {self.snap} "),
                    self.exec_cmd(extra=f"--add-dir {self.scratch} "),
                    self.exec_cmd(cwd=f"{self.snap}-other"),
                    f"codex.exe exec -C {self.snap} -",
                    self.exec_cmd(extra='-c sandbox_mode="danger-full-access" '),
                    self.exec_cmd(extra="-s workspace-write "),
                    self.exec_cmd(extra="--sandbox=workspace-write "),
                    self.exec_cmd(extra="--full-auto "),
                    self.exec_cmd(extra="--yolo "),
                    self.exec_cmd(extra="--dangerously-bypass-approvals-and-sandbox ")):
            with self.subTest(cmd=cmd):
                self.status["processes"][0]["cmd"] = cmd
                self.check(False)

    def test_unsafe_metadata_refused(self):
        for change in ({"sandbox": "workspace-write"}, {"cwd": str(self.scratch)},
                       {"cwd": str(self.main)}, {"cwd": str(self.snap / "nested")},
                       {"cwd": str(self.root / "codexr-research")}, {"cwd": ""},
                       {"cwd": "relative/codexr-research"}, {"cwd": None},
                       {"cwd": str(self.snap.parent / "codexv-research")},
                       {"run_id": "different"}):
            with self.subTest(change=change):
                self.meta_file.write_text(json.dumps(dict(self.meta, **change)), encoding="utf-8")
                self.check(False)

    def test_unreadable_or_malformed_metadata_refused(self):
        self.meta_file.unlink()
        self.check(False)
        for data in ("{", "[]", "null", "{}"):
            self.meta_file.write_text(data, encoding="utf-8")
            self.check(False)

    def test_extra_unattributed_process_refused(self):
        for cmd in ("codex exec", self.exec_cmd(cwd=f"{self.snap}-other"),
                    self.exec_cmd(cwd=f"prefix{self.snap}")):
            self.status["processes"] = [self.status["processes"][0],
                                        {"pid": 42, "kind": "headless-task", "cmd": cmd}]
            self.check(False)

    def test_pin_refused(self):
        (self.root / "pin").touch()
        self.check(False)

    def test_free_bridge_cannot_exempt_process(self):
        self.status["bridge_slot"]["busy"] = False
        self.check(False)

    def test_bad_status_refused(self):
        for status in ([], None, {"processes": None}, {"processes": [None]},
                       {"processes": [], "bridge_slot": []}):
            self.status = status
            self.check(False)

    def test_no_codex_running_still_refuses_research(self):
        with mock.patch.object(cw, "pin_file", return_value=self.root / "pin"), \
                mock.patch.object(cw.subprocess, "run", return_value=SimpleNamespace(
                    returncode=0, stdout=json.dumps(self.status))), \
                redirect_stdout(io.StringIO()), redirect_stderr(io.StringIO()):
            with self.assertRaises(SystemExit):
                cw.no_codex_running(self.main)

    def test_idle_still_checks_processes_for_scratch(self):
        with mock.patch.object(cw, "pin_file", return_value=self.root / "pin"), \
                mock.patch.object(cw.subprocess, "run", side_effect=[
                    SimpleNamespace(returncode=0, stdout=json.dumps(self.status)),
                    SimpleNamespace(returncode=0, stdout=f'editor "{self.scratch}"')]), \
                redirect_stdout(io.StringIO()), redirect_stderr(io.StringIO()):
            with self.assertRaises(SystemExit):
                cw.codex_idle(self.main, self.scratch)


class LedgerProblems(unittest.TestCase):
    def test_reviewable_latest_pass_ok(self):
        self.assertEqual(cw.ledger_problems({FUNC: (b"", rec("PASS").encode())}, True), [])

    def test_reviewable_fail_after_pass_refused(self):
        new = (rec("PASS") + rec("FAIL", reviewer="r2")).encode()
        p = cw.ledger_problems({FUNC: (b"", new)}, True, {FUNC})
        self.assertEqual(len(p), 1)
        self.assertIn("latest layer-2 record is FAIL", p[0])

    def test_unlanded_fail_on_unchanged_function_says_land_it_first(self):
        p = cw.ledger_problems({FUNC: (b"", rec("FAIL").encode())}, True, set())
        self.assertTrue(p and "land that FAIL first" in p[0], p)

    def test_reviewable_fail_then_new_pass_ok(self):
        old = (rec("PASS") + rec("FAIL")).encode()
        self.assertEqual(cw.ledger_problems({FUNC: (old, old + rec("PASS", H2).encode())}, True), [])

    def test_reviewable_needs_user_refused(self):
        self.assertTrue(cw.ledger_problems({FUNC: (b"", rec("NEEDS_USER").encode())}, True, {FUNC}))

    def test_ledger_only_fail_ok(self):
        old = rec("PASS").encode()
        self.assertEqual(cw.ledger_problems({FUNC: (old, old + rec("FAIL").encode())}, False), [])

    def test_ledger_only_pass_refused(self):
        p = cw.ledger_problems({FUNC: (b"", rec("PASS").encode())}, False)
        self.assertTrue(p and "only FAIL / NEEDS_USER" in p[0])

    def test_not_append_only_refused(self):
        old = (rec("PASS") + rec("FAIL")).encode()
        p = cw.ledger_problems({FUNC: (old, rec("PASS").encode())}, True)
        self.assertTrue(p and "append-only" in p[0])

    def test_empty_commit_refused(self):
        self.assertTrue(cw.ledger_problems({}, False))
        self.assertEqual(cw.ledger_problems({}, True), [])


class CopyProblems(unittest.TestCase):
    BLOB = b'{"decision": "FAIL"}'
    SHA = hashlib.sha1(BLOB).hexdigest()
    PATH = f"memory/grind/{FUNC}/layer2_verdicts/{SHA}.json"

    def changes(self, vf):
        line = json.dumps({"func": FUNC, "verdict": "FAIL", "body_hash": H1, "verdict_file": vf}) + "\n"
        return {FUNC: (b"", line.encode())}

    def test_new_referenced_copy_ok(self):
        self.assertEqual(cw.copy_problems({self.PATH: ("A", self.BLOB)}, self.changes(self.PATH)), [])

    def test_modified_or_deleted_copy_refused(self):
        for st in ("M", "D"):
            p = cw.copy_problems({self.PATH: (st, self.BLOB)}, self.changes(self.PATH))
            self.assertTrue(p and "write-once" in p[0], p)

    def test_wrong_content_refused(self):
        p = cw.copy_problems({self.PATH: ("A", b"forged")}, self.changes(self.PATH))
        self.assertTrue(p and "SHA1" in p[0], p)

    def test_unreferenced_copy_refused(self):
        p = cw.copy_problems({self.PATH: ("A", self.BLOB)}, self.changes("elsewhere.json"))
        self.assertTrue(p and "no record" in p[0], p)


class PassAfterVeto(unittest.TestCase):
    FAIL_AT = {"verdict": "FAIL", "date": "2026-10-22T12:00:00Z", "reviewer": "r2"}

    def refused(self, records, reviews, sha="b" * 40, mine=None):
        mine = [len(reviews) - 1] if mine is None else mine
        with redirect_stdout(io.StringIO()), redirect_stderr(io.StringIO()) as err:
            try:
                cw.pass_after_last_veto(records, reviews, mine, sha)
            except SystemExit as e:
                return e.code, err.getvalue()
        return 0, ""

    def test_review_older_than_fail_refused(self):
        code, err = self.refused([{"verdict": "PASS"}, self.FAIL_AT], [{"ts": "2026-10-22T06:59:00-05:00"}])
        self.assertEqual(code, 2)
        self.assertIn("fresh cheat-reviewer", err)

    def test_review_newer_than_fail_ok(self):
        self.assertEqual(self.refused([self.FAIL_AT], [{"ts": "2026-10-22T07:00:01-05:00"}])[0], 0)

    def test_same_verdict_json_refused(self):
        code, err = self.refused([{"verdict": "PASS", "verdict_sha1": "a" * 40}], [], sha="a" * 40, mine=[])
        self.assertEqual(code, 2)
        self.assertIn("already recorded", err)

    def test_veto_recorded_here_is_ordered_by_position_not_clock(self):
        veto = dict(self.FAIL_AT, verdict_sha1="c" * 40)
        here = {"verdict": "FAIL", "verdict_sha1": "c" * 40, "ts": "2026-10-22T07:00:00-05:00"}
        future = {"verdict": "PASS", "ts": "2030-01-01T00:00:00+00:00"}
        past = {"verdict": "PASS", "ts": "2000-01-01T00:00:00+00:00"}
        self.assertEqual(self.refused([veto], [future, here], mine=[0])[0], 2)   # before it, clock says after
        self.assertEqual(self.refused([veto], [here, past], mine=[1])[0], 0)     # after it, clock says before


class LoadVerdict(unittest.TestCase):
    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp(prefix="cw-test-"))

    def tearDown(self):
        shutil.rmtree(self.tmp, onerror=_rm_readonly)  # the test's own temp dir

    def vf(self, obj, prose=False):
        f = self.tmp / "v.json"
        text = json.dumps(obj)
        f.write_text(f"Verdict follows.\n{text}\nend" if prose else text, encoding="utf-8")
        return f

    def test_every_decision_accepted(self):
        for d in cw.VERDICTS:
            v = cw.load_verdict(self.vf({"decision": d, "function": FUNC, "body_hash": H1}, prose=True), FUNC)
            self.assertEqual(v["decision"], d)

    def test_refusals(self):
        for bad in ({"decision": "MAYBE", "function": FUNC, "body_hash": H1},
                    {"decision": "FAIL", "function": "other", "body_hash": H1},
                    {"decision": "FAIL", "function": FUNC, "body_hash": "xyz"}):
            with self.assertRaises(SystemExit) as e, \
                    redirect_stdout(io.StringIO()), redirect_stderr(io.StringIO()):
                cw.load_verdict(self.vf(bad), FUNC)
            self.assertEqual(e.exception.code, 2)


class Repo(unittest.TestCase):
    """main == trusted == one throwaway repo: branch `main` plus the item branch checked out."""
    ITEM = "fake-test"

    def g(self, *args):
        return subprocess.run(["git", *args], cwd=self.repo, check=True, capture_output=True, text=True,
                              env=cw.clean_env()).stdout.strip()

    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp(prefix="cw-test-"))
        self.repo = self.tmp / "repo"
        self.repo.mkdir()
        self.hooks = self.tmp / "hooks"
        self.hooks.mkdir()
        self.g("init", "-q", "-b", "main")
        for k, v in (("user.name", "t"), ("user.email", "t@example.invalid"), ("commit.gpgsign", "false"),
                     ("core.hooksPath", str(self.hooks)), ("core.autocrlf", "false")):
            self.g("config", k, v)
        (self.repo / ".gitignore").write_text("tmp/\n", encoding="utf-8", newline="\n")
        (self.repo / "src").mkdir()
        (self.repo / "src" / "a.c").write_text(f"int f(void) {{ return 0; }}\nvoid {FUNC}(void) {{ g(0); }}\n",
                                               encoding="utf-8", newline="\n")
        (self.repo / "engine").mkdir()
        (self.repo / "engine" / "queue.json").write_text(json.dumps({"items": [{"func": FUNC}]}),
                                                         encoding="utf-8", newline="\n")
        (self.repo / "src" / "b.c").write_text('INCLUDE_ASM("asm/funcs", asm_func);\n', encoding="utf-8",
                                               newline="\n")
        (self.repo / "asm" / "funcs").mkdir(parents=True)
        (self.repo / "asm" / "funcs" / "asm_func.s").write_text("glabel asm_func\n nop\n", encoding="utf-8",
                                                                newline="\n")
        (self.repo / "memory" / "grind" / "asm_func").mkdir(parents=True)
        (self.repo / "memory" / "grind" / "asm_func" / "layer2.jsonl").write_text(rec("PASS"), encoding="utf-8",
                                                                                  newline="\n")
        led = self.repo / "memory" / "grind" / FUNC
        led.mkdir(parents=True)
        (led / "layer2.jsonl").write_text(rec("PASS"), encoding="utf-8", newline="\n")
        self.g("add", "-A")
        self.g("commit", "-q", "-m", "base")
        self.branch = f"codex/{self.ITEM}"
        self.g("checkout", "-q", "-b", self.branch)
        self.reviews = self.tmp / "reviews.jsonl"
        self.engine_calls = []
        patches = [
            mock.patch.object(cw, "layout", lambda item: (self.repo, self.tmp / "scratch", self.repo, self.branch)),
            mock.patch.object(cw, "verify_trusted", lambda *a, **k: None),
            mock.patch.object(cw, "codex_idle", lambda *a, **k: None),
            mock.patch.object(cw, "scratch_committed", lambda *a, **k: None),
            mock.patch.object(cw, "recreate_scratch", lambda *a, **k: None),
            mock.patch.object(cw, "pin_file", lambda main: self.tmp / "no-pin.json"),
            mock.patch.object(cw, "reviews_file", lambda main, item: self.reviews),
            mock.patch.object(cw, "engine_layer2_record", self.fake_record),
            mock.patch.object(cw, "layer2_module", lambda main: REAL_L2MOD(REPO_ROOT)),
        ]
        for p in patches:
            p.start()
            self.addCleanup(p.stop)

    def tearDown(self):
        shutil.rmtree(self.tmp, onerror=_rm_readonly)  # the test's own temp repo

    def fake_record(self, main, trusted, func, rel, verdict, reviewer, scope, notes):
        """What `engine.cli layer2 record --verdict-file` writes: one ledger line + the verdict copy."""
        self.engine_calls.append((func, verdict["decision"], verdict["body_hash"], reviewer, scope, notes))
        raw = (trusted / rel).read_bytes()
        sha = hashlib.sha1(raw).hexdigest()
        d = trusted / "memory" / "grind" / func
        (d / "layer2_verdicts").mkdir(parents=True, exist_ok=True)
        (d / "layer2_verdicts" / f"{sha}.json").write_bytes(raw)
        line = json.loads(rec(verdict["decision"], verdict["body_hash"], reviewer))
        line.update(date=dt.datetime.now(dt.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"), verdict_sha1=sha,
                    verdict_file=f"memory/grind/{func}/layer2_verdicts/{sha}.json")
        with open(d / "layer2.jsonl", "a", encoding="utf-8", newline="\n") as fh:
            fh.write(json.dumps(line) + "\n")

    def verdict(self, decision, h=H1, name="v.json"):
        f = self.tmp / name
        f.write_text(json.dumps({"decision": decision, "function": FUNC, "body_hash": h,
                                 "summary": name}), encoding="utf-8")
        return f

    def layer2(self, decision, reviewer="r2", msg=None, h=H1, notes="", name="v.json"):
        a = SimpleNamespace(item=self.ITEM, func=FUNC, verdict_file=str(self.verdict(decision, h, name)),
                            reviewer=reviewer, scope="cheat-cleanup", notes=notes,
                            message_file=str(msg) if msg else "")
        with redirect_stdout(io.StringIO()), redirect_stderr(io.StringIO()) as err:
            try:
                cw.cmd_layer2(a)
            except SystemExit as e:
                return e.code, err.getvalue()
        return 0, err.getvalue()

    def code_commit(self, body=True):
        """Change f and, with `body`, FUNC's body (which has a ledger)."""
        (self.repo / "src" / "a.c").write_text(
            f"int f(void) {{ return 1; }}\nvoid {FUNC}(void) {{ g({int(body)}); }}\n", encoding="utf-8",
            newline="\n")
        self.g("commit", "-q", "-am", "code")

    def review_pass(self, agent, later=0):
        key = cw.diff_key(self.repo, self.g("rev-parse", self.branch))
        ts = (dt.datetime.now().astimezone() + dt.timedelta(seconds=later)).isoformat(timespec="seconds")
        with self.reviews.open("a", encoding="utf-8") as fh:
            fh.write(json.dumps({"ts": ts, "kind": "cheat-reviewer", "verdict": "PASS", "key": key,
                                 "agent": agent}) + "\n")

    def msg(self):
        m = self.tmp / "msg.txt"
        m.write_text(f"cheat-cleanup: record {FUNC}'s layer-2 FAIL\n", encoding="utf-8")
        return m


class Layer2Record(Repo):
    def test_fail_without_item_commit_makes_ledger_only_commit(self):
        code, err = self.layer2("FAIL", msg=self.msg())
        self.assertEqual(code, 0, err)
        self.assertEqual(self.engine_calls, [(FUNC, "FAIL", H1, "r2", "cheat-cleanup", "")])
        tip = self.g("rev-parse", "HEAD")
        files = self.g("diff", "--name-only", f"{tip}^", tip).splitlines()
        self.assertIn(f"memory/grind/{FUNC}/layer2.jsonl", files)
        self.assertTrue(all(cw.LEDGER.match(f) for f in files), files)
        self.assertEqual(self.g("rev-list", "--count", "main..HEAD"), "1")
        self.assertEqual(cw.required_kinds(self.repo, tip)[0], [])
        changes = cw.ledger_changes(self.repo, tip)
        self.assertEqual(json.loads(changes[FUNC][1].splitlines()[-1])["verdict"], "FAIL")
        self.assertEqual(cw.ledger_problems(changes, False), [])

    def test_needs_user_without_item_commit_ok(self):
        self.assertEqual(self.layer2("NEEDS_USER", msg=self.msg())[0], 0)

    def test_fail_without_item_commit_needs_message(self):
        code, err = self.layer2("FAIL")
        self.assertEqual(code, 2)
        self.assertIn("--message-file", err)
        self.assertEqual(self.engine_calls, [])

    def test_pass_without_item_commit_refused(self):
        code, err = self.layer2("PASS", msg=self.msg())
        self.assertEqual(code, 2)
        self.assertEqual(self.engine_calls, [])

    def test_pass_needs_review_record(self):
        self.code_commit()
        code, err = self.layer2("PASS", reviewer="r1")
        self.assertEqual(code, 2)
        self.assertIn("no cheat-reviewer PASS by r1", err)

    def test_unsafe_notes_refused(self):
        code, err = self.layer2("FAIL", msg=self.msg(), notes="x; rm $HOME")
        self.assertEqual(code, 2)
        self.assertEqual(self.engine_calls, [])

    def test_later_fail_blocks_code_and_keeps_key(self):
        self.code_commit()
        self.review_pass("r1")
        key = cw.diff_key(self.repo, self.g("rev-parse", "HEAD"))
        self.assertEqual(self.layer2("PASS", reviewer="r1", h=H2)[0], 0)
        tip = self.g("rev-parse", "HEAD")
        self.assertEqual(cw.ledger_problems(cw.ledger_changes(self.repo, tip), True), [])
        self.assertEqual(self.layer2("FAIL", reviewer="r2", h=H2, name="v2.json")[0], 0)
        tip = self.g("rev-parse", "HEAD")
        self.assertEqual(self.g("rev-list", "--count", "main..HEAD"), "1")     # amended, not stacked
        self.assertEqual(cw.diff_key(self.repo, tip), key)
        self.assertEqual(cw.changed_layer2_bodies(self.repo, tip), {FUNC})
        p = cw.ledger_problems(cw.ledger_changes(self.repo, tip), True, {FUNC})
        self.assertTrue(p and "latest layer-2 record is FAIL" in p[0], p)

    def test_fail_on_code_commit_blocks_its_key(self):
        self.code_commit()
        self.review_pass("r1")
        self.assertEqual(self.layer2("FAIL", reviewer="r2", h=H2)[0], 0)
        tip = self.g("rev-parse", "HEAD")
        p = cw.review_gate(self.repo, self.ITEM, tip)[2]
        self.assertTrue(any("FAIL recorded on this exact diff" in x and "r2" in x for x in p), p)

    def test_old_pass_cannot_be_rerecorded_over_a_fail(self):
        self.code_commit()
        self.review_pass("r1")
        self.assertEqual(self.layer2("PASS", reviewer="r1", h=H2)[0], 0)
        self.assertEqual(self.layer2("FAIL", reviewer="r2", h=H2, name="v2.json")[0], 0)
        code, err = self.layer2("PASS", reviewer="r1", h=H2)            # the same verdict JSON
        self.assertEqual(code, 2)
        self.assertIn("already recorded", err)
        code, err = self.layer2("PASS", reviewer="r1", h=H2, name="v3.json")   # a new JSON, old review
        self.assertEqual(code, 2)
        self.assertIn("fresh cheat-reviewer", err)

    def test_fail_folded_on_unchanged_function_refused(self):
        self.code_commit(body=False)
        self.review_pass("r1")
        code, err = self.layer2("FAIL", reviewer="r2")
        self.assertEqual(code, 2)
        self.assertIn("does not change", err)
        self.assertEqual(self.engine_calls, [])

    def test_fail_uncompleting_a_finished_function_refused(self):
        (self.repo / "engine" / "queue.json").write_text('{"items": []}', encoding="utf-8", newline="\n")
        self.g("commit", "-q", "-am", "queue empty")
        self.g("branch", "-f", "main", "HEAD")
        code, err = self.layer2("FAIL", msg=self.msg())                         # the PASS'd body H1
        self.assertEqual(code, 2)
        self.assertIn("queue reopen", err)
        self.assertEqual(self.layer2("FAIL", msg=self.msg(), h=H2)[0], 0)      # another body: no completion

    def test_engine_command(self):
        calls = []
        with mock.patch.object(cw, "run_checked", lambda cmd, what, **k: calls.append(cmd)):
            REAL_RECORD(Path("M"), Path("T"), FUNC, "tmp/codex/v.json",
                        {"decision": "FAIL", "body_hash": H1}, "r2", "cheat-cleanup", "two words")
        cmd = calls[0]
        i = cmd.index("record")
        self.assertEqual(cmd[i - 1:], ["layer2", "record", FUNC, "--verdict-file", "tmp/codex/v.json",
                                       "--verdict", "FAIL", "--expect-hash", H1, "--reviewer", "r2",
                                       "--scope", "cheat-cleanup", "--notes", "two words"])


class LandGate(Repo):
    """cmd_land up to its build: the review + ledger gates (exit 9) and the engine `layer2 check`."""

    def land(self):
        checks = []
        a = SimpleNamespace(item=self.ITEM, dry_run=True, no_push=True)
        with mock.patch.dict(os.environ, {"CLAUDE_CODE_SESSION_ID": "test"}), \
                mock.patch.object(cw, "verify_tip", lambda *a, **k: None), \
                mock.patch.object(cw, "format_tree", lambda main, item, tree, paths: tree), \
                mock.patch.object(cw, "engine_layer2_check", lambda main, trusted, func: checks.append(func)), \
                redirect_stdout(io.StringIO()) as out, redirect_stderr(io.StringIO()):
            try:
                cw.cmd_land(a)
            except SystemExit as e:
                return e.code, checks, out.getvalue()
        return 0, checks, out.getvalue()

    def test_ledger_only_fail_lands(self):
        self.assertEqual(self.layer2("FAIL", msg=self.msg())[0], 0)
        code, checks, out = self.land()
        self.assertEqual(code, 0, out)
        self.assertIn("DRY RUN", out)
        self.assertEqual(checks, [])

    def test_code_with_pass_lands_after_engine_check(self):
        self.code_commit()
        self.review_pass("r1")
        self.assertEqual(self.layer2("PASS", reviewer="r1", h=H2)[0], 0)
        code, checks, out = self.land()
        self.assertEqual(code, 0, out)
        self.assertEqual(checks, [FUNC])

    def test_body_change_without_layer2_refused(self):
        self.code_commit()
        self.review_pass("r1")
        code, checks, out = self.land()
        self.assertEqual(code, 9, out)
        self.assertIn(f"{FUNC}: this commit changes its body", out)
        self.assertEqual(checks, [])

    def test_change_to_unreviewed_function_lands(self):
        self.code_commit(body=False)               # only f, which has no ledger
        self.review_pass("r1")
        code, checks, out = self.land()
        self.assertEqual(code, 0, out)
        self.assertEqual(checks, [])

    def test_asm_only_body_change_refused(self):
        (self.repo / "asm" / "funcs" / "asm_func.s").write_text("glabel asm_func\n jr $ra\n", encoding="utf-8",
                                                                newline="\n")
        self.g("commit", "-q", "-am", "asm")
        self.assertEqual(cw.changed_layer2_bodies(self.repo, self.g("rev-parse", "HEAD")), {"asm_func"})
        self.review_pass("r1")
        code, checks, out = self.land()
        self.assertEqual(code, 9, out)
        self.assertIn("asm_func: this commit changes its body", out)

    def test_needs_user_then_owner_ruling_and_fresh_pass_lands(self):
        self.code_commit()
        self.review_pass("r1")
        self.assertEqual(self.layer2("NEEDS_USER", reviewer="r2", h=H2)[0], 0)
        tip = self.g("rev-parse", "HEAD")
        self.assertTrue(cw.review_gate(self.repo, self.ITEM, tip)[2])          # blocked until a ruling
        self.review_pass("r3", later=-3600)                                     # clock skew: position wins
        self.assertEqual(cw.review_gate(self.repo, self.ITEM, tip)[2], [])
        self.assertEqual(self.layer2("PASS", reviewer="r3", h=H2, name="v3.json")[0], 0)
        code, checks, out = self.land()
        self.assertEqual(code, 0, out)
        self.assertEqual(checks, [FUNC])

    def test_code_with_later_fail_refused(self):
        self.code_commit()
        self.review_pass("r1")
        self.assertEqual(self.layer2("PASS", reviewer="r1", h=H2)[0], 0)
        self.assertEqual(self.layer2("FAIL", reviewer="r2", h=H2, name="v2.json")[0], 0)
        code, checks, out = self.land()
        self.assertEqual(code, 9, out)
        self.assertIn("latest layer-2 record is FAIL", out)
        self.assertEqual(checks, [])


class LandMovingMain(unittest.TestCase):
    """Mock commands, but compute tree deltas from independent file-content snapshots."""

    def land(self, moved=("docs/peer.md",), item=("src/item.c",), moves=1,
             extra=(), held=False, dry_run=False, failure=None, ancestor=True,
             changed_item=None, initial_behind=False, rebase_race=False,
             override=None, already=False):
        events = []
        state = {"main": "m0", "base": "old" if initial_behind else "m0",
                 "tip": "t0", "acquires": 0, "revision": 0, "rebases": 0,
                 "merges": 0}
        trees = {"old": {}, "m0": {}}
        trees["t0"] = {p: "item" for p in item}
        main, scratch, trusted = Path("main"), Path("scratch"), Path("trusted")

        def advance():
            state["revision"] += 1
            self.assertLess(state["revision"], 12, "land failed to bound retries")
            tree = dict(trees[state["main"]])
            for path in moved:
                tree[path] = "main revision " + str(state["revision"])
            state["main"] = "m" + str(state["revision"])
            trees[state["main"]] = tree

        def git(*args, **kwargs):
            if args == ("rev-parse", "codex/item"):
                return state["tip"]
            if args == ("rev-parse", "main"):
                return state["main"]
            if args == ("rev-parse", "HEAD"):
                return state["tip"] if kwargs["cwd"] == trusted else state["main"]
            if args == ("status", "--porcelain"):
                events.append("clean:" + state["tip"])
                return " M dirty" if failure == "dirty" and state["rebases"] else ""
            if args[0] == "rev-parse" and args[1].endswith("^{tree}"):
                return "tree-" + args[1].split("^")[0]
            if args[0] == "symbolic-ref":
                return "main"
            if args[0] == "diff":
                left, right = (trees[p.removeprefix("tree-")] for p in args[-2:])
                return "\0".join(sorted(p for p in left.keys() | right.keys()
                                        if left.get(p) != right.get(p)))
            if args[0] == "log":
                return "item commit"
            self.fail(f"unexpected git call: {args}")

        def run(cmd, **kwargs):
            if cmd[1] == "merge-base":
                return SimpleNamespace(returncode=int((cmd[-1] == "main" and not already) or not ancestor))
            events.append(cmd[1])
            if cmd[1] == "rebase":
                state["rebases"] += 1
                if rebase_race and state["rebases"] == 1:
                    advance()
                state["base"] = state["main"]
                state["tip"] = "t" + str(state["revision"])
                tree = dict(trees[state["base"]])
                tree.update({p: "item" for p in item})
                tree.update({p: "unexpected" for p in extra})
                trees[state["tip"]] = tree
            if cmd[1] == "merge":
                state["merges"] += 1
                if failure == "merge":
                    return SimpleNamespace(returncode=1, stdout="", stderr="failed")
                if failure == "locked-merge-move" or (failure == "merge-move" and state["merges"] == 1):
                    advance()
                    return SimpleNamespace(returncode=1, stdout="", stderr="main moved")
                state["main"] = state["tip"]
            return SimpleNamespace(returncode=0, stdout="", stderr="")

        def lock(_main, action, _item):
            events.append(action)
            if action == "acquire":
                state["acquires"] += 1
                # A peer lands just before acquisition succeeds, never while we hold it.
                if state["acquires"] <= moves:
                    advance()
            return True, "(yours)" if held else ""

        def build(*args):
            events.append("build")
            if failure == "first-verify" or (failure == "verify" and state["acquires"] >= 4):
                raise SystemExit(2)

        def checked(cmd, what, **kwargs):
            events.append(what)
            if failure == "locked-gate-move" and what == "completion-integrity audit":
                advance()
            if failure == "audit" and state["rebases"] and what == "completion-integrity audit":
                raise SystemExit(2)

        def review(*args):
            events.append("review")
            paths = changed_item if changed_item is not None and state["rebases"] else item
            return ["code-reviewer"], list(paths), []

        def write_marker(path, *args, **kwargs):
            events.append("marker:" + path.name)

        patches = {
            "layout": lambda _: (main, scratch, trusted, "codex/item"),
            "pin_file": lambda _: SimpleNamespace(exists=lambda: False),
            "verify_trusted": lambda *a: None,
            "scratch_committed": lambda *a: None,
            "reset_trusted": lambda *a: None,
            "single_commit": lambda *a: (state["base"], state["tip"]),
            "codex_idle": lambda *a: None,
            "after_rebase": lambda *a: None,
            "review_gate": review,
            "ledger_changes": lambda *a: {FUNC: {}},
            "changed_layer2_bodies": lambda *a: {FUNC},
            "ledger_problems": lambda *a: [],
            "verdict_copies": lambda *a: {},
            "copy_problems": lambda *a: [],
            "format_tree": lambda m, i, tree, p: tree,
            "diff_key": lambda *a: "key",
            "verify_build": build,
            "run_checked": checked,
            "engine_layer2_check": lambda *a: events.append("layer2"),
            "lock": lock,
            "peer_paths": lambda *a: set(),
            "save_state": lambda *a, **k: events.append("landed"),
            "push": lambda *a: events.append("push"),
            "load_state": lambda *a: {"landed": "t0"},
            "git": git,
        }
        with ExitStack() as stack:
            stack.enter_context(mock.patch.dict(os.environ, {"CLAUDE_CODE_SESSION_ID": "test"}))
            for key in ("PROLOGUE_CONFIG", "DELAY_SLOT_RA_FUNCS", "FRAME_FIX_FUNCS"):
                stack.enter_context(mock.patch.dict(os.environ, {key: "redirected" if key == override else ""}))
            for name, replacement in patches.items():
                stack.enter_context(mock.patch.object(cw, name, replacement))
            stack.enter_context(mock.patch.object(cw.subprocess, "run", side_effect=run))
            stack.enter_context(mock.patch.object(Path, "exists", return_value=False))
            stack.enter_context(mock.patch.object(Path, "mkdir"))
            stack.enter_context(mock.patch.object(Path, "write_text", autospec=True, side_effect=write_marker))
            output = stack.enter_context(redirect_stdout(io.StringIO()))
            stack.enter_context(redirect_stderr(io.StringIO()))
            code = 0
            try:
                cw.cmd_land(SimpleNamespace(item="item", dry_run=dry_run, no_push=False))
            except SystemExit as e:
                code = e.code
            self.last_output = output.getvalue()
        return code, events

    def assert_locked_rebase(self, events, acquire_number=2):
        acquire = [i for i, e in enumerate(events) if e == "acquire"][acquire_number - 1]
        rebase = next(i for i in range(acquire, len(events)) if events[i] == "rebase")
        self.assertLess(acquire, rebase)
        self.assertNotIn("release", events[acquire:events.index("push")])
        self.assertEqual(events[-1], "release")

    def test_docs_move_reuses_build_but_reruns_gates(self):
        code, events = self.land()
        self.assertEqual(code, 0)
        self.assertEqual(events.count("build"), 1)
        self.assertEqual(events.count("completion-integrity audit"), 2)
        self.assertEqual(events.count("layer2"), 2)
        self.assertEqual(events.count("review"), 2)
        self.assertEqual(events.count("commit"), 2)
        self.assertEqual([e for e in events if e.startswith("marker:")], ["marker:t0.ok"])
        self.assertIn("clean:t1", events)
        self.assert_locked_rebase(events)

    def test_naming_census_move_runs_suite_audit_layer2_without_build_or_marker(self):
        code, events = self.land(moved=("docs/naming/census_manifests.py",), item=("docs/item.md",))
        self.assertEqual(code, 0)
        self.assertEqual(events.count("build"), 1)
        self.assertEqual(events.count("completion-integrity audit"), 2)
        self.assertEqual(events.count("engine test suite"), 1)
        self.assertEqual(events.count("layer2"), 2)
        self.assertEqual([e for e in events if e.startswith("marker:")], ["marker:t0.ok"])
        self.assert_locked_rebase(events)

    def test_quick_suite_triggers(self):
        for path in ("tools/trace/peer.PY", "DOCS/NAMING/function-names.csv"):
            with self.subTest(path=path):
                code, events = self.land(moved=(path,), item=("docs/item.md",))
                self.assertEqual(code, 0)
                self.assertEqual(events.count("build"), 1)
                self.assertEqual(events.count("engine test suite"), 1)

    def test_item_paths_still_trigger_suite_on_quick_path(self):
        code, events = self.land(item=("tools/item.py",))
        self.assertEqual(code, 0)
        self.assertEqual(events.count("engine test suite"), 2)

    def test_shared_docs_path_requires_full_verify_case_insensitively(self):
        code, events = self.land(moved=("DOCS/peer.md",), item=("docs/PEER.md",))
        self.assertEqual(code, 0)
        self.assertEqual(events.count("build"), 2)

    def test_build_read_and_other_nonexempt_paths_require_full_verify(self):
        for path in ("src/peer.c", "include/game.h", "asm/peer.s", "Makefile",
                     "tools/prologue_config.json", "tools/maspsx/maspsx.py", "bb2.ld",
                     "named_syms.txt", "engine/buildstamp.py", "memory/grind/f/layer2.jsonl"):
            with self.subTest(path=path):
                code, events = self.land(moved=(path,))
                self.assertEqual(code, 0)
                self.assertEqual(events.count("build"), 2)
                self.assertEqual(events.count("layer2"), 2)

    def test_unexpected_tree_delta_requires_full_verify_before_merge(self):
        code, events = self.land(extra=("src/unexpected.c",))
        self.assertEqual(code, 0)
        self.assertEqual(events.count("build"), 2)
        self.assertLess(max(i for i, e in enumerate(events) if e == "build"), events.index("merge"))

    def test_missing_main_delta_requires_full_verify(self):
        # Same-case item write masks main's changed path: the independently computed
        # tip-tree delta is empty, even if a bug bypasses the disjointness check.
        real = cw.land_quick_paths
        def bypass_overlap(main, verified, base):
            if verified:
                verified = (*verified[:3], set())
            return real(main, verified, base)
        with mock.patch.object(cw, "land_quick_paths", side_effect=bypass_overlap):
            code, events = self.land(moved=("docs/peer.md",), item=("docs/peer.md",))
        self.assertEqual(code, 0)
        self.assertEqual(events.count("build"), 2)

    def test_item_path_set_change_requires_full_verify(self):
        code, events = self.land(changed_item=("src/different.c",))
        self.assertEqual(code, 0)
        self.assertEqual(events.count("build"), 2)

    def test_rewound_main_requires_full_verify(self):
        code, events = self.land(ancestor=False)
        self.assertEqual(code, 0)
        self.assertEqual(events.count("build"), 2)

    def test_two_full_reverifies_lock_next_attempt_through_push(self):
        code, events = self.land(moved=("src/peer.c",), moves=3)
        self.assertEqual(code, 0)
        self.assertEqual(events.count("build"), 4)
        acquires = [i for i, e in enumerate(events) if e == "acquire"]
        rebases = [i for i, e in enumerate(events) if e == "rebase"]
        # First two re-verifies run unlocked; the next attempt locks before rebase.
        self.assertLess(rebases[1], acquires[2])
        self.assertLess(acquires[3], rebases[2])
        self.assert_locked_rebase(events, acquire_number=4)

    def test_stream_of_quick_moves_finishes_under_lock(self):
        code, events = self.land(moves=20)
        self.assertEqual(code, 0)
        self.assertEqual(events.count("acquire"), 2)
        self.assertEqual(events.count("build"), 1)
        self.assert_locked_rebase(events)

    def test_locked_attempt_can_finish_after_unlocked_budget(self):
        # Shrink the unlocked budget so the quick retry crosses it deterministically.
        with mock.patch.object(cw, "LAND_UNLOCKED_ATTEMPTS", 1):
            code, events = self.land()
        self.assertEqual(code, 0)
        self.assertIn("landed", events)
        self.assert_locked_rebase(events)

    def test_move_between_read_and_rebase_uses_actual_base(self):
        code, events = self.land(initial_behind=True, rebase_race=True, moves=0)
        self.assertEqual(code, 0)
        self.assertEqual(events.count("build"), 1)
        self.assertEqual(events.count("acquire"), 1)
        self.assertEqual(events.count("rebase"), 1)

    def test_preheld_lock_is_never_released(self):
        for moved, moves in ((("docs/peer.md",), 1), (("src/peer.c",), 3)):
            with self.subTest(moved=moved):
                code, events = self.land(moved=moved, moves=moves, held=True)
                self.assertEqual(code, 0)
                self.assertNotIn("release", events)
                self.assertIn("landed", events)

    def test_early_lock_released_on_verify_refusal(self):
        code, events = self.land(moved=("src/peer.c",), moves=3, failure="verify")
        self.assertEqual(code, 2)
        self.assertEqual(events[-1], "release")
        self.assertNotIn("merge", events)

    def test_quick_tip_cleanliness_is_checked_after_hook(self):
        code, events = self.land(failure="dirty")
        self.assertEqual(code, 2)
        # Refuse before executing quick gates, not merely at their final dirty check.
        self.assertEqual(events.count("completion-integrity audit"), 1)
        self.assertEqual(events[-1], "release")
        self.assertNotIn("merge", events)

    def test_quick_audit_failure_refuses_and_releases_lock(self):
        code, events = self.land(failure="audit")
        self.assertEqual(code, 2)
        self.assertNotIn("merge", events)
        self.assertEqual(events[-1], "release")

    def test_merge_failure_without_main_move_refuses(self):
        code, events = self.land(moves=0, failure="merge")
        self.assertEqual(code, 2)
        self.assertEqual(events.count("merge"), 1)
        self.assertNotIn("landed", events)
        self.assertEqual(events[-1], "release")

    def test_main_move_during_merge_retries_and_runs_quick_gates(self):
        code, events = self.land(moves=0, failure="merge-move")
        self.assertEqual(code, 0)
        self.assertEqual(events.count("merge"), 2)
        self.assertEqual(events.count("build"), 1)
        self.assertEqual(events.count("completion-integrity audit"), 2)
        self.assertEqual(events.count("layer2"), 2)
        # The failing merge happened before this retry's locked rebase.
        acquires = [i for i, e in enumerate(events) if e == "acquire"]
        self.assertLess(events.index("merge"), acquires[1])
        self.assert_locked_rebase(events)

    def test_dry_run_never_takes_lock(self):
        code, events = self.land(initial_behind=True, dry_run=True)
        self.assertEqual(code, 0)
        self.assertIn("rebase", events)
        self.assertIn("build", events)
        self.assertNotIn("acquire", events)
        self.assertNotIn("merge", events)

    def test_pipeline_environment_overrides_disable_build_reuse(self):
        for key in ("PROLOGUE_CONFIG", "DELAY_SLOT_RA_FUNCS", "FRAME_FIX_FUNCS"):
            with self.subTest(key=key):
                code, events = self.land(override=key)
                self.assertEqual(code, 0)
                self.assertEqual(events.count("build"), 2)

    def test_refusal_releases_only_a_lock_acquired_here(self):
        for held, failure in ((True, "audit"), (False, "first-verify")):
            with self.subTest(held=held, failure=failure):
                code, events = self.land(held=held, failure=failure)
                self.assertEqual(code, 2)
                self.assertEqual("acquire" in events, held)
                self.assertNotIn("release", events)
                self.assertNotIn("push", events)

    def test_locked_retries_are_bounded_even_when_writer_ignores_lock(self):
        for failure in ("locked-gate-move", "locked-merge-move"):
            for held in (False, True):
                with self.subTest(failure=failure, held=held):
                    code, events = self.land(moves=0, held=held, failure=failure)
                    self.assertEqual(code, 2)
                    self.assertIn("main kept moving", self.last_output)
                    self.assertIn("reintegration lock", self.last_output)
                    self.assertIn("3 locked attempts", self.last_output)
                    self.assertEqual(events.count("acquire"), 5)  # one unlocked, three locked, refusal
                    self.assertEqual(events.count("release"), 0 if held else 5)
                    self.assertNotIn("push", events)
                    self.assertNotIn("landed", events)

    def test_already_landed_dry_run_never_locks_or_pushes(self):
        code, events = self.land(already=True, dry_run=True)
        self.assertEqual(code, 0)
        self.assertIn("DRY RUN", self.last_output)
        self.assertIn("would push main to origin", self.last_output)
        for event in ("status", "acquire", "release", "push", "merge", "build"):
            self.assertNotIn(event, events)

    def test_odd_case_source_path_runs_engine_suite(self):
        code, events = self.land(item=("SRC/item.c",))
        self.assertEqual(code, 0)
        self.assertEqual(events.count("engine test suite"), 2)

    def test_build_inert_prefixes_exclude_buildstamp_inputs(self):
        from engine import buildstamp

        # Importing buildstamp/buildconfig only defines data/functions. Exercise the
        # actual input discovery, but avoid hashing sources, tools and the runtime.
        directories = set()
        real_rglob = Path.rglob
        def rglob(directory, pattern):
            directories.add(directory.as_posix().casefold().rstrip("/") + "/")
            return real_rglob(directory, pattern)
        with mock.patch.object(buildstamp, "digest", return_value="digest"), \
                mock.patch.object(buildstamp.shutil, "which", return_value=None), \
                mock.patch.object(Path, "rglob", rglob):
            paths = set(buildstamp.CONFIGS) | set(buildstamp.inputs())
        for path in paths:
            with self.subTest(path=path):
                self.assertFalse(path.casefold().startswith(cw.BUILD_INERT_PREFIXES), path)
        # Pin glob roots too, even when a future input directory is empty today.
        for directory in directories:
            for prefix in cw.BUILD_INERT_PREFIXES:
                self.assertFalse(directory.startswith(prefix) or prefix.startswith(directory), directory)

    def test_disjointness_helper_casefolds_both_sides(self):
        verified = ("m0", "t0", "tree-t0", {"docs/PEER.md"})
        with mock.patch.object(cw.subprocess, "run", return_value=SimpleNamespace(returncode=0)), \
                mock.patch.object(cw, "git", return_value="DOCS/peer.md"):
            self.assertIsNone(cw.land_quick_paths(Path("main"), verified, "m1"))


if __name__ == "__main__":
    unittest.main()
