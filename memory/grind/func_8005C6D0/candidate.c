/* func_8005C6D0 — grind s1 (recon) candidate, honest sandbox score 5 (target 118 insns,
 * build 119).  Register allocation is IDENTICAL to the target for every pseudo
 * (s4=next, s3=i, s2=off, s1=p, s0=voice, s5=1, s6=&D_800EFC38); frame layout,
 * stack-arg slots and every %hi/%lo relocation match.
 *
 * SEMANTICS: sweep the 24-entry sound-request pool at D_800EFB78 (8 bytes/entry:
 * +0 s32 pointer set by func_8005C650, +4/+5 the two u8 volumes).  For every live
 * entry whose VAB slot D_800EFC38[*p] holds a KSEG0 pointer (< 0 as s32), find the
 * next free SPU voice starting from where the previous key-on stopped (`next`
 * persists across pool entries), remap VAB id 6 -> 3 when D_800EFC50 == D_800EFC44,
 * unpack the packed note word (bits 0-6 prog, 7-10 tone, 11-17 note, 18-24 fine)
 * and SsUtKeyOnV it.  Each pool slot is cleared at the end of its iteration.
 *
 * RESIDUAL (score 5, +1 insn): a single delay-slot/sched2 difference.  Target fills
 * the `beq $v0,$s5` delay slot with `addiu $v0,$s0,0x1` (the loop-continue increment
 * pulled out of the .L8005C838 thread by reorg.c) and puts `li 6` in the lhu load-delay
 * slot; our build schedules `li v0,6` above the branch, so reorg takes it for the branch
 * slot, a nop is needed after `lhu $a1`, and the increment stays split as
 * `addiu v0,s0,1 / move s0,v0` at the loop bottom.  Everything else is byte-identical
 * in shape.
 *
 * The three locals `o` / `off` / inline `i * 8` are NOT interchangeable — see
 * memory/grind/func_8005C6D0/evidence.md, hypotheses H2/H3: `o` (pool load site) keeps
 * loop.c from hoisting %hi(D_800EFB78) into a callee-saved register, `off` inside the
 * voice guard produces the target's `addu $s2,$v1,$zero` copy and keeps jump.c from
 * cross-jumping the guard test into the do-while bottom test, and the inline `i * 8`
 * at the clear site is what makes GCC recompute `sll/sra 13` at the 4-way join
 * (.L8005C850/.L8005C854).
 */
extern s32 D_800EFC44;
extern s32 D_800EFC50;
extern void SpuGetAllKeysStatus(u8 *);
extern s32 SpuGetKeyStatus(s32);
extern s32 SsUtKeyOnV(s16, s16, s16, s16, s16, s16, s16, s16);
void func_8005C6D0(void) {
    extern s32 *D_800EFC38[];
    u8 keys[24];
    s16 i;
    s16 voice;
    s16 next;
    u16 vab;
    u16 *p;
    s32 off;
    s32 o;
    u32 *ev;

    SpuGetAllKeysStatus(keys);
    next = 0;
    for (i = 0; (s16)i < 0x18; i = (s16)(i + 1)) {
        o = i * 8;
        p = *(u16 **)((u8 *)&D_800EFB78 + o);
        if (p != 0 && (s32)D_800EFC38[*p] < 0) {
            voice = next;
            if ((s16)voice < 0x18) {
                off = i * 8;
                do {
                    if (SpuGetKeyStatus(1 << voice) != 1) {
                        vab = *p;
                        if (vab == 6 && D_800EFC50 == D_800EFC44) {
                            vab = 3;
                        }
                        ev = &((u32 *)D_800EFC38[vab][0])[p[1]];
                        next = (s16)(voice + 1);
                        SsUtKeyOnV((s16)voice, (s16)vab,
                                   (s16)(*ev & 0x7F),
                                   (s16)((*ev >> 7) & 0xF),
                                   (s16)((*ev >> 11) & 0x7F),
                                   (s16)((*ev >> 18) & 0x7F),
                                   *((u8 *)&D_800EFB7D + off),
                                   *((u8 *)&D_800EFB7C + off));
                        break;
                    }
                    voice = (s16)(voice + 1);
                } while ((s16)voice < 0x18);
            }
        }
        *(s32 *)((u8 *)&D_800EFB78 + i * 8) = 0;
    }
}
