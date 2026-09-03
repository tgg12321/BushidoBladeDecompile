# SELF-VET — func_80033550  (session 18, 2026-09-03; re-measured in full this session)

CONSTRUCTS: (1) aggregate merge of splat per-word scalars D_80107850/54/58 into
one canonical `extern LeafPos D_80107850[6];` declaration in include/code6cac.h,
with both per-word siblings deleted from include/code6cac.h,
undefined_syms_auto.txt:995-996 and named_syms.txt:2562-2563; (2) declaration
correction of the slot-state flag object from `extern u8 D_800A3918;` (scalar)
to `extern u8 D_800A3918[6];` in include/code6cac.h; (3) the function body —
an ordinary `for` search loop, an early `return`, `D_800A3918[i] = 1;` and a
struct assignment `D_80107850[i] = *arg0;`. No FAKE construct, no dead local,
no wrap, no cast, no pointer pun, no volatile, no asm.

## T1 semantic purpose
(1) and (2) are DECLARATIONS, not statements: they change what the C says the
storage IS. Both are load-bearing for meaning, not just bytes — with the
scalar declarations the source cannot even express "store the caller's record
into slot i" or "read flag i" without address arithmetic. (3) every statement
is required by the specification: find the first free slot of six, bail if
none, mark it taken, write the caller's position record into it. Delete any
line and the function stops doing its job. Nothing in the diff is
behaviour-neutral.

## T2 human-programmer
Yes — this is the form a programmer writes from the spec, and it is strictly
simpler than the alternative. `D_80107850[i] = *arg0;` and `D_800A3918[i]` are
what you write when you know the data layout; the previous spellings
(`*(&D_800A3918 + i)`, and the banned `*(Word3 *)(((u8 *)(&D_80107850)) + i*12)`)
are what you are forced to write when the declarations lie about the layout.
A reader asks "why is this here?" about the OLD form, not this one. The magic
stride constant 12 has disappeared from the C entirely.

## T3 GCC-internals justification
No. Each line is justified by program logic alone; the diff would be written
this way by someone who had never heard of GCC's allocator. A GCC mechanism
(the MIPS block-move expansion in the struct assignment keeps the source
address register live across all three loads, so pseudo 72's conflict set grows
from {2,3,4,29} to {2,3,4,5,6,29} and global.c's pass-0 first-free lands on
$a3 instead of $a1) EXPLAINS why the correct object model happens to match the
target — but it is not the reason any construct is present, and no construct
exists solely to reach that mechanism. The mechanism is a consequence of
writing the truth, not the motivation for a device.

