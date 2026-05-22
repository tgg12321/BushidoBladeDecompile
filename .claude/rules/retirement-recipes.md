---
name: retirement-recipes
paths: ["asmfix.txt"]
description: "Five recipes/gotchas accumulated across 20+ asmfix retirements (GTE wrappers + decomp). Each saves 5-15 minutes of trial-and-error if known beforehand."
metadata:
  type: recipe
---

> **TIER-4 note ([[tier4-sota-standard]], 2026-05-21):** convert each bridge to
> **pure C**. In-file `__asm__()` is allowed ONLY for a construct with no C form
> (GTE / BIOS / whole-function hand-asm), evidence-gated via
> [[canonical-asm-retirement]]. Do **not** introduce new regfix / pins / inline-move
> to "finish" a retirement — those are hard-blocked by `audit_asm_cheats.py --check-new`.

These recipes apply to *retirement* work — `dc.sh next-asmfix` items that need to convert `replace_with_asmfile` bridges into pure C (or, for genuinely hand-written assembly, in-file `__asm__()` per project policy). Read this file BEFORE pulling the first asmfix item of any session.

## 1. `.word`-encoded asm does NOT honor `"=r"` register-asm bindings; mnemonics do

**Symptom:** You write
```c
register s32 v0 asm("v0");
__asm__ volatile (".word 0x4802C800" : "=r"(v0));   /* mfc2 $v0, $25 */
```
expecting the output to land in `$v0`. GCC compiles it but ignores the register-asm binding because the `.word` is opaque to GCC's operand placement. The output goes to whatever register GCC picked, not `$v0`. Subsequent uses of `v0` in C reference the wrong register and the byte match fails.

**Fix:** Use the mnemonic form with operand substitution:
```c
register s32 v0 asm("v0");
__asm__ volatile ("mfc2 %0, $25" : "=r"(v0));
```
GCC now inserts the operand into the asm string and honors the `$v0` pin.

