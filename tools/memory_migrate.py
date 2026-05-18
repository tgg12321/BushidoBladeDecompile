#!/usr/bin/env python3
"""Stage 5 — memory/ restructure migration.

Reads the current memory/ directory, applies the Stage 2 mapping (kebab-case
filenames, directory taxonomy, dropped feedback_/project_ prefixes), splits
workflow_rules.md into atomic rule files, pulls in pre-staged consolidations
from tmp/_memory_staging/, applies refresh-diff patches, and rewrites all
[[link]] references using a global slug remap.

Output goes to tmp/_memory_new/. Original memory/ is NOT touched until the
user reviews the output and runs the atomic swap.
"""
from __future__ import annotations

import re
import shutil
import sys
from collections import defaultdict
from pathlib import Path
from typing import Any

MEMORY_DIR = Path.home() / ".claude" / "projects" / "C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile" / "memory"
REPO_ROOT = Path(__file__).resolve().parent.parent
STAGING_DIR = REPO_ROOT / "tmp" / "_memory_staging"
NEW_DIR = REPO_ROOT / "tmp" / "_memory_new"

# ---------------------------------------------------------------------------
# Mapping data — Stage 2 output
# ---------------------------------------------------------------------------

# Files where Stage 3/4 already produced the canonical content. Copy verbatim.
STAGED_OVERRIDES = {
    "recipes/voice-control-playbook.md":           "recipes/voice-control-playbook.md",
    "recipes/exec-game-plateau.md":                "recipes/exec-game-plateau.md",
    "naming/kengo-name-unreliable.md":             "naming/kengo-name-unreliable.md",
    "history/2026-05-17-cleanup-and-cheats.md":    "history/2026-05-17-cleanup-and-cheats.md",
    "project/status-and-attack.md":                "project/status-and-attack.md",
    "user/role.md":                                "user/role.md",
}

