# func_80056CB8 — structure analysis + first-measurement plan

`src/text1b.c:1705` · target 204 insns · 4 asmfix (region blob on **FRAGILE slot-ordinal** anchors)
+ 11 regfix · scanner tier=LOW 1/8 · **0 cop2 instructions** → pure C, no canonical-asm path.
Origin commit calls it the *judge collision draw-list builder*.

Draft: `tmp/authoring/func_80056CB8.c`. Raw m2c: `tmp/authoring/func_80056CB8.m2c.c`.

Banked state: **none** (`memory/wip/func_80056CB8/`, `memory/grind/func_80056CB8/` do not exist).
The only prior knowledge is encoded in the in-tree harness — see §4.

## 1. Phases

| phase | asm range | what happens |
|---|---|---|
| prologue | 80056CB8–80056D20 | frame 0xA8; save `$ra $fp $s0-$s7`; `$s7 = arg0`; `n = *(u16*)(arg0+0x3E8) & 3`; `$s6 = n*2`; **constant-folded loop entry guard** (`addiu $v0,1` / `beqz $v0,exit`) with `sw $s6,0x60($sp)` in the delay slot; hoist `&sp28`, `&sp58`, `0x1F8002B8` into `0x68/0x70/0x78($sp)`; `$fp = n*4` |
| angle select | 80056D24–80056D90 | `flags = D_8009A821[i*2] << 8`; `obj = (flags & 0x1000) ? *(s32*)arg0 : arg0`; type at `arg0+0x6A` in {0x13, 6} → `ang = flags + *(s16*)(obj+0x1CA)`, else `ang = flags + ratan2(D_800F6608 - obj->F4, D_800F6610 - obj->FC)` |
| build segment #1 | 80056D94–80056E3C | `sin_p = &Judge[ang & 0xFFF]`, `cos_p = &Judge[(ang+0x400) & 0xFFF]`, `scale = D_8009A820[i*2] << 8`; fill `pt0 = {x, y-0x320, z}` and `pt1 = {x + (scale*sin)>>12, y-0x320, z + (scale*cos)>>12}`; call `func_80053614(pt0, pt1, hit0, work, 0x1F8002B8)` |
| extend on hit | 80056E40–80056E84 | if r1 != 0: `x += (sin * 125) >> 8`, `z += (cos * 125) >> 8` (the `sll5/subu/sll2/addu/sra8` expansion) |
| build segment #2 | 80056E88–80056EC8 | vertical probe: `pt0 = {x, y-0x834, z}`, `pt1 = {x, y+0x1004, z}`; second `func_80053614` into `hit1` |
| disposition | 80056ECC–80056F94 | `code = (r1 \| (r2<<1)) + 1`; `code==3` and `hit1[1] - obj->BC < 5` → 0; `code==4` and `dx²+dz² > 0x3D0900` and `\|obj->BC - hit1[1]\| >= 0x3E9` → 5 |
| store / loop | 80056F98–80056FB0 | `*(s8*)(arg0 + 0x444 + i) = code`; `i++`, `$fp += 2`, `i < sp60 + 2` |

**Loop:** two iterations (`i` from `n*2` to `n*2+1`), one per judge of the selected pair.
`$fp == i*2` throughout — see §2.

## 2. Data objects and evident layouts

### `D_8009A820` / `D_8009A821` — one table, two byte columns
`$fp` starts at `n*4 == (n*2)*2` and steps by 2 while `i` steps by 1, so **`$fp == i*2`**. The two
externs are adjacent bytes of the same 2-byte-per-entry table:
`D_8009A820[i*2]` = reach scale, `D_8009A821[i*2]` = flag byte. m2c's `&D_8009A821 + var_fp` with a
free-running `var_fp` is the single biggest artifact in its output — writing it that way costs you
the `i*2` induction variable GCC actually builds.

### The object (`obj`, `$s1`)

| offset | type | role |
|---|---|---|
| +0x000 | s32 | pointer to the alternate object (selected by flag bit 0x1000) — read off `arg0`, not `obj` |
| +0x06A | u16 | object type; 0x13 or 6 → use stored facing — read off `arg0` |
| +0x0B8 / +0x0BC / +0x0C0 | s32 | position x / y / z |
| +0x0F4 / +0x0FC | s32 | world x / z used for the `ratan2` fallback |
| +0x1CA | s16 | facing angle |
| +0x444 + i | s8 | per-judge result code (written on `arg0`, always) |

Note the asymmetry the draft preserves: `0x3E8`, `0x6A` and `0x444` are read/written through
`arg0` (`$s7`); everything else through `obj` (`$s1`).

### `Judge` — Q12 sin/cos table
`s16` table indexed by a 0xFFF-masked angle; cos via `+0x400`. TU spelling is
`(&Judge)[angle]` / `*(&Judge + angle)` (`src/text1b.c:1912-1913`).

### Frame map (0xA8)

| range | object |
|---|---|
| 0x00–0x13 | outgoing args (0x10 = 5th arg of `func_80053614`) |
| 0x18–0x27 | `pt0` — segment start, 16 bytes |
| 0x28–0x37 | `pt1` — segment end, 16 bytes |
| 0x38–0x47 | `hit0` — result of call #1 (reads at +0x00, +0x08) |
| 0x48–0x57 | `hit1` — result of call #2 (read at +0x04) |
| 0x58–0x5F(?) | `work` — handed to `func_80053614` arg4, **never initialised here** |
| 0x60 / 0x68 / 0x70 / 0x78 | GCC **spill slots**, 8-byte stride: `start`, `&pt1`, `&work`, `0x1F8002B8` |
| 0x80–0xA7 | `$s0-$s7`, `$fp`, `$ra` |

