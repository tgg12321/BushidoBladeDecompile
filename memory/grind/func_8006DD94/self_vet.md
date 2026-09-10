# SELF-VET — func_8006DD94

Session s6 (synthesis, 2026-09-10). Body submitted: the one cleared by the Judge's
2026-09-10 08:40 PASS ruling (docs/grind/decisions.md, body hash b00f9e03c891cf0e),
verbatim from memory/grind/func_8006DD94/pending-ruling-oversized-descriptor-0x34-oracle-match.c,
now also saved as memory/grind/func_8006DD94/candidate.c and spliced into src/text1b.c.

MEASURED THIS SESSION on the current chassis:
  `sandbox func_8006DD94 --disable all` -> score 0, target_insns 117, build_insns 117,
    scorable true, rules_dropped 0.
  `verify-oracle` -> ok true, build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa,
    expected identical, build_matches true.

CONSTRUCTS: function-local `typedef struct EnvB` whose declared size is 0x34 with the
trailing two words `pad2C, pad30` never written and never read (the "unwritten tail" of
an otherwise live, address-taken descriptor object `EnvB s;`), carrying a single
`/* FAKE: oversized locals object ... */` annotation. Everything else in the diff is
ordinary C: the 3-iteration loop, the descriptor field stores, the func_8007352C /
SetDrawMode / AddPrim / func_8006D808 calls, and the live `u16 rect[4]` passed to
func_80069898.

## T1 semantic purpose
The one construct under scrutiny is the unwritten tail of `EnvB`. Its observable effect
is the size of the declared locals object, which is what the target bytes encode: the
target frame is `addiu sp,-0x78` (asm/funcs/func_8006DD94.s:2) with seven callee-saves
$s0-$s5/$ra at sp+0x58..0x70 (ALIGN8(28) = 0x20) and a 0x18 outgoing-args area for the
5-argument func_8006D808 call, so the locals region is 0x78 - 0x20 - 0x18 = 0x40 = 64
bytes. The only bytes any instruction in the target touches inside that region are the
0x2C descriptor at sp+0x18..0x43 and the 8-byte rect at sp+0x50..0x57 — 52 bytes. A
fully-written locals set measures vars=56 and yields 0x70, not 0x78. So the declared
size is a real, byte-observable property of the original source that a 0x2C descriptor
cannot express; the construct is not behaviour-neutral padding added for effect, it is
the recovery of the original declaration. The remaining constructs are all live values
consumed by calls.

## T2 human-programmer
Yes. A programmer writing this descriptor type writes a struct; whether its tail words
are used by THIS caller is not something the writer of the struct controls, and reserving
declared-but-unused trailing members in a working buffer/descriptor is everyday C. The
in-tree exemplar is func_80041BF4's `s16 rect[8]` at src/text1a_post.c:387-400, accepted
on main; SOTN ships the identical shape (`u8 _pad[40]; // n.b.! needs to be 33-40 bytes
(inclusive)`, boss/bo4/unk_45354.c:463). No reader asks "why is this here?" of a struct
tail; they ask what the fields mean, and the FAKE annotation answers honestly that the
exact tail size is recoverable only as a range.

## T3 GCC-internals justification
The MECHANISM cited in the annotation is the frame equation itself (mips.c
compute_frame_size / get_frame_size: frame = ALIGN8(vars) + ALIGN8(args) + ALIGN8(gp_regs)),
plus stmt.c:3419 clamping a BLKmode automatic to BIGGEST_ALIGNMENT, which is why the size
is a RANGE (0x34 and 0x38 are byte-identical; 0x30 puts the rect back at sp+0x48 and
scores 21). This is the frame-math proof the OVERSIZED-LOCALS carve-out REQUIRES, not a
lever exploiting an allocator/scheduler/DCE quirk. The construct does not steer register
allocation, scheduling, or CSE; it declares an object of a size the target bytes prove.

