# Inline-asm audit — 2026-10-01

Owner-requested audit of every function carrying inline asm: count, sizes, whole-asm vs partial,
any function marked asm while canonically C, and how SOTN handles the same classes. Read-only
measurement; the one decided outcome (three de-authorizations) is recorded in
`docs/grind/decisions.md` ("2026-10-01 — OWNER RULING — inline-asm audit").

Scratch scripts and per-function data: `tmp/asmaudit2/` (gitignored) — `census.py` →
`census.csv`, `bucket.py`, `frameshape.py`, `wholetest.py`, `s7slot.py`, `gate.py`.
Predecessor: the 2026-09-24 re-audit (`tmp/asmaudit/FINDINGS.md`, memory
`canonical-asm-reaudit-2026-09-24`). Its "119 C-with-islands / 88 whole-body" split counted any
non-`INCLUDE_ASM` entry as C-with-islands, so it misfiled the 89 inline `glabel` blocks; the
split below parses the source form.

## 1. Population (state at 2026-10-01, before the de-authorizations)

**222 entries** in `inline_asm_canonical.txt`, ~23,540 target instructions (~94 KB).

| Form | Count | Target insns |
|---|---|---|
| Whole-body, inline `__asm__("glabel …")` in a `.c` | 89 | — |
| Whole-body, `INCLUDE_ASM` | 88 | — |
| Whole-body, raw linked `.s` object (`save_vc_ctrl`) | 1 | 16 |
| **Whole-body total** | **178** | **11,380** |
| C body with asm islands | 44 | 12,159 (967 island insns ≈ 8%) |

Not listed but carrying GTE inline asm: 11 COMPLETED-C functions using `include/gte.h` macros
(`check_completion_integrity.py` accepts them — by policy, not a defect).

Size distribution (target instructions):

| Insns | All 222 | Whole-body | C + islands |
|---|---|---|---|
| ≤4 | 45 | 43 | 2 |
| 5–20 | 40 | 37 | 3 |
| 21–50 | 30 | 30 | 0 |
| 51–100 | 19 | 13 | 6 |
| 101–200 | 62 | 49 | 13 |
| >200 | 26 | 6 | 20 |

## 2. Whole-body entries — evidence class

| Class | Count | Verdict |
|---|---|---|
| BIOS A/B/C trampolines (3–4 insns) | 38 | asm — certain |
| `syscall` / `break` / cop0 primitives (PC*, `_SN_read`, …) | 10 | asm (`func_800164F8` — a loop around one `break` — could be C + 1-insn island) |
| Decisive scanner signal: S7 47, S6 6, S1/S2/S8 8 | 61 | asm. 45 of the 48 S7 functions ALSO carry trapping arithmetic and 43 have a cop2/trap op in a branch delay slot (GCC cannot place an `__asm__` there) — independent of S7 |
| GTE-only (LIBGTE in `display.c`, `gte_*` wrappers in `text1b.c`) | 40 | asm — Sony LIBGTE is hand asm; wrappers use `$t0–$t7` sequentially and put cop2 ops in `jr $ra` delay slots |
| Trapping `add`/`addi`/`sub`/`neg` only | 17 | asm — 0 occurrences in ~1,150 COMPLETED-C functions |
| No no-C-form instruction | 12 | 9 asm by construct (frameless fn-pointer tail jumps, `$sp` swap, BIOS-patched code template, data-as-code, `$t9` absolute poke, SN crt `__main`/`__do_global_dtors`, `RotTransPers4`); **3 suspects — §3** |

The engine gate (`canonical-scan`) labels 104 whole-body entries "ASM-PARTIAL" because it counts
only no-C-form instructions; ABI and delay-slot evidence overrides that label.

C-with-islands (44): island content is GTE ops plus the PsyQ `inline_o.h` macro preamble
(`move`/`lw`/`lhu`/`sll`/`or`/`addu`); no general-purpose steering asm found.

## 3. Marked asm, likely canonically C — DE-AUTHORIZED 2026-10-01

| Function | Insns | Cited evidence (2026-06-07) | Why it fails | Honest floor (2026-10-01) |
|---|---|---|---|---|
| `func_80044010` | 34 | 8-byte frame, no locals; frame alloc in `bnez` delay slot; `addiu sp / jr ra / nop` | COMPLETED-C shows all three: alloc-in-delay-slot 11, phantom frame 35, that epilogue 779 (e.g. `func_8001F938`). Phantom-frame mechanism measured 2026-07-13, after the authorization. Body is ordinary cc1 output (`addu …,$zero` moves, `%hi/%lo($at)` indexing, `slt` loop) | 3 |
| `save_vc_ctrl` | 16 | same frame/epilogue shape | same counter-evidence; GCC count-down loop with `-1` sentinel | 2 |
| `func_8006BD28` | 103 | difficulty of reproduction (37 cheats, frame skew) | zero no-C-form insns, scanner 0/8 LOW, gate verdict C. Overrides the 2026-08-06 "stands" ruling | 48 |

Floors: `sandbox --disable all --candidate` on the pre-authorization C bodies (`19d75899c^`,
`385126fcd^`, `6e0476f0f^`) with the `volatile` frame pads removed; banked as
`memory/grind/<func>/migration_pin.json` + `preauth_body.c`.

