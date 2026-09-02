# Grinder Session-Efficiency Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Cut the sessions-per-completion on plateaued functions by removing the four measured waste sources found in the 2026-09-01 post-mortem of the six longest grinder runs, without lowering any standard, adding any cheat surface, or changing the ladder's gating semantics.

**Architecture:** Every change is either (a) a text correction that aligns a role brief / rule index with an owner ruling that already exists, (b) a ledger-schema addition that makes a KILLED hypothesis carry the state it was measured under, (c) a brief-time injection of authoritative current rule scope, or (d) a standalone read-only diagnostic script under `tools/`. No task edits the oracle compiler, the build pipeline, the Judge's policy, the frozen SOTN list, the ladder rungs, or the escalation trigger. All driver-side logic lives in `tools/grinder/grindlib.py` (Python, unit-tested with stdlib `unittest`); `tools/grinder/grind.ps1` gets two small text-injection edits only.

**Tech Stack:** Python 3 (stdlib only), PowerShell 7 for the driver, GCC 2.7.2 `-da` dumps via `tools/grinder/dump.ps1`, the instrumented cc1 (`tools/gcc-2.7.2/cc1`, `BB2_ALLOC_DEBUG`) via `tools/ra_solver/extract.py`.

---

## Evidence this plan rests on (post-mortem 2026-09-01)

| Function | Sessions | Closer first on disk | Applied | Waste cause |
|---|---:|---:|---:|---|
| func_80038C70 | 49 | s3 | s49 | ladder re-dispatch of an owner-gated item (fixed 2026-08-31); wall premise never measured |
| func_80057CC8 | 46 | s8 (score 0) | s46 | per-function ban superseded by the F3 family grant of 2026-08-18; Judge kept citing the old refusal |
| func_80041188 | 37 | s19 | s37 | chassis foreclosed on an incomplete loop.c predicate (missed `n_times_set == 1`) |
| func_800283D0 | 27 | s4 | s22 | do-while(0) find banked as "out of scope" quoting a scoping abolished 2026-07-06 |
| func_800324D0 | 22 | s6 | s22 | lever killed on 3-of-12 arms with an antagonistic staged read, then cited as settled |
| func_8002EA24 | 21 | s8 | s21 | lever recorded "inert" while a FAKE carrier occupied the target pseudo |

Over 149 of 202 sessions ran after the closing form was already banked. Four causes, in order of sessions lost: (1) kills on a confounded instance cited as class-level law; (2) stale rule scope / superseded bans; (3) ladder no-op sessions (the largest instance is already fixed); (4) permuter rungs on problems the permuter cannot express.

The stale do-while(0) scoping is **live damage right now**: `tools/grinder/roles/grind-session.md:86` and `:115` and `.claude/agents/cheat-reviewer.md:96-101` still say "LABEL_OUTSIDE_LOOP_P / reorg.c interaction ONLY", and the current queue-top ledger (`memory/grind/func_8003C714/hypotheses.md`, s1 frontier item 1) quoted that abolished scoping as a policy caveat.

---

## Preconditions and sequencing (operator)

The Grinder is running. Its end-of-session scope check discards sessions when tracked files change under it, and `tools/grinder/*` must not be edited while the loop runs. Therefore:

1. `pwsh tools/grinder/grind.ps1 -Stop` and wait until `tmp/grind/grind.lock` is gone (the driver stops at the next session boundary; a session can take up to 90 minutes).
2. `git status --short` must be clean apart from `metrics/events.jsonl`. Commit any driver-left ledger dirt with `git add memory/grind docs/grind metrics/events.jsonl && git commit -m "grind: ledger sweep before efficiency work [skip-park-src-guard]"`.
3. Execute Tasks 1 to 11 in order. Each ends in its own commit.
4. Task 12 runs the drills and relaunches.

All Python tests in this plan run from the repo root on the Windows side with `python <path> -v` (the grinder tests are stdlib `unittest`, no venv needed). Scripts that compile C (Tasks 7, 8, 9, 10) run under WSL with the venv active, exactly like `tools/sweep_variants.py`.

---

## File map

| File | Responsibility | Task |
|---|---|---|
| `tools/grinder/roles/grind-session.md` | session role brief: frozen list line, family table row, hypothesis fields, kill re-audit protocol | 1, 5 |
| `.claude/agents/cheat-reviewer.md` | layer-1 reviewer: do-while(0) prerequisite 2 | 1 |
| `.claude/rules/codegen-technique-index.md` | symptom index line for do-while-zero-exception | 1 |
| `.claude/rules/no-new-park-categories.md` | frozen-list entry body for do-while(0) | 1 |
| `tools/grinder/grindlib.py` | `rule_descriptions`, `cited_rule_scopes`, kill-scope validation, `kills` state list, brief blocks, `rule-scopes` CLI | 2, 4 |
| `tools/grinder/tests/test_grindlib.py` | unit tests for the above | 2, 4 |
| `tools/grinder/grind.ps1` | inject the rule-scope block into the layer-1 and Judge task text | 3 |
| `tools/grinder/roles/judge.md` | dated-rulings supersession paragraph | 3 |
| `tools/sweep_variants.py` | `find_function_span` learns the `INCLUDE_ASM` representation | 6 |
| `tools/fake_ablate.py` + `tools/test_fake_ablate.py` | FAKE-ablation sweep | 7 |
| `tools/loop_movables.py` + `tools/test_loop_movables.py` | loop.c movable-decision report from the `.loop` dump | 8 |
| `tools/nrefs_census.py` + `tools/test_nrefs_census.py` | reg_n_refs / allocno-order census + what-if lift calculator | 9 |
| `tools/label_census.py` + `tools/test_label_census.py` | branch-target / predecessor / callee-saved-ref census, target vs build | 10 |
| `tools/grinder/grant_rescan.py` + test in `test_grindlib.py` | re-adjudication sweep after a family grant | 11 |
| `docs/STATUS.md`, `.claude/skills/decomp-grind/SKILL.md` | refresh counts; document the new surfaces | 12 |

---

### Task 1: Align every do-while(0) scope statement with the 2026-07-06 owner ruling

The authoritative text is `.claude/rules/do-while-zero-exception.md` (frontmatter `description:` — "SANCTIONED (owner ruling 2026-07-06, supersedes the 2026-06-04 mechanism-scoping): ... for ANY codegen effect incl. register allocation"). Four surfaces still carry the abolished scoping. This is a `rules:` commit per [[ruling-record-lands-before-code]]; it spends no new ruling, it corrects records of an existing one.

**Files:**
- Modify: `tools/grinder/roles/grind-session.md:86`
- Modify: `tools/grinder/roles/grind-session.md:115`
- Modify: `.claude/agents/cheat-reviewer.md:96-101`
- Modify: `.claude/rules/codegen-technique-index.md:99`
- Modify: `.claude/rules/no-new-park-categories.md:256-270`

- [ ] **Step 1: Confirm the stale hits before editing**

Run:
```bash
grep -rn 'reorg.c interaction ONLY\|LABEL_OUTSIDE_LOOP_P / reorg.c invert-jump-peephole interaction (or' tools/grinder/roles .claude/agents .claude/rules
```
Expected: exactly 3 hits (grind-session.md:86, grind-session.md:115, cheat-reviewer.md:98).

- [ ] **Step 2: Edit `tools/grinder/roles/grind-session.md` line 86**

Replace:
```
`do { ... } while (0);` wrap (LABEL_OUTSIDE_LOOP_P / reorg.c interaction ONLY) ·
```
with:
```
`do { ... } while (0);` wrap (ANY codegen effect incl. register allocation — owner ruling 2026-07-06; FAKE-annotated) ·
```

- [ ] **Step 3: Edit `tools/grinder/roles/grind-session.md` line 115 (the family-selection table row)**

Replace:
```
| `do { } while (0);` wrap | `.claude/rules/do-while-zero-exception.md` | LABEL_OUTSIDE_LOOP_P / reorg.c interaction ONLY; FAKE required; for/while/if equivalents NOT sanctioned. |
```
with:
```
| `do { } while (0);` wrap | `.claude/rules/do-while-zero-exception.md` | sanctioned for ANY codegen effect incl. register allocation (owner ruling 2026-07-06 SUPERSEDES the reorg.c-only scoping — do not quote the old scope); FAKE required; nested wraps need a single-level-insufficient justification; for/while/if equivalents NOT sanctioned. |
```

- [ ] **Step 4: Edit `.claude/agents/cheat-reviewer.md` prerequisite 2 (lines 96-101)**

Replace:
```
  2. The wrap is being used SPECIFICALLY to defeat the
     LABEL_OUTSIDE_LOOP_P / reorg.c invert-jump-peephole interaction (or
     the analogous loop-note interaction with loop.c's other passes).
     Using it to bend a different GCC pass is OUT OF SCOPE for this
     exception — that's a fresh borderline construct needing its own
     SOTN evidence. If the worker can't articulate which GCC-pass
     interaction the wrap is defeating, **FAIL**.
```
with:
```
  2. The worker NAMES the GCC-pass mechanism the wrap bends. Any pass
     qualifies — reorg.c LABEL_OUTSIDE_LOOP_P, loop.c, global.c register
     allocation, sched.c — because the owner ruling of 2026-07-06
     (do-while-zero-exception.md, "supersedes the 2026-06-04
     mechanism-scoping") sanctions the wrap for ANY codegen effect. A FAIL
     citing "wrong pass" or "out of scope for this exception" is a
     stale-scope error, not a policy ground. If the worker cannot
     articulate WHICH GCC-pass interaction the wrap is defeating, **FAIL**
     (mechanism unnamed), and say so in those words.
```

- [ ] **Step 5: Edit `.claude/rules/codegen-technique-index.md` line 99**

Replace:
```
- **do-while-zero-exception** — the ONE sanctioned no-semantic-purpose wrapper; strict prerequisites; READ BEFORE using `do {...} while (0);`.
```
with:
```
- **do-while-zero-exception** — the ONE sanctioned no-semantic-purpose wrapper; sanctioned for ANY codegen effect incl. register allocation (owner ruling 2026-07-06); FAKE-annotated, lever-exhaustion documented; READ BEFORE using `do {...} while (0);`.
```

- [ ] **Step 6: Edit `.claude/rules/no-new-park-categories.md` lines 256-270 (the frozen-list entry body)**

Replace the block that begins `- **\`do { ... } while (0);\` wrap** (empty or non-empty body)` and ends `independently.` with:
```
- **`do { ... } while (0);` wrap** (empty or non-empty body)
  ([[do-while-zero-exception]] / [[sotn-do-while-zero-research-2026-06-04]]):
  sanctioned as a pure-C match device for ANY codegen effect, including
  register allocation (owner ruling 2026-07-06, which SUPERSEDES the
  2026-06-04 reorg.c/LABEL_OUTSIDE_LOOP_P-only scoping; that scoping is
  abolished and must not be cited as a FAIL ground). Last-resort, mandatory
  `/* FAKE: ... */` annotation, documented lever-exhaustion, named GCC-pass
  mechanism; nested wraps need a single-level-insufficient justification.
  SOTN evidence: 18+ instances in master across `sprintf.c`, `5087C.c`,
  `c_004.c`, `w_045.c`, etc., with two PR-merge messages explicitly
  accepting it. User policy 2026-06-04: this is the ONE no-semantic-purpose
  wrapper sanctioned in BB2 source. Other syntactic equivalents
  (`for (i=0;i<1;i++)`, `while(1) { ...; break; }`, `if (1) { }`) are NOT
  sanctioned by this rule's existence — they have to clear the same
  SOTN-evidence bar independently.
```

- [ ] **Step 7: Verify no stale scoping remains**

Run:
```bash
grep -rn 'reorg.c interaction ONLY\|reorg.c invert-jump-peephole interaction (or' tools/grinder/roles .claude/agents .claude/rules
```
Expected: no output. Then `python tools/normalize_lf.py --check tools/grinder/roles/grind-session.md .claude/agents/cheat-reviewer.md .claude/rules/codegen-technique-index.md .claude/rules/no-new-park-categories.md` (if `--check` is unsupported, run `file` on each and confirm no CRLF).

- [ ] **Step 8: Commit**

Write `tmp/msg.txt`:
```
rules: align every do-while(0) scope statement with owner ruling 2026-07-06

The 2026-09-01 post-mortem found func_800283D0 banked its decisive
permuter find (s4, 28->26) as "out of scope" by quoting the reorg.c-only
scoping the owner abolished on 2026-07-06 (5c3a192f), costing 18
sessions; func_8003C714's s1 frontier carried the same caveat. The stale
text lived in the session role brief, the layer-1 reviewer brief, the
technique index, and the frozen-list entry body. All four now state the
current scope (ANY codegen effect incl. register allocation) and name the
superseded scoping so no reader re-derives it.

No standard changes: do-while-zero-exception.md is untouched and remains
the authority.

Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01Dn2fdFSbP8sT54D4aRw2gW
```
Run:
```bash
git add tools/grinder/roles/grind-session.md .claude/agents/cheat-reviewer.md .claude/rules/codegen-technique-index.md .claude/rules/no-new-park-categories.md
git commit -F tmp/msg.txt
```

---

### Task 2: Brief-time injection of every cited rule's CURRENT scope

A ledger paraphrases rule scope at the time it was written. Rule text moves by owner ruling. This task makes the brief print the current `description:` line of every rule slug the ledger mentions, so a stale kill cannot survive a dispatch unchallenged.

**Files:**
- Modify: `tools/grinder/grindlib.py` (new functions after `knowledge_sweep`, new block in `build_brief`, new CLI verb)
- Test: `tools/grinder/tests/test_grindlib.py`

- [ ] **Step 1: Write the failing tests**