# Source filename -> new path. Mostly content-preserving moves (rename + dir).
# A handful get small refresh-diff patches applied (see REFRESH_DIFFS below).
SIMPLE_MOVES = {
    # rules/ - durable behavior
    "feedback_voluntary_stop_forbidden.md":        "rules/no-voluntary-stops.md",
    "feedback_only_impossible_stops.md":           "rules/only-impossible-stops.md",
    "feedback_no_stopping_framing.md":             "rules/no-stopping-framing.md",
    "feedback_bridge_is_not_decomp.md":            "rules/bridge-is-not-decomp.md",
    "feedback_hand_coded_asm_recognition.md":      "rules/hand-coded-asm-recognition.md",
    "feedback_evidence_driven_authorization.md":   "rules/evidence-driven-authorization.md",
    "feedback_canonical_asm_retirement.md":        "rules/canonical-asm-retirement.md",
    "feedback_minimize_regfix.md":                 "rules/minimize-regfix.md",
    "feedback_minimize_asm_when_blocked.md":       "rules/minimize-asm-when-blocked.md",
    "feedback_community_standard_inline_asm.md":   "rules/community-standard.md",
    "feedback_cu_split_not_pure_c.md":             "rules/cu-split-evidence-required.md",
    "feedback_cc1psx_not_a_build_target.md":       "rules/cc1psx-calibration-only.md",

    # workflow/ - harness mechanics
    "feedback_parallel_worktree_work.md":          "workflow/parallel-worktrees.md",
    "feedback_naming_work_in_worktree.md":         "workflow/naming-worktree.md",
    "feedback_parent_cd_after_subagent.md":        "workflow/parent-cd-after-subagent.md",
    "feedback_parallel_harness_gotchas.md":        "workflow/parallel-harness-gotchas.md",
    "feedback_retire_auxiliary_asmfix.md":         "workflow/retire-auxiliary-asmfix.md",
    "feedback_auto_drift_repair.md":               "workflow/auto-drift-repair.md",
    "feedback_active_marker_quirks.md":            "workflow/active-marker-quirks.md",
    "feedback_crlf_via_edit_write.md":             "workflow/crlf-via-edit-write.md",
    "feedback_cheat_cleanup_parallel_orchestration.md": "workflow/cheat-cleanup-orchestration.md",

    # reference/ - deep technique reference
    "feedback_matching_playbook.md":               "reference/matching-playbook.md",
    "feedback_quick_reference.md":                 "reference/quick-reference.md",
    "feedback_regfix_reference.md":                "reference/regfix-reference.md",
    "feedback_regfix_subst_multi_and_splice.md":   "reference/regfix-subst-multi-splice.md",
    "feedback_register_asm_pin_reliability.md":    "reference/register-asm-pins.md",
    "feedback_inline_move_aliasing.md":            "reference/inline-move-aliasing.md",
    "feedback_store_before_jal.md":                "reference/store-before-jal.md",
    "feedback_strength_reduce_defeat.md":          "reference/strength-reduce-defeat.md",
    "feedback_dead_branch_scheduling.md":          "recipes/dead-branch-scheduling.md",
    "feedback_dead_vars_local_array_idiom.md":     "reference/dead-vars-local-array.md",
    "feedback_maspsx_set_noreorder_stripping.md":  "reference/maspsx-noreorder-stripping.md",
    "feedback_cc1_first_pass_scheduler_bug.md":    "reference/cc1-first-pass-scheduler-bug.md",
    "feedback_cc1psx_available.md":                "reference/cc1psx-calibration.md",
    "reference_decomp_projects.md":                "reference/decomp-projects.md",
    "reference_new_search_tools.md":               "reference/search-tools.md",

    # recipes/ - named playbooks
    "feedback_retirement_recipes.md":              "recipes/retirement-recipes.md",
    "feedback_gte_3x3_recipe.md":                  "recipes/gte-3x3.md",
    "feedback_scratchpad_gte_recipe.md":           "recipes/scratchpad-gte.md",
    "feedback_shared_end_label_recipe.md":         "recipes/shared-end-label.md",
    "feedback_cheat_cleanup_techniques.md":        "recipes/cheat-cleanup-techniques.md",
    "feedback_cu_split_asmfix_drift.md":           "recipes/cu-split-asmfix-drift.md",
    "feedback_satan2_kabuto_s2_pin.md":            "recipes/satan2-kabuto-pin.md",

    # audit/ - cheat detection + authorization
    "feedback_audit_pattern_observations.md":      "audit/pattern-observations.md",
    "feedback_lost_codegen_insert_cheat.md":       "audit/lost-codegen-insert-cheat.md",
    "feedback_inline_asm_lost_codegen_injection.md": "audit/inline-asm-injection.md",
    "feedback_bridge_signature_audit.md":          "audit/bridge-signature.md",

    # naming/
    "feedback_naming_bulk_promotion.md":           "naming/bulk-promotion.md",
    "feedback_naming_placeholder_refinement.md":   "naming/placeholder-refinement.md",
    "feedback_raw_dx_promotion_unsafe.md":         "naming/raw-dx-unsafe.md",

    # project/ - current state
    "project_inline_asm_offenders.md":             "project/inline-asm-offenders.md",
    "project_slog_kengo_dead_end.md":              "project/slog-kengo-dead-end.md",
    "project_satan0main_midi_dispatch.md":         "project/satan0main-midi-dispatch.md",
    "project_build_and_internals.md":              "project/build-and-internals.md",
    "project_tools_reference.md":                  "reference/tools.md",
    "project_splat_upgrade_step1_partial.md":      "project/splat-upgrade-attempt.md",

    # history/ - archived
    "feedback_dead_vars_padding_technique.md":     "history/dead-vars-padding-research.md",
}

