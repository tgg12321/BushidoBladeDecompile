/* REJECTED — func_800453E0, session 1. Honest floor 37 (66 insns vs target 76).
 *
 * WHY IT IS DEAD: binding the table base to a LOCAL pointer variable
 * (`SubEntry *tbl = (SubEntry *)D_800EED10;`) makes the base an unknown value, so
 * GCC strength-reduces the search loop to a WALKING POINTER (`lh $v0,0($v1)`,
 * `lw $a1,8($v1)`) and CSEs that one base register across the entire function —
 * including the tail clear, which collapses to `addu $v1,$v1,$s2` + two displaced
 * stores instead of target's two separate `lui`/`addu` pairs. Net: 10
 * instructions SHORT of target. Target re-materializes %hi(SYM) + an offset
 * register at every access outside the copy loop.
 *
 * The fix is a constant-address base (a macro / cast expression, never a local
 * pointer) plus source-level byte offsets for the search and clear accesses —
 * see candidate.c.
 *
 * KEEP: this form is what first proved the frame comes out HONESTLY at
 * `vars= 8` / 0x30 with all five save offsets matching and NO declaration
 * reserving those 8 bytes (the inherited `volatile s32 sp_pad;` cheat was
 * therefore never necessary). Do not re-derive that.
 */

typedef struct {
    s16 id;
    s16 unk2;
    s32 unk4;
    s32 amt;
    void (*fn)(s16, s32);
} SubEntry;

void func_800453E0(s32 a0) {
    SubEntry *tbl = (SubEntry *)D_800EED10;   /* <-- the fatal line */
    s32 i;
    s32 j;
    s32 last;

    for (i = 0; i < D_800A33AC; i++) {
        if (tbl[i].id == a0) {
            func_80045294(i + 1, -tbl[i].amt);
            if (i < D_800A33AC - 1) {
                for (j = i + 1; j < D_800A33AC; j++) {
                    tbl[j - 1] = tbl[j];
                }
            }
            last = D_800A33AC - 1;
            tbl[last].id = -1;
            tbl[last].fn = 0;
            D_800A33AC = last;
            return;
        }
    }
}