## T4 permuter/search provenance
No. Provenance is the object model, established by committed naming-census rows
that predate this grind by two months (the named_syms.txt row for 0x80107850
dates to commit e44dcd95, 2026-05-17; this function's grind opened 2026-07-21)
and by base-register stride indexing in the ORIGINAL BINARY in a different
function (asm/funcs/func_800335D8.s: `&D_80107850` into $s2 at 0x800335EC/F0,
walked with `addiu $s2, $s2, 0xC` at 0x80033704). Six permuter campaigns across
s4/s5 (~138k iterations) never found this; it came from reading the data layout.

## T5 family check
Constructs (1) and (2) are both instances of the sanctioned "Per-word splat
symbol → aggregate merge" family (owner ruling 2026-08-17), claimed below with
all five prongs answered. Construct (3) is ordinary C and matches no
forbidden family at all: it is not a pin, not asm, not a barrier, not a
volatile coercion, not a dead store, not a dead local, not a wrap, not a
manufactured branch, not a duplicated statement, not a cast. It is notably NOT a respelling of either
banned construct — both bans are on PER-USE POINTER PUNS at the call site, and
this diff removes address arithmetic from the body rather than respelling it:
the fix lives at the declaration, which is exactly the defect each ban named.

## T6 naming-announces-intent
No. One name is introduced: the type `LeafPos` (the census calls the table a
"position array"; the members are x/y/z). Every other identifier is the
pre-existing splat name. No `pad`, `dummy`, `unused`, `spill`, `tmp`, `slack`.
Every declared object is read and written.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Per-word splat symbol → aggregate merge (owner ruling 2026-08-17)
  SCOPE: "two or more splat-invented `D_<addr>` scalars may be replaced by a single aggregate declaration."
  PRECEDENT: .claude/rules/no-new-park-categories.md:238

  Prongs, for construct (1) — D_80107850/54/58 → LeafPos D_80107850[6]:
  (a) Object model independent of and predating the byte-chasing:
      named_syms.txt:1556 committed census row
      `g_leaf_position_table = 0x80107850;  /* 12-byte stride per leaf, 6
      entries = 72-byte position array */` (committed 2026-05-17, e44dcd95),
      PLUS base-register stride indexing in the original binary in a DIFFERENT
      function: asm/funcs/func_800335D8.s loads &D_80107850 into $s2
      (0x800335EC/F0) and walks with `addiu $s2, $s2, 0xC` (0x80033704), six
      iterations. This is stride evidence, not adjacency
      ([[splat-symbol-names-are-not-evidence]] respected).
  (b) Declaration reflects that shape: a 6-entry table of 12-byte records,
      `extern LeafPos D_80107850[6];` with `LeafPos = {s32 x, y, z}`. The stride
      lives in the type; the disqualifying magic-stride index (`i * 12`) is
      absent — the use site is `D_80107850[i] = *arg0;`.
  (c) Complete: both merged per-word symbols removed from C
      (include/code6cac.h:479-480) AND from the splat symbol configs
      (undefined_syms_auto.txt:995-996, named_syms.txt:2562-2563), leaving
      exactly one C handle per storage location.
  (d) Spelled at the canonical declaration in the shared header
      include/code6cac.h — not TU-local, no per-use pointer pun anywhere.
  (e) Byte-neutral for every other consumer: grep over src/ and include/ finds
      no C consumer of D_80107854/D_80107858; the only other reader of the
      table (func_800335D8) is still INCLUDE_ASM and unaffected; full
      `verify-oracle` this session returned build_sha1
      62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle, build_matches true.
      Layer-2 cheat-reviewer still owed at acceptance.

  Prongs, for construct (2) — D_800A3918 scalar → u8 D_800A3918[6]:
  (a) named_syms.txt:1554 committed census row
      `g_leaf_slot_state = 0x800A3918;  /* 6-byte slot state table (per-leaf
      counter byte) */`, with :1555 recording `g_leaf_slot_state_end =
      0x800A391E; /* end marker for slot iteration (6 bytes after base) */`.
      Corroborated in the original binary (func_800335D8.s walks the flag array
      at stride 1 in lockstep with the 12-byte table, bounded by D_800A391E)
      and in already-COMPLETED-C source: src/code6cac_b.c:2856-2870
      (func_80033510) clears exactly six bytes downward from base+5.
  (b) Declaration reflects that shape: a FLAT 6-byte array where the evidence
      shows a flat array of per-slot bytes — not a struct. No stride constant.
  (c) Complete: `extern u8 D_800A3918[6];` is the ONLY C declaration of that
      object anywhere in src/ or include/ (grep, this session), and there are
      no per-word sibling symbols for 0x800A3918 in undefined_syms_auto.txt or
      named_syms.txt to delete — so exactly one C handle per storage location
      is already the post-state. Two neighbours are deliberately NOT folded and
      are disclosed here rather than hidden:
        - D_800A391E is the one-past-the-end iteration bound (base+6) per
          named_syms.txt:1555 — it is not an element, and the layer-1 reviewer
          that prescribed this fix said so explicitly. It keeps its own
          declaration and symbol.
        - D_800A391D (named_syms.txt:824, `g_motion_select_byte`) occupies
          base+5 and therefore aliases D_800A3918[5]. It is a distinct
          pre-existing splat symbol with consumers OUTSIDE this function
          (src/code6cac_b.c:2865 in func_80033510, plus unused externs in six
          other TUs), so retiring it is a project-wide symbol change outside
          this function's granted surface (`func_80033550 include/code6cac.h
          undefined_syms_auto.txt named_syms.txt`). It is banked as a follow-on
          in the ledger frontier, not smuggled: this diff neither creates nor
          uses that alias.
  (d) Spelled at the canonical declaration in include/code6cac.h:240. The body
      uses `D_800A3918[i]` — the per-use pointer pun `*(&D_800A3918 + i)`
      (a `banned_constructs` entry) is DELETED by this diff, not respelled.
  (e) Byte-neutral for every other consumer: there is no other C consumer at
      all; full `verify-oracle` SHA1 == oracle as above. Layer-2 still owed.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. The diff contains zero
`/* FAKE */` constructs: the aggregate-merge family's prong list does not
mandate an annotation (unlike the dead-store, constant-holder, pointer-alias,
duplicated-statement, dead-array and do-while(0) families), and no construct in
this diff belongs to any annotation-requiring family. Both changed declarations
declare real objects; every statement in the body is semantically load-bearing.
The previous candidate's `do { } while (0);` wrap is gone.
