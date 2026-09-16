# Evidence — _SsSndCrescendo

## s1 (recon)

OBJECT MODEL: `_ss_score` MATCHES — the target asm's addressing
(`sll a0,16; sra ,14` == `(s16)a0*4`, then `addu` onto `%hi/%lo(_ss_score)`
base, then `lw` the resulting pointer, then `+ (s16)a1*0xB0`) is byte-for-byte
the SAME pattern already used, matched, and byte-proven at 10+ call sites in
this same TU (`src/main.c:318-345,370-372,583,609-642,809-855,1262-1285`,
e.g. the byte-matched `_SsSeqPlay` at main.c:341-366). `_ss_score` is
declared `extern s32 _ss_score;` (scalar handle, main.c:37/231) and accessed
via `(s32 *)&_ss_score` + byte-offset pointer arithmetic — this is the
established, ALREADY-PROVEN idiom for this global in this codebase. No
declaration-fix hypothesis applies; proceeding straight to codegen structure
is correct here. (No sandbox measurement needed for this conclusion — it's
a structural match against 10 already-oracle-verified call sites, not a
guess.)

## SOTN precedent found (STRONG — exact function name, not analog)

`tmp/sotn-decomp/src/main/psxsdk/libsnd/cres.c` contains a **matched PSX
`_SsSndCrescendo`** (SOTN's own decomp of the same Sony LIBSND `CRES`
module BB2 links — confirmed identity via `memory/closer/libsnd-hunt-report.md:70`,
`memory/closer/sony-naming-map.md:26`: "func_800841E0 -> _SsSndCrescendo
(LIBSND CRES) ; identity by call position in SsSeqCalledTbyT"). Full text
read this session (49 lines):

```c
void _SsSndCrescendo(s16 arg0, s16 arg1) {
    struct SeqStruct* score = &_ss_score[arg0][arg1];
    u16 voll, volr;
    score->unk98--;
    if (score->unk42 > 0) {
        if ((score->unk98 % score->unk42) == 0) {
            score->unk40--;
            if (score->unk40 >= 0) {
                SpuVmGetSeqVol(arg0|(arg1<<8), &voll, &volr);
                if ((voll+1) <= (voll+score->unk40))
                    SpuVmSetSeqVol(arg0|(arg1<<8), voll+1, volr+1, 0);
            } else {
                SpuVmSetSeqVol(arg0|(arg1<<8), 0x7F, 0x7F, 0);
                _ss_score[arg0][arg1].unk90 &= ~0x10;
            }
            if ((score->unk98==0)||(score->unk40==0))
                _ss_score[arg0][arg1].unk90 &= ~0x10;
        }
    } else if (score->unk42 < 0) {
        score->unk40 += score->unk42;
        if (score->unk40 >= 0) {
            SpuVmGetSeqVol(arg0|(arg1<<8), &voll, &volr);
            if (((voll-score->unk42)>=0x7F) && ((volr-score->unk42)>=0x7F))
                SpuVmSetSeqVol(arg0|(arg1<<8), 0x7F, 0x7F, 0);
            if (((score->unk94-score->unk98)*-score->unk42) < score->unk3E)
                SpuVmSetSeqVol(arg0|(arg1<<8), voll-score->unk42, volr-score->unk42, 0);
        } else {
            SpuVmSetSeqVol(arg0|(arg1<<8), 0x7F, 0x7F, 0);
            _ss_score[arg0][arg1].unk90 &= ~0x10;
        }
        if ((score->unk98==0)||(score->unk40==0))
            _ss_score[arg0][arg1].unk90 &= ~0x10;
    }
    SpuVmGetSeqVol(arg0|(arg1<<8), &score->unk78, &score->unk7A);
}
```

Caveat: [[psyq-reference-c-version-skew]] applies — BB2 links a DIFFERENT
PsyQ/LIBSND build than the sotn-decomp source tree (hunt report identifies
BB2's build as the "4.1 CRES/DECRES fix" — the changelog explicitly says
CRES/DECRES were *modified* going into 4.1), so this is a strong STRUCTURAL
guide, not a byte-for-byte transcript. Field offsets differ (BB2 struct
stride 0xB0 confirmed from asm; SOTN's `struct SeqStruct` field names are
its own reverse-engineering guesses, not proven offsets for BB2's build).

## BB2 asm field-offset mapping derived this session (from `asm/funcs/_SsSndCrescendo.s`, cross-read against cres.c control flow)

Channel struct base = `s0` = `&_ss_score[a0][a1]` (stride 0xB0, matches
census `memory/closer/*` + main.c precedent).

| BB2 offset | asm evidence | SOTN field (name only, offset unproven) | confidence |
|---|---|---|---|
| `0xA0` | decremented at function entry (`lw;addiu -1;sw`), sign-tested immediately after | `unk98` (SOTN also decrements this field first, line 9) | HIGH — same "decrement first" position |
| `0x4C` | `lh a2,0x4C(s0)`; `blez a2,.L80084318` branches pos/non-pos | `unk42` (SOTN branches `>0` / `<0` on this field right after the decrement) | HIGH |
| `0x4A` | `lhu v0,0x4A(s0); -1; sh back`, then sign-tested (`bltz -> .L8008441C`) inside BOTH the pos-42 and non-pos-42 arms | `unk40` (SOTN decrements/adds-to this field then tests `>= 0` in BOTH arms) | HIGH |
| `0x98` | `lw;andi -0x11(~0x10);sw` — clears bit 0x10, appears 3x (once per exit path) | `unk90` (SOTN clears `& ~0x10` in the same 3 positions: neg-40 branch, and the "unk98==0 \|\| unk40==0" tail check in each of the two 42-sign arms) | HIGH |

## Structural DIVERGENCE found (the actual frontier — not yet resolved)

The BB2 asm has an **outer early-exit that SOTN's cres.c does NOT have**:
after decrementing the `0xA0` field (SOTN's `unk98--`), asm branches
`bgez $v0, .L80084260` — i.e. if the POST-decrement value is still
`>= 0`, fall into the `0x4C` (unk42)-sign logic; but if it went NEGATIVE,
jump straight past ALL of the unk42/unk40 logic directly to the tail
(clear the `0x98` bit + the final `SpuVmGetSeqVol` call at label
`.L800844AC`/`.L800844C0`). SOTN's `cres.c` runs the `unk42`-sign checks
**unconditionally** every call, regardless of `unk98`'s sign after
decrement.

This is evidence the BB2/4.1 build's `_SsSndCrescendo` gained an extra
guard clause not present in whatever build sotn-decomp's `cres.c` reflects
— consistent with the hunt report's note that CRES was one of the modules
"fixed after 4.0" going into 4.1. This guard is the #1 thing to get right
before transcribing the rest; getting it wrong will cost ~150 instructions
of distance in the two skipped branches alone (0x4C/0x4A logic is ~half
the function body).

- [s1] OBJECT MODEL: _ss_score access pattern in the target asm is byte-identical to 10+ already-oracle-verified call sites in this TU (main.c:318-345,370-372,583,609-642,809-855,1262-1285, incl. byte-matched _SsSeqPlay) - no declaration-fix needed, extern s32 _ss_score; scalar-handle idiom is already proven here.

- [s1] tmp/sotn-decomp/src/main/psxsdk/libsnd/cres.c contains SOTN's own matched _SsSndCrescendo for the same Sony LIBSND CRES module (identity confirmed via memory/closer/libsnd-hunt-report.md:70 and memory/closer/sony-naming-map.md:26 by call-position in SsSeqCalledTbyT) - full 49-line reference read this session.

- [s1] BB2 links a different PsyQ/LIBSND build than sotn-decomp's tree (psyq-reference-c-version-skew) - hunt report explicitly notes CRES/DECRES were modules 'fixed after 4.0' going into the 4.1 build BB2 uses, so cres.c is a strong structural guide but not a verbatim transcript.

- [s1] Derived BB2 struct offset mapping (channel struct stride 0xB0, base = &_ss_score[a0][a1]) by cross-reading asm/funcs/_SsSndCrescendo.s against cres.c control flow: 0xA0=unk98 (outer decremented counter), 0x4C=unk42 (sign-branch field), 0x4A=unk40 (inner decremented/added field, sign-tested in both arms), 0x98=unk90 (bit 0x10 cleared in 3 places). Table with per-field asm evidence in evidence.md.

- [s1] Found a genuine STRUCTURAL divergence from the SOTN reference (not yet resolved): BB2 asm wraps the entire unk42-sign logic in an outer guard on the post-decrement sign of unk98 (0xA0) - `bgez v0,.L80084260` falls into the unk42 logic, but the negative case jumps straight past ALL of it to the tail. SOTN's cres.c runs the unk42-sign checks unconditionally regardless of unk98's sign. This guard, once confirmed and spelled, is expected to close roughly half the function's distance by itself since the skipped branches cover most of the body.

- [s1] Final SpuVmGetSeqVol-analog call in the tail writes directly into struct fields at s0+0x5C/0x5E (matches cres.c's final call writing &score->unk78/&score->unk7A) - confirmed by static asm read, not yet compiled.

- [s1] Open call-signature question: every jal to func_80087770 (the SpuVmSetSeqVol analog) passes a literal 1 as the 4th arg where cres.c's SpuVmSetSeqVol always passes literal 0 - func_80087770 is only a WEAK/unproven identity per the hunt report, so this needs checking against func_80087770's own body/other call sites before transcribing calls verbatim.
