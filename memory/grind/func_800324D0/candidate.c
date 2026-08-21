/* func_800324D0 — BEST FORM (s5 ledger entry, brief-session 4, 2026-08-20):
 * sandbox --disable all = 15, build_insns 68 == target 68. FIRST floor drop
 * since s1 (27 -> 15).
 *
 * Chassis: the s4 combined respelling (probe-A no-cmd-copy + V1 bare-switch +
 * V2 while-form + probe-B literal-0xFF — each measured flat 27 individually in
 * s2/s3, combination measured flat 27 this session) PLUS the load-bearing
 * staged loop-tail read: `c = *ptr` written as `cmd = *ptr; c = cmd;`.
 *
 * The staged read is a permuter find (campaign #2, output-105-1) vetted and
 * hand-measured: it borrows the EXISTING, currently-dead u32 `cmd` for the
 * tail stream byte. Family: staged-value-reused-variable
 * (.claude/rules/staged-value-reused-variable.md, SANCTIONED 2026-07-03) —
 * bounds check: (1) value real, consumed next line by `c = cmd`; (2) cmd
 * exists for a real job (payload command selector); (3) borrow provably safe
 * (cmd's arm value dead at the tail — it is re-derived from c at the next
 * arm entry; staged value not needed after cmd's next assignment); (4) FAKE
 * annotation in place at the site; (5) receipts = s1-s4 exhaustion + two dry
 * campaigns. SOTN PSX precedent: the `// fake reuse of i?` staged-load shape,
 * docs/reference/sotn-construct-index.md:51,81,92,97,109 (i = *scriptCur++
 * through an existing variable). CITATION QUESTION RESOLVED [s7, synthesis]:
 * the rule's six bounds are mechanism-silent; bound 4 requires naming the
 * ACTUAL pass (ours: global.c allocno census — the extra cmd set splits cmd
 * into head web 75 + arm web 85 and re-seeds find_reg); the sched.c
 * reference is descriptive Origin text, not a bound; the 2026-08-17
 * clarification (no-new-park-categories.md:193-214) establishes SOTN
 * acceptance is shape-based, and the SOTN exemplar is literally this shape.
 * Full argument: tmp/grind/func_800324D0/s6/probes_and_partition.md — quote
 * it in the submitting session's self-vet.
 *
 * u32 cmd is load-bearing for the staging (borrowing u8 val instead: 27;
 * u8 c for val's load: flat; placement `ptr++` between read and copy: 28/69).
 *
 * Residual 15 = exact 2-register swap: walker ours $6 / target $3($v1), both
 * cmd webs ours $3 / target $6($a2). find_reg ground truth (s4/findreg*.log):
 * order 75,76,85,72,74,73,91,86; 75 takes first-free 3. For $3 to survive to
 * the walker, 75 AND 76 AND 85 must all skip it => a conflicting allocno with
 * hard-reg-pref $3 => the walker itself (defs are self-increments + lw from
 * mem(pad): set_preference CANNOT plant — s2 proof unchanged) or an invented
 * overlapping pair (the BANNED base/ff family). Priority inversion (walker
 * allocated first WOULD yield the full target cascade with zero constructs —
 * verified against the exclusion sets) needs walker density ~5x: arithmetically
 * dead (s1). The wall is the SAME single sufficient condition as s1, one
 * construct short of it.
 */
void func_800324D0(u8 *pad) {
    u8 *ptr;
    u8 c;
    u32 cmd;
    u8 val;

    ptr = *(u8 **)(pad + 0x58);
    pad[0xA1] = 0xFF;
    pad[0xA3] = 0xFF;
    pad[0xA2] = 0xFF;
    pad[0xA4] = 0xFF;
    pad[0xAA] = 0;
    pad[0xA7] = 0;
    pad[0xA8] = 0;
    pad[0xA5] = 0;
    pad[0xA6] = 0xFF;
    pad[0xAB] = 0xFF;
    pad[0xAC] = 0xFF;

    c = ptr[4];
    ptr += 5;
    while (c != 0) {
        if (c == 0xFF) {
            ptr += 6;
        } else if (c < 0x80) {
            ptr++;
        } else {
            cmd = c - 0x80;
            val = *ptr;
            ptr++;
            {
                switch (cmd) {
                    case 0: pad[0xA1] = val; break;
                    case 1: pad[0xA3] = val; break;
                    case 2: pad[0xA7] = val; break;
                    case 3: pad[0xA8] = val; break;
                    case 4: pad[0xA9] = val; break;
                    case 5: pad[0xA5] = val; break;
                    case 6: pad[0xA6] = val; break;
                    case 7: pad[0xA2] = val; break;
                    case 8: pad[0xA4] = val; break;
                    case 9: pad[0xAA] = val; break;
                    case 10: pad[0xAB] = val; break;
                    case 11: pad[0xAC] = val; break;
                }
            }
        }
        /* FAKE: loop-tail stream byte staged through the currently-dead cmd
         * (c = *ptr written as cmd = *ptr; c = cmd), mechanism: global.c
         * allocno census - the extra cmd set splits cmd into head/arm webs and
         * re-seeds find_reg (val/byte/holders land in target regs; floor 27->15,
         * 68/68), lever-exhaustion: memory/grind/func_800324D0/hypotheses.md
         * s1-s4 + two dry permuter campaigns (evidence.md s4/s5) */
        cmd = *ptr;
        c = cmd;
        ptr++;
    }
}
