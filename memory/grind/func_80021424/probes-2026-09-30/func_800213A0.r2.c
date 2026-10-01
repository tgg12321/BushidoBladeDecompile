void func_800213A0(PracticeMenuRec *arg0) {
    s16 a1 = arg0->unk_86;
    if (a1 != arg0->unk_88) {
        if (a1 != arg0->unk_8E) {
            return;
        }
    }
    arg0->unk_86 = (s16)((a1 + 1) % D_800A3860[arg0->unk_4A]->f14);
}
