# initDrawMode (src/gpu.c) — WIP

## TL;DR

Honest pure-C floor lowered 5 → 1. Remaining diff is a SINGLE OR-operand-order
flip at the last instruction (`or v0,v0,v1` vs target `or v0,v1,v0`, both
encode `cmd | val`). HEAD carries cheat-asm `register asm("v1")` + `register
asm("v0")` pins on cmd/val — committed cheats per
[[inline-asm-policy]]. The candidate retires both pins and reaches sandbox 1
in pure C; the last byte is blocked by a coupling between cc1's combine pass
(folds val's andi into the dying a3) and the desired `cmd | val` source-order
arg pattern.

## Resume instructions

1. Apply `memory/wip/initDrawMode/candidate.c` to `src/gpu.c::initDrawMode`
   (replaces the 2-pin HEAD body).
2. `& tools/eng.ps1 sandbox initDrawMode --disable all` should report
   score 1 (target_insns=11, build_insns=11).
3. `& tools/eng.ps1 verify-oracle --rebuild` will FAIL (build sha1
   `a0e903255c01b8d254c4865550c1ebc13ea9bf6c` vs oracle
   `62efab4f73f992798c43e8c730aa43baa10bb4fa`).
4. Diff the function vs target:
   `wsl mipsel-linux-gnu-objdump -d build/src/gpu.o | grep -A12 '<initDrawMode>'`
   — the only diff is the last `or` instruction's operand order.

## The wall

Target asm (build/src/gpu.o, canonical):
```
li      v0, 1
sb      v0, 3(a0)
beqz    a2, .L1
 lui    v1, 0xE100      ; cmd in v1
ori     v1, v1, 0x200
.L1:
beqz    a1, .L2
 andi   v0, a3, 0x9FF   ; val in v0, NEW dest (not a3)
ori     v0, v0, 0x400
.L2:
or      v0, v1, v0      ; cmd | val, dst reuses val's reg (v0)
jr      ra
 sw     v0, 4(a0)
```

`*(u32*)(a0+4) = cmd | val` source order — but val IS in v0 (not aliased
to a3). Reaching that allocation in our fork requires keeping a3 alive past
val's andi. Combine pass otherwise rewrites `val = a3 & 0x9FF` from
"andi <newpseudo>, a3, 0x9FF" to "andi a3, a3, 0x9FF" (a3 dies, reuses reg).

Mine (val|cmd source) gets val→v0, cmd→v1 correctly but the OR source order
is wrong: emits `or v0, v0, v1`. Mine (cmd|val source) gets val→a3 due to
combine fold: emits `or v0, v0, a3`. Neither matches target bytes.

## What didn't work

See `meta.json::rejected_forms` for the full list of variants tested
(reorder, split val, in-place a3, result temp var, decls-first form).
The common failure: every variant that doesn't add extra instructions
either folds val→a3 or emits the wrong OR operand order.

## Next-step hypotheses

See `meta.json::next_hypotheses`. The key technical question: is there
ANY pure-C construct that prevents combine from folding val's andi dest
into a3, without falling into the forbidden `(void)a3` / `a3 = 0` /
volatile-coerce / chain-extender family per [[no-new-park-categories]] /
[[register-alloc-pure-c]] §6 + Lever D?

Cheat-reviewer was NOT invoked on the candidate body in this session.
The candidate body itself is plain C (no pins, no asm injection, no
dead stores) — it's purely "doesn't quite match" rather than "matches
via a cheat." But the policy gate is the byte match, not the absence of
cheats — and this body doesn't match.

## Related rules

- [[register-alloc-pure-c]] — the parent RA-via-C-structure playbook.
  Step 0 diagnosis confirms our build is the anomaly (target uses the
  preferred-lower reg for val).
- [[inline-asm-policy]] — the HEAD pins are cheat-asm and the function
  cannot be COMPLETED-C with them.
- [[difficult-is-not-impossible]] — the matching C exists; we just
  haven't found it.
- [[no-new-park-categories]] — keep this as parked + WIP; do NOT
  propose a "commutative-or wall" as a new category.
