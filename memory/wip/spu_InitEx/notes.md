# spu_InitEx — WIP (blocked-lane triage 2026-06-14)

## TL;DR
`spu_InitEx` (main.c:0x800885cc) body is **already COMPLETED-C-clean**: floor 0,
0 regfix/asmfix, 0 cheat-asm. It is blocked NOT by its own body but by a
**file-scope coupling**: it shares two globals, `D_800A28A0` and `D_800A2CD4`,
declared `extern volatile s32` (main.c:48/50). `queue done` needs the FULL
build SHA1 to match, and the volatile is load-bearing for the byte match of
the CONSUMER functions — so spu_InitEx cannot close on its own.

## Proof of the coupling
- Removing `volatile` from both decls and rebuilding the full driver →
  **-40 bytes (10 insns)**, all in `coli_HitPauseKatana_2` (main.c:2017) and
  `func_8008AAD4` (main.c:2326). spu_InitEx's own bytes are unchanged (it only
  WRITES the globals to 0 at main.c:1366/1368).
- The drop is GCC CSE-ing the repeated `if (D_800A2CD4 & 1)` switch tests that
  the target re-loads each time.

## Why volatile is a cheat here (not a carve-out)
`legitimate-volatile-interrupt-touched` needs BOTH (1) an IRQ-handler writer
AND (2) a spin-wait / double-read / IRQ-loop-bound use-site. NEITHER holds:
- Writers of these globals: only `spu_InitEx` (init) and `|=` updates inside
  `coli_HitPauseKatana_2` / `func_8008AAD4` — none installed as an
  interrupt/VSync/CD callback.
- Use-sites: plain `if (D_800A2CD4 & 1)` switch/if tests — not any of the three
  cataloged shapes.
=> Correctly NOT in `volatile_extern_allowlist.txt`; do NOT self-grant.
`split-read-defeats-hoist` (the rule whose worked example IS
coli_HitPauseKatana_2) already records: that function's old volatile-based
match is "NO LONGER VALID under the new bar … needs re-derivation: either find
a structural lever that doesn't require volatile globals, OR canonical-asm
authorize."

## Resume steps (next session)
1. This is a **3-function coordinated cleanup**, not a single-function pass.
   Target the CONSUMERS: `coli_HitPauseKatana_2` + `func_8008AAD4`.
2. split-read-defeats-hoist levers #1 (duplicate read into arms) + #2
   (direct-symbol index in the known arm) are ALREADY in place (main.c:
   2024-2028). They do NOT defeat the repeated `if (D_800A2CD4 & 1)` reads.
   Need a NEW no-volatile structural lever that forces GCC to re-read the flag
   per switch arm.
3. `func_8008AAD4` also carries register-asm pins (asm("a1"/"a2"/"a3"/"t0"),
   main.c:2327-2330) — also cheats; close them in the same pass.
4. Once the consumers byte-match without volatile, drop `volatile` from both
   decls, rebuild → SHA1==oracle → `queue done spu_InitEx` (and the consumers).

## Ruled out
- Remove volatile alone → full build -40 bytes (consumers break). [rejected/]
- Add to volatile allowlist → not an IRQ carve-out; would be an unauthorized
  self-grant. [rejected/]

## Pointers
- `.claude/rules/split-read-defeats-hoist.md` (worked example = coli_HitPauseKatana_2)
- `.claude/rules/legitimate-volatile-interrupt-touched.md` (the carve-out that does NOT apply)
