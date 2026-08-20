void func_80056CB8(s32 arg0) {
    /* Bind locals to specific callee-save regs to match target's allocation. */
    register s32 r_arg0 asm("$23") = arg0;     /* $s7 */
    register s32 var_s6 asm("$22");            /* $s6 - used by asmfix-slice */
    s32 var_fp;                                 /* GCC picks; clobber forces save */
    /* Target struct base lands at sp+0x18 (8-byte param-save gap above sp+0x10). */
    struct {
        s32 sp18, sp1C, sp20, _g0;
        s32 sp28, sp2C, sp30, _g1;
        s32 sp38, sp3C, sp40, _g2;
        s32 sp48, sp4C, sp50, _g3;
        s32 sp58, sp5C, sp60, _g4;
        s32 *sp68;
        s32 _g5;
        s32 *sp70;
        s32 _g6;
        s32 sp78;
    } f;
    s32 var_s0_2;
    s16 *p_pos1, *p_pos2;
    s32 angle_val;
    s32 var_s1;
    s32 var_s2, var_s3;
    s32 temp_s0;
    s32 temp_v1_3;
    u16 obj_type;
    s32 r1, r2;
#define sp18 f.sp18
#define sp1C f.sp1C
#define sp20 f.sp20
#define sp28 f.sp28
#define sp2C f.sp2C
#define sp30 f.sp30
#define sp38 f.sp38
#define sp3C f.sp3C
#define sp40 f.sp40
#define sp48 f.sp48
#define sp4C f.sp4C
#define sp50 f.sp50
#define sp58 f.sp58
#define sp5C f.sp5C
#define sp60 f.sp60
#define sp68 f.sp68
#define sp70 f.sp70
#define sp78 f.sp78
    {
        register s32 _low2 asm("$3");
        s32 _guard;
        s32 _hi3e8 = *(u16 *)(r_arg0 + 0x3E8);
        __asm__("andi %0,%1,0x3" : "=r"(_low2) : "r"(_hi3e8));
        var_s6 = _low2 << 1;
        __asm__ __volatile__("addiu %0,$0,1" : "=r"(_guard));
        if (_guard != 0) {
            sp60 = var_s6;
            sp68 = (s32 *)&sp28;
            sp70 = (s32 *)&sp58;
            sp78 = 0x1F8002B8;
            var_fp = _low2 << 2;
            do {
                /* Body replaced wholesale by asmfix-slice. Stub keeps GCC saving
                 * callee-saves and allocating struct stack slots. */
                __asm__ volatile ("" : : "r"(var_fp) : "$16","$17","$18","$19","$20","$21","memory");
                sp18 = 0; sp1C = 0; sp20 = 0;
                sp28 = 0; sp2C = 0; sp30 = 0;
                sp38 = 0; sp3C = 0; sp40 = 0;
                sp48 = 0; sp4C = 0; sp50 = 0;
                sp58 = 0; sp5C = 0;
                func_80053614((s32 *)&sp18, sp68, (s32)&sp38, (s32)sp70, var_fp);
                func_80053614((s32 *)&sp18, sp68, (s32)&sp48, (s32)sp70, sp78);
                *(s8 *)(r_arg0 + 0x444 + var_s6) = 0;
                var_s6 += 1;
                var_fp += 2;
            } while (var_s6 < sp60 + 2);
        }
    }
}