# Files that get consolidated and the originals deleted (no destination).
DELETIONS_CONSUMED = {
    "feedback_voice_contorol_intractable.md",
    "feedback_voice_contorol_cc1psx_calibration_2026_05_17.md",
    "feedback_voice_contorol_liberal_barriers_2026_05_17.md",
    "feedback_voice_contorol_session_2026_05_17_pure_c.md",
    "feedback_exec_game_lessons.md",
    "feedback_exec_game_plateau.md",
    "feedback_kengo_name_match_unreliable.md",
    "feedback_kengo_name_unique_mass_rename_unreliable.md",
    "feedback_session_2026_05_17_autonomous_attempts.md",
    "feedback_session_2026_05_17_cheat_cleanup_batch.md",
    "feedback_workflow_rules.md",  # split into atomic
    "MEMORY.md",  # rewritten in Stage 6
}

# workflow_rules.md atomic breakup. Each entry: dest path, frontmatter name,
# description, and the H2 section heading(s) to extract. Sections not listed
# here get dropped (covered by existing focused files like voluntary-stop).
WORKFLOW_SPLITS = [
    {
        "dest": "rules/hard-rule.md",
        "name": "hard-rule",
        "description": "Once a function is selected, work it end-to-end to 100% pure-C match. No tabling, no skipping, no quick-win hunting. Hard blockers (auto-detected by dc.sh classify) are the only exits.",
        "sections": [
            "THE HARD RULE — finish every function you start",
            "The only valid stopping points",
            "Working mode",
        ],
    },
    {
        "dest": "rules/queue-selection.md",
        "name": "queue-selection",
        "description": "Pull from WORK_QUEUE.md top via dc.sh next. No searching src/ for easier targets, no re-classifying queue rows, no skipping entries that look hard. The active-marker hook enforces this — commits and next-pulls are blocked while a function is in progress.",
        "sections": [
            "Selecting the next function — pull from `WORK_QUEUE.md` (HOOK-ENFORCED)",
        ],
    },
    {
        "dest": "rules/escalation-ladder.md",
        "name": "escalation-ladder",
        "description": "When stuck, switch technique — not target, not direction. Eight-rung ladder: C variants → register pins → permuter → regfix → compound regfix → named recipes → new transformation pass → new named recipes.",
        "sections": [
            "Escalation ladder when stuck (in order)",
        ],
    },
    {
        "dest": "rules/inline-asm-allowed.md",
        "name": "inline-asm-allowed",
        "description": "Narrow exceptions where inline __asm__ in src/ is acceptable: GTE coprocessor ops, BIOS trampolines, compiler hint barriers. Anything else means the function is not done.",
        "sections": [
            "Inline asm rules",
        ],
    },
    {
        "dest": "rules/communication.md",
        "name": "communication",
        "description": "DO NOT ASK FOR DIRECTION while a function is in progress. Silent until matched; brief one-line summary on match. Only stop for impossibility, not difficulty.",
        "sections": [
            "Communication — DO NOT ASK FOR DIRECTION",
        ],
    },
    {
        "dest": "rules/pre-dive-analysis.md",
        "name": "pre-dive-analysis",
        "description": "Before writing any C, run dc.sh classify + agent-brief + read asm + check Kengo + read siblings. Form an explicit hypothesis. ~1-2 tool calls, mandatory.",
        "sections": [
            "Pre-dive analysis (MANDATORY)",
        ],
    },
    {
        "dest": "rules/programmatic-tools-first.md",
        "name": "programmatic-tools-first",
        "description": "If a task involves examining multiple files or applying a pattern, write or use a tool. Manual reasoning is for novel one-off analysis or judgment calls only.",
        "sections": [
            "Programmatic tools first",
        ],
    },
    {
        "dest": "rules/integration-discipline.md",
        "name": "integration-discipline",
        "description": "Never use dc.sh replace for final integration (copies typedefs/externs). Use dc.sh inline-replace or write the function body directly via WSL python3. After match: build-active + verify-c + verify --all + verify_labels + commit. Catches label drift in siblings.",
        "sections": [
            "Integration discipline",
            "After every match",
        ],
    },
    {
        "dest": "rules/post-match-retro.md",
        "name": "post-match-retro",
        "description": "After committing a match, briefly assess if anything is worth retaining for future agents. Document conservatively (technique + ≥1 other applicable function + non-obvious gotcha). Anti-noise rules apply.",
        "sections": [
            "Post-match retrospective",
        ],
    },
    {
        "dest": "rules/tool-error-handling.md",
        "name": "tool-error-handling",
        "description": "Never swallow stderr in tools/. Pipelines use set -eo pipefail. Validate output artifacts (.o has non-empty .text). No bare except: pass. Run fix_gte_asm.py before assembly.",
        "sections": [
            "Tool error handling rules",
        ],
    },
    {
        "dest": "rules/debugging-discipline.md",
        "name": "debugging-discipline",
        "description": "When tools fail: diagnose root cause, fix permanently, commit the fix, update rules if needed. Every error should happen once. Read the actual artifact before guessing.",
        "sections": [
            "Debugging discipline",
        ],
    },
    {
        "dest": "workflow/file-editing-wsl.md",
        "name": "file-editing-wsl",
        "description": "Build files (src/*.c, *.txt in pipeline) MUST be edited via WSL to avoid CRLF — Windows Edit/Write silently breaks the toolchain. Non-build files (memory/, CLAUDE.md, tools/*.py) are fine via native Edit/Write.",
        "sections": [
            "File editing rules",
            "WSL execution discipline",
        ],
    },
    {
        "dest": "workflow/queue-maintenance.md",
        "name": "queue-maintenance",
        "description": "WORK_QUEUE.md is regenerated from tmp/batch_attempt.csv by dc.sh refresh-queue (~2 min). Run after every match. If a queue row looks wrong, extend classify_func.py or add to known_blocked.txt — don't skip silently.",
        "sections": [
            "Queue / classifier maintenance",
        ],
    },
    {
        "dest": "workflow/pipeline-cache-busting.md",
        "name": "pipeline-cache-busting",
        "description": "When a pipeline config change (regfix.txt, sdata_funcs.txt, expand_lb_funcs.txt, asmfix.txt) isn't reflected in build output: rm -f build/src/<file>.o && make. Don't debug the config file — it's the stale .o.",
        "sections": [
            "Pipeline cache busting",
        ],
    },
    {
        "dest": "workflow/scratch-file-policy.md",
        "name": "scratch-file-policy",
        "description": "All temp scripts go in tmp/ only — never _tmp_* in project root. Overwrite instead of version-suffixing (no _v2, _v3). diff_settings.py is the one root exception (asm-differ requires it there).",
        "sections": [
            "Scratch file policy",
        ],
    },
    {
        "dest": "reference/token-efficiency.md",
        "name": "token-efficiency",
        "description": "Tool-use efficiency rules: Read with limits, don't re-read files, tail -5 on make / tail -20 on permuter, parallel reads in one message, try_match.sh replaces compile+score+diff.",
        "sections": [
            "Token efficiency",
        ],
    },
    {
        "dest": "reference/tool-catalog.md",
        "name": "tool-catalog",
        "description": "Approved tool list (Read/Grep/Glob/Edit native, WSL python3 for build files) + path rules (Git Bash uses /c/..., WSL uses /mnt/c/...).",
        "sections": [
            "Approved tool list",
        ],
    },
    {
        "dest": "reference/scoring-systems.md",
        "name": "scoring-systems",
        "description": "Two distinct scoring systems — permuter score (weighted penalty: regs×5, reorderings×60, ins/del×100) vs objdump score (raw differing instruction count). Same code: 13 by objdump = 85 by permuter. Never compare across.",
        "sections": [
            "Two scoring systems — never compare across",
        ],
    },
    {
        "dest": "naming/kengo-cross-reference.md",
        "name": "kengo-cross-reference",
        "description": "Workflow for using Kengo (PS2 sibling) as a decomp reference. Run kengo_ref.py first for named stubs; verify by call structure / constants / control flow before trusting. False renames: ignore reference, decomp from BB2 asm alone — but still finish the BB2 function.",
        "sections": [
            "Kengo reference workflow",
        ],
    },
    {
        "dest": "reference/m2c-usage.md",
        "name": "m2c-usage",
        "description": "Always use m2c --valid-syntax with #include m2c_macros.h. Without: 6.4% compile rate. With: 66.1%. auto_decompile.py already uses these.",
        "sections": [
            "m2c usage",
        ],
    },
]