**When to use `.word`:** Only when GCC has no other access to those registers (e.g., the `ctc2 $tN, $cN` ops with control-register operands the assembler can't represent symbolically). Bare `.word` is fine if there are no `"=r"`/`"r"()` operands you care about.

**Lesson cost:** ~10 min per misapplication. Surfaced on `func_80052754` (commit 642f276).

## 2. `move %0, %1` for register moves — `addu` triggers the inline-asm-debt audit

**Symptom:** You wrap a register-move in inline asm to control its placement:
```c
__asm__ volatile ("addu %0, %1, $zero" : "=r"(v0) : "r"(a2));
```
The function byte-matches, but `dc.sh refresh-queue` reclassifies it as `inline_asm_debt` and puts it back at the top of the active decomp queue, where `dc.sh next` will hand it out again.

**Cause:** `audit_inline_asm.py`'s SUSPECT regex catches `\baddu\b`. The audit can't tell the difference between a placement-control move and a real "should be pure C" arithmetic op.

**Fix:** Use the MIPS pseudo-mnemonic `move`, which expands to the same bytes (`addu rd, rs, $zero`) but isn't in the SUSPECT regex:
```c
__asm__ volatile ("move %0, %1" : "=r"(v0) : "r"(a2));
```

**Lesson cost:** Function gets re-handed-out on next `dc.sh next`. Surfaced on `func_8007E8AC` (commit aad567f) and corrected on `func_8007ED6C` (commit 57081f6).

(Note: this is a workaround for the audit, not a fundamental fix. The audit itself has been patched to recognize `.word 0x4[89A-F]......` as GTE-op-encoded acceptable inline asm — see commit notes for tooling change. The `move` workaround still applies to non-GTE register moves.)

## 3. Don't use `wsl bash -c 'python3 -c "..."'` heredocs with `$N` tokens in the inner string

**Symptom:** An agent writes a Python edit script via:
```
wsl bash -c 'python3 -c "
import re
text = open(...).read().replace(\"foo $8\", \"bar $9\")
...
"'
```
The outer Windows-to-WSL bash shell expands `$0`, `$1`, `$8`, etc. inside the inner string BEFORE Python sees it. Result: `register asm("$8")` becomes `register asm("")`, `"ctc2 %0, $0"` becomes `"ctc2 %0, /bin/bash"` (since `$0` is the script name). cc1 then emits an inline-asm body with empty operands; maspsx errors out with "Unable to parse load/store instruction" or silently emits a malformed function.

**Fix:** Save the Python edit script to a file and run it with `python3 <file>`:
```python
# tools/tmp_edit.py
import re
text = open('src/foo.c').read()
text = text.replace('foo $8', 'bar $9')
open('src/foo.c', 'w').write(text)
```
Then `wsl bash -c "cd /mnt/c/...path... && python3 tools/tmp_edit.py"`.

The Edit tool and Write tool handle this automatically (no shell layer). Only worry when explicitly writing through `bash -c`.

**Lesson cost:** 15-30 min of "why is the build silently producing wrong bytes?" debugging. Surfaced on `game_2d_CheckLifeGaugeNoDisp` (commit cbf8e91 retro).

## 4. `regfix.py` renumbers indices after deletes; prefer `insert_after @ idx` over `insert @ idx+1`

**Symptom:** You write a regfix recipe like:
```
func_X: delete @ 5
func_X: insert @ 6 "nop"
```
expecting `insert @ 6` to land at the position that was originally idx 6 (one after the deleted instruction). But `delete @ 5` shifts all subsequent indices down by 1, so what was idx 6 is now idx 5. The `insert @ 6` either lands at the wrong position or fails with "index out of bounds."

**Fix:** Use `insert_after @ idx` which uses the *original* (pre-delete) index, OR write the rules in dependency order so each rule's `idx` refers to the state after prior rules.

`insert_after` semantics:
```
func_X: insert_after @ 5 "nop"   # insert one position AFTER what idx 5 currently is
```
This survives subsequent deletes/inserts because each rule is applied to the rolling state.

**Diagnostic:** `REGFIX_DEBUG_DUMP=<func>` env var prints `(pos, idx, text)` after each regfix phase — see `feedback_regfix_idx_introspection.md`. Use this any time you're chaining 3+ regfix rules.

**Lesson cost:** An agent on `func_80078F60` spent ~10 minutes on this (commit 8b1f6bf).

## 5. GCC 2.7.2 cannot express "saved register as unsaved input" — hand-written-asm functions stay as in-file `__asm__()`

**Symptom:** A function uses `$s0` (or any `$sN`) as if it's an input parameter — reads from it, mutates it, returns without restoring it. The original game binary has NO prologue saving `$s0`. Every attempt at pure C in GCC 2.7.2 produces a prologue save:
```c
register int *vp asm("$s0");   /* try to capture caller's $s0 */
*vp = ...; vp += 24;            /* GCC spills vp to v0/v1, breaking the invariant */
```
or with `__attribute__((naked))` — not supported in the GCC 2.7.2 MIPS backend.

**Cause:** Custom calling convention. The function was hand-written assembly where the caller passes a value via `$s0` directly without going through the standard `$a0..$a3` parameter regs. GCC has no C-level construct for "I promise `$s0` is set up correctly on entry and I'll trash it on exit."

**Fix:** Per project policy ("no inline asm unless the original canonical code was hand-written assembly"), accept the in-file `__asm__()` block as the canonical retirement form. The 89 sibling functions in `text1b.c` already use this convention. Replace the bridge stub with a top-level `__asm__()` block that emits the function body directly. Bridge is removed (`# RETIRE: ` in asmfix.txt); the asm now lives in the source rather than `asm/funcs/<func>.s`.

**How to detect this case early:**
- Function uses `$sN` register without seeing a prologue `sw $sN, X($sp)` for it
- Function has no `addiu $sp, $sp, -N` prologue at all
- Function lives in a file already dominated by inline-asm blocks (text1b.c, code6cac*.c)

If you spot this on `dc.sh classify` / first read of the asm, don't waste time on pure-C attempts — go directly to the file-scope `__asm__()` retirement form.

**Lesson cost:** An agent on `func_8004A76C` spent ~23 minutes (commit bc04b47) exhausting 5 pure-C techniques before falling back. User authorized the in-file asm form afterward.

## Cross-cutting: always run `verify --all` after every match

All five gotchas above share a common failure mode: the function's own `dc.sh verify <func>` passes (the function's bytes happen to align at its declared address), but `dc.sh verify --all` finds that sibling functions broke (`.L<N>` drift, address cascade-shift, or the audit silently dropped instructions). Run `verify --all` IMMEDIATELY after every match before considering it done. It SHA1-short-circuits if clean, so it's cheap.

## 6. The build-cache regression trap — use `verify --clean` for register-allocation-sensitive matches

**Symptom:** An agent reports "MATCHED first try" and `dc.sh verify <func>` says MATCH. `verify --all` is green. SHA1 matches. Commit lands. Weeks later, a fresh checkout + `make` produces a DIFFERENT binary that doesn't match.

**Cause:** `make` uses cached `build/src/*.o` files. When you edit a `.c` file, `make` recompiles only that one .o. If your new C source has different register allocation than the prior .o on disk, BUT the link still produces matching bytes because OTHER .o files (cached, compiled from old C) happen to cancel out the difference — you get a false match. A clean rebuild (`rm -rf build && make`) is the only honest test.

This bit us between commits **54a5e54 → c71ff0a** (Sep 2026). The 54a5e54 commit "matched" func_8003D39C with a C body that, on a clean rebuild, produces 34 instructions different from the target. Seven subsequent commits accumulated on this broken baseline; nobody caught it because `dc.sh verify --all` SHA1-short-circuits on the dirty-cache build's hash. The regression surfaced only when a later session did a clean rebuild and noticed.