## 4. SOTN comparison (local clone `tmp/sotn-decomp`, 2026-07-10; psyz `tmp/psyz-ref`)

| Class | SOTN | BB2 | Aligned? |
|---|---|---|---|
| BIOS trampolines | `BIOS_B_FUNCTION(DeliverEvent, 0x7);` macro in a `.c` (include/bios.h) — counted decompiled | 38 hand-written `glabel` blocks — COMPLETED | same bytes/status; spelling differs |
| Syscall wrappers | C function + island: `void EnterCriticalSection() { SYSCALL(1) }` | whole-body `glabel` | both defensible |
| GTE in game code | C with `gte_*` header macros — counted matched | 44 C+islands with raw `__asm__` text; 11 via `gte.h` | substance aligned; SOTN spells named macros |
| LIBGTE hand asm | splat `asm` segments (`psxsdk/libgte/mtx_00` …) — **counted NOT decompiled** by `tools/progress.py`; psyz keeps 647 `INCLUDE_ASM` in libgte | ~40 entries COMPLETED | same designation, BB2 more generous accounting |
| Hand-written game code | `asm, handwritten/<name>` splat segments (2: `DecDCTvlc`, `func_801BAB70`) — **counted NOT decompiled** | ~135 entries COMPLETED | same designation, more generous accounting |
| C-expressible but hard | stays `INCLUDE_ASM` until matched — no "too hard ⇒ asm" state | 3 authorized (§3) | **not aligned → fixed 2026-10-01** |

Whole-body hand asm counted COMPLETED here but undecompiled by SOTN's tracker: ~11,200 insns ≈ 9%
of the ~121k-insn binary.

## 5. Action items

| # | Item | Status |
|---|---|---|
| A1 | Re-queue `func_80044010`, `save_vc_ctrl`, `func_8006BD28` as INCOMPLETE (remove from allowlist; `func_80044010` `glabel` → `INCLUDE_ASM`; `save_vc_ctrl` raw linked object → its own one-function TU `src/text1a_svc.c` so the engine/Grinder can score C for it) | **DONE 2026-10-01** |
| A2 | Progress reporting split: report "matched C" and "hand-written asm (canonical)" as separate lines so BB2 numbers are comparable to SOTN's | **DONE 2026-10-01** — `tools/check_completion_integrity.py` prints a code-size split. First run: COMPLETED-C 68.9%, canonical C+islands 10.0%, canonical whole-body asm 9.2%, INCOMPLETE 11.9% of 486,196 bytes; "in C" (SOTN sense) 78.9%, completed incl. canonical asm 88.1% |
| A3 | Replace the 38 hand-pasted BIOS trampoline blocks with one `BIOS_FUNCTION(name, vector, id)` macro (SOTN `include/bios.h` shape). Byte-neutral | **DONE 2026-10-01** — `include/bios.h` `BIOS_[ABC]_FUNCTION`; all 38 converted; engine recognises the macro as whole-body asm |
| A4 | Spell the 44 C+islands bodies' raw GTE `__asm__` text as named `gte_*` macros (SOTN / PsyQ `inline_o.h` shape). Byte-neutral if the macro text matches; readability only | **NOT DONE — needs an owner ruling.** Moving islands into header macros hides their GPR instructions (`move`/`lw`/`addu` preamble) from the cheat detectors and the region-grant hashes, which the 2026-09-25 scorer ruling relies on seeing inline. Only ~7 of the 44 are pure cop2. Revisit with engine support for header-macro recognition, or for a port build only |
| A5 | Source form for whole-body hand asm: SOTN keeps it as `.s` (splat `asm` segment / `INCLUDE_ASM`) rather than pasting `glabel` text into `.c` files (89 blocks here). Byte-neutral; low value | **DONE 2026-10-01** — 46 blocks → `INCLUDE_ASM` (`asm/funcs/RotTransPers4.s` created). Kept as pasted blocks: `_patch_gte`, `DelDrv`, `_96_remove` (splat files don't line up with the code) |
| A6 | `func_8004A76C` (39 insns) rests mainly on S7 (unsaved callee-saved register). A file-scope GCC global register variable (`register T x asm("$16")`) produces that shape from C; never considered in-tree. Decide whether that is an admissible C form and, if so, re-test this one function (the rest of the S7 family has independent evidence) | **CLOSED, no change** — both callers (`func_8004A4E0`) are whole-body hand asm, and the loop is hand software-pipelined (`rtpt` issued before the previous iteration's stores, `j` into the loop's second half). A global register variable is also a register pin under policy |
| A7 | `func_800164F8` (7 insns, loop around `break 1`) is plausibly C + a 1-insn island rather than whole-body | **DONE 2026-10-01** — now C (`for (i = 9999; i >= 0; i--)`) + one `.word 0x0001000D` break island; compiles to the target's 7 words exactly; region grant added; stays canonical |
| A8 | Gate gap: the engine `canonical` gate does not recognise BIOS trampolines (returns verdict C for all 38) and labels S7 bodies ASM-PARTIAL. Consider teaching it the trampoline shape and the S7/delay-slot signals so its verdicts match the allowlist | **DONE 2026-10-01** — gate signals added: BIOS vector jump, non-GCC `break` code, no-C-form op in a delay slot, unsaved callee-saved register. `canonical-scan` ASM-WHOLE 10 → 113; no COMPLETED-C function or queue item changed verdict |