# Sections of workflow_rules.md to drop entirely (covered by existing focused files).
DROPPED_SECTIONS = set()  # All sections handled above; nothing dropped explicitly.

# Refresh-diff patches — find/replace per source file.
REFRESH_DIFFS = {
    "feedback_minimize_asm_when_blocked.md": [
        ("commit 819f3d1", "commit d33ea6b"),
    ],
    "feedback_cu_split_not_pure_c.md": [
        # Append voice-resolution note (see APPEND_PATCHES)
    ],
    "feedback_cc1psx_not_a_build_target.md": [
        # Append voice-resolution note (see APPEND_PATCHES)
    ],
    "feedback_maspsx_set_noreorder_stripping.md": [
        # [[inline-asm-canonical]] is not a memory file — it's a project file ref
        ("the\ncanonical-asm form in [[inline-asm-canonical]])",
         "the\ncanonical-asm form recorded in `inline_asm_canonical.txt`)"),
    ],
    "feedback_parallel_worktree_work.md": [
        # Forward-reference to a non-existent memory file
        (" — see [[parallel-worktree-naming-research]] if separate notes exist", ""),
    ],
}

# Per-file append patches (added at end of body, before any trailing newline).
APPEND_PATCHES = {
    "feedback_cu_split_not_pure_c.md":
        "\n\n## Update 2026-05-18\n\n"
        "Voice eventually landed via the [[minimize-asm-when-blocked]] playbook "
        "(3 §6.1 barriers + 5 specific non-wildcard regfix rules, commit `d33ea6b`). "
        "The CU-split refusal was correct — barriers + specific rules turned out to "
        "be the right path, not a workaround-split.\n",
    "feedback_cc1psx_not_a_build_target.md":
        "\n\n## Update 2026-05-18\n\n"
        "Voice ultimately landed via [[minimize-asm-when-blocked]] (commit `d33ea6b`) "
        "without changing the build target. cc1psx calibration was load-bearing for "
        "confirming the toolchain limit, but the build stays on decompals.\n",
}

