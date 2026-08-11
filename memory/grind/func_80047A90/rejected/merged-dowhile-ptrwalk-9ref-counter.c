/* REJECTED session 2: merged counter k across both loops + do-while pointer-walk
 * loop 1 (hand-hoisted jb = Judge, source p558/p59C pointers) + k==8-arms tail
 * duplication. Sandbox 17 (84/84 insns, renames only).
 *
 * Measured (s2 lreg): merged k = 9 refs / 65 live -> floor_log2(9)=3 -> prio .415,
 * ABOVE pa2 (6/32 = .375) and a3off (6/33 = .364) -> k allocates before them -> a2.
 * The window k needs is (.244 pt1, .364 a3off); 8 refs at live>=66 would fit (.369)
 * but the merged counter's ref floor is NINE: loop1 do-while contributes
 * init(1)+inc(2)+test(1)=4 (all byte-load-bearing: target has addu t0,zero,zero /
 * addiu t0,t0,1 / slti v0,t0,0x11), loop2 contributes =1(1)+==8(1)+inc(2)+<9(1)=5.
 * No spelling removes a ref without changing bytes (tested/derived: pre-inc test
 * same count; != compares change bne/slti bytes; continuing i past 0x11 changes
 * the ==8/<9 immediates). Live extension to >=74 (9-ref path) requires k live into
 * the 10-insn tail block = a dead read = forbidden.
 * ALSO: hand-hoisted jb got live 48 (extends past loop1) in this form -> conflicts
 * pa2 -> jb cannot share a2. KILLED both ways.
 */
void func_80047A90(void) { /* see description; body was candidate.c's with:
    k = 0; jb = Judge; p558 = D_800EF558; p59C = D_800EF59C;
  loop1:
    k++; *p59C = ((s32)jb[*p558 & 0xFFF] * 0x271) >> 10; p59C++;
    *p558 += 0x12; p558++;
    if (k < 0x11) goto loop1;
    ... then loop2 with k re-init and duplicated arms */ }
