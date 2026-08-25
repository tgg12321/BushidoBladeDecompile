# SELF-VET — func_80038C70 (motion_SetMotion), grind session s49

CONSTRUCTS: (1) `goto case12_sel;` and `goto case9_11_sel;` replacing the two duplicated
assignment bodies of the `D_800A3207 == 3` if-arm; (2) two ordinary C statement labels
`case9_11_sel:` and `case12_sel:` placed on the existing `case 9: case 11:` and `case 12:`
handler bodies; (3) the constant correction `sel = 0xC;` -> `sel = 0xD;` in the case 9/11
handler, which removes the semantic lie that regfix.txt:1080 existed to patch.

## T1 semantic purpose
(1)+(2) YES. The if-arm's job is "when D_800A3207 == 3, map this v0 onto the same handler the
switch would have used". Before: v0 == 8 was expressed as `goto case8_sel;` (already in HEAD,
src/code6cac_c_mid.c:880) while v0 == 10 and the residual case were expressed by re-typing the
handler bodies (`sel = 0xF;` / `sel = 0xD;`). After: all three are expressed the same way. The
construct removes duplicated program text and makes one behavioural fact ("this is the case-12
handler / this is the case-9-and-11 handler") explicit instead of implicit. Its observable
effect is the ordinary effect of control flow: the case bodies are now single-sourced, so a
future edit to `case 12:` cannot silently diverge from the `v0 == 10` arm.
(3) YES and load-bearing on correctness: the shipped executable selects 13 at that site
(asm/funcs/func_80038C70.s:170-172, `.L80038EC8: j .L80038EDC / addiu $s0,$zero,0xD`). The
committed C said 12 and a build-time regfix subst rewrote the emitted immediate to 13. The C
now states the value the program actually has. Removing a semantic lie is the opposite of a
byte-neutral decoration.

## T2 human-programmer test
YES — and the function itself is the witness. `goto case8_sel;` is already the HEAD spelling of
exactly this idea in exactly this if-arm; a reader who accepts line 880 has no "why is this
here?" question about lines 882-885. A human writing "if D_800A3207 == 3, v0 == 10 behaves like
case 12 and everything else behaves like cases 9/11" writes this. No reader needs to know
anything about GCC to read it; the diff is shorter and less duplicated than what it replaces.

## T3 GCC-internals justification test
The construct is NOT justified by a GCC internal — it is justified by the program logic above,
and I would defend it on a source-quality review with no compiler in the room. GCC internals
appear only in the post-hoc EXPLANATION of why 48 prior sessions could not reach these bytes
with the duplicated spelling (jump.c find_cross_jump merged the two identical `set 13; j join`
blocks). That is the diagnosis of the old form's failure, not the mechanism the new form
exploits: the new form has no `set 13` in the if-arm at all because the if-arm no longer
contains that statement in the C. No lever naming, no pass steering, no allocator manipulation.

## T4 permuter/search provenance
Not permuter output. No permuter or auto-search ran this session. The form was derived by
reading the shipped bytes (`beq -> join` with `addiu $s0,0xF` in the delay slot, then
`j -> join` with `addiu $s0,0xD`) and asking what C produces two *jumps* and no *sets* in that
block; the in-file `goto case8_sel;` precedent supplied the spelling. It passes because the
program is spelled the way the original program was spelled, not because a detector missed it.

## T5 family check
No forbidden family is matched, by shape or by analogy. It is not a register pin, not inline
asm, not a volatile/alias coercion, not a dead or unused local, not a dead store or self-assign,
not a dead conditional/goto pad (both labels are live targets of live gotos and both were
already reachable case bodies), not an `if (1)` / `do{}while(0)` wrapper, not a width cast, not
an aggregate or declaration-order trick, and it manufactures no USE/CLOBBER — the owner-refused
F5 family (docs/grind/decisions.md:789) is not touched, and nothing in the diff exists to break
find_cross_jump by interposition. It adds no variable, no type, no declaration and no operator;
it is a control-flow edit only, and it strictly DELETES program text (two assignment statements
replaced by two gotos to the statements that already existed).

## T6 naming-announces-intent
Labels are `case9_11_sel` and `case12_sel`. They name the switch cases whose handlers they mark
— the same convention as HEAD's `case8_sel` and `load_sel2`. No `pad`, `dummy`, `unused`,
`spill`, `slack` or `tail` naming; nothing in the diff is unused, discarded, or address-taken.

SANCTIONED-FAMILY-CLAIMS: none — the diff is ordinary C control flow (a `goto` to a labelled
statement, C89 6.6.6). No sanctioned-exception family is being claimed or relied on, so no
scope sentence or FAKE annotation applies to it. In-repo precedent for the identical idiom in
the identical if-arm of this same function, present on main before this session:
  PRECEDENT: src/code6cac_c_mid.c:880

ANNOTATION-CONFORMANCE: n/a — no FAKE construct is introduced by this diff.
  Disclosure (not part of this diff, unchanged from HEAD): the `case 0:` handler carries a
  pre-existing `/* FAKE: duplicate of load_sel2's store ... SOTN duplicate-into-arms family. */`
  annotation on `sel2 = D_800A3350;`. I did not add, move, reword or rely on it. I did MEASURE
  it this session: deleting it scores 13, and respelling it as `goto load_sel2;` also scores 13,
  so it remains load-bearing exactly as its annotation states (rejected form banked at
  memory/grind/func_80038C70/rejected/s49-goto-load-sel2-drops-ra-priority-score13.c).

VERIFICATION THIS SESSION
  `engine sandbox func_80038C70 --disable all` = score 0, target_insns 402, build_insns 402,
  rules_dropped 1, cheat_asm_stripped 7 (honest, cheat-invisible distance zero).
  `engine build` = build/bb2.exe sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle, MATCH.
  regfix.txt:1080 (`subst "addiu\t$16,$zero,12" -> ",13" @ 149`) is now INERT — its pattern no
  longer exists in the emitted stream — which is why the full build matches with the rule still
  physically present. Deleting that one line (operator `retire func_80038C70`) takes the
  function to zero rules / COMPLETED-C. I did not touch regfix.txt.