# Section heading -> H2 line regex (matches `## <name>` exactly).
def section_re(name: str) -> re.Pattern:
    escaped = re.escape(name)
    return re.compile(rf"^## {escaped}\s*$", re.MULTILINE)


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

FRONTMATTER_RE = re.compile(r"^---\n(.*?)\n---\n", re.DOTALL)
LINK_RE = re.compile(r"\[\[([a-zA-Z0-9_\-]+)\]\]")


def parse_frontmatter(text: str) -> tuple[dict[str, Any], str]:
    m = FRONTMATTER_RE.match(text)
    if not m:
        return {}, text
    fm_text = m.group(1)
    body = text[m.end():]
    fm: dict[str, Any] = {}
    for raw in fm_text.splitlines():
        if not raw.strip() or raw.lstrip().startswith("#"):
            continue
        stripped = raw.strip()
        if ":" in stripped and not stripped.startswith("- "):
            key, _, val = stripped.partition(":")
            key = key.strip()
            val = val.strip()
            if val.startswith('"') and val.endswith('"'):
                val = val[1:-1]
            fm[key] = val if val else {}
    return fm, body


def render_frontmatter(name: str, description: str, type_: str) -> str:
    desc = description.replace('"', '\\"')
    return (
        "---\n"
        f"name: {name}\n"
        f'description: "{desc}"\n'
        "metadata:\n"
        f"  type: {type_}\n"
        "---\n"
    )


