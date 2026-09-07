# SELF-VET — func_80022F34 (grind session s12, 2026-09-06)

CONSTRUCTS: (1) header declaration corrections `extern u8 D_80102782[];`,
`extern s32 D_801027BC[][5];`, `extern u16 D_80102778[2];`; (2) byte-offset
record pointer `u8 *rec = (u8 *)&D_80101EC8 + offset;` with `offset += 0x44C`;
(3) walking table pointer `u16 *tbl = D_80102778; ... tbl++;`; (4) a `switch`
on the audio state with `case 3:` empty and `case 1/2/default:` merged;
(5) block-scoped locals `mode`, `idx1`, `val1`, `idx2`; (6) `rec = *(u8 **)rec;`
link-follow; (7) a bottom-tested loop spelled `loop_22F34: ... if (i < 2) goto
loop_22F34;`; (8) two call-site updates inside the already-matched
`func_8001C444` (`D_8010277A = 0x800;` / `D_80102778 = 0x800;` become
`D_80102778[1] = 0x800;` / `D_80102778[0] = 0x800;`) required by construct (1).

## T1 semantic purpose
Every construct carries an observable value. (1) states the object model the
target's own addressing implies (indexed loads, see asm/funcs/func_80022F34.s
lines 38-63) and is required for the code to type-check without a cast.
(2)/(3) compute the addresses of the two records and the two table entries
actually read. (4) selects which value is stored into `rec + 8`. (5) name the
values that are read and passed. (6) follows the record's first-word link
pointer to the opponent record. (7) is the loop over the two fighters.
(8) writes the same two u16 objects that the old scalar spelling wrote, at
the same addresses. Deleting any one of them changes the function's behaviour.
No construct is present that the output does not depend on.

## T2 human-programmer
Yes. A programmer writing "for each of the two fighters, if the record is
live, store the current display value, then hand the fighter's status-up
figure and the opponent's to single_game_SetStatusUpData" writes exactly this.
There is no construct here a reader would ask "why is this here?" about.

## T3 GCC-internals justification
None. No construct in this diff is justified by, or reasoned about in terms
of, a GCC pass. The reasoning for every construct is the program's data model
and the target's addressing modes. (The GCC-pass forensics recorded in
sessions s6-s9 explained why the SCALAR declarations could not match; the fix
adopted is a declaration correction, and the ledger argument for it is the
census + the target's own indexed loads, not an allocator mechanism.)

## T4 permuter/search provenance
None. The permuter campaigns (s4, s5) were both KILLED and contributed
nothing to this form. This body was derived by hand from the target's
addressing (s10), re-derived on clean HEAD (s11), and refined this session.

## T5 family check
No construct matches any forbidden family. There are no register pins, no
`__asm__`, no scheduling barriers, no volatile, no dead stores, no dead
locals, no unused arrays, no `(void)&x`, no alias renames, no `if (1)`
wrappers, no `do { } while (0)` wraps, no opaque constant holders. Every local
declared is written once and read. The previously flagged declaration puns
`(&D_801027BC)[idx*5]` and `(&D_80102782)[i]` are GONE — they were replaced
by the array declarations in construct (1), i.e. fixed at the declaration as
the aggregate/object-model policy requires, not respelled at the use site.

## T6 naming-announces-intent
No name in the diff is `pad`, `dummy`, `unused`, `spill`, `slack`, `tmp`, or
similar. Names are `i`, `tbl`, `offset`, `rec`, `mode`, `idx1`, `val1`,
`idx2` — each describing the value it holds.

### The one remaining raw-offset access, argued (owner directive 2026-09-06)
`u8 *rec = (u8 *)&D_80101EC8 + offset;` is retained deliberately:
 - It is the established repo-wide spelling for this symbol, already shipping
   inside functions that are COMPLETED-C: src/code6cac.c:846, 1052, 1570,
   2103, 2220; src/code6cac_b.c:666, 868, 880; src/text1b.c:1520, 1531.
   It is not a construct invented for this function.
 - It mirrors the target's own addressing: the symbol base is re-materialised
   inside the loop (`lui/addiu %hi/%lo(D_80101EC8)`, func_80022F34.s:12-13)
   and combined with a byte-offset register stepping 0x44C
   (`addu $a0, $s1, $v0`, line 14) — a base+byte-offset walk, not a
   subscripted array access.
 - It does not misrepresent the referent's type: D_80101EC8 is declared `u8`
   and the expression addresses bytes. This is unlike the two puns that WERE
   corrected, which subscripted past the end of a declared `s32`/`u8` scalar.
 - Correcting the declaration to `u8 D_80101EC8[][0x44C]` cannot be done here:
   it requires simultaneous edits to src/code6cac_b.c, src/code6cac_c_ab.c and
   src/text1b.c (all outside this function's scope grant) and would perturb
   already-matched functions. It is banked as an integration handoff.

SANCTIONED-FAMILY-CLAIMS: none — this diff contains no FAKE construct and
claims no SOTN carve-out. Every construct is ordinary C.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.

## Measurements backing this vet (this session, clean HEAD)
  sandbox func_80022F34 --disable all -> score 0, 70/70 insns, rules_dropped 0
  sandbox func_8001C444 --disable all -> score 0 (construct (8) byte-neutral)
  verify-oracle -> ok true, build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa
                   == original_sha1_locked
