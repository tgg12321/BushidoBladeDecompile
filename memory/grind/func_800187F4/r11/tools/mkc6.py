"""c6 = c5 with work and delta split into single-value locals, each closed by a FAKE dead store
(dead-store-fake-exception), neutral naming, corrected header text. Writes tmp/func_800187F4/c6.c."""
import re

D = "tmp/func_800187F4/"
s = open(D + "c5.c").read()


def once(a, b):
    global s
    assert s.count(a) == 1, (a[:80], s.count(a))
    s = s.replace(a, b)


# ---- header comment ----
once("""/* func_800187F4 -- COMPLETED-INLINE-ASM-CANONICAL (manual lane, 2026-09-28).
 * Rope/cloth node integrator. func_8001924C calls it for each 16-byte rope record
 * (arg0; +0xC enables collision) whose flag bit 0 is clear, with the record's
 * descriptor (arg1: +0 table of 8-byte anchor vectors, +4 s16 node count, +0xC the
 * 64-byte nodes). func_80018094 first sets up the GTE rotation/translation. Per
 * node (words 0-2 position, 3-5 velocity, 6 state, 7/8 force counts, 9-12 packed
 * force-table indices): state >= 0 springs the node toward its GTE-transformed
 * anchor (or snaps to it at 0); state -0xFF..-1 pulls the position back toward the
 * anchor; otherwise the node is integrated freely: the indexed scratchpad forces
 * are added / subtracted, the node is pushed out of the ground and out of each
""", """/* func_800187F4 -- COMPLETED-INLINE-ASM-CANONICAL (manual lane, 2026-09-28).
 * Node-chain integrator. func_8001924C calls it for each 16-byte record (arg0;
 * +0xC enables collision) whose flag bit 0 is clear, with the record's descriptor
 * (arg1: +0 table of 8-byte anchor vectors, +4 s16 node count, +0xC the 64-byte
 * nodes). func_80018094 first sets up the GTE rotation/translation. Per node
 * (words 0-2 position, 3-5 velocity, 6 state, 7/8 force counts, 9-12 packed
 * force-table indices): state >= 0 springs the node toward its GTE-transformed
 * anchor (or snaps to it at 0) and ends there; state -0xFF..-1 first pulls the
 * position toward the anchor and then integrates like state < -0xFF: the indexed
 * scratchpad forces are added / subtracted, the node is pushed out of the ground and out of each
""")
s = s.replace("RopeScratch", "Scr1F800000")

# ---- delta -> dg (ground depth) + dy0 (Y delta to focus 0) ----
once("""            /* Ruling 11 (proof r11/proof.md): two values, both Y deltas -- the
             * node's depth below the ground, then the Y delta to focus 0. */
            s32 delta;

            delta = SCR->pos[1] - SCR->ground;
            if (delta > 0) {
                if (delta > 0x3200) {
                    vy_new = vy - 0x400;
                } else {
                    vy_new = vy - delta / 8;
                }""", """            s32 depth;

            depth = SCR->pos[1] - SCR->ground;
            if (depth > 0) {
                if (depth > 0x3200) {
                    vy_new = vy - 0x400;
                } else {
                    vy_new = vy - depth / 8;
                }""")
once("""                r = SCR->rad[idx];
                delta = SCR->cpos[1] - SCR->sph[idx][1];
                if (delta < -r || r < delta) {
                    continue;
                }
                SCR->d0[1] = delta;""", """                r = SCR->rad[idx];
                dy0 = SCR->cpos[1] - SCR->sph[idx][1];
                if (dy0 < -r || r < dy0) {
                    continue;
                }
                SCR->d0[1] = dy0;""")
once("                s32 dx0, dz0, dy1, dx1, dz1;\n", "                s32 dy0, dx0, dz0, dy1, dx1, dz1;\n")
once("""                @gte_stlvl(SCR->vel);
            }
        }
        node[3] = (SCR->vel[0] * 7) >> 3;""", """                @gte_stlvl(SCR->vel);
            }
            /* FAKE: dead store; mechanism: reg_scan records it as depth's last
             * reference (regclass.c:1764 counts sets), so in cse the `depth / 8`
             * expansion's copy does not outlive depth and cse.c make_regs_eqv
             * (:840-857) keeps depth as the class head: the sign test reads depth
             * (target `bgez $v1`, copy in the delay slot) instead of the copy
             * (`bgez $v0`, 4 insns). flow deletes the store (no bytes). Lever
             * exhaustion: memory/grind/func_800187F4/r11/proof.md (depth). */
            depth = 0;
        }
        node[3] = (SCR->vel[0] * 7) >> 3;""")

