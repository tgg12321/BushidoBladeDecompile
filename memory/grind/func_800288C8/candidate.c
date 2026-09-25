typedef struct { s32 x, y, z; } Vec3i;

/* Scratchpad layout read here: ten vectors at 0x1F800000 (func_8002C61C copies
 * them into the character records), then two body points per character at
 * 0x1F800078 (this function builds them), then the 22 joint positions of each
 * character at 0x1F8000A8 (stride 0x108; func_8002A458 reads the same joints). */
typedef struct {
    Vec3i other[10];
    Vec3i pt[2][2];
    Vec3i joint[2][22];
} HitScratch;
#define HIT_SCR ((HitScratch *)0x1F800000)

extern s32 D_800A3144[];

/* Body-overlap push between the two characters (records D_80101EC8 and
 * D_80101EC8 + 0x44C). Skipped while either character's +0x6A state is 0x28.
 * Each character gets two points: joint 1, and the midpoint of joints 15 and
 * 19. Every point of character 1 is tested against every point of character
 * 2 with the per-point radii D_800A3144[0..1]: when the points are closer than
 * the summed radius, the overlap (at most 0x80) pushes the two characters
 * apart along x/z, split by the difference of their +0x20 fields and scaled
 * by the distance, which is the D_8008D118 byte-LUT integer square root with
 * the GTE leading-zero count for large inputs (the idiom of func_8002E838).
 * After any hit, the +0x6A states (0x13/0x1B/0x30 with +0x14E above 0x9C4,
 * and 0x15) and the facing difference of the +0x1CA angles pick one of: both
 * characters to state 0xF (no push), one character to state 0x10 or 5 through
 * func_80027A58 / func_80032854, or D_800A38A8/D_800A3876. The accumulated
 * push is then added to each character's +0x134/+0x13C.
 *
 * GTE ISLANDS: census member of the 2026-08-17 owner cluster ruling
 * (.claude/rules/cop2-addressing-preamble-cluster.md:68, LZCS sub-family, one
 * idiom site). The six statements are PsyQ gte_Lzc(r1, r2) (gtemac.h:230-236)
 * written out statement for statement from inline_o.h, the "DMPSX version 3"
 * header (vendored copy tmp/croc-ref/include/psyq/inline_o.h, sha256
 * 27a4abd6...81a9d6): gte_ldlzc(r1) (:645-655, two statements), gte_nop() twice
 * (:3067-3068), gte_stlzc(r2) (:2999-3009, two statements). Instruction text,
 * operands and clobber lists are the header's, character for character,
 * except that the header's `swc2 $31,($12)` is written `0($12)` (same
 * encoding; maspsx cannot parse the bare form). Each operand is left to cc1
 * through %0 (the target's `addu $t4,$a0,$zero` is the macro's
 * `move $12,%0`, and `addiu $v0,$sp,0x10` is cc1 materializing &lzc_out).
 * Everything else is ordinary C. */
