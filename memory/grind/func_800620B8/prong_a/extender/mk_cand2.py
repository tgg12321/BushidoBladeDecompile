"""Landable split candidate v2: 4 FAKE aliases + ONE FAKE chain-extender (sel_a), both frame
selections spelled as `index * sizeof(record) + table` (ordinary C, no FAKE)."""
D = 'tmp/func_800620B8/s4/'
s = open('tmp/func_800620B8/s3/split.c', encoding='utf-8').read()


def sub(a, b):
    global s
    assert s.count(a) == 1, a
    s = s.replace(a, b)


sub('    s16 height;\n', '''    s16 height;
    /* FAKE: pointer aliases of the four sprite tables (pointer-alias-fake-exception).
       All four stay live across the loop: strip32 is set at the loop top so loop.c
       hoists it into the pre-header, after the entry test, where the target sets $fp;
       the other three lose global allocation, so reload rebuilds each address in $t0
       at its use, as the target does. Tables used directly: 47; any one of the three
       used directly: 3-7. */
    u16 (*strip32)[4]; /* FAKE: alias of D_8009BA00 */
    u16 *alt32; /* FAKE: alias of D_8009BA50 */
    u16 (*strip16)[4]; /* FAKE: alias of D_8009BA30 */
    u16 *alt16; /* FAKE: alias of D_8009BA58 */
''')
sub('''    *(s32 *)D_800A3490 = 0x2F;
    for (i = 0; D_800F1198[i].unk0 & 1; i++) {
''', '''    *(s32 *)D_800A3490 = 0x2F;
    alt32 = D_8009BA50; /* FAKE: alias */
    strip16 = D_8009BA30; /* FAKE: alias */
    alt16 = D_8009BA58; /* FAKE: alias */
    for (i = 0; D_800F1198[i].unk0 & 1; i++) {
        strip32 = D_8009BA00; /* FAKE: alias, set here so loop.c hoists it */
''')
sub('''            D_800A348C = D_800A3488 = (s32)D_8009BA00[(u32)D_800A32B8 % 6];
            if (D_8009BD44[0] & 1) {
                D_800A348C = (s32)D_8009BA50;''', '''            /* FAKE: `- strip32 + strip32` round trip (combine-foldable chain-extender):
               combine folds it back to the direct `frame * 8 + strip32` (same RTL, zero
               bytes), but flow.c has already counted the two extra uses of strip32
               (nrefs 3 -> 7), so global.c ranks it above sv/flag and gives it $fp, as
               the target has it. Without it: 35. */
            D_800A348C = D_800A3488 = ((u32)D_800A32B8 % 6) * sizeof(*strip32) + (s32)strip32 - (s32)strip32 + (s32)strip32;
            if (D_8009BD44[0] & 1) {
                D_800A348C = (s32)alt32;''')
sub('''            D_800A348C = D_800A3488 = (s32)D_8009BA30[D_800A32B8 & 3];
            if (D_8009BD44[0] & 1) {
                D_800A348C = (s32)D_8009BA58;''', '''            D_800A348C = D_800A3488 = (D_800A32B8 & 3) * sizeof(*strip16) + (s32)strip16;
            if (D_8009BD44[0] & 1) {
                D_800A348C = (s32)alt16;''')
open(D + 'cand_split1.c', 'w', encoding='utf-8', newline='\n').write(s)
print('ok')