Append to `tools/grinder/tests/test_grindlib.py`:
```python
class TestCitedRuleScopes(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = self.tmp.name
        rules = os.path.join(self.root, ".claude", "rules")
        os.makedirs(rules)
        with open(os.path.join(rules, "do-while-zero-exception.md"), "w",
                  encoding="utf-8", newline="\n") as f:
            f.write('---\nname: do-while-zero-exception\n'
                    'description: "SANCTIONED for ANY codegen effect"\n---\nbody\n')
        with open(os.path.join(rules, "unrelated-rule.md"), "w",
                  encoding="utf-8", newline="\n") as f:
            f.write('---\nname: unrelated-rule\ndescription: "never cited"\n---\n')
        G.init_ledger(self.root, "func_X", "stem")

    def tearDown(self):
        self.tmp.cleanup()

    def test_rule_descriptions_reads_frontmatter(self):
        d = G.rule_descriptions(self.root)
        self.assertEqual(d["do-while-zero-exception"], "SANCTIONED for ANY codegen effect")
        self.assertEqual(d["unrelated-rule"], "never cited")

    def test_cited_scopes_from_rejected_header(self):
        rj = os.path.join(self.root, "memory", "grind", "func_X", "rejected", "x.c")
        with open(rj, "w", encoding="utf-8", newline="\n") as f:
            f.write("/* REJECTED: do-while-zero-exception scope is reorg.c ONLY */\n")
        got = G.cited_rule_scopes(self.root, "func_X")
        self.assertEqual(got, [("do-while-zero-exception", "SANCTIONED for ANY codegen effect")])

    def test_cited_scopes_from_state_and_hypotheses(self):
        G.add_judge_constraint(self.root, "func_X", "see unrelated-rule for why")
        G.append_hypothesis(self.root, "func_X",
                            {"statement": "uses do-while-zero-exception", "mechanism": "m",
                             "probe": "p", "result": "15", "verdict": "KILLED"}, session=1)
        slugs = [s for s, _ in G.cited_rule_scopes(self.root, "func_X")]
        self.assertEqual(slugs, ["do-while-zero-exception", "unrelated-rule"])

    def test_brief_carries_current_scope_block(self):
        G.add_judge_constraint(self.root, "func_X", "do-while-zero-exception applies")
        brief = G.build_brief(self.root, "func_X", "structural", "/tmp/o.json")
        self.assertIn("CURRENT SCOPE OF EVERY RULE THIS LEDGER CITES", brief)
        self.assertIn('do-while-zero-exception: "SANCTIONED for ANY codegen effect"', brief)

    def test_brief_omits_block_when_nothing_cited(self):
        brief = G.build_brief(self.root, "func_X", "structural", "/tmp/o.json")
        self.assertNotIn("CURRENT SCOPE OF EVERY RULE", brief)
```

- [ ] **Step 2: Run the tests to verify they fail**

Run: `python tools/grinder/tests/test_grindlib.py -v 2>&1 | tail -15`
Expected: 5 errors with `AttributeError: module ... has no attribute 'rule_descriptions'` (and `cited_rule_scopes`).

- [ ] **Step 3: Implement `rule_descriptions` and `cited_rule_scopes`**

Insert into `tools/grinder/grindlib.py` immediately before `def build_brief(`:
```python
# ── Current-scope injection (2026-09-01 post-mortem) ─────────────────────────
# A ledger paraphrases a rule's scope at the time it was written; rule text
# moves by owner ruling. func_800283D0 banked its decisive find as "out of
# scope" by quoting a scoping abolished seven weeks earlier and lost 18
# sessions; func_8003C714 s1 carried the same caveat. The brief now prints the
# CURRENT `description:` line of every rule slug the ledger mentions, so a
# stale kill cannot survive a dispatch unchallenged.
_RULE_DESC_RE = re.compile(r'(?m)^description:\s*"?(.+?)"?\s*$')


def rule_descriptions(root):
    """{slug: current description line} for every .claude/rules/*.md."""
    d = os.path.join(root, ".claude", "rules")
    out = {}
    if not os.path.isdir(d):
        return out
    for fn in sorted(os.listdir(d)):
        if not fn.endswith(".md"):
            continue
        try:
            with open(os.path.join(d, fn), encoding="utf-8", errors="replace") as f:
                head = f.read(4000)
        except OSError:
            continue
        m = _RULE_DESC_RE.search(head)
        out[fn[:-3]] = m.group(1).strip() if m else "(no description line)"
    return out


def cited_rule_scopes(root, func):
    """[(slug, current_description)] for every rule slug that appears anywhere
    in this function's ledger: state.json constraints/bans/frontier,
    hypotheses.md, evidence.md, and the first 60 lines of each rejected/*.c."""
    descs = rule_descriptions(root)
    if not descs:
        return []
    d = ledger_dir(root, func)
    texts = []
    st = load_state(root, func) or {}
    texts += [str(x) for x in st.get("judge_constraints", [])]
    texts += [str(x) for x in st.get("banned_constructs", [])]
    texts += [json.dumps(f) for f in st.get("frontier", [])]
    for name in ("hypotheses.md", "evidence.md"):
        p = os.path.join(d, name)
        if os.path.isfile(p):
            with open(p, encoding="utf-8", errors="replace") as f:
                texts.append(f.read())
    rj = os.path.join(d, "rejected")
    if os.path.isdir(rj):
        for fn in sorted(os.listdir(rj)):
            try:
                with open(os.path.join(rj, fn), encoding="utf-8", errors="replace") as f:
                    texts.append("".join(f.readlines()[:60]))
            except OSError:
                pass
    blob = "\n".join(texts)
    return [(s, descs[s]) for s in sorted(descs) if s in blob]


def render_rule_scopes(pairs):
    if not pairs:
        return ""
    lines = "\n".join(f'  - {s}: "{d}"' for s, d in pairs)
    return ("\n## CURRENT SCOPE OF EVERY RULE THIS LEDGER CITES (authoritative NOW)\n"
            "Rule text changes by owner ruling. A scope quoted in hypotheses.md, a\n"
            "rejected/ header, or an older Judge ruling may be SUPERSEDED. The lines\n"
            "below are the rules' CURRENT `description:` lines, extracted at dispatch.\n"
            "If a banked kill, rejection, or ban rests on a NARROWER scoping than what is\n"
            "printed here, that kill is VOID: re-measure the form under the current scope\n"
            "before spending a session elsewhere.\n" + lines + "\n")
```

- [ ] **Step 4: Wire the block into `build_brief`**

In `build_brief`, directly after the line `chassis = (f"\n## CHASSIS CHECK ...` block ends (the statement assigning `chassis`), add:
```python
    scopes = render_rule_scopes(cited_rule_scopes(root, func))
```
and in the returned f-string change the line
```
{MODALITY_PLAYBOOK[modality]}
{fixup}{directive}{consistency}{banned}
```
to
```
{MODALITY_PLAYBOOK[modality]}
{scopes}{fixup}{directive}{consistency}{banned}
```

- [ ] **Step 5: Add the CLI verb**

In the `if __name__ == "__main__":` dispatcher, before `else:` add:
```python
    elif cmd == "rule-scopes":
        # rule-scopes <root> <func> -> prints the CURRENT SCOPE block (empty if none cited)
        print(render_rule_scopes(cited_rule_scopes(sys.argv[2], sys.argv[3])))
```
and add the line `#   grindlib.py rule-scopes <root> <func>   -> prints the current-scope block` to the CLI comment header.

- [ ] **Step 6: Run the tests**

Run: `python tools/grinder/tests/test_grindlib.py -v 2>&1 | tail -5`
Expected: `OK` with the 5 new tests passing and every pre-existing test still passing.

- [ ] **Step 7: Smoke it on a real ledger**

Run: `python tools/grinder/grindlib.py rule-scopes . func_8003C714 | head -12`
Expected: the block header plus at least `do-while-zero-exception` and `no-compiler-divergence` lines (both are cited in that ledger).

- [ ] **Step 8: Commit**

```bash
git add tools/grinder/grindlib.py tools/grinder/tests/test_grindlib.py
git commit -m "grinder: inject every cited rule's CURRENT scope into the session brief

Post-mortem 2026-09-01: stale rule scope quoted inside ledgers cost
func_800283D0 18 sessions. The brief now lists the current description
line of every .claude/rules slug the ledger mentions and declares any
narrower banked scoping void. Read-only; no gate semantics change.

Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01Dn2fdFSbP8sT54D4aRw2gW"
```

---

### Task 3: Same scope block for the layer-1 reviewer and the Judge; dated-rulings paragraph

The 2026-08-20 layer-1 FAILs on func_80057CC8 cited a per-function refusal that the F3 grant of 2026-08-18 had superseded. Reviewers need the same current-scope block and an explicit rule that owner rulings are dated.

**Files:**
- Modify: `tools/grinder/grind.ps1` (function `Invoke-Layer1`, around line 288; function `Invoke-CandidatePath` FINAL CALL task text, around line 903)
- Modify: `tools/grinder/roles/judge.md` (policy list)

- [ ] **Step 1: Inject into the layer-1 task text**

In `Invoke-Layer1`, immediately before the line that begins `$task = @"` (the here-string starting `LAYER-1 REVIEW for $func`), add:
```powershell
    $scopeBlock = ''
    try { $scopeBlock = (python tools/grinder/grindlib.py rule-scopes . $func 2>$null | Out-String) } catch { }
```
Then inside the here-string, after the paragraph that starts `Verify against the ledger yourself:` add a blank line followed by:
```
$scopeBlock
DATED RULINGS: every ban in state.json and every ruling in decisions.md carries a
date. A family grant in .claude/rules/no-new-park-categories.md dated AFTER a
per-function refusal or ban SUPERSEDES that refusal for the construct it covers.
Before citing a ban or an older ruling as a FAIL ground, check whether a later
dated grant covers the construct; if it does, the ban is stale and the correct
verdict is decided on the grant's own prerequisites.
```

- [ ] **Step 2: Inject into the Judge FINAL CALL task text**

In `Invoke-CandidatePath`, immediately before `$task = @"` (the here-string starting `FINAL CALL for $func`), add:
```powershell
    $scopeBlock = ''
    try { $scopeBlock = (python tools/grinder/grindlib.py rule-scopes . $func 2>$null | Out-String) } catch { }
```
and inside the here-string, after the `Ledger: $led/state.json ...` paragraph, add:
```
$scopeBlock
```

- [ ] **Step 3: Add the dated-rulings policy line to `tools/grinder/roles/judge.md`**

In the section `## The owner's static policy (verbatim intent — apply it as written)`, after the `- **Default-FAIL.**` bullet, add:
```
- **Rulings are dated; grants supersede earlier refusals (2026-09-01
  process fix).** A per-function ban in `state.json`, a layer-1 FAIL, or a
  decisions.md refusal that PREDATES a family grant in
  `.claude/rules/no-new-park-categories.md` covering the same construct is
  superseded by that grant. func_80057CC8 sat 38 sessions on a 2026-07-20
  refusal after the 2026-08-18 F3 grant covered its closing form, because
  three reviews cited the refusal without checking the grant's date. Decide
  such a construct on the grant's own prerequisites; when you do, set
  `unban_construct` to the superseded ban's text so the driver clears the
  mechanical tripwire.
```

- [ ] **Step 4: Syntax-check the driver without running it**

Run: `pwsh -NoProfile -Command "[scriptblock]::Create((Get-Content tools/grinder/grind.ps1 -Raw)) | Out-Null; 'PARSE OK'"`
Expected: `PARSE OK`.

- [ ] **Step 5: Commit**

```bash
git add tools/grinder/grind.ps1 tools/grinder/roles/judge.md
git commit -m "grinder: reviewers see the cited rules' current scope + dated-rulings supersession

Layer-1 and the Judge now receive the same CURRENT SCOPE block the session
brief carries, and the Judge policy states that a family grant dated after
a per-function refusal supersedes it (func_80057CC8: 38 sessions lost to a
superseded ban cited by three reviews).

Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01Dn2fdFSbP8sT54D4aRw2gW"
```

---

### Task 4: Kill hygiene in the ledger schema (`kill_scope`, `measured_on`, `predicate_cite`)

Four of six long runs were prolonged by a kill measured on one confounded instance and then cited as class-level law. The outcome schema now makes every KILLED hypothesis declare its scope; class-level claims must cite the gate predicate by file:line; instance kills are recorded with the chassis and FAKE state they were measured under. Kills are stored structurally in `state.json` so the brief can list them and, on a flat floor, demand a re-audit before new probes.

**Files:**
- Modify: `tools/grinder/grindlib.py` (`validate_outcome`, `append_hypothesis`, `apply_outcome`, `init_ledger`, `build_brief`)
- Test: `tools/grinder/tests/test_grindlib.py`

- [ ] **Step 1: Write the failing tests**