void func_800288C8(void) {
    s32 lzc_out;
    s32 hits;
    s32 i;
    s32 j;
    s32 c;
    s32 push1_x;
    s32 push1_z;
    s32 push2_x;
    s32 push2_z;
    s32 dx;
    s32 dy;
    s32 dz;
    s32 reach;
    s32 *rad;
    s32 dist;
    s32 tbl;
    s32 pen;
    s32 bias;
    s32 flag1;
    s32 flag2;
    s32 facing;

    if (*(u16 *)(&D_80101EC8 + 0x6A) == 0x28 || *(u16 *)(&D_80101EC8 + 0x44C + 0x6A) == 0x28) {
        return;
    }
    for (c = 0; c < 2; c++) {
        HIT_SCR->pt[c][0].x = HIT_SCR->joint[c][1].x;
        HIT_SCR->pt[c][0].y = HIT_SCR->joint[c][1].y;
        HIT_SCR->pt[c][0].z = HIT_SCR->joint[c][1].z;
        HIT_SCR->pt[c][1].x = (HIT_SCR->joint[c][15].x + HIT_SCR->joint[c][19].x) / 2;
        HIT_SCR->pt[c][1].y = (HIT_SCR->joint[c][15].y + HIT_SCR->joint[c][19].y) / 2;
        HIT_SCR->pt[c][1].z = (HIT_SCR->joint[c][15].z + HIT_SCR->joint[c][19].z) / 2;
    }
    /* FAKE: local pointer to the radius table (pointer-alias family).
     * mechanism: loop.c strength reduction + cse2. Spelled D_800A3144[i] /
     * D_800A3144[j], the j-loop's hoisted table base and the i-row pointer
     * loop.c derives are both the symbol, and the base ends up initialized
     * from the i-row pointer, which gains a reference (8 vs 7); global.c then
     * seats it ahead of the i*12 point index ($s1/$s2 swapped against the
     * target, 7 differing words). memory/grind/func_800288C8/evidence.md. */
    rad = D_800A3144;
    hits = 0;
    push1_x = push1_z = push2_x = push2_z = 0;
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            dx = HIT_SCR->pt[0][i].x - HIT_SCR->pt[1][j].x;
            dy = HIT_SCR->pt[0][i].y - HIT_SCR->pt[1][j].y;
            dz = HIT_SCR->pt[0][i].z - HIT_SCR->pt[1][j].z;
            reach = rad[i] + rad[j];
            if (dx > reach || dx < -reach || dy > reach || dy < -reach
                || dz > reach || dz < -reach) {
                continue;
            }
            /* FAKE: the squared distance is staged in `dist` until its square
             * root replaces it (staged-value-reused-variable; the square is
             * last read by the table index / shift below, before `dist` is
             * written with the root). A separate once-set squared-distance
             * local takes sched.c's birthing priority (reg_n_sets == 1) and
             * reorders the r*r test and the copy below (20 differing words).
             * memory/grind/func_800288C8/hypotheses.md. */
            dist = dx * dx + dy * dy + dz * dz;
            if (reach * reach < dist) {
                continue;
            }
            hits++;
            /* FAKE: the LZC input is staged in `tbl`, the table-byte variable
             * (staged-value-reused-variable; tbl holds nothing live here and
             * the staged copy is dead after gte_ldlzc). The copy shares tbl's
             * pseudo, which global.c seats in $a0 as the target does
             * (`move $a0,$a3`); a fresh copy local takes $v1 (2 differing
             * words). memory/grind/func_800288C8/hypotheses.md. */
            tbl = dist;
            if ((u32)dist < 0x400) {
                dist = (u32)*(((u8 *)&D_8008D118) + dist) >> 3;
            } else {
                /* gte_Lzc(tbl, &lzc_out) -- gtemac.h:230-236 */
                /* gte_ldlzc(tbl) -- inline_o.h:645-655 */
                __asm__ volatile("move  $12,%0"
                                 :
                                 : "r"(tbl)
                                 : "$12", "$13", "$14", "$15", "memory");
                __asm__ volatile("mtc2  $12,$30"
                                 :
                                 :
                                 : "$12", "$13", "$14", "$15", "memory");
                /* gte_nop() x2 -- inline_o.h:3067-3068 */
                __asm__ volatile("nop   " : : : "$12", "$13", "$14", "$15", "memory");
                __asm__ volatile("nop   " : : : "$12", "$13", "$14", "$15", "memory");
                /* gte_stlzc(&lzc_out) -- inline_o.h:2999-3009 */
                __asm__ volatile("move  $12,%0"
                                 :
                                 : "r"(&lzc_out)
                                 : "$12", "$13", "$14", "$15", "memory");
                __asm__ volatile("swc2  $31,0($12)"
                                 :
                                 :
                                 : "$12", "$13", "$14", "$15", "memory");
                {
                    s32 lz;
                    s32 shift;
                    lz = ~1;
                    lz &= lzc_out;
                    shift = 0x16 - lz;
                    tbl = *(((u8 *)&D_8008D118) + ((u32)dist >> shift));
                    dist = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
                }
            }
            pen = reach - dist;
            if (pen > 0x80) {
                pen = 0x80;
            }
            if (dist == 0) {
                dist = 1;
            }
            bias = (*(s16 *)(&D_80101EC8 + 0x44C + 0x20) - *(s16 *)(&D_80101EC8 + 0x20)) / 4;
            push1_x += (dx * (pen * (0x400 + bias)) / dist) >> 10;
            push1_z += (dz * (pen * (0x400 + bias)) / dist) >> 10;
            push2_x -= (dx * (pen * (0x400 - bias)) / dist) >> 10;
            push2_z -= (dz * (pen * (0x400 - bias)) / dist) >> 10;
        }
    }
    if (hits == 0) {
        return;
    }
    flag1 = 0;
    if (*(u16 *)(&D_80101EC8 + 0x6A) == 0x13 || *(u16 *)(&D_80101EC8 + 0x6A) == 0x1B
        || *(u16 *)(&D_80101EC8 + 0x6A) == 0x30) {
        if (*(s16 *)(&D_80101EC8 + 0x14E) > 0x9C4) {
            flag1 = 1;
        }
    }
    flag2 = 0;
    if (*(u16 *)(&D_80101EC8 + 0x44C + 0x6A) == 0x13 || *(u16 *)(&D_80101EC8 + 0x44C + 0x6A) == 0x1B
        || *(u16 *)(&D_80101EC8 + 0x44C + 0x6A) == 0x30) {
        if (*(s16 *)(&D_80101EC8 + 0x44C + 0x14E) > 0x9C4) {
            flag2 = 1;
        }
    }
    facing = (*(s16 *)(&D_80101EC8 + 0x1CA) - *(s16 *)(&D_80101EC8 + 0x44C + 0x1CA)) & 0xFFF;
    if (facing >= 0x800) {
        facing = 0x1000 - facing;
    }
    if (flag1) {
        if (flag2) {
            if (facing > 0x600) {
                func_80032854(0, 0x21, &D_80101EC8 + 0xF4, 0);
                *(s16 *)(&D_80101EC8 + 0x44C + 0x286) = 0xF;
                *(s16 *)(&D_80101EC8 + 0x286) = 0xF;
                func_80027A58((s32 *)&D_80101EC8);
                func_80027A58((s32 *)(&D_80101EC8 + 0x44C));
                hits = 0;
            }
        } else {
            if (facing < 0x400 && *(u16 *)(&D_80101EC8 + 0x44C + 0x6A) == 0x15) {
                *(s16 *)(&D_80101EC8 + 0x44C + 0x286) = 0x10;
                func_80027A58((s32 *)(&D_80101EC8 + 0x44C));
            }
            if (facing > 0x600 && *(u16 *)(&D_80101EC8 + 0x44C + 0x6A) == 0x15) {
                if ((u32)(*(u16 *)(&D_80101EC8 + 0xE) - 6) < 2
                    || (u32)(*(u16 *)(&D_80101EC8 + 0x44C + 0xE) - 6) < 2) {
                    func_80032854(1, 0x21, &D_80101EC8 + 0x44C + 0xF4, 0);
                    func_80032854(1, 0x2D, &D_80101EC8 + 0x44C + 0xF4, 0);
                    *(s16 *)(&D_80101EC8 + 0x44C + 0x286) = 5;
                } else {
                    D_800A38A8 = 1;
                    D_800A3876 = 0;
                }
            }
        }
    } else if (flag2) {
        if (facing < 0x400 && *(u16 *)(&D_80101EC8 + 0x6A) == 0x15) {
            *(s16 *)(&D_80101EC8 + 0x286) = 0x10;
            func_80027A58((s32 *)&D_80101EC8);
        }
        if (facing > 0x600 && *(u16 *)(&D_80101EC8 + 0x6A) == 0x15) {
            if ((u32)(*(u16 *)(&D_80101EC8 + 0xE) - 6) < 2
                || (u32)(*(u16 *)(&D_80101EC8 + 0x44C + 0xE) - 6) < 2) {
                func_80032854(0, 0x21, &D_80101EC8 + 0xF4, 0);
                func_80032854(0, 0x2D, &D_80101EC8 + 0xF4, 0);
                *(s16 *)(&D_80101EC8 + 0x286) = 5;
            } else {
                D_800A38A8 = 1;
                D_800A3876 = 1;
            }
        }
    }
    if (hits != 0) {
        *(s32 *)(&D_80101EC8 + 0x134) += push1_x;
        *(s32 *)(&D_80101EC8 + 0x13C) += push1_z;
        *(s32 *)(&D_80101EC8 + 0x44C + 0x134) += push2_x;
        *(s32 *)(&D_80101EC8 + 0x44C + 0x13C) += push2_z;
    }
}
