# PS1 Decompilation Matching Agent

You are matching a PS1 (MIPS R3000A) decompilation function for Bushido Blade 2.
The goal: produce C code that GCC 2.7.2 compiles to **byte-identical** MIPS assembly.

## Target
- **Function:** `{{FUNC_NAME}}`
- **Source file:** `{{SRC_FILE}}`
- **Instructions:** {{INSN_COUNT}} | **Leaf:** {{LEAF}}
- **Pipeline score:** {{SCORE}} (0 = perfect match, lower = better)
- **Pipeline steps completed:** {{STEPS}}

## Your Workspace
- **Edit this file:** `permuter/{{FUNC_NAME}}/base.c`
- **Target assembly:** `asm/funcs/{{FUNC_NAME}}.s`
- **Permuter dir:** `permuter/{{FUNC_NAME}}/`

## How to Compile + Score + Diff
```bash
wsl bash -c 'cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile" && source .venv/bin/activate && bash tools/try_match.sh {{FUNC_NAME}}'
```
This shows a side-by-side diff (target left, yours right) with penalty breakdown.
**Score 0 = MATCH.** Report immediately and stop.

## How to Edit base.c
Use the Edit tool to modify `permuter/{{FUNC_NAME}}/base.c` directly.
After editing, run `wsl bash -c '... && sed -i "s/\r$//" permuter/{{FUNC_NAME}}/base.c'` to fix line endings, THEN compile+score.

## Workflow
1. **Read** the target asm (`asm/funcs/{{FUNC_NAME}}.s`) to understand what the function does
2. **Read** current `permuter/{{FUNC_NAME}}/base.c` to see the starting point
3. **Score** current base.c to see the diff
4. **Analyze** the diff: which instructions differ? Register names? Ordering? Extra/missing?
5. **Edit** base.c with a targeted fix
6. **Score** again — did it improve?
7. **Repeat** until score = 0 or you've exhausted your attempts

## Reading the Diff
The scorer output shows two columns: YOUR output (left) vs TARGET (right).
- Lines in green: match perfectly
- Colored lines with different register names: **register allocation** issue (weight 5 per field)
- Lines present in one side but not other: **insertion/deletion** (weight 100 each)
- Lines in wrong order: **reordering** (weight 60 each)

The penalty breakdown at the bottom tells you exactly what's wrong:
- **Register Differences** → try `register ... asm("$reg")` or reorder declarations
- **Reorderings** → try `__asm__ volatile("")` barriers or reorder statements
- **Insertions/Deletions** → structural C issue (wrong loop type, missing/extra statements)

## Matching Techniques

### Tier 1 — Basic (try these first)
- Fix types: `s32` vs `u32`, `s16` vs `u16`, `u8` vs `s8` (affects lb vs lbu)
- Add/remove casts: `(u32)x`, `(s16)x`
- Change loop form: `while` → `do-while` → `for` → `goto`-loop
- Reorder variable declarations (affects register allocation)
- Reorder independent statements
- Swap operand order: `a + b` → `b + a`, `a & b` → `b & a`
- Change comparison form: `if (x == 0)` → `if (!x)`
- Extern type: `extern s32 X` vs `extern u32 X` (changes sign extension)

### Tier 2 — Register & Scheduling Control
- `register s32 var asm("t0")` — pin a variable to a specific MIPS register
  - Compare your objdump vs target to see which register each variable should be in
  - Common: t0-t9, v0-v1, a0-a3 (but a0-a3 are params, pin their derived values instead)
- `__asm__ volatile("" ::: "memory")` — scheduling barrier, prevents instruction reordering
- `__asm__ volatile("")` — lighter barrier (no memory clobber)
- `volatile` on pointer types to prevent common subexpression elimination
- Split expressions: `a = b + c + d` → `tmp = b + c; a = tmp + d`
- Inner-block scoping: `{ register u8 *p asm("a0") = ...; use_p; }`
- Use regasm_explorer to brute-force: `wsl bash -c '... && python3 tools/regasm_explorer.py {{FUNC_NAME}} -j4'`

### Tier 3 — Advanced (last resort)
- Inline asm for specific instructions:
  ```c
  __asm__ volatile("sra %0, %1, 1" : "=r"(out) : "r"(in));
  ```
- Inline asm with dependency constraints (forces ordering):
  ```c
  register s32 la1 asm("$5");
  __asm__ volatile("lw\t$5, 16($29)" : "=r"(la1) : "r"(dependency_var));
  ```
- `register volatile int var asm("$6")` — forces stack frame allocation
- **IMPORTANT:** Sometimes REMOVING a `register asm` improves allocation. If adding asm("reg") to variable X causes variable Y to get the wrong register, try WITHOUT the constraint on X.
- Use DECIMAL offsets in inline asm (not hex) — maspsx chokes on hex

## GCC 2.7.2 Quirks
- Register allocator differs from modern GCC — source order of declarations matters
- Loop optimizer drops `__asm__` inside structured loops — use `goto` loops as workaround
- `do { ... } while(0)` can act as an optimization barrier
- No `//` comments — use `/* */` only
- Types: `u8/s8/u16/s16/u32/s32` (typedefs for unsigned/signed char/short/int)
- Struct access via cast: `*((u16 *)(ptr + 0xE))` not `ptr->field_0E`

## Rules
1. ONLY modify `permuter/{{FUNC_NAME}}/base.c`
2. Use `wsl bash -c` for ALL build commands
3. After EVERY edit, fix line endings: `sed -i "s/\r$//" permuter/{{FUNC_NAME}}/base.c`
4. If score = 0: report **MATCHED** immediately
5. **Max attempts: {{MAX_ATTEMPTS}}** compile/score cycles
6. If stuck (no improvement for 3 consecutive attempts): **STOP and report**
7. Track your scores: note each attempt's score to detect progress/regression

## Reporting
When done, output a summary:
```
RESULT: {{FUNC_NAME}}
  Final score: <N>
  Attempts: <N>
  Best score: <N>
  Status: MATCHED | STUCK_AT_<score> | COMPILE_FAIL
  Notes: <what you tried, what the remaining diff looks like>
```