def extract_sections(content: str, section_names: list[str]) -> str:
    """Return concatenated body text for the given H2 section names.

    First section's H2 (`## Heading`) is promoted to H1 (`# Heading`) since
    the extracted section becomes the file's main heading. Subsequent
    sections stay as H2 within the same file.
    """
    h2_re = re.compile(r"^## (.+)$", re.MULTILINE)
    h2_matches = list(h2_re.finditer(content))
    section_ranges: dict[str, tuple[int, int, str]] = {}
    for i, m in enumerate(h2_matches):
        name = m.group(1).strip()
        start = m.start()
        end = h2_matches[i + 1].start() if i + 1 < len(h2_matches) else len(content)
        section_ranges[name] = (start, end, content[start:end])
    out_parts: list[str] = []
    for idx, needed in enumerate(section_names):
        if needed not in section_ranges:
            print(f"  WARN: section not found: {needed!r}", file=sys.stderr)
            continue
        chunk = section_ranges[needed][2].rstrip()
        if idx == 0:
            # Promote first H2 to H1
            chunk = re.sub(r"^## ", "# ", chunk, count=1)
        out_parts.append(chunk)
    return "\n\n".join(out_parts) + "\n"


# Plain-text filename substitution map — built from SIMPLE_MOVES so body
# text like "see `feedback_matching_playbook.md`" gets rewritten to
# "see `reference/matching-playbook.md`".
def build_filename_subs() -> list[tuple[str, str]]:
    subs: list[tuple[str, str]] = []
    for src, dest in SIMPLE_MOVES.items():
        # Order matters: longer src first to avoid partial matches
        subs.append((f"`{src}`", f"`{dest}`"))
        # Also the .md-less form
        src_stem = src.removesuffix(".md")
        dest_stem = dest.removesuffix(".md")
        # Avoid double-substituting the .md form
        subs.append((f"`{src_stem}.md`", f"`{dest}`"))
    # Workflow_rules.md doesn't have a single dest — replace with the rules/ dir mention
    subs.append(("`feedback_workflow_rules.md`", "the rules in `memory/rules/`"))
    # Sort by source length descending so longer matches replace first
    subs.sort(key=lambda x: -len(x[0]))
    return subs


def rewrite_filenames(content: str, subs: list[tuple[str, str]]) -> str:
    for find, replace in subs:
        content = content.replace(find, replace)
    return content


def derive_type_from_dest(dest: str) -> str:
    first = dest.split("/", 1)[0]
    return {
        "rules": "rule",
        "workflow": "workflow",
        "reference": "reference",
        "recipes": "recipe",
        "audit": "audit",
        "naming": "naming",
        "project": "project",
        "user": "user",
        "history": "history",
    }.get(first, "feedback")


# ---------------------------------------------------------------------------
# Main migration
# ---------------------------------------------------------------------------