The four 8-byte-stride slots at the top are LICM-hoisted invariants that got spilled — the same
`BIGGEST_ALIGNMENT == 64` phenomenon as func_8002CA8C's `0x18/0x20/0x28/0x30`. They are **not**
source objects, and the harness's decision to model them as struct members `sp60/sp68/sp70/sp78`
with `_g5`/`_g6` gap fields is the core reason the current C cannot converge.

### `func_80053614` — signature correction required
`src/text1b.c:1493` declares it `void`, but this call site consumes `$v0`
(`addu $s0,$v0,$zero`, `sll $v0,$v0,1`). Its own asm ends `jal func_80052D00` immediately followed
by the epilogue with no further `$v0` traffic, so re-typing it `s32` with
`return func_80052D00(arg2, arg3);` should be byte-neutral for that function — but it *is* a matched
function, so prove it with a full build + SHA1 before anything else. The forbidden shortcut here is
a second declaration under an `asm("func_80053614")` alias; that is the alias-rename cheat family.

## 3. First-measurement plan

1. **Delete the harness wholesale, in one change.** Everything at `src/text1b.c:1705-1795` goes: the
   21-field anonymous struct with `_g0.._g6`, all 18 `#define spNN` / `#undef spNN` macros, the
   three `asm("$23")/asm("$22")/asm("$3")` pins, `__asm__("andi %0,%1,0x3")`, the
   `__asm__("addiu %0,$0,1")` guard and its `if (_guard != 0)`, and the clobber-list `__asm__`.
   None of it is an end state; all of it exists to satisfy the blob.
2. **Re-type `func_80053614` to `s32` and full-build.** Do this as its own verified step so a SHA1
   change is attributable.
3. **⚠ ANCHOR RISK — plan the blob's fate before the first edit.** The asmfix rules resolve
   `{lbl#1}`/`{lbl#2}` by *cc1 label ordinal*. Removing the fake `_guard` branch removes a label
   from the prologue and **renumbers every slot**; `delete_between` then deletes the wrong span and
   duplicates bytes **silently** — the failure that cost the func_800393C8 session (+452 bytes,
   caught only by the oracle). Either re-anchor the two `rename` rules to the new ordinals in the
   same commit, or retire the blob outright. Never commit an intermediate where the C moved and the
   ordinals did not.
4. **`sandbox func_80056CB8 --disable all`.** The queue's 188 is the honest distance *of the
   harness*, not of an attempt; the real floor is unknown. Recording it is the session's deliverable.
5. **Then read the residual in this order:**
   - *the loop entry guard* ([S1]). `for (i = start; i < start + 2; i++)` is the bet: GCC 2.7.2's
     `loop.c` emits the entry test, `simplify_relational` folds `start < start+2` to 1, and the
     branch survives as `addiu $v0,$zero,1 / beqz`. If the branch disappears, the prologue is
     short by 2 and every slot ordinal moves — do not paper over it with an `__asm__`.
   - *the frame*: read the `.frame vars=` gradient rather than forcing offsets
     ([[phantom-slot-frame-lever]]). The open question is whether `work` is 8 or 16 bytes.
   - *the 11 regfix rules* are `frame-reg-rename` ×5 and `delete-instruction` ×3 — frame-adjacent
     and small, consistent with the frame being the primary axis. Expect them to fall out once the
     frame is right rather than needing individual attention.
6. **Apply whatever works to func_80070C70 immediately** — same TU, same `asmfix_slice.py` shape,
   same week, same slot-ordinal anchors (`_SHARED.md` §6 / this function's brief §6).

## 4. What the harness got right (worth preserving as *facts*, never as spelling)

- The five 16-byte-spaced local blocks at `0x18/0x28/0x38/0x48/0x58` — the draft keeps the layout
  as five ordinary arrays and lets GCC place them.
- `sp68 = &sp28`, `sp70 = &sp58`, `sp78 = 0x1F8002B8` being loop-invariant — correct observation,
  wrong conclusion: they are LICM output, so the source should just pass `&pt1`, `&work` and the
  literal at each call and let GCC hoist.
- The loop entry guard exists and is constant-1 — correct observation, faked implementation.
- `func_80053614`'s argument list and order, including the `(s32)` casts on args 3 and 4.
- `*(s8 *)(r_arg0 + 0x444 + var_s6) = ...` — the store is on `arg0` and indexed by the loop
  counter, not by `obj`.

## 5. Biggest unknowns

1. **Does the loop entry guard survive natural C?** Everything downstream (slot ordinals, prologue
   length, the whole asmfix anchor question) hangs on it. Highest-value single experiment.
2. **Size of `work` (sp+0x58)** — 8 or 16 bytes; determines whether `0x60` is a spill slot or a
   member. Resolvable from the `.frame vars=` gradient.
3. **Is `*(u16 *)(arg0 + 0x6A)` really read twice?** Target loads it once into `$v1` and compares
   twice; the draft writes two reads of the same lvalue, which GCC should CSE. If it does not, hoist
   it to a local.
4. **`code`'s type.** m2c inferred `s8` from the `sb`; the arithmetic (`(r1 | r2<<1) + 1`, compares
   against 3/4, assignment of 5) is all `s32` in target with the narrowing only at the store. The
   draft keeps `s32` + a cast at the store; if a spurious `andi`/`sll/sra` pair appears, this is why.
