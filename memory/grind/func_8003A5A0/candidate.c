/* func_8003A5A0 — candidate, session 1 (recon), honest sandbox distance 0
 * (`sandbox func_8003A5A0 --disable all` → score 0, 87/87 insns, all 21 regfix
 * rules dropped + cheat-asm stripped). Pure C; no volatile, no pins, no asm,
 * no dead stores, no FAKE constructs.
 *
 * Four independent levers took the floor 21 → 16 → 6 → 1 → 0:
 *  L1 (21→16) The poll-loop temporaries must NOT reuse the same local as the
 *      return value. The old body did `v0 = (_comb_control(0,0,0) >> 7) & 3;`
 *      and `v0 = GetRCnt(...) - s0;` into the shared `v0`. That single allocno
 *      lost $v0 to the short block-local temps (local_alloc runs before
 *      global_alloc and takes the first free reg), flipping the whole
 *      function's $v0/$v1 usage. Writing both poll conditions as direct
 *      expressions lets the compare temps coalesce with the call return in $v0.
 *  L2 (16→6) Drop the `epilogue: return v0;` shared join and return the value
 *      at each exit. The join forced a `move $v0,$v1` copy the target does not
 *      have (target's .L8003A6E4 is a bare epilogue) and cost an extra insn.
 *  L3 (6→0 partly) Both `if (s1 >= 5)` retry-exhausted exits must `goto` ONE
 *      shared `ret0_tramp` label. With two inline `return 0;` bodies, jump2's
 *      find_cross_jump merged them into a single fall-through block (85 insns,
 *      2 short) and inverted the poll branch. With two DISTINCT labels they
 *      re-merge (measured: 7) and the surviving block lands after `match`,
 *      breaking match's fall-through into the epilogue. One shared label gives
 *      target's exact shape: the loop_check branch keeps its out-of-line 2-insn
 *      trampoline (its delay slot is already taken by the fall-through's
 *      `a0 = 2` arg setup) while reorg copies `v0 = 0` from that block into the
 *      poll branch's empty delay slot and redirects it straight to the epilogue.
 *  L4 (1→0) `v0 = a1 ^ (a1 >> 16);` as ONE expression. Written as two statements
 *      (`v0 = a1 >> 16; v0 = v0 ^ a1;`) the xor's destination IS operand 1, so
 *      expand_binop swaps the commutative operands to avoid a copy and emits
 *      `xor $v0,$v0,$a1`; target has `xor $v0,$a1,$v0`. Source-level operand
 *      order does not fix it (measured — still 1); a destination distinct from
 *      both operands does.
 */
extern s32 D_800A38D0;
s32 func_8003A5A0(void) {
    s32 s0;
    s32 s1;
    s32 a1;
    s32 a0;
    s32 v0;

    s1 = 0;
    s0 = GetRCnt(0xF2000001);
    if (s0 >= 0x401) {
        goto overflow;
    }
    goto loop_check;
overflow:
    ResetRCnt(0xF2000001);
    s0 = 0;
loop_check:
    if (TestEvent(D_800A3738) != 0) {
        goto success;
    }
    if (TestEvent(D_800A3810) == 0) {
        goto poll;
    }
    s1 += 1;
    if (s1 >= 5) {
        goto ret0_tramp;
    }
    _comb_control(2, 0, 0);
    s0 = 0;
    func_8003A574();
    ResetRCnt(0xF2000001);
poll:
    if (((_comb_control(0, 0, 0) >> 7) & 3) == 1) {
        goto loop_check;
    }
    if (GetRCnt(0xF2000001) - s0 < 0x3C01) {
        goto loop_check;
    }
    s1 += 1;
    if (s1 >= 5) {
        goto ret0_tramp;
    }
    goto overflow;
success:
    a1 = D_800A3688;
    a0 = D_800A368C;
    v0 = a1 ^ (a1 >> 16);
    v0 = v0 ^ (a0 >> 16);
    v0 = v0 & 0xFFFF;
    if ((a0 & 0xFFFF) == v0) {
        goto match;
    }
    D_800A38D0 += 1;
    return 0;
ret0_tramp:
    return 0;
match:
    D_800A36C0 = a1;
    D_800A36C4 = a0;
    return 1;
}