def main() -> int:
    if NEW_DIR.exists():
        shutil.rmtree(NEW_DIR)
    NEW_DIR.mkdir(parents=True)

    # Phase 1: build slug remap (old slug -> new slug).
    # Old slugs can be: filename stem, frontmatter `name:` slug, or various
    # kebab/snake variants. Map all to the new file's stem (kebab-case).
    slug_remap: dict[str, str] = {}

    def register_slug(old: str, new_stem: str) -> None:
        slug_remap[old] = new_stem
        slug_remap[old.replace("_", "-")] = new_stem
        slug_remap[old.replace("-", "_")] = new_stem
        # Strip feedback_/project_ prefix variants
        for prefix in ("feedback_", "feedback-", "project_", "project-"):
            if old.startswith(prefix):
                stripped = old[len(prefix):]
                slug_remap[stripped] = new_stem
                slug_remap[stripped.replace("_", "-")] = new_stem
                slug_remap[stripped.replace("-", "_")] = new_stem

    # From SIMPLE_MOVES
    for src, dest in SIMPLE_MOVES.items():
        src_stem = src.removesuffix(".md")
        new_stem = Path(dest).stem
        register_slug(src_stem, new_stem)
        # Also try the frontmatter name from the actual file
        src_path = MEMORY_DIR / src
        if src_path.exists():
            fm, _ = parse_frontmatter(src_path.read_text(encoding="utf-8", errors="replace"))
            fm_name = (fm.get("name") or "").strip()
            if fm_name:
                register_slug(fm_name, new_stem)

    # From STAGED_OVERRIDES (read frontmatter from staged file)
    for dest, staged_rel in STAGED_OVERRIDES.items():
        staged_path = STAGING_DIR / staged_rel
        if staged_path.exists():
            fm, _ = parse_frontmatter(staged_path.read_text(encoding="utf-8", errors="replace"))
            new_stem = Path(dest).stem
            fm_name = (fm.get("name") or "").strip()
            if fm_name:
                register_slug(fm_name, new_stem)

    # From WORKFLOW_SPLITS
    for split in WORKFLOW_SPLITS:
        register_slug(split["name"], Path(split["dest"]).stem)

    # Consumed source files redirect to their consolidation target
    consolidation_redirects = {
        "feedback_voice_contorol_intractable":                  "voice-control-playbook",
        "feedback_voice_contorol_cc1psx_calibration_2026_05_17": "voice-control-playbook",
        "feedback_voice_contorol_liberal_barriers_2026_05_17":   "voice-control-playbook",
        "feedback_voice_contorol_session_2026_05_17_pure_c":     "voice-control-playbook",
        "feedback_exec_game_lessons":                            "exec-game-plateau",
        "feedback_exec_game_plateau":                            "exec-game-plateau",
        "feedback_kengo_name_match_unreliable":                  "kengo-name-unreliable",
        "feedback_kengo_name_unique_mass_rename_unreliable":     "kengo-name-unreliable",
        "feedback_session_2026_05_17_autonomous_attempts":       "history-2026-05-17-cleanup-and-cheats",
        "feedback_session_2026_05_17_cheat_cleanup_batch":       "history-2026-05-17-cleanup-and-cheats",
        "feedback_workflow_rules":                               "hard-rule",
    }
    for old, new in consolidation_redirects.items():
        register_slug(old, new)

    # Build filename-substitution table (plain-text refs in body)
    filename_subs = build_filename_subs()

    # Phase 2: Process each source file.
    processed: list[str] = []
    deleted: list[str] = []
    moved: list[tuple[str, str]] = []
    workflow_rules_path = MEMORY_DIR / "feedback_workflow_rules.md"

    # 2a) Staged overrides (copy from STAGING_DIR)
    for dest, staged_rel in STAGED_OVERRIDES.items():
        src_path = STAGING_DIR / staged_rel
        dest_path = NEW_DIR / dest
        if not src_path.exists():
            print(f"  ERR: staging file missing: {src_path}", file=sys.stderr)
            continue
        dest_path.parent.mkdir(parents=True, exist_ok=True)
        content = src_path.read_text(encoding="utf-8", errors="replace")
        content = rewrite_filenames(content, filename_subs)
        dest_path.write_text(rewrite_links(content, slug_remap), encoding="utf-8")
        processed.append(dest)

    # 2b) workflow_rules.md atomic breakup
    if workflow_rules_path.exists():
        wf_content = workflow_rules_path.read_text(encoding="utf-8", errors="replace")
        _, wf_body = parse_frontmatter(wf_content)
        for split in WORKFLOW_SPLITS:
            dest_path = NEW_DIR / split["dest"]
            dest_path.parent.mkdir(parents=True, exist_ok=True)
            body = extract_sections(wf_body, split["sections"])
            frontmatter = render_frontmatter(
                split["name"],
                split["description"],
                derive_type_from_dest(split["dest"]),
            )
            content = frontmatter + "\n" + body
            content = rewrite_filenames(content, filename_subs)
            dest_path.write_text(rewrite_links(content, slug_remap), encoding="utf-8")
            processed.append(split["dest"])
        deleted.append("feedback_workflow_rules.md")

    # 2c) Simple moves
    for src, dest in SIMPLE_MOVES.items():
        src_path = MEMORY_DIR / src
        if not src_path.exists():
            print(f"  WARN: source missing: {src}", file=sys.stderr)
            continue
        dest_path = NEW_DIR / dest
        if dest_path.exists():
            # Already populated by STAGED_OVERRIDES; skip
            print(f"  SKIP {src} (already staged at {dest})", file=sys.stderr)
            continue
        dest_path.parent.mkdir(parents=True, exist_ok=True)
        content = src_path.read_text(encoding="utf-8", errors="replace")

        # Update frontmatter: kebab-case name, type from destination dir
        new_stem = Path(dest).stem
        fm, body = parse_frontmatter(content)
        fm["name"] = new_stem
        description = fm.get("description", new_stem)
        # Rewrite [[links]] in the description too (not just body)
        description = rewrite_links(description, slug_remap)
        type_ = derive_type_from_dest(dest)
        new_fm = render_frontmatter(new_stem, description, type_)

        # Apply per-file find/replace patches
        if src in REFRESH_DIFFS:
            for find, replace in REFRESH_DIFFS[src]:
                body = body.replace(find, replace)
        # Apply append patches
        if src in APPEND_PATCHES:
            body = body.rstrip() + APPEND_PATCHES[src]

        # Rewrite plain-text filename references and [[links]]
        body = rewrite_filenames(body, filename_subs)
        body = rewrite_links(body, slug_remap)
        dest_path.write_text(new_fm + "\n" + body, encoding="utf-8")
        moved.append((src, dest))

    # 2d) Mark deletions (just track them; we don't touch original files)
    for src in DELETIONS_CONSUMED:
        if (MEMORY_DIR / src).exists():
            deleted.append(src)

    # Phase 3: Validate — every [[link]] in NEW_DIR resolves
    print()
    print(f"=== Migration written to {NEW_DIR} ===")
    print(f"Staged overrides:    {len(STAGED_OVERRIDES)}")
    print(f"workflow_rules splits: {len(WORKFLOW_SPLITS)}")
    print(f"Simple moves:        {len(moved)}")
    print(f"Sources consumed:    {len(deleted)}")
    print(f"Total new files:     {len(processed) + len(moved)}")

    # Find broken links in new dir
    new_stems = set(p.stem for p in NEW_DIR.rglob("*.md"))
    broken: list[tuple[str, str]] = []
    for p in NEW_DIR.rglob("*.md"):
        text = p.read_text(encoding="utf-8", errors="replace")
        for m in LINK_RE.finditer(text):
            target = m.group(1)
            if target not in new_stems:
                broken.append((str(p.relative_to(NEW_DIR)), target))
    if broken:
        print()
        print(f"WARN: {len(broken)} broken [[link]] references (after migration):")
        for src, tgt in sorted(set(broken))[:30]:
            print(f"  {src} -> [[{tgt}]]")
        if len(broken) > 30:
            print(f"  ... and {len(broken) - 30} more")
    else:
        print("OK: All [[link]] references resolve")

    return 0


def rewrite_links(content: str, slug_map: dict[str, str]) -> str:
    """Rewrite [[old_slug]] -> [[new_stem]] using the slug map."""
    def repl(m: re.Match) -> str:
        slug = m.group(1)
        new = slug_map.get(slug, slug)
        return f"[[{new}]]"
    return LINK_RE.sub(repl, content)


if __name__ == "__main__":
    sys.exit(main())