Append to `tools/grinder/tests/test_grindlib.py`:
```python
class TestKillHygiene(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = self.tmp.name
        G.init_ledger(self.root, "func_X", "stem")

    def tearDown(self):
        self.tmp.cleanup()

    def killed(self, **kw):
        h = {"statement": "tail duplication into 3 arms", "mechanism": "flow.c reg_n_refs",
             "probe": "sandbox", "result": "flat 15", "verdict": "KILLED",
             "kill_scope": "instance", "measured_on": "HEAD chassis, staged read L3 present"}
        h.update(kw)
        return {"result": "progress", "floor": 15, "headline": "h",
                "hypotheses": [h], "evidence": ["e"], "frontier": [], "artifacts": []}

    def test_instance_kill_valid(self):
        ok, why = G.validate_outcome(self.killed(), "structural", self.root)
        self.assertTrue(ok, why)

    def test_killed_requires_kill_scope(self):
        o = self.killed(); del o["hypotheses"][0]["kill_scope"]
        ok, why = G.validate_outcome(o, "structural", self.root)
        self.assertFalse(ok); self.assertIn("kill_scope", why)

    def test_killed_requires_measured_on(self):
        o = self.killed(measured_on="")
        ok, why = G.validate_outcome(o, "structural", self.root)
        self.assertFalse(ok); self.assertIn("measured_on", why)

    def test_class_claim_wording_needs_class_scope(self):
        o = self.killed(statement="tail duplication is unreachable by any natural geometry")
        ok, why = G.validate_outcome(o, "structural", self.root)
        self.assertFalse(ok); self.assertIn("class-level claim", why)

    def test_class_kill_needs_predicate_cite(self):
        o = self.killed(kill_scope="class")
        ok, why = G.validate_outcome(o, "structural", self.root)
        self.assertFalse(ok); self.assertIn("predicate_cite", why)

    def test_class_kill_with_cite_valid(self):
        o = self.killed(kill_scope="class", predicate_cite="tools/gcc-2.7.2/loop.c:705",
                        statement="no movable can pass with n_times_set != 1 — all forms")
        ok, why = G.validate_outcome(o, "structural", self.root)
        self.assertTrue(ok, why)

    def test_confirmed_needs_no_scope(self):
        o = self.killed(verdict="CONFIRMED"); del o["hypotheses"][0]["kill_scope"]
        ok, why = G.validate_outcome(o, "structural", self.root)
        self.assertTrue(ok, why)

    def test_apply_records_kills_structurally(self):
        G.apply_outcome(self.root, "func_X", self.killed(), "structural")
        st = G.load_state(self.root, "func_X")
        self.assertEqual(len(st["kills"]), 1)
        k = st["kills"][0]
        self.assertEqual(k["session"], 1)
        self.assertEqual(k["kill_scope"], "instance")
        self.assertEqual(k["measured_on"], "HEAD chassis, staged read L3 present")
        hyp = open(os.path.join(self.root, "memory", "grind", "func_X", "hypotheses.md"),
                   encoding="utf-8").read()
        self.assertIn("- kill_scope: instance", hyp)
        self.assertIn("- measured_on: HEAD chassis, staged read L3 present", hyp)

    def test_legacy_state_without_kills_key_still_applies(self):
        st = G.load_state(self.root, "func_X"); del st["kills"]; G.save_state(self.root, "func_X", st)
        G.apply_outcome(self.root, "func_X", self.killed(), "structural")
        self.assertEqual(len(G.load_state(self.root, "func_X")["kills"]), 1)

    def test_brief_reaudit_block_on_flat_floor(self):
        for _ in range(3):
            G.apply_outcome(self.root, "func_X", self.killed(), "structural")
        brief = G.build_brief(self.root, "func_X", "permuter", "/tmp/o.json")
        self.assertIn("KILL RE-AUDIT REQUIRED", brief)
        self.assertIn("tail duplication into 3 arms", brief)

    def test_brief_no_reaudit_while_floor_moves(self):
        G.apply_outcome(self.root, "func_X", self.killed(floor=20), "structural")
        G.apply_outcome(self.root, "func_X", self.killed(floor=15), "structural")
        brief = G.build_brief(self.root, "func_X", "permuter", "/tmp/o.json")
        self.assertNotIn("KILL RE-AUDIT REQUIRED", brief)
        self.assertIn("## KILL LEDGER", brief)
```
Note: `self.killed(floor=20)` sets the outcome floor because `kw` keys not in the hypothesis are ignored; adjust the helper so that a `floor` kwarg sets the outcome floor:
```python
    def killed(self, floor=15, **kw):
        ...
        return {"result": "progress", "floor": floor, ...}
```

- [ ] **Step 2: Run the tests to verify they fail**

Run: `python tools/grinder/tests/test_grindlib.py -v 2>&1 | grep -E 'FAIL|ERROR|OK' | tail -15`
Expected: `test_killed_requires_kill_scope`, `test_killed_requires_measured_on`, `test_class_claim_wording_needs_class_scope`, `test_class_kill_needs_predicate_cite`, `test_apply_records_kills_structurally`, `test_legacy_state_without_kills_key_still_applies`, `test_brief_reaudit_block_on_flat_floor`, `test_brief_no_reaudit_while_floor_moves` fail.

- [ ] **Step 3: Add the validation**

In `grindlib.py`, after `def _has_measurement(text):` add:
```python
# ── Kill hygiene (2026-09-01 post-mortem) ────────────────────────────────────
# Four of six 20+-session runs were prolonged by a KILLED hypothesis measured
# on one confounded instance (3 of 12 arms with a staged read; a lever under a
# FAKE carrier on the same pseudo; a loop.c predicate missing one term) and
# then cited by later sessions as class-level law. A kill now declares its
# scope: `instance` kills are chassis-relative and re-testable; `class` kills
# must cite the gate predicate by file:line.
KILL_SCOPES = ("instance", "class")
_CLASS_CLAIM_RE = re.compile(
    r"\b(any (natural )?(geometry|form|spelling|shape)|every (form|spelling|chassis)|"
    r"all (forms|spellings|chassis)|no natural|unreachable|impossible|permanently|"
    r"foreclosed|closed[- ]form|cannot (be|reach)|by construction)\b", re.I)


def _validate_kill(h):
    """(ok, reason) for one KILLED hypothesis dict."""
    scope = str(h.get("kill_scope", "")).strip().lower()
    if scope not in KILL_SCOPES:
        return False, ("KILLED hypothesis lacks kill_scope ('instance' or 'class'): "
                       f"{str(h.get('statement', ''))[:80]!r}")
    if not str(h.get("measured_on", "")).strip():
        return False, ("KILLED hypothesis lacks measured_on (the chassis + FAKE-construct "
                       "state it was measured under, e.g. 'HEAD chassis, L3 carrier present'): "
                       f"{str(h.get('statement', ''))[:80]!r}")
    text = f"{h.get('statement', '')} {h.get('result', '')}"
    if scope == "instance" and _CLASS_CLAIM_RE.search(text):
        m = _CLASS_CLAIM_RE.search(text).group(0)
        return False, (f"KILLED hypothesis makes a class-level claim ({m!r}) with "
                       "kill_scope='instance'. Either narrow the wording to the instance you "
                       "measured (which arms, which chassis, which FAKE state) or set "
                       "kill_scope='class' and cite the gate predicate in predicate_cite.")
    if scope == "class":
        cite = str(h.get("predicate_cite", "")).strip()
        if not cite or not _CITATION.search(cite):
            return False, ("class-scope KILLED hypothesis requires predicate_cite as file:line "
                           "(e.g. tools/gcc-2.7.2/loop.c:705) naming the gate predicate "
                           "the whole class fails; a search that came back empty is an "
                           "instance kill, not a class kill.")
    return True, ""
```
In `validate_outcome`, replace the block
```python
    proven = [h for h in o.get("hypotheses", [])
              if h.get("verdict") in ("CONFIRMED", "KILLED")
              and _has_measurement(h.get("result", ""))]
```
with
```python
    for h in o.get("hypotheses", []):
        if h.get("verdict") == "KILLED":
            ok, why = _validate_kill(h)
            if not ok:
                return False, why
    proven = [h for h in o.get("hypotheses", [])
              if h.get("verdict") in ("CONFIRMED", "KILLED")
              and _has_measurement(h.get("result", ""))]
```
(the loop must run before the `proven` check so the reason is specific).

- [ ] **Step 4: Persist kills and render the fields**

Replace `append_hypothesis` with:
```python
def append_hypothesis(root, func, h, session=None):
    p = os.path.join(ledger_dir(root, func), "hypotheses.md")
    tag = f"s{session}" if session else "?"
    extra = ""
    if h.get("verdict") == "KILLED":
        extra = (f"- kill_scope: {h.get('kill_scope', 'instance')}\n"
                 f"- measured_on: {h.get('measured_on', '?')}\n")
        if h.get("predicate_cite"):
            extra += f"- predicate_cite: {h['predicate_cite']}\n"
    with open(p, "a", encoding="utf-8", newline="\n") as f:
        f.write(f"\n## [{tag}] {h.get('statement', '?')}\n"
                f"- mechanism: {h.get('mechanism', '?')}\n"
                f"- probe: {h.get('probe', '?')}\n"
                f"- result: {h.get('result', '?')}\n"
                f"- verdict: {h.get('verdict', '?')}\n" + extra)
```
In `init_ledger`, add `"kills": [],` to the initial state dict (after `"banned_constructs": [],`).
In `apply_outcome`, after `st["session_count"] = n`, add:
```python
    kills = st.setdefault("kills", [])
    for h in o.get("hypotheses", []):
        if h.get("verdict") == "KILLED":
            kills.append({"session": n,
                          "statement": str(h.get("statement", ""))[:200],
                          "kill_scope": str(h.get("kill_scope", "instance")),
                          "measured_on": str(h.get("measured_on", ""))[:200],
                          "predicate_cite": str(h.get("predicate_cite", ""))[:120],
                          "result": str(h.get("result", ""))[:120]})
```

- [ ] **Step 5: Render the kill ledger and the re-audit trigger in the brief**

In `build_brief`, after the `scopes = ...` line added in Task 2, add:
```python
    kills = st.get("kills") or []
    inst = [k for k in kills if k.get("kill_scope") != "class"]
    cls = [k for k in kills if k.get("kill_scope") == "class"]
    floors_only = [e.get("floor") for e in st["floor_history"][-3:]]
    flat3 = (len(floors_only) == 3 and all(isinstance(f, int) for f in floors_only)
             and len(set(floors_only)) == 1)
    kill_block = ("\n## KILL LEDGER\n"
                  f"  instance kills (chassis-relative, RE-TESTABLE): {len(inst)}\n"
                  f"  class kills (predicate-cited, standing): {len(cls)}\n")
    if inst:
        kill_block += "  newest instance kills:\n" + "\n".join(
            f"    s{k['session']}: {k['statement'][:110]}\n"
            f"        measured on: {k['measured_on'][:110]}"
            for k in inst[-6:]) + "\n"
    if flat3 and inst:
        kill_block += (
            "\n## KILL RE-AUDIT REQUIRED (floor flat 3 sessions; instance kills exist)\n"
            "An instance kill is only as good as the chassis and FAKE state it was measured\n"
            "under. Before ANY new probe this session: pick the instance kill whose form sat\n"
            "closest to the target, and re-measure it (a) on the CURRENT chassis and (b) with\n"
            "every FAKE construct ablated (`python3 tools/fake_ablate.py --func " + func +
            " --file " + st['file'] + " --candidate <form.c>`). A lever measured 'inert' while\n"
            "a FAKE carrier occupied its target pseudo is not a kill (func_8002EA24 s8);\n"
            "a lever killed on 3 of 12 arms is not a class kill (func_800324D0 s6). Record\n"
            "the re-measurement as a hypothesis either way.\n")