# ---- work -> sq1 (squared distance) + dist1 (distance, then its push factor) ----
once("""                /* Ruling 11 (proof r11/proof.md): two values -- the focus-0 squared
                 * distance, then the distance (scaled to its push factor below). */
                s32 work;
""", """                s32 sq1, dist1;
""")
once("""                 * squared length for the leading-zero-count macro (a value under""",
     """                 * squared distance for the leading-zero-count macro (a value under""")
once("""                work = SCR->sq[0] + SCR->sq[1] + SCR->sq[2];
                temp = work;
                if (work < 0x400) {
                    work = (&D_8008D118)[work] >> 3;
                } else {""", """                sq1 = SCR->sq[0] + SCR->sq[1] + SCR->sq[2];
                temp = sq1;
                if (sq1 < 0x400) {
                    dist1 = (&D_8008D118)[sq1] >> 3;
                } else {""")
once("""                    temp = (&D_8008D118)[work >> nbits];
                    work = (temp << 16) >> (0x13 - (nbits >> 1));""",
     """                    temp = (&D_8008D118)[sq1 >> nbits];
                    dist1 = (temp << 16) >> (0x13 - (nbits >> 1));""")
once("                if (work >= r) {\n", "                if (dist1 >= r) {\n")
once("""                    dist2 = (temp << 16) >> (0x13 - (nbits2 >> 1));
                }
                tot = work + dist2;""", """                    dist2 = (temp << 16) >> (0x13 - (nbits2 >> 1));
                }
                /* FAKE: dead store; mechanism: reg_scan records it as sq1's last
                 * reference (regclass.c:1764 counts sets), later than temp's (the
                 * focus-1 table byte above), so cse.c make_regs_eqv (:840-857)
                 * keeps sq1 as the class head of `temp = sq1;`: the compare and the
                 * table indexes read sq1 ($a1) and the copy stays its own move (the
                 * target's `addu $a0,$a1,$zero` at 0x80018E18); otherwise temp
                 * becomes the head and the copy is folded away (26 insns). flow
                 * deletes the store (no bytes). Lever exhaustion:
                 * memory/grind/func_800187F4/r11/proof.md (sq1). */
                sq1 = 0;
                tot = dist1 + dist2;""")
once("""                if (work != 0) {
                    work = pen / work;
                }""", """                if (dist1 != 0) {
                    dist1 = pen / dist1;
                }""")
once("                @gte_lddp(work);\n", "                @gte_lddp(dist1);\n")
assert not re.search(r"\bwork\b", s) and not re.search(r"\bdelta\b(?! to focus)", s), "leftover"
# ---- lz annotation ----
once("""     * back. Frame from asm/funcs/func_800187F4.s alone: frame 0x78 = outgoing args
     * 0x10 + locals 0x40 + ten saves $s0-$s7/$fp/$ra at 0x50-0x74; the only locals
     * traffic is lz[0]/lz[1] at sp+0x10/0x14 and the count spill at sp+0x48.
     * Measured: lz[2]..lz[4] give frame 0x68, lz[5]/lz[6] 0x78, lz[7]/lz[8] 0x80.
     * lever-exhaustion: memory/grind/func_800187F4/evidence.md [s2] item 7 and
     * r11/proof.md "Other constructs". */""", """     * back. Frame from asm/funcs/func_800187F4.s alone: frame 0x78 = outgoing args
     * 0x10 + locals 0x40 + ten saves $s0-$s7/$fp/$ra at 0x50-0x74; the only locals
     * traffic is lz[0]/lz[1] at sp+0x10/0x14 and the count spill at sp+0x48.
     * Of the 0x40, 8 are the spill slot and 32 (0x28-0x47) are the four 8-byte
     * phantom slots of the combine orphan-USE loop-guard pseudos (the frame of the
     * lz[2] form: 0x68); the 16 bytes left are this object's.
     * Measured: lz[2]..lz[4] give frame 0x68, lz[5]/lz[6] 0x78, lz[7]/lz[8] 0x80.
     * lever-exhaustion: memory/grind/func_800187F4/evidence.md [s2] item 7 and
     * r11/proof.md section 7 (the phantom-slot producer census). */""")
open(D + "c6.c", "w", newline="\n").write(s)
print("ok")