**Fix:**
- `dc.sh verify --clean` — `rm -rf build && make && verify --all`. Run this BEFORE committing any function that involved register-asm pinning, `register asm("$N")`, or non-trivial regfix rules. ~2-3 min, much cheaper than discovering the regression 10 commits later.
- ~~The active-func commit hook did a clean rebuild on every match commit~~ — that gate (`active_func_guard.sh`) was **deprecated 2026-05-22**, so this trap is no longer caught automatically. **Run `dc.sh verify --clean` yourself before committing** any match that involved register-asm pins or non-trivial regfix rules.

**Heuristic for when `--clean` is worth the time:** any function whose match involved >2 register-asm pins, >3 regfix rules, or a `move`/`addu`-via-asm placement trick. The riskier the codegen-influencing edit, the more likely your dirty-cache verify is wrong.

**Lesson cost:** 7 commits accumulated on a broken baseline; the cleanup was a 2-hour repair session (commit c71ff0a) to fix func_8003D39C's register allocation in pure C plus a few cascade label-drift fixes in regfix.txt. The 6 GTE-wrapper retirements between were genuinely byte-matching even on a clean rebuild — only one bad match poisoned the verify state. The damage was the false sense of safety, not the actual bytes.

## 7. The asmfix-bridge false-match trap — use `dc.sh verify-c` for retirement work

**Symptom:** During retirement of a bridged function, `dc.sh verify <func>` reports MATCH, the clean-rebuild SHA1 check passes, the hook lets the commit through. Later, someone notices the function "doesn't actually match" — removing the bridge entry from asmfix.txt produces a binary that diverges from the original.

**Cause:** The build pipeline ends with `asmfix.py`. When `asmfix.txt` has an active `<func>: replace_with_asmfile "asm/funcs/<func>.s"` line, asmfix.py **overwrites the bytes** in the .o for that function with the raw asm. Any C body for that function is dead code — its compiled output never reaches the binary.

The bridge override happens regardless of:
- Whether the C body is in src/*.c
- Whether `make` is clean or cached
- Whether the hook does a clean rebuild

`dc.sh verify <func>` reads bytes at the function's address in `build/bb2.exe`. For a bridged function, those bytes are always the bridge's bytes (the original game's), so verify ALWAYS reports MATCH for bridged functions.

This bit us at **commit 836d9a1 for func_8007B3A8** (May 2026). The retire workflow's `# RETIRE: ` comment got silently restored to active (probably by `refresh-queue`'s auto-purge), the bridge was still in effect during my "match commit," the verify reported MATCH (correctly — from the bridge's perspective), the hook's clean-rebuild check passed (SHA1 was clean, because the bridge was producing the right bytes). My C body produced 63 instruction diffs when later tested without the bridge.

**Fix (shipped):**
- `dc.sh verify-c <func>` — bridge-aware verify. Refuses if the function has an active `replace_with_asmfile` line in asmfix.txt; otherwise runs the normal verify. **Use this during retirement work** instead of plain `verify`.
- `dc.sh verify <func>` — now warns (doesn't block) when the target has an active bridge. The warning points at `verify-c`.
- ~~The commit hook refused commits when the active function had a live bridge entry~~ — that check was part of `active_func_guard.sh`, **deprecated 2026-05-22**. Use `dc.sh verify-c <func>` (bridge-aware) yourself before committing retirement work.
- `refresh-queue` (both modes) — **no longer auto-purges `# RETIRE: ` lines.** They're kept as permanent historical markers. The old auto-purge couldn't distinguish "SHA1 matches because C body is correct" from "SHA1 matches because bridge is still producing the bytes," and would silently restore the bridge to active by removing the `# RETIRE: ` comment.
- `dc.sh purge-retirements [--dry-run]` — explicit, safe manual cleanup. For each `# RETIRE: <func>: ...` line, it runs `verify-c <func>` and only purges if that reports MATCH. The `verify-c` check guarantees no active bridge for the function exists, so a MATCH proves the C body's bytes are correct.

**The right retirement workflow now is:**
1. `dc.sh retire <func>` — comments out the bridge
2. Write C body, iterate (`make`, `dc.sh diff-align <func>`, edit, repeat)
3. `dc.sh verify-c <func>` — must report MATCH (refuses if any bridge remains)
4. `git commit` — hook does clean rebuild + checks for live bridge entry; both must be clean
5. (optional, later) `dc.sh purge-retirements` — clean up the `# RETIRE: ` line once you're confident

The intermediate `# RETIRE: ` lines accumulate; that's intentional. They're harmless (comments, ignored by the build) and serve as a record of which bridges have been retired. Run `purge-retirements` only when you want to clean up cosmetic clutter and have manually verified each one.

**Lesson cost:** ~90 minutes on commit 836d9a1 → revert ae1420a → understanding the trap → designing/shipping the fix.
