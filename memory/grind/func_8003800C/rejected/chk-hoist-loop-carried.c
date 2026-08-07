/* REJECTED (s2, structural) — score 17 (from floor 8). KILLED.
 * Region A probe: hoist the post-loop check value before the inner loop
 *   s32 chk = *(s32*)((u8*)chkptr + 0x6C);   // before the do-while
 *   ... loop ...; if (sum == chk) break;
 * WHY DEAD: the hoisted load becomes a loop-carried pseudo live across the
 * whole inner loop, adding register pressure that cascades the entire
 * allocation (score 8 -> 17, build_insns 79 -> 78). Does NOT isolate the
 * sum/j a0 tie; it explodes it. The frontier's "hoist chk to reprioritize
 * sum" idea is disproven — chk lives too long and poisons the alloc.
 */
        s32 chk;
        j = 0;
        bp = base + offset;
        sum = 0;
        chk = *(s32 *)((u8 *)chkptr + 0x6C);
        do {
            sum += *bp;
            bp++;
            j++;
        } while (j < 0x24U);
        if (sum == chk) {
            break;
        }