## T4 permuter/search provenance
Not permuter output. The form was derived from the target's own frame equation in s5 by
reading asm/funcs/func_8006DD94.s, and the Judge independently re-derived the same
equation from the asm in the 08:40 ruling before granting it. The 56k permuter iterations
banked in s2 found a DIFFERENT construct (an interior volatile pad) which was rejected;
this body is not it. It passes review on its evidence, not on detector blind spots — the
sandbox does NOT strip it (the object is live and partially written), so the honest floor
reads true at 0 rather than being masked.

## T5 family check
It is the OVERSIZED-LOCALS carve-out's prong-2 (live-object-with-unwritten-tail) shape,
which is a frozen sanctioned family — see SANCTIONED-FAMILY-CLAIMS below. It is NOT the
phantom-frame-slot volatile pad family (which requires first-decl array position and an
_SANCTIONED_UNWRITTEN_PADS row) — that family was adjudicated and closed for this function
by the 05:59 and 07:42 rulings, and this body does not claim it. It is NOT the banned
`u16 rects[2][4]` construct nor the merged-rect declaration: no separate dead object is
declared, no dead array row exists, and no fully-dead pad local is added (the carve-out
deprioritizes that fallback precisely because a live object exists to extend here). The
08:40 ruling states in terms: "Ban entry 1 (the trailing-member spelling) is narrowed by
this ruling; the rects[2][4] entries stay banned."

## T6 naming-announces-intent
The tail members are named `pad2C, pad30`, and the earlier interior gaps `pad0C`,
`pad20, pad24`. These are offset-descriptive names for struct members of an unnamed
original type, the ordinary convention for un-reverse-engineered struct fields across this
repo and SOTN (`_pad`, `unk_XX`). They are not local variables named to announce coercion
intent (`dummy`, `spill`, `slack`, `_frame_pad`), and the object they belong to, `s`, is
live: its address is passed to func_8007352C every iteration (`addiu $a0,$sp,0x18` /
`jal func_8007352C`, asm/funcs/func_8006DD94.s:68-69). The FAKE annotation states plainly
that the tail is unwritten and that its exact width is range-recoverable — it announces the
uncertainty rather than hiding it.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: OVERSIZED-LOCALS carve-out (owner ruling 2026-07-13), live-object unwritten-tail prong
  SCOPE: "**OVERSIZED-LOCALS CARVE-OUT (owner ruling 2026-07-13):** a stack-locals object with an **unwritten tail** (a written-prefix buffer, e.g. func_80037540's `s32 sp[8]` with only `sp[0..5]` stored and the buffer passed live to a callee) — or, as the fallback when no live object exists to extend, a dead pad local — is sanctioned as a last-resort matching construct **when the target bytes PROVE the original declared it**."
  PRECEDENT: src/text1a_post.c:387
  PRECEDENT: docs/grind/decisions.md:443
  PRECEDENT: docs/grind/decisions.md:26632

ANNOTATION-CONFORMANCE:
  /* FAKE: oversized locals object - `s` is the LIVE descriptor whose address is
     passed to func_8007352C every iteration; pad2C/pad30 are its unwritten tail.
     mechanism: mips.c compute_frame_size / get_frame_size -
     frame = ALIGN8(vars) + ALIGN8(args) + ALIGN8(gp_regs). ...
     Lever-exhaustion: memory/grind/func_8006DD94/hypotheses.md - 5 sessions,
     1,080 enumerated spellings (973 loop-tail + 65 rect-block + 42 declaration
     orders), 56k permuter iterations over 2 campaigns, 4 class kills ... */
  (single annotation, quoted here abridged; the full text in src/text1b.c and
  candidate.c carries WHAT — the oversized live locals object and which members are the
  unwritten tail; MECHANISM — mips.c compute_frame_size/get_frame_size frame equation
  plus stmt.c:3419 BLKmode alignment clamp, with the full frame-math derivation and the
  0x34/0x38 range statement; and LEVER-EXHAUSTION — a pointer to hypotheses.md with the
  session count, spelling count, permuter iteration count and the four class kills.)