```
and include `{kill_block}` in the returned f-string directly after `Floor history:\n{floors}\n`.

- [ ] **Step 6: Update the schema line in the brief**

In the returned f-string of `build_brief`, replace
```
  Schema: {{"result": "progress"|"candidate-ready"|"ruling-request"|"owner-gated", "floor": <int>,
  "headline": "<one line>", "hypotheses": [{{"statement","mechanism","probe","result","verdict":"CONFIRMED"|"KILLED"}}],
```
with
```
  Schema: {{"result": "progress"|"candidate-ready"|"ruling-request"|"owner-gated", "floor": <int>,
  "headline": "<one line>", "hypotheses": [{{"statement","mechanism","probe","result","verdict":"CONFIRMED"|"KILLED",
     "kill_scope":"instance"|"class" (KILLED only, REQUIRED), "measured_on":"<chassis + FAKE state>" (KILLED only, REQUIRED),
     "predicate_cite":"<file:line of the gate predicate>" (class kills only, REQUIRED)}}],
```
and after the `- "candidate-ready" means:` bullet add:
```
- KILL SCOPE IS MANDATORY. `instance` = "this form, on this chassis, with these FAKE constructs present, measured N" — the default, and re-testable. `class` = "every form fails predicate P" and needs `predicate_cite` as file:line. Wording like "unreachable", "any natural geometry", "all forms", "foreclosed", "by construction" on an instance kill makes the session INVALID: say what you measured, not what you inferred.
```

- [ ] **Step 7: Run the full grinder test suite**

Run: `python tools/grinder/tests/test_grindlib.py -v 2>&1 | tail -5`
Expected: `OK`. If a pre-existing test constructs a KILLED hypothesis without `kill_scope` (search the file for `"verdict": "KILLED"`), add `"kill_scope": "instance", "measured_on": "test chassis"` to that fixture; the `good()` helper in `TestValidateOutcome` is the one known site.

- [ ] **Step 8: Commit**

```bash
git add tools/grinder/grindlib.py tools/grinder/tests/test_grindlib.py
git commit -m "grinder: KILLED hypotheses declare kill_scope + measured_on; flat floor forces a kill re-audit

Post-mortem 2026-09-01: four of six 20+-session runs were prolonged by an
instance kill (confounded chassis / FAKE carrier / partial predicate)
cited as class law. Instance kills now record the chassis+FAKE state they
were measured under and are listed as re-testable; class kills must cite
the gate predicate file:line; class-claim wording on an instance kill
invalidates the session. state.json gains a structured kills[] list, and
a 3-session flat floor makes the brief demand a re-measurement of the
strongest instance kill before new probes.

Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01Dn2fdFSbP8sT54D4aRw2gW"
```

---

### Task 5: Playbook and role text: kill re-audit protocol, permuter chassis rule, foreclosure contradiction rule

Prose-only. No ladder code changes: the second permuter rung stays, but the playbook now forbids re-seeding the same chassis after a 0-find campaign and names the problem classes the permuter cannot express.

**Files:**
- Modify: `tools/grinder/grindlib.py` (`MODALITY_PLAYBOOK["permuter"]`, `MODALITY_PLAYBOOK["synthesis"]`, `MODALITY_PLAYBOOK["forensics"]`)
- Modify: `tools/grinder/roles/grind-session.md` (new subsection under `## Mechanics`)

- [ ] **Step 1: Extend the permuter playbook**

Append to the `"permuter"` string in `MODALITY_PLAYBOOK` (before the closing parenthesis):
```python
               "CHASSIS RULE (2026-09-01): if the ledger already banks a permuter campaign "
               "on the SAME candidate chassis with 0 novel finds after >=20k iterations, "
               "re-seeding it is not a probe — the session must permute a structurally "
               "different chassis (a banked instance-kill form is a good seed) or a "
               "different lever hint. The permuter cannot express N-way statement "
               "duplication into arms, goto-into-existing-body, chassis swaps, or "
               "FAKE-construct removal — if the frontier names one of those, spend the "
               "session on `tools/sweep_variants.py` over hand-written variants and "
               "`tools/fake_ablate.py`, and bank those measurements as the artifact."
```

- [ ] **Step 2: Extend the synthesis playbook**

Append to the `"synthesis"` string:
```python
                  " KILL RE-AUDIT (2026-09-01): list every instance kill in state.json "
                  "kills[] whose measured_on differs from the current chassis or names a "
                  "FAKE construct no longer present; re-measure the two closest-to-target "
                  "ones with tools/fake_ablate.py BEFORE proposing anything new. "
                  "CONTRADICTION RULE: if the ledger now marks EVERY chassis as foreclosed, "
                  "impossible, or dead, at least one of those verdicts is wrong — the "
                  "matching C exists. Re-audit the WEAKEST foreclosure (the one with no "
                  "predicate_cite, or the oldest) first; func_80041188 spent 14 sessions "
                  "proving one chassis impossible while the other sat foreclosed on an "
                  "incomplete loop.c predicate."
```

- [ ] **Step 3: Extend the forensics playbook**

Append to the `"forensics"` string:
```python
                  " PASS-INPUT ENUMERATION: naming the pass that produced the divergence "
                  "is half the job; the other half is enumerating the source-side INPUT "
                  "shapes that change what the pass sees (func_80038C70: 46 sessions on "
                  "how to stop find_cross_jump, when the closer was to give the block no "
                  "set-insn at all). Use tools/loop_movables.py (loop.c decisions, every "
                  "predicate term), tools/nrefs_census.py (reg_n_refs / allocno order + "
                  "what-if lifts), tools/label_census.py (branch targets, predecessors, "
                  "callee-saved ref counts, target vs build) and bank their output as "
                  "artifacts."
```

- [ ] **Step 4: Add the hypothesis-field guidance to `grind-session.md`**

Under `## Mechanics`, after the bullet that starts `- Judge constraints and BANNED CONSTRUCTS in your brief are mechanically binding`, add:
```
- KILLS CARRY THEIR CONDITIONS. Every `KILLED` hypothesis in your outcome
  JSON needs `kill_scope` and `measured_on`; a `class` kill also needs
  `predicate_cite` (file:line of the GCC predicate the whole class fails).
  The driver discards an outcome that uses class wording ("unreachable",
  "any natural geometry", "all forms", "foreclosed", "by construction") on
  an `instance` kill. State the arms you tried, the chassis, and which FAKE
  constructs were present. A lever measured with a FAKE carrier on the same
  pseudo, or on a partial instance (3 of 12 arms), is an INSTANCE result.
- WHEN EVERYTHING LOOKS DEAD, ONE VERDICT IS WRONG. The matching C exists.
  If the brief's KILL LEDGER shows a flat floor, re-measure before you
  re-derive: `tools/fake_ablate.py` on the closest banked form first.
```

- [ ] **Step 5: Run the tests and commit**

Run: `python tools/grinder/tests/test_grindlib.py 2>&1 | tail -3` → `OK`.
```bash
git add tools/grinder/grindlib.py tools/grinder/roles/grind-session.md
git commit -m "grinder: playbooks carry the kill re-audit, permuter chassis rule, and pass-input enumeration

Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01Dn2fdFSbP8sT54D4aRw2gW"
```

---

### Task 6: `sweep_variants.find_function_span` learns the `INCLUDE_ASM` representation

Since asm-until-matched, an INCOMPLETE function is `INCLUDE_ASM("asm/funcs", <func>);` on main. `find_function_span` raises on that, which blocks Task 7 and every session that wants to sweep hand-written variants from a clean tree.

**Files:**
- Modify: `tools/sweep_variants.py:51-134`
- Create: `tools/test_sweep_variants.py`

- [ ] **Step 1: Write the failing test**

Create `tools/test_sweep_variants.py`:
```python
#!/usr/bin/env python3
"""Unit tests for tools/sweep_variants.py span finder. Run: python tools/test_sweep_variants.py -v"""
import os, sys, unittest
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from tools.sweep_variants import find_function_span


class TestSpan(unittest.TestCase):
    def test_c_body(self):
        t = "s32 f(void);\n\ns32 f(void)\n{\n    return 1;\n}\n\nvoid g(void) { f(); }\n"
        s, e = find_function_span(t, "f")
        self.assertEqual(t[s:e], "s32 f(void)\n{\n    return 1;\n}\n")

    def test_include_asm_line(self):
        t = 'void a(void) {}\n\nINCLUDE_ASM("asm/funcs", func_80027640);\n\nvoid b(void) {}\n'
        s, e = find_function_span(t, "func_80027640")
        self.assertEqual(t[s:e], 'INCLUDE_ASM("asm/funcs", func_80027640);\n')

    def test_missing_raises(self):
        with self.assertRaises(ValueError):
            find_function_span("int x;\n", "nope")


if __name__ == "__main__":
    unittest.main()
```

- [ ] **Step 2: Run to verify it fails**

Run: `python tools/test_sweep_variants.py -v 2>&1 | tail -6`
Expected: `test_include_asm_line` fails with `ValueError: definition of func_80027640 not found`.

- [ ] **Step 3: Implement**

In `tools/sweep_variants.py`, replace the final line of `find_function_span`
```python
    raise ValueError(f"definition of {func} not found")
```
with
```python
    # asm-until-matched (owner ruling 2026-08-19): an INCOMPLETE function is
    # committed as INCLUDE_ASM("asm/funcs", <func>); — splice candidates over
    # that line so variants can be swept from a clean tree.
    inc = re.search(r'(?m)^[ \t]*INCLUDE_ASM\s*\(\s*"asm/funcs"\s*,\s*'
                    + re.escape(func) + r'\s*\)\s*;[ \t]*\n?', text)
    if inc:
        return inc.start(), inc.end()
    raise ValueError(f"definition of {func} not found")
```

- [ ] **Step 4: Run the test**

Run: `python tools/test_sweep_variants.py -v 2>&1 | tail -3` → `OK`.

- [ ] **Step 5: Commit**

```bash
git add tools/sweep_variants.py tools/test_sweep_variants.py
git commit -m "tools: sweep_variants splices over the INCLUDE_ASM line for asm-until-matched functions

Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01Dn2fdFSbP8sT54D4aRw2gW"
```

---

### Task 7: `tools/fake_ablate.py` — score a form across the grid of FAKE constructs removed

func_8002EA24 recorded its winning lever "inert" in s8 because the L3 FAKE carrier occupied the target pseudo; the sum-split applied to the no-L3 control scores 0. This tool takes a candidate function definition, finds every FAKE-annotated construct, generates the removal grid, and scores every variant with `sweep_variants.py`.

**Files:**
- Create: `tools/fake_ablate.py`
- Create: `tools/test_fake_ablate.py`

- [ ] **Step 1: Write the failing tests**

Create `tools/test_fake_ablate.py`:
```python
#!/usr/bin/env python3
"""Unit tests for tools/fake_ablate.py (pure text logic; no build). Run: python tools/test_fake_ablate.py -v"""
import os, sys, unittest
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from tools.fake_ablate import find_fake_units, remove_units, ablation_masks

BODY = """s32 f(s32 a)
{
    s32 three = 3; /* FAKE: constant holder, mechanism: global.c, lever-exhaustion: s4 */
    s32 r;
    do { /* FAKE: wrap, mechanism: reorg.c LABEL_OUTSIDE_LOOP_P, lever-exhaustion: s6 */
        r = a * three;
    } while (0);
    r = r; /* FAKE: dead store, mechanism: flow.c, lever-exhaustion: s7 */
    return r;
}
"""


class TestUnits(unittest.TestCase):
    def test_finds_three_units_with_kinds(self):
        units = find_fake_units(BODY)
        self.assertEqual([u.kind for u in units], ["line", "wrap", "line"])
        self.assertEqual(units[0].lines, [2])
        self.assertEqual(units[1].lines, [4, 6])      # `do {` and `} while (0);`
        self.assertEqual(units[2].lines, [7])

    def test_remove_wrap_keeps_body(self):
        units = find_fake_units(BODY)
        out = remove_units(BODY, [units[1]])
        self.assertNotIn("do {", out)
        self.assertNotIn("while (0)", out)
        self.assertIn("r = a * three;", out)

    def test_remove_line(self):
        units = find_fake_units(BODY)
        out = remove_units(BODY, [units[2]])
        self.assertNotIn("r = r;", out)
        self.assertIn("return r;", out)

    def test_masks_full_grid_small(self):
        self.assertEqual(len(ablation_masks(3)), 8)
        self.assertIn((True, True, True), ablation_masks(3))

    def test_masks_capped(self):
        m = ablation_masks(6)
        self.assertIn(tuple([True] * 6), m)               # all removed
        self.assertEqual(sum(1 for x in m if sum(x) == 1), 6)   # each singleton
        self.assertLessEqual(len(m), 1 + 6 + 15 + 1)       # none, singles, pairs, all


if __name__ == "__main__":
    unittest.main()
```

- [ ] **Step 2: Run to verify it fails**

Run: `python tools/test_fake_ablate.py 2>&1 | tail -3`
Expected: `ModuleNotFoundError: No module named 'tools.fake_ablate'`.

- [ ] **Step 3: Implement `tools/fake_ablate.py`**

```python
#!/usr/bin/env python3
"""FAKE-ablation sweep — score one candidate form across the grid of its
FAKE-annotated constructs removed.

WHY (post-mortem 2026-09-01): func_8002EA24 recorded its winning lever
"inert" in s8 because a /* FAKE */ carrier occupied the pseudo the lever
targeted; the same lever applied to the no-carrier control scores 0.
func_800324D0's staged read diverted a reference lift to the wrong web the
same way. A lever may not be called dead while a FAKE construct sits on the
register it is trying to move — so measure every lever on every subset of
the FAKE constructs, in one call.

Usage (WSL, venv active, repo root):
  python3 tools/fake_ablate.py --func <func> --file <stem> --candidate <form.c> [--json] [--keep]

<form.c> is a COMPLETE function definition (the memory/grind/<func>/candidate.c
shape). Variants are written under tmp/grind/<func>/ablate/ and scored with
tools/sweep_variants.py (which restores src/<stem>.c byte-exact afterwards).
The grid: all 2^n subsets for n <= 4 FAKE units; for n > 4: none, every
singleton, every pair, and all-removed. A variant that fails to compile
(e.g. a removed holder still referenced) is reported as ERR, not skipped.

Pure-C policy note: removing a FAKE construct can only make a form MORE
honest; this tool never adds anything. A score-0 variant is still a PROPOSAL
and goes through the self-vet, layer-1, and Judge like any candidate.
"""
from __future__ import annotations

import argparse
import itertools
import json
import re
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
FAKE_RE = re.compile(r"/\*\s*FAKE\b|//\s*FAKE\b")
DO_RE = re.compile(r"^\s*do\s*\{")
WHILE0_RE = re.compile(r"^\s*\}\s*while\s*\(\s*0\s*\)\s*;")


@dataclass
class Unit:
    kind: str          # "line" | "wrap"
    lines: list        # 0-based line indexes removed when the unit is ablated
    text: str          # first line, for reports


def _match_while0(lines: list[str], do_idx: int) -> int | None:
    """Index of the `} while (0);` closing the `do {` at do_idx (brace depth)."""
    depth = 0
    for i in range(do_idx, len(lines)):
        depth += lines[i].count("{") - lines[i].count("}")
        if i > do_idx and depth == 0 and WHILE0_RE.match(lines[i]):
            return i
        if i > do_idx and depth <= 0 and not WHILE0_RE.match(lines[i]):
            return None
    return None


def find_fake_units(text: str) -> list[Unit]:
    lines = text.split("\n")
    units: list[Unit] = []
    for i, ln in enumerate(lines):
        if not FAKE_RE.search(ln):
            continue
        if DO_RE.match(ln):
            j = _match_while0(lines, i)
            if j is not None:
                units.append(Unit("wrap", [i, j], ln.strip()))
                continue
        units.append(Unit("line", [i], ln.strip()))
    return units


def remove_units(text: str, units: list[Unit]) -> str:
    drop = {i for u in units for i in u.lines}
    lines = text.split("\n")
    return "\n".join(ln for i, ln in enumerate(lines) if i not in drop)


def ablation_masks(n: int) -> list[tuple]:
    if n <= 4:
        return list(itertools.product([False, True], repeat=n))
    masks = {tuple([False] * n), tuple([True] * n)}
    for k in (1, 2):
        for combo in itertools.combinations(range(n), k):
            masks.add(tuple(i in combo for i in range(n)))
    return sorted(masks, key=lambda m: (sum(m), m))


def mask_name(mask: tuple) -> str:
    return "keep-all" if not any(mask) else "drop-" + "".join("1" if b else "0" for b in mask)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--func", required=True)
    ap.add_argument("--file", required=True, help="src stem")
    ap.add_argument("--candidate", required=True, help="complete function definition to ablate")
    ap.add_argument("--json", action="store_true")
    ap.add_argument("--keep", action="store_true", help="keep the variant files after scoring")
    a = ap.parse_args()

    text = Path(a.candidate).read_text(encoding="utf-8", errors="replace")
    units = find_fake_units(text)
    if not units:
        print(f"no FAKE-annotated constructs found in {a.candidate}; nothing to ablate")
        return 0
    outdir = ROOT / "tmp" / "grind" / a.func / "ablate"
    outdir.mkdir(parents=True, exist_ok=True)
    for old in outdir.glob("*.c"):
        old.unlink()
    plan = []
    for mask in ablation_masks(len(units)):
        removed = [u for u, b in zip(units, mask) if b]
        p = outdir / f"{mask_name(mask)}.c"
        p.write_text(remove_units(text, removed), encoding="utf-8", newline="\n")
        plan.append({"variant": str(p), "removed": [u.text for u in removed]})

    r = subprocess.run([sys.executable, "tools/sweep_variants.py", "--func", a.func,
                        "--file", a.file, "--variants", str(outdir), "--json"],
                       capture_output=True, text=True, cwd=ROOT)
    try:
        sweep = json.loads(r.stdout)
    except json.JSONDecodeError:
        print("sweep_variants produced no JSON:\n" + (r.stdout + r.stderr)[-1500:], file=sys.stderr)
        return 1
    scores = {rec["variant"]: rec for rec in sweep["results"]}
    rows = []
    for item in plan:
        rec = scores.get(item["variant"], {})
        rows.append({"variant": Path(item["variant"]).name, "score": rec.get("score"),
                     "build_insns": rec.get("build_insns"), "removed": item["removed"],
                     "error": rec.get("error")})
    rows.sort(key=lambda x: (x["score"] is None, x["score"] if x["score"] is not None else 0))
    if a.json:
        print(json.dumps({"func": a.func, "file": a.file,
                          "units": [{"kind": u.kind, "lines": u.lines, "text": u.text} for u in units],
                          "results": rows}, indent=2))
    else:
        print(f"FAKE units in {a.candidate}: {len(units)}")
        for k, u in enumerate(units):
            print(f"  [{k}] {u.kind:4s} L{u.lines[0] + 1}: {u.text[:100]}")
        print("\nscore  bi    variant        removed")
        for x in rows:
            sc = " ERR" if x["score"] is None else f"{x['score']:4d}"
            print(f"{sc}  {str(x['build_insns']):>4}  {x['variant']:<14} "
                  f"{'; '.join(t[:40] for t in x['removed']) or '(none)'}")
        best = next((x for x in rows if x["score"] is not None), None)
        base = next((x for x in rows if x["variant"] == "keep-all.c"), None)
        if best and base and base["score"] is not None and best["score"] < base["score"]:
            print(f"\nABLATION WIN: {best['variant']} scores {best['score']} vs keep-all {base['score']} "
                  "— a FAKE construct was masking this form. Bank the winning variant.")
    if not a.keep:
        for p in outdir.glob("*.c"):
            p.unlink()
    return 0


if __name__ == "__main__":
    sys.exit(main())
```

- [ ] **Step 4: Run the unit tests**

Run: `python tools/test_fake_ablate.py -v 2>&1 | tail -3` → `OK`.

- [ ] **Step 5: Live smoke (WSL)**

Run under WSL with the venv active (write `tmp/ablate_smoke.sh` containing the two lines and run `bash tools/wsl.sh 'bash tmp/ablate_smoke.sh'`):
```bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile" && source .venv/bin/activate
python3 tools/fake_ablate.py --func func_8003C714 --file code6cac_c2 --candidate memory/grind/func_8003C714/rejected/balanced-biv-noise-64-insns-d0-but-inadmissible.c
```
Expected: a table with `keep-all.c` scoring 0 (that file is the banked d0 form) and at least one drop variant scoring higher; `src/code6cac_c2.c` unchanged afterwards (`git status --short src/` empty). If the ledger for func_8003C714 has been deleted by then (function completed or foreclosed with ledger retained), substitute any `memory/grind/<func>/candidate.c` that contains `FAKE`.

- [ ] **Step 6: Commit**

```bash
git add tools/fake_ablate.py tools/test_fake_ablate.py
git commit -m "tools: fake_ablate.py — score a form across every subset of its FAKE constructs

Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01Dn2fdFSbP8sT54D4aRw2gW"
```

---

### Task 8: `tools/loop_movables.py` — every term of every loop.c movable decision

func_80041188 foreclosed the winning chassis on an incomplete predicate (it never listed `n_times_set == 1`); func_8003C714 spent eight sessions on one `move_movables` inequality. The `.loop` dump already prints `Loop from A to B: N real insns.` and one line per movable (`Insn U: regno R (life L), [consec C, ][cond ][force ][global ][done ][move-insn ][matches M ][forces F ]savings S [halved since already moved ] moved to X | not desirable | not safe`). The threshold and `n_times_set` are not printed; the tool derives them: threshold from `loop_has_call` (a `call_insn` inside the loop's UID range in the dump RTL) and `n_non_fixed_regs` (60 on this hard-float chassis); `n_times_set` by counting sets of each pseudo inside the loop range in the pre-loop `.cse` dump. Both derivations are labelled as such in the output.

**Files:**
- Create: `tools/loop_movables.py`
- Create: `tools/test_loop_movables.py`

- [ ] **Step 1: Write the failing tests**

Create `tools/test_loop_movables.py`:
```python
#!/usr/bin/env python3
"""Unit tests for tools/loop_movables.py parsers. Run: python tools/test_loop_movables.py -v"""
import os, sys, unittest
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from tools.loop_movables import parse_loops, count_sets_in_range, function_segment

LOOP_DUMP = """;; Function func_A

Loop from 12 to 88: 56 real insns.
Insn 20: regno 74 (life 1), savings 1  moved to 9
Insn 31: regno 77 (life 1), consec 1, move-insn savings 1  moved to 10
Insn 40: regno 80 (life 15), savings 1 not desirable
Insn 45: regno 81 (life 2), cond not safe

;; Function func_B

Loop from 5 to 9 is phony.

"""

CSE_RTL = """(note 12 11 13 "" NOTE_INSN_LOOP_BEG)
(insn 20 13 21 (set (reg:SI 74) (const_int 42)) -1 (nil) (nil))
(insn 22 21 23 (set (reg:SI 80) (plus:SI (reg:SI 80) (const_int 1))) -1 (nil) (nil))
(insn 24 23 25 (set (reg:SI 80) (const_int 7)) -1 (nil) (nil))
(call_insn 30 25 31 (call (mem:SI (symbol_ref:SI ("f"))) (const_int 0)) -1 (nil) (nil))
(note 88 87 89 "" NOTE_INSN_LOOP_END)
"""


class TestParsers(unittest.TestCase):
    def test_function_segment(self):
        seg = function_segment(LOOP_DUMP, "func_A")
        self.assertIn("56 real insns", seg)
        self.assertNotIn("phony", seg)

    def test_parse_loops(self):
        loops = parse_loops(function_segment(LOOP_DUMP, "func_A"))
        self.assertEqual(len(loops), 1)
        lp = loops[0]
        self.assertEqual((lp["start"], lp["end"], lp["insn_count"]), (12, 88, 56))
        self.assertEqual(len(lp["movables"]), 4)
        m0, m1, m2, m3 = lp["movables"]
        self.assertEqual((m0["insn"], m0["regno"], m0["life"], m0["savings"], m0["decision"]),
                         (20, 74, 1, 1, "moved"))
        self.assertEqual(m1["consec"], 1)
        self.assertTrue(m1["move_insn"])
        self.assertEqual(m2["decision"], "not desirable")
        self.assertTrue(m3["cond"])
        self.assertEqual(m3["decision"], "not safe")

    def test_count_sets_and_calls(self):
        sets, has_call = count_sets_in_range(CSE_RTL, 12, 88)
        self.assertEqual(sets[74], 1)
        self.assertEqual(sets[80], 2)
        self.assertTrue(has_call)


if __name__ == "__main__":
    unittest.main()
```

- [ ] **Step 2: Run to verify it fails**

Run: `python tools/test_loop_movables.py 2>&1 | tail -3` → `ModuleNotFoundError`.

- [ ] **Step 3: Implement `tools/loop_movables.py`**

```python
#!/usr/bin/env python3
"""loop.c movable-decision report — every term of `move_movables`' inequality
for every movable in every loop of one function, from the -da dumps.

WHY (post-mortem 2026-09-01): func_80041188 foreclosed its winning chassis
on an incomplete reading of scan_loop's movable test (it never enumerated
`n_times_set == 1`); func_8003C714 spent eight sessions on the single
inequality at loop.c:1631. The dump already carries most terms — this tool
lines them up and names the C-level lever for each.

    threshold * savings * lifetime >= insn_count   (loop.c:1631, moved if true)
    threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)    (loop.c:532)

Usage (WSL, venv active, repo root):
  pwsh tools/grinder/dump.ps1 <func>        # produces tmp/grind/<func>/dumps/<stem>.loop and .cse
  python3 tools/loop_movables.py --func <func> --file <stem> [--soft-float] [--json]

Derived (not printed by cc1) and labelled as such in the output:
  * loop_has_call — a call_insn whose UID lies inside the loop's [start,end]
    range in the .loop dump RTL.
  * n_non_fixed_regs — 60 on the shipped hard-float chassis (ledger
    func_8003C714 s1/s8), 28 with --soft-float. CC_FLAGS is not a lever
    (no-compiler-divergence); the flag exists only to reproduce a ledger
    measurement.
  * n_times_set[regno] — count of `(set (reg:M R) ...)`/`(clobber (reg:M R))`
    in the .cse dump within the loop range (count_loop_regs_set also counts
    SUBREG/STRICT_LOW_PART destinations; those are folded in here).
  * threshold decrement — loop.c:1719/1904 subtract 3 after each moved
    movable; the report shows the RUNNING threshold each movable faced.
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
LOOP_HDR = re.compile(r"^Loop from (\d+) to (\d+): (\d+) real insns\.", re.M)
MOVABLE = re.compile(
    r"^Insn (\d+): regno (\d+) \(life (\d+)\), "
    r"(?:consec (\d+), )?(?P<flags>(?:(?:cond|force|global|done|move-insn) )*)"
    r"(?:matches (\d+) )?(?:forces (\d+) )?"
    r"(?:savings (\d+) )?(?P<halved>halved since already moved )?"
    r"(?P<decision>moved to (\d+)|not desirable|not safe)?", re.M)
SET_RE = re.compile(r"\((?:insn|call_insn) (\d+) \d+ \d+ (.*?)\) -1", re.S)
DEST_RE = re.compile(r"\((?:set|clobber) \((?:subreg:\w+ )?\((?:strict_low_part )?\(?reg:\w+ (\d+)\)?")

LEVERS = {
    "savings": "loop.c:1596-1604 — 1 per insn moved, +1 for consec chains; a CONST_INT src is 1",
    "lifetime": "life = uid_luid span of the reg's uses; a second in-loop USE of the pseudo raises it",
    "insn_count": "count_loop_regs_set counts insns BEFORE loop_optimize deletes them: loop-carried "
                  "arithmetic that biv elimination later removes still counts (func_8003C714 s6)",
    "threshold": "loop.c:532 (loop_has_call halves it; a call in the loop is the only C-level input) "
                 "then -3 per movable already moved (loop.c:1719/1904)",
    "n_times_set": "scan_loop admits a movable only if n_times_set[dest]==1 (loop.c:705/740): a second "
                   "in-loop store to the same variable (even a dead one) removes the movable entirely "
                   "(func_80041188 s37 closer)",
    "cond": "conditionally executed — only movable if maybe_never allows; loop.c:760",
    "force/global": "REG_EQUIV/global effects — see loop.c:849-898",
}


def function_segment(dump: str, func: str) -> str:
    m = re.search(r"^;; Function " + re.escape(func) + r"\b.*?$", dump, re.M)
    if not m:
        return dump
    rest = dump[m.end():]
    n = re.search(r"^;; Function ", rest, re.M)
    return rest[:n.start()] if n else rest


def parse_loops(seg: str) -> list[dict]:
    loops = []
    heads = list(LOOP_HDR.finditer(seg))
    for k, h in enumerate(heads):
        end = heads[k + 1].start() if k + 1 < len(heads) else len(seg)
        block = seg[h.end():end]
        movables = []
        for m in MOVABLE.finditer(block):
            flags = m.group("flags") or ""
            dec = m.group("decision") or ""
            movables.append({
                "insn": int(m.group(1)), "regno": int(m.group(2)), "life": int(m.group(3)),
                "consec": int(m.group(4) or 0),
                "cond": "cond" in flags, "force": "force" in flags, "global": "global" in flags,
                "done": "done" in flags, "move_insn": "move-insn" in flags,
                "matches": int(m.group(5)) if m.group(5) else None,
                "forces": int(m.group(6)) if m.group(6) else None,
                "savings": int(m.group(7)) if m.group(7) else None,
                "halved": bool(m.group("halved")),
                "decision": "moved" if dec.startswith("moved") else (dec or "(no decision line)"),
                "moved_to": int(m.group(9)) if dec.startswith("moved") else None,
            })
        loops.append({"start": int(h.group(1)), "end": int(h.group(2)),
                      "insn_count": int(h.group(3)), "movables": movables})
    return loops


def count_sets_in_range(rtl: str, start: int, end: int) -> tuple[dict, bool]:
    """(n_times_set approximation per regno, loop_has_call) over insns whose UID
    lies within [start, end]."""
    sets: dict[int, int] = {}
    has_call = False
    for m in SET_RE.finditer(rtl):
        uid = int(m.group(1))
        if uid < start or uid > end:
            continue
        if m.group(0).startswith("(call_insn"):
            has_call = True
        for d in DEST_RE.finditer(m.group(2)):
            r = int(d.group(1))
            sets[r] = sets.get(r, 0) + 1
    return sets, has_call


def report(func: str, stem: str, soft_float: bool) -> dict:
    dumps = ROOT / "tmp" / "grind" / func / "dumps"
    loop_p = dumps / f"{stem}.loop"
    cse_p = dumps / f"{stem}.cse"
    if not loop_p.exists():
        raise SystemExit(f"{loop_p} missing — run: pwsh tools/grinder/dump.ps1 {func}")
    seg = function_segment(loop_p.read_text(errors="replace"), func)
    cse_seg = function_segment(cse_p.read_text(errors="replace"), func) if cse_p.exists() else ""
    n_non_fixed = 28 if soft_float else 60
    out = {"func": func, "stem": stem, "n_non_fixed_regs": n_non_fixed, "loops": []}
    for lp in parse_loops(seg):
        sets, has_call = count_sets_in_range(cse_seg or seg, lp["start"], lp["end"])
        thr0 = (1 if has_call else 2) * (1 + n_non_fixed)
        thr = thr0
        rows = []
        for m in lp["movables"]:
            ic = lp["insn_count"] * (2 if m["halved"] else 1)
            lhs = (thr * (m["savings"] or 0) * m["life"]) if m["savings"] is not None else None
            rows.append({**m, "n_times_set": sets.get(m["regno"]), "threshold_faced": thr,
                         "insn_count_faced": ic, "lhs": lhs,
                         "inequality": (f"{thr}*{m['savings']}*{m['life']}={lhs} >= {ic}"
                                        if lhs is not None else "(no savings line)")})
            if m["decision"] == "moved":
                thr -= 3
        out["loops"].append({**lp, "loop_has_call": has_call, "threshold_initial": thr0,
                             "movables": rows, "in_loop_sets": sets})
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--func", required=True)
    ap.add_argument("--file", required=True, help="src stem")
    ap.add_argument("--soft-float", action="store_true", help="reproduce a -msoft-float measurement (diagnostic only)")
    ap.add_argument("--json", action="store_true")
    a = ap.parse_args()
    r = report(a.func, a.file, a.soft_float)
    if a.json:
        print(json.dumps(r, indent=2))
        return 0
    print(f"loop.c movable report — {a.func} (n_non_fixed_regs={r['n_non_fixed_regs']} [derived])")
    for lp in r["loops"]:
        print(f"\nLoop insns {lp['start']}..{lp['end']}: insn_count={lp['insn_count']} "
              f"loop_has_call={lp['loop_has_call']} [derived] threshold={lp['threshold_initial']} [derived]")
        print("  insn regno life consec nset  sav  thr  cnt   inequality                 decision")
        for m in lp["movables"]:
            print(f"  {m['insn']:>4} {m['regno']:>5} {m['life']:>4} {m['consec']:>6} "
                  f"{str(m['n_times_set']):>4} {str(m['savings']):>4} {m['threshold_faced']:>4} "
                  f"{m['insn_count_faced']:>4}   {m['inequality']:<26} {m['decision']}"
                  + (" [cond]" if m["cond"] else "") + (" [move-insn]" if m["move_insn"] else ""))
        extra = {r_: n for r_, n in lp["in_loop_sets"].items() if n > 1}
        if extra:
            print(f"  pseudos set >1 time in loop (NOT admissible as movables): {extra}")
    print("\nLevers per term:")
    for k, v in LEVERS.items():
        print(f"  {k:12s} {v}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
```

- [ ] **Step 4: Run the unit tests**

Run: `python tools/test_loop_movables.py -v 2>&1 | tail -3` → `OK`.

- [ ] **Step 5: Live smoke (WSL + PowerShell)**

Run: `pwsh tools/grinder/dump.ps1 func_8003C714` (or the current queue top that contains a loop), then under WSL `python3 tools/loop_movables.py --func func_8003C714 --file code6cac_c2`.
Expected: at least one loop block whose movable table shows the `0x91A2B3C5`-holding pseudo with `decision moved` and an inequality like `119*1*1=119 >= 56` (numbers will vary by chassis). If the function is INCLUDE_ASM on main, the loop list is empty; apply `memory/grind/<func>/candidate.c` via `tools/sweep_variants.py --apply-best` semantics is NOT appropriate — instead temporarily splice the candidate with `python3 tools/sweep_variants.py --func <f> --file <stem> --variants memory/grind/<f>/candidate.c` is also a scoring run; for the smoke, copy the candidate body over the INCLUDE_ASM line by hand, run `dump.ps1`, run the tool, then `git checkout -- src/`.

- [ ] **Step 6: Commit**

```bash
git add tools/loop_movables.py tools/test_loop_movables.py
git commit -m "tools: loop_movables.py — every term of every loop.c movable decision from the -da dumps

Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01Dn2fdFSbP8sT54D4aRw2gW"
```

---

### Task 9: `tools/nrefs_census.py` — reg_n_refs / allocno order + what-if lift calculator

func_800324D0 had the mechanism in s6 and the threshold in s20; nobody joined them. This tool prints, per pseudo, `nrefs`, `livelen`, priority, allocation order, and the first defining insn, then answers "how many extra references does pseudo P need to rank above pseudo Q" and re-sorts under a hypothetical lift. It reuses `tools/ra_solver/extract.py` (`run_dumps`, `split_allocdbg`, `split_flow`, `parse_flow_regs`, `ent_order`).

**Files:**
- Create: `tools/nrefs_census.py`
- Create: `tools/test_nrefs_census.py`

- [ ] **Step 1: Write the failing tests**

Create `tools/test_nrefs_census.py`:
```python
#!/usr/bin/env python3
"""Unit tests for tools/nrefs_census.py (pure logic). Run: python tools/test_nrefs_census.py -v"""
import os, sys, unittest
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from tools.nrefs_census import priority, reorder, min_lift_to_beat, first_def_insn

ROWS = [
    {"ord": 0, "pseudo": 76, "hardreg": 2, "nrefs": 40, "livelen": 30, "pri": 0},
    {"ord": 1, "pseudo": 73, "hardreg": 16, "nrefs": 24, "livelen": 60, "pri": 0},
]
RTL = """(insn 5 4 6 (set (reg:SI 73) (mem:SI (reg:SI 70))) -1 (nil) (nil))
(insn 9 8 10 (set (reg:SI 76) (plus:SI (reg:SI 73) (const_int 4))) -1 (nil) (nil))
"""


class TestCensus(unittest.TestCase):
    def test_priority_formula_matches_global_c(self):
        # global.c:615 — floor(log2(nrefs)) * nrefs / livelen * 10000 * size
        self.assertEqual(priority(40, 30, 1), int((5 * 40 / 30) * 10000 * 1))
        self.assertEqual(priority(24, 60, 1), int((4 * 24 / 60) * 10000 * 1))

    def test_reorder_by_priority(self):
        order = [r["pseudo"] for r in reorder(ROWS)]
        self.assertEqual(order, [76, 73])

    def test_min_lift(self):
        k = min_lift_to_beat(ROWS, 73, 76)
        self.assertGreater(k, 0)
        lifted = [dict(r, nrefs=r["nrefs"] + (k if r["pseudo"] == 73 else 0)) for r in ROWS]
        self.assertEqual([r["pseudo"] for r in reorder(lifted)][0], 73)
        lifted2 = [dict(r, nrefs=r["nrefs"] + (k - 1 if r["pseudo"] == 73 else 0)) for r in ROWS]
        self.assertEqual([r["pseudo"] for r in reorder(lifted2)][0], 76)

    def test_first_def_insn(self):
        self.assertIn("(mem:SI (reg:SI 70))", first_def_insn(RTL, 73))
        self.assertIn("plus:SI", first_def_insn(RTL, 76))
        self.assertEqual(first_def_insn(RTL, 99), "")


if __name__ == "__main__":
    unittest.main()
```

- [ ] **Step 2: Run to verify it fails**

Run: `python tools/test_nrefs_census.py 2>&1 | tail -3` → `ModuleNotFoundError`.

- [ ] **Step 3: Implement `tools/nrefs_census.py`**

```python
#!/usr/bin/env python3
"""reg_n_refs / allocno-order census with a what-if lift calculator.

WHY (post-mortem 2026-09-01): func_800324D0 named the mechanism in s6
(duplicated tail statements raise the walker's reg_n_refs before
allocno_compare ranks it; cross-jump re-merges them for free) and priced the
threshold in s20 (nrefs ~50), but nobody joined the two until s22. This
tool prints the allocation order with every input, and answers "how many
extra references does P need to rank above Q".

Usage (WSL, venv active, repo root):
  python3 tools/nrefs_census.py --func <func> --file <stem>                 # census
  python3 tools/nrefs_census.py --func <func> --file <stem> --above 73:76   # min lift for 73 to beat 76
  python3 tools/nrefs_census.py --func <func> --file <stem> --lift 73=+72   # re-sort under a hypothetical lift

Inputs come from the INSTRUMENTED cc1 (BB2_ALLOC_DEBUG, tools/gcc-2.7.2/cc1)
via tools/ra_solver/extract.run_dumps — read-only, work files under
tmp/ra_solver_work/. Priority formula is global.c:615 exactly:
    floor(log2(nrefs)) * nrefs / livelen * 10000 * size
`size` is 1 for SImode pseudos and 2 for DImode; the census assumes 1 unless
the pseudo's first def shows DI/DF mode.

Free reference lifts (the sanctioned duplicated-statement-into-arms family,
2026-07-01) raise nrefs by (arms-1) * refs-per-statement with zero emitted
bytes IF cross-jump re-merges the tails; measure with sweep_variants after
using this calculator — the calculator says how many, not whether it is free.
"""
from __future__ import annotations

import argparse
import math
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))
from tools.ra_solver import extract as X  # noqa: E402


def priority(nrefs: int, livelen: int, size: int = 1) -> int:
    if nrefs <= 0 or livelen <= 0:
        return 0
    return int(((math.floor(math.log2(nrefs)) * nrefs) / livelen) * 10000 * size)


def reorder(rows: list[dict]) -> list[dict]:
    """global.c allocno_compare: higher priority first; ties keep input order
    (qsort is not stable, so ties are reported as ties, not resolved)."""
    keyed = [(priority(r["nrefs"], r["livelen"], r.get("size", 1)), -i, r) for i, r in enumerate(rows)]
    keyed.sort(key=lambda t: (t[0], t[1]), reverse=True)
    return [dict(r, pri_calc=p) for p, _, r in keyed]


def min_lift_to_beat(rows: list[dict], p: int, q: int, cap: int = 4096) -> int:
    rp = next(r for r in rows if r["pseudo"] == p)
    rq = next(r for r in rows if r["pseudo"] == q)
    target = priority(rq["nrefs"], rq["livelen"], rq.get("size", 1))
    for k in range(0, cap):
        if priority(rp["nrefs"] + k, rp["livelen"], rp.get("size", 1)) > target:
            return k
    return -1


def first_def_insn(rtl: str, pseudo: int) -> str:
    m = re.search(r"\((?:insn|call_insn) \d+ \d+ \d+ \((?:set|clobber) \(reg:\w+ %d\) .*?\) -1" % pseudo,
                  rtl, re.S)
    return " ".join(m.group(0).split())[:160] if m else ""


def _mode_size(rtl: str, pseudo: int) -> int:
    m = re.search(r"\(reg:(\w+) %d\)" % pseudo, rtl)
    return 2 if m and m.group(1) in ("DI", "DF") else 1


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--func", required=True)
    ap.add_argument("--file", required=True, help="src stem")
    ap.add_argument("--above", help="P:Q — minimum nrefs lift for pseudo P to rank above Q")
    ap.add_argument("--lift", action="append", default=[], help="P=+K — re-sort with pseudo P lifted by K (repeatable)")
    a = ap.parse_args()

    d = X.run_dumps(a.file)
    dbg = X.split_allocdbg(d["stderr"])
    rows = dbg["rows"].get(a.func)
    if not rows:
        print(f"no ALLOCDBG rows for {a.func} — is the instrumented cc1 at tools/gcc-2.7.2/cc1 and "
              "the function a C body in src/ (not INCLUDE_ASM)?", file=sys.stderr)
        return 1
    ents = X.ent_order(d["asm"])
    fi = ents.index(a.func) if a.func in ents else -1
    flow_seg = X.split_flow(d["flow"])[fi] if fi >= 0 and fi < len(X.split_flow(d["flow"])) else ""
    flow_regs = X.parse_flow_regs(flow_seg) if flow_seg else {}
    greg_segs = X.split_greg(d["greg"])
    rtl = greg_segs[fi] if 0 <= fi < len(greg_segs) else d["greg"]
    for r in rows:
        r["size"] = _mode_size(rtl, r["pseudo"])
        r["calls_crossed"] = flow_regs.get(r["pseudo"], {}).get("calls_crossed")
        r["first_def"] = first_def_insn(rtl, r["pseudo"])

    print(f"allocno census — {a.func}  (order = global.c allocno_compare; pri = global.c:615)")
    print("  ord pseudo hard nrefs livelen  pri(hook) pri(calc) xcalls  first def")
    for r in reorder(rows):
        print(f"  {r['ord']:>3} {r['pseudo']:>6} {r['hardreg']:>4} {r['nrefs']:>5} {r['livelen']:>7} "
              f"{r['pri']:>10} {r['pri_calc']:>9} {str(r['calls_crossed']):>6}  {r['first_def'][:70]}")

    if a.above:
        p, q = (int(x) for x in a.above.split(":"))
        k = min_lift_to_beat(rows, p, q)
        rp = next(r for r in rows if r["pseudo"] == p)
        print(f"\nlift needed: pseudo {p} (nrefs {rp['nrefs']}) must gain +{k} refs to rank above pseudo {q}"
              if k >= 0 else f"\npseudo {p} cannot out-rank {q} by nrefs alone within 4096 refs")
        print("  free-lift arithmetic: each arm that duplicates a statement with R refs to the pseudo adds R;\n"
              f"  N arms x R refs per statement >= {k}  (e.g. 12 arms x 6 refs = 72)")
    if a.lift:
        lifted = [dict(r) for r in rows]
        for spec in a.lift:
            p, k = spec.split("=")
            for r in lifted:
                if r["pseudo"] == int(p):
                    r["nrefs"] += int(k.replace("+", ""))
        print("\nre-sorted under lift " + ", ".join(a.lift) + ":")
        for r in reorder(lifted):
            print(f"  pseudo {r['pseudo']:>4}  nrefs {r['nrefs']:>4}  pri {r['pri_calc']:>9}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
```

- [ ] **Step 4: Run the unit tests**

Run: `python tools/test_nrefs_census.py -v 2>&1 | tail -3` → `OK`.

- [ ] **Step 5: Live smoke (WSL)**

Pick any COMPLETED-C function with several pseudos (e.g. `func_80027640`'s TU `code6cac_b` after its match, or `sys_VSync` in `ings2`). Run `python3 tools/nrefs_census.py --func <f> --file <stem> --above <p>:<q>` for two pseudos printed in the census.
Expected: a table with `pri(hook)` equal to `pri(calc)` on every row (this validates the formula against the hook), and a lift number.

- [ ] **Step 6: Commit**

```bash
git add tools/nrefs_census.py tools/test_nrefs_census.py
git commit -m "tools: nrefs_census.py — allocno order with every input + what-if reference-lift calculator

Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01Dn2fdFSbP8sT54D4aRw2gW"
```

---

### Task 10: `tools/label_census.py` — branch targets, predecessors, callee-saved refs: target vs build

func_800283D0's s27 note: "twenty-six sessions of pass forensics never asked where `.L80028488` pointed"; s16 mis-attributed an arm as duplicated-calls until a `$s2`/`$s3` ref census refuted it in s21. This tool answers both questions in one read for the target (`asm/funcs/<func>.s`) and for the build (`tmp/grind/<func>/dumps/<stem>.s` from `dump.ps1`, or any `.s` you pass).

**Files:**
- Create: `tools/label_census.py`
- Create: `tools/test_label_census.py`

- [ ] **Step 1: Write the failing tests**

Create `tools/test_label_census.py`:
```python
#!/usr/bin/env python3
"""Unit tests for tools/label_census.py. Run: python tools/test_label_census.py -v"""
import os, sys, unittest
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from tools.label_census import parse_asm, census

TARGET = """glabel func_X
    /* 0 80000000 00000000 */  addiu      $sp, $sp, -0x18
    /* 4 80000004 00000000 */  beqz       $a0, .L8000001C
    /* 8 80000008 00000000 */   nop
    /* C 8000000C 00000000 */  lw         $s0, 0x0($a1)
    /* 10 80000010 00000000 */  b          .L80000024
    /* 14 80000014 00000000 */   addu      $s1, $s0, $zero
  .L8000001C:
    /* 1C 8000001C 00000000 */  addiu      $s0, $zero, 0x1
  .L80000024:
    /* 24 80000024 00000000 */  jr         $ra
    /* 28 80000028 00000000 */   addu      $v0, $s0, $zero
"""

BUILD = """\t.ent\tfunc_X
func_X:
\taddiu\t$sp,$sp,-24
\tbeq\t$4,$0,$L3
\tlw\t$16,0($5)
\tj\t$L4
$L3:
\tli\t$16,1
$L4:
\tmove\t$2,$16
\tj\t$31
\t.end\tfunc_X
"""


class TestCensus(unittest.TestCase):
    def test_parse_target_labels_and_branches(self):
        ins, labels = parse_asm(TARGET)
        self.assertEqual(labels, {".L8000001C": 6, ".L80000024": 7})
        self.assertEqual([i["op"] for i in ins][:3], ["addiu", "beqz", "nop"])

    def test_predecessors_and_fallthrough(self):
        c = census(TARGET, "func_X")
        self.assertEqual(sorted(c["labels"][".L8000001C"]["preds"]), ["beqz@1"])
        preds = sorted(c["labels"][".L80000024"]["preds"])
        self.assertEqual(preds, ["b@4", "fallthrough@6"])
        self.assertEqual(c["labels"][".L80000024"]["n_preds"], 2)

    def test_callee_saved_refcounts(self):
        c = census(TARGET, "func_X")
        self.assertEqual(c["reg_refs"]["$s0"], 4)
        self.assertEqual(c["reg_refs"]["$s1"], 1)

    def test_build_style_asm(self):
        c = census(BUILD, "func_X")
        self.assertEqual(c["labels"]["$L4"]["n_preds"], 2)
        self.assertEqual(c["reg_refs"]["$s0"], 3)   # $16 normalised to $s0


if __name__ == "__main__":
    unittest.main()
```

- [ ] **Step 2: Run to verify it fails**

Run: `python tools/test_label_census.py 2>&1 | tail -3` → `ModuleNotFoundError`.

- [ ] **Step 3: Implement `tools/label_census.py`**

```python
#!/usr/bin/env python3
"""Label / predecessor / callee-saved-register census — target vs build.

WHY (post-mortem 2026-09-01): func_800283D0 s27 — "twenty-six sessions of
pass forensics never asked where .L80028488 pointed"; its s16 mis-read an
arm as duplicated-calls (nrefs=9) until a $s2/$s3 reference count refuted
it five sessions later. Both are one read of the asm.

Usage:
  python3 tools/label_census.py --func <func>                         # target only (asm/funcs/<func>.s)
  python3 tools/label_census.py --func <func> --build tmp/grind/<func>/dumps/<stem>.s   # + diff vs cc1 output
  python3 tools/label_census.py --func <func> --build <any .s>  --json

Accepts both the splat target format (`/* off addr bytes */  op  args`,
labels `.L8xxxxxxx:`) and cc1/maspsx output (`\\top\\targs`, labels `$Ln:`,
numeric registers). Numeric registers are normalised to ABI names so the
two sides compare directly. Pure text; never builds.
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ABI = ["zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
       "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"]
NUMREG = re.compile(r"\$(\d{1,2})\b")
LABEL = re.compile(r"^\s*(\.L[0-9A-Fa-f]+|\$L\d+|[A-Za-z_]\w*):\s*$")
TARGET_INSN = re.compile(r"^\s*/\*\s*[0-9A-Fa-f]+\s+[0-9A-Fa-f]{8}\s+[0-9A-Fa-f]{8}\s*\*/\s+(\S+)\s*(.*)$")
BUILD_INSN = re.compile(r"^\s+([a-z][\w.]*)\s*(.*)$")
BRANCHES = {"b", "j", "beq", "bne", "beqz", "bnez", "blez", "bgtz", "bltz", "bgez", "bgezal", "bltzal", "beql", "bnel"}
UNCOND = {"b", "j", "jr"}
CALLEE_SAVED = ["$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7"]


def _norm_regs(args: str) -> str:
    return NUMREG.sub(lambda m: "$" + ABI[int(m.group(1))] if int(m.group(1)) < 32 else m.group(0), args)


def parse_asm(text: str, func: str | None = None) -> tuple[list[dict], dict]:
    """(instructions, {label: index-of-next-instruction}). Restricts to the
    function's .ent/.end or glabel span when func is given."""
    lines = text.split("\n")
    if func:
        s = next((i for i, l in enumerate(lines) if re.match(r"^\s*(glabel\s+|\.ent\s+)?%s\b" % re.escape(func), l)), 0)
        e = next((i for i in range(s + 1, len(lines)) if re.match(r"^\s*(\.end\s+%s|glabel\s+\w+)" % re.escape(func), lines[i])), len(lines))
        lines = lines[s:e]
    ins, labels = [], {}
    for raw in lines:
        m = LABEL.match(raw)
        if m and m.group(1) != func:
            labels[m.group(1)] = len(ins)
            continue
        m = TARGET_INSN.match(raw) or BUILD_INSN.match(raw)
        if not m or m.group(1).startswith("."):
            continue
        op, args = m.group(1), _norm_regs(m.group(2).strip())
        ins.append({"op": op, "args": args, "raw": raw.strip()})
    return ins, labels


def census(text: str, func: str | None = None) -> dict:
    ins, labels = parse_asm(text, func)
    preds = {l: [] for l in labels}
    for i, x in enumerate(ins):
        if x["op"] in BRANCHES or x["op"] == "jal":
            tgt = x["args"].split(",")[-1].strip()
            if tgt in preds:
                preds[tgt].append(f"{x['op']}@{i}")
    for l, idx in labels.items():
        # fallthrough: the insn two back is not an unconditional jump whose
        # delay slot is the insn one back
        prev_branch = ins[idx - 2]["op"] if idx >= 2 else None
        if idx > 0 and prev_branch not in UNCOND:
            preds[l].append(f"fallthrough@{idx - 1}")
    reg_refs = {r: 0 for r in CALLEE_SAVED}
    for x in ins:
        for r in CALLEE_SAVED:
            reg_refs[r] += len(re.findall(re.escape(r) + r"\b", x["args"]))
    return {"n_insns": len(ins),
            "labels": {l: {"at": idx, "preds": preds[l], "n_preds": len(preds[l])} for l, idx in labels.items()},
            "reg_refs": reg_refs}


def _print(side: str, c: dict) -> None:
    print(f"{side}: {c['n_insns']} insns, {len(c['labels'])} labels")
    for l, v in sorted(c["labels"].items(), key=lambda kv: kv[1]["at"]):
        print(f"  {l:<14} @{v['at']:<4} preds={v['n_preds']}  {', '.join(v['preds'])}")
    print("  callee-saved refs: " + "  ".join(f"{r}={n}" for r, n in c["reg_refs"].items() if n))


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--func", required=True)
    ap.add_argument("--target", default=None, help="target .s (default asm/funcs/<func>.s)")
    ap.add_argument("--build", default=None, help="cc1/maspsx .s to compare against")
    ap.add_argument("--json", action="store_true")
    a = ap.parse_args()
    tp = Path(a.target) if a.target else ROOT / "asm" / "funcs" / f"{a.func}.s"
    t = census(tp.read_text(errors="replace"), a.func)
    out = {"func": a.func, "target": t}
    if a.build:
        b = census(Path(a.build).read_text(errors="replace"), a.func)
        out["build"] = b
        tp_ = sorted(v["n_preds"] for v in t["labels"].values())
        bp_ = sorted(v["n_preds"] for v in b["labels"].values())
        out["diff"] = {"label_count": (len(t["labels"]), len(b["labels"])),
                       "pred_multiset": (tp_, bp_),
                       "reg_refs": {r: (t["reg_refs"][r], b["reg_refs"][r]) for r in CALLEE_SAVED
                                    if t["reg_refs"][r] != b["reg_refs"][r]}}
    if a.json:
        print(json.dumps(out, indent=2))
        return 0
    _print("TARGET", t)
    if a.build:
        _print("BUILD", out["build"])
        d = out["diff"]
        print(f"\nDIFF: labels {d['label_count'][0]} vs {d['label_count'][1]}; "
              f"pred multiset {d['pred_multiset'][0]} vs {d['pred_multiset'][1]}")
        for r, (x, y) in d["reg_refs"].items():
            print(f"  {r}: target {x} refs, build {y} refs")
    return 0


if __name__ == "__main__":
    sys.exit(main())
```

- [ ] **Step 4: Run the unit tests**

Run: `python tools/test_label_census.py -v 2>&1 | tail -3` → `OK`. If `test_predecessors_and_fallthrough` fails on the fallthrough rule, check the index arithmetic: in the fixture, `.L80000024` sits after `addiu` (index 6) whose preceding instruction (index 5) is the delay slot of `b` (index 4); the label at index 7 sees `ins[5]` = `addu` and `ins[4]`... the rule looks at `idx - 2` = 5 (`addu`), which is not unconditional, so fallthrough is recorded from index 6. That is the intended answer (the `addiu` at 6 falls through).

- [ ] **Step 5: Live smoke**

Run: `python tools/label_census.py --func func_80027640` (Windows side is fine; pure text).
Expected: labels `.L8002767C`, `.L800276A0`, `.L80027778`, ... each with predecessor lists and callee-saved counts (`$s0`, `$s1`, `$s2` non-zero).

- [ ] **Step 6: Commit**

```bash
git add tools/label_census.py tools/test_label_census.py
git commit -m "tools: label_census.py — branch targets, predecessor sets, callee-saved ref counts, target vs build

Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01Dn2fdFSbP8sT54D4aRw2gW"
```

---

### Task 11: `tools/grinder/grant_rescan.py` — re-adjudicate ledgers after a family grant

func_80057CC8's closing form sat in `rejected/` at score 0 from s8; the F3 grant of 2026-08-18 covered it; nothing connected the two for 18 sessions and three reviews. After the owner grants a family, this script finds every ledger whose rejected forms, bans, or constraints mention the grant's terms, and with `--apply` injects a re-adjudication constraint, moves matching bans into a `superseded_bans` record (so the mechanical banned-construct check cannot auto-discard the resubmission), and un-forecloses the item.

**Files:**
- Create: `tools/grinder/grant_rescan.py`
- Test: `tools/grinder/tests/test_grindlib.py` (new class)
- Modify: `tools/grinder/grindlib.py` (add `supersede_bans`)

- [ ] **Step 1: Write the failing tests**

Append to `tools/grinder/tests/test_grindlib.py`:
```python
class TestGrantRescan(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = self.tmp.name
        G.init_ledger(self.root, "func_A", "s")
        G.init_ledger(self.root, "func_B", "s")
        rj = os.path.join(self.root, "memory", "grind", "func_A", "rejected", "inline-both-call-sites.c")
        with open(rj, "w", encoding="utf-8", newline="\n") as f:
            f.write("/* REJECTED s8: compound-address duplication across call arg-lists scores 0 but cheat-by-spelling */\n")
        G.add_banned_construct(self.root, "func_A", "duplicating the compound address expression *(s16**)(arg0+4) at each call site")
        G.add_judge_constraint(self.root, "func_B", "unrelated constraint")

    def tearDown(self):
        self.tmp.cleanup()

    def test_scan_finds_only_matching_ledger(self):
        from tools.grinder import grant_rescan as R
        hits = R.scan(self.root, ["compound-address duplication", "compound address expression"])
        self.assertEqual(sorted(hits), ["func_A"])
        self.assertTrue(any("rejected/inline-both-call-sites.c" in h for h in hits["func_A"]))
        self.assertTrue(any(h.startswith("banned_constructs[0]") for h in hits["func_A"]))

    def test_apply_injects_constraint_and_supersedes_ban(self):
        from tools.grinder import grant_rescan as R
        hits = R.scan(self.root, ["compound address expression"])
        R.apply(self.root, hits, family="F3 compound-address duplication", ref=".claude/rules/no-new-park-categories.md:377", date="2026-09-01")
        st = G.load_state(self.root, "func_A")
        self.assertEqual(st["banned_constructs"], [])
        self.assertEqual(len(st["superseded_bans"]), 1)
        self.assertIn("F3 compound-address duplication", st["superseded_bans"][0]["superseded_by"])
        self.assertTrue(any("RE-ADJUDICATE" in c for c in st["judge_constraints"]))
        self.assertEqual(G.load_state(self.root, "func_B")["banned_constructs"], [])
```

- [ ] **Step 2: Run to verify it fails**

Run: `python tools/grinder/tests/test_grindlib.py TestGrantRescan 2>&1 | tail -3` → `ModuleNotFoundError: No module named 'tools.grinder.grant_rescan'`.

- [ ] **Step 3: Add `supersede_bans` to `grindlib.py`**

After `def unban_construct(...)` add:
```python
def supersede_bans(root, func, needles, superseded_by):
    """Move every banned_constructs entry containing any needle (case-insensitive)
    into state['superseded_bans'] with the grant reference. Returns the count.
    The ban text is preserved for the audit trail; only the mechanical tripwire
    (check_banned_constructs) stops seeing it."""
    st = load_state(root, func)
    if not st:
        return 0
    keep, moved = [], []
    for b in st.get("banned_constructs", []):
        if any(n.lower() in str(b).lower() for n in needles):
            moved.append({"text": b, "superseded_by": superseded_by, "when": _now()})
        else:
            keep.append(b)
    if moved:
        st["banned_constructs"] = keep
        st.setdefault("superseded_bans", []).extend(moved)
        save_state(root, func, st)
    return len(moved)
```

- [ ] **Step 4: Implement `tools/grinder/grant_rescan.py`**

```python
#!/usr/bin/env python3
"""Re-adjudication sweep after a family grant.

WHY (post-mortem 2026-09-01): func_80057CC8's closing form sat in rejected/
at score 0 from s8; the F3 grant (2026-08-18) covered it; the ledger's ban and
three reviews kept citing the 2026-07-20 refusal for 18 more sessions. A
grant must reach every ledger it affects the day it lands.

Usage (repo root, Windows or WSL):
  python tools/grinder/grant_rescan.py --term "compound-address duplication" --term "call arg-list"
  python tools/grinder/grant_rescan.py --term ... --apply --family "F3 compound-address duplication" \
        --ref ".claude/rules/no-new-park-categories.md:377"

Without --apply: prints every ledger whose rejected/*.c headers, banned
constructs, judge constraints, or hypotheses mention any term.
With --apply, for each hit:
  * appends a RE-ADJUDICATE judge constraint naming the grant and the hits;
  * moves matching bans into state.json superseded_bans (audit-preserved) so
    check_banned_constructs no longer auto-discards the resubmission;
  * if the queue item is foreclosed/parked/escalated, returns it to active
    via engine.queue.mark_unparked with the grant as the reason.
The resubmission still passes sandbox 0, self-vet, layer-1, full-build SHA1,
and the Judge — this widens nothing but the ledger's memory.
"""
from __future__ import annotations

import argparse
import os
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
from tools.grinder import grindlib as G  # noqa: E402


def _ledger_texts(root: str, func: str) -> list[tuple[str, str]]:
    d = G.ledger_dir(root, func)
    st = G.load_state(root, func) or {}
    out = [(f"banned_constructs[{i}]", str(b)) for i, b in enumerate(st.get("banned_constructs", []))]
    out += [(f"judge_constraints[{i}]", str(c)) for i, c in enumerate(st.get("judge_constraints", []))]
    for name in ("hypotheses.md", "evidence.md"):
        p = os.path.join(d, name)
        if os.path.isfile(p):
            with open(p, encoding="utf-8", errors="replace") as f:
                out.append((name, f.read()))
    rj = os.path.join(d, "rejected")
    if os.path.isdir(rj):
        for fn in sorted(os.listdir(rj)):
            try:
                with open(os.path.join(rj, fn), encoding="utf-8", errors="replace") as f:
                    out.append((f"rejected/{fn}", "".join(f.readlines()[:80])))
            except OSError:
                pass
    return out


def scan(root: str, terms: list[str]) -> dict[str, list[str]]:
    base = os.path.join(root, "memory", "grind")
    hits: dict[str, list[str]] = {}
    if not os.path.isdir(base):
        return hits
    low = [t.lower() for t in terms]
    for func in sorted(os.listdir(base)):
        if not os.path.isfile(os.path.join(base, func, "state.json")):
            continue
        for where, text in _ledger_texts(root, func):
            tl = text.lower()
            for t in low:
                if t in tl:
                    hits.setdefault(func, []).append(f"{where}: '{t}'")
    return hits


def apply(root: str, hits: dict[str, list[str]], family: str, ref: str, date: str) -> None:
    for func, where in hits.items():
        needles = sorted({w.split("'")[1] for w in where if "'" in w})
        n = G.supersede_bans(root, func, needles, f"{family} ({ref}, {date})")
        G.add_judge_constraint(root, func, (
            f"RE-ADJUDICATE (grant rescan {date}): family '{family}' granted at {ref} covers "
            f"terms {needles}. Hits: {'; '.join(where[:6])}. Restore the matching banked form, "
            "re-measure on the current chassis, and if it reaches 0 submit under that family with "
            f"its SCOPE quoted verbatim. {n} superseded ban(s) moved to superseded_bans."))
        try:
            os.chdir(root)
            sys.path.insert(0, root)
            from engine import queue as Q
            item = next((i for i in Q.load().get("items", []) if i.get("func") == func), None)
            if item and item.get("status") in ("foreclosed", "parked", "escalated"):
                Q.mark_unparked(func, reason=f"grant rescan {date}: {family} ({ref})")
        except Exception as e:  # engine unavailable in unit tests / Windows without venv
            print(f"  (queue not updated for {func}: {e})", file=sys.stderr)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--term", action="append", required=True, help="search term (repeatable, case-insensitive)")
    ap.add_argument("--apply", action="store_true")
    ap.add_argument("--family", default="", help="granted family name (required with --apply)")
    ap.add_argument("--ref", default="", help="file:line of the grant (required with --apply)")
    ap.add_argument("--date", default="", help="grant date YYYY-MM-DD (default today)")
    a = ap.parse_args()
    hits = scan(str(ROOT), a.term)
    if not hits:
        print("no ledger mentions any term")
        return 0
    for func, where in hits.items():
        print(f"{func}:")
        for w in where:
            print(f"  {w}")
    if a.apply:
        if not (a.family and a.ref):
            print("--apply requires --family and --ref", file=sys.stderr)
            return 2
        import datetime
        apply(str(ROOT), hits, a.family, a.ref, a.date or datetime.date.today().isoformat())
        print(f"applied to {len(hits)} ledger(s); commit memory/grind + engine/queue.json")
    return 0


if __name__ == "__main__":
    sys.exit(main())
```

- [ ] **Step 5: Run the tests**

Run: `python tools/grinder/tests/test_grindlib.py -v 2>&1 | tail -3` → `OK`.

- [ ] **Step 6: Make the brief show superseded bans**

In `build_brief`, after the `banned` block computation, add:
```python
    sup = st.get("superseded_bans") or []
    if sup:
        banned += ("\n## SUPERSEDED BANS (cleared by a later family grant — no longer enforced)\n"
                   + "\n".join(f"  - {b['text'][:160]}\n      superseded by: {b['superseded_by']}" for b in sup) + "\n")
```
Run the tests again → `OK`.

- [ ] **Step 7: Document the operator step**

In `.claude/skills/decomp-grind/SKILL.md` under `## OTHER VERBS`, add a row:
```
| "a family was granted" / "re-scan after grant" | `python tools/grinder/grant_rescan.py --term "<grant keywords>"`; review the hits; re-run with `--apply --family "<name>" --ref "<file:line>"`; commit `memory/grind engine/queue.json`. Do this the same day the grant lands, before relaunching. |
```

- [ ] **Step 8: Commit**

```bash
git add tools/grinder/grant_rescan.py tools/grinder/grindlib.py tools/grinder/tests/test_grindlib.py .claude/skills/decomp-grind/SKILL.md
git commit -m "grinder: grant_rescan.py — a family grant reaches every ledger it covers the day it lands

Scans rejected/ headers, bans, constraints, and hypotheses for the grant's
terms; --apply injects a RE-ADJUDICATE constraint, moves matching bans to
superseded_bans (audit-preserved, tripwire cleared), and returns foreclosed
items to active. func_80057CC8 lost 38 sessions to exactly this gap.

Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01Dn2fdFSbP8sT54D4aRw2gW"
```

---

### Task 12: Docs refresh, drills, relaunch

**Files:**
- Modify: `docs/STATUS.md` (Build table Grinder row; Function inventory counts)
- Modify: `.claude/skills/decomp-grind/SKILL.md` (mention the four diagnostics in the status verb row)

- [ ] **Step 1: Refresh `docs/STATUS.md`**

Run `bash tools/wsl.sh 'python3 tools/check_completion_integrity.py' | head -5` and update the inventory table with the printed COMPLETED-C / COMPLETED-INLINE-ASM-CANONICAL / INCOMPLETE counts and today's date; set the Grinder row to `relaunched 2026-09-01 after the session-efficiency changes (docs/superpowers/plans/2026-09-01-grinder-session-efficiency.md)`.

- [ ] **Step 2: Add the diagnostics to the skill's status verb**

In `.claude/skills/decomp-grind/SKILL.md`, in the `## OTHER VERBS` table's "status" row, append: `Diagnostics a session may be told to run: tools/fake_ablate.py, tools/loop_movables.py, tools/nrefs_census.py, tools/label_census.py (all read-only; see each file's docstring).`

- [ ] **Step 3: Run every test suite**

```bash
python tools/grinder/tests/test_grindlib.py 2>&1 | tail -2
python tools/test_sweep_variants.py 2>&1 | tail -2
python tools/test_fake_ablate.py 2>&1 | tail -2
python tools/test_loop_movables.py 2>&1 | tail -2
python tools/test_nrefs_census.py 2>&1 | tail -2
python tools/test_label_census.py 2>&1 | tail -2
```
Expected: `OK` six times. Then `& tools/wteng.ps1 main test` → engine suite green (nothing under `engine/` changed, this is a regression check). Then `& tools/wteng.ps1 main verify-oracle` → SHA1 match (no build input changed, this is a sanity check).

- [ ] **Step 4: Commit the docs**

```bash
git add docs/STATUS.md .claude/skills/decomp-grind/SKILL.md
git commit -m "docs: STATUS refresh + decomp-grind skill lists the new diagnostics

Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01Dn2fdFSbP8sT54D4aRw2gW"
```

- [ ] **Step 5: Drill with the live Judge**

Run: `pwsh tools/grinder/drill.ps1 -WithJudge`
Expected: `DRILL VERDICT: GO`. A NO-GO on drill A or B means the validator change in Task 4 broke the mock outcomes: check `tools/grinder/drill.ps1`'s mock outcome JSON for a KILLED hypothesis and add `"kill_scope": "instance", "measured_on": "drill chassis"` to it, commit, re-drill.

- [ ] **Step 6: Relaunch**

```powershell
Start-Process pwsh -WindowStyle Hidden -ArgumentList '-NoProfile','-File',"`"$PWD\tools\grinder\grind.ps1`""
Start-Sleep -Seconds 5
pwsh tools/grinder/status.ps1
```
Expected: `driver: RUNNING (pid N)` and a new `grinder starting (pid …)` line in `tmp/grind/grind.log`.

- [ ] **Step 7: First-session verification**

After the first session completes (`docs/grind/journal.md` tail), open `tmp/grind/brief_<func>.md` and confirm it contains the `CURRENT SCOPE OF EVERY RULE THIS LEDGER CITES` block (if the ledger cites any rule) and the `KILL LEDGER` block, and that `memory/grind/<func>/state.json` has a `kills` list after the session's `apply`.

---

## Deliberately NOT in this plan (and why)

- **Ladder rung changes** (dropping the second permuter rung, reordering solver/forensics): the ladder's yield data drove the 2026-08-19 R2 placement; changing rungs without a new measurement risks regressing the plateau-breaker slot. The permuter chassis rule in Task 5 captures the measured waste without touching the rungs.
- **Escalation-trigger changes**: the 2026-08-31 silent-foreclosure ruling already removed the 37-session re-dispatch loop (func_80038C70's class). Nothing further is evidenced.
- **A loop.c instrumentation hook in the diagnostic cc1**: the `.loop` dump plus the `.cse` set count gives every term; adding a hook would touch the compiler manifest for no additional signal.
- **Auto-unbanning on grant without operator review**: `grant_rescan.py` prints first and applies only with `--apply` plus an explicit `--family`/`--ref`, so a keyword collision cannot silently clear a legitimate ban.
- **Queue regeneration**: unrelated to session efficiency; the queue-top mismatch noted on 2026-09-01 (`queue status` vs the driver) is a separate item.

## Self-review

- Spec coverage: cause 1 → Tasks 4, 5, 7; cause 2 → Tasks 1, 2, 3, 11; cause 3 → already fixed, documented under "NOT in this plan"; cause 4 → Task 5 (playbook) + Task 6 (enables hand-variant sweeps from a clean tree); the three named diagnostics → Tasks 8, 9, 10.
- Placeholder scan: every code step has full code; every command has an expected result.
- Type consistency: `cited_rule_scopes` returns `[(slug, desc)]` and `render_rule_scopes` consumes that; `kills[]` entries use `session/statement/kill_scope/measured_on/predicate_cite/result` in both `apply_outcome` and `build_brief`; `supersede_bans(root, func, needles, superseded_by)` is called with four positional args in `grant_rescan.apply`; `find_function_span(text, func)` signature unchanged.
