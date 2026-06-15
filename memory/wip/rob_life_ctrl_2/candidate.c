/* rob_life_ctrl_2 (text1a_c.c) — HSV->RGB conversion.
 * This is HEAD's body. It matches ONLY WITH the 2 regfix subst rules
 * (mflo $8->$5, sra $5,$8,12 -> $5,$5,12). No pure-C form found yet that
 * reaches target's RA for the 5th mflo (t's mult result) without the rules.
 * See notes.md — documented mflo-dest plateau, instrumented allocno dump is
 * the next un-tried modality. */
void rob_life_ctrl_2(s32 *hsv, s32 *rgb) {
    s32 h = hsv[0];
    s32 s = hsv[1];
    s32 v = hsv[2];
    s32 i, f, p, q, t;
    s32 r, g, b;

    if (func_800486FC(hsv)) {
        s = 0;
    }
    if (s == 0) {
        r = v;
        g = v;
        b = v;
        goto out;
    }
    if (h == 0x1000) {
        h = 0;
    }
    p = v * (0x1000 - s);
    p = p >> 12;
    h = h * 6;
    f = h & 0xFFF;
    i = h >> 12;
    q = (v * (0x1000 - ((s * f) >> 12))) >> 12;
    t = (v * (0x1000 - ((s * (0x1000 - f)) >> 12))) >> 12;

    if ((u32)i >= 6U) goto out;
    switch (i) {
    case 0: r = v; g = t; b = p; goto out;
    case 1: r = q; g = v; b = p; goto out;
    case 2: r = p; g = v; b = t; goto out;
    case 3: r = p; g = q; b = v; goto out;
    case 4: r = t; g = p; b = v; goto out;
    case 5: r = v; g = p; b = q; goto out;
    }
out:
    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;
}
