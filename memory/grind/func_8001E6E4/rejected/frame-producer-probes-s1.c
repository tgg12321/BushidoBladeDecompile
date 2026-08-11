/* REJECTED (mechanically DEAD, not policy) — session 1 frame-producer probes.
 * All three spellings are honest C but produce ZERO frame movement
 * (.frame stays 104/vars=72; need 112/vars=80). Measured with
 * tmp/grind/func_8001E6E4/s1/frame_probe.sh. Do not re-propose.
 *
 * Probe 2 — named range compare (phantom-slot producer #1, folded loop-guard
 * compare). DEAD: this is a straight-line range check, not a loop guard; the
 * compare pseudo is allocated to $a0 and consumed by the materialized sltiu,
 * so it never orphans.
 */
void probe2(s32 arg0) {
    /* ... */
    u32 range = arg0 - 0x555;
    if (range >= 0x556U) { /* s2 = ...; */ }
}

/* Probe 3 — named u16 component temps (hoped-for HImode stack temps per the
 * tslLineG5Init witness). DEAD: lhu loads are immediately SImode-widened and
 * the adds are SImode; there is no HImode *expression* anywhere, so cc1 never
 * creates an HImode stack temp. The tslLineG5Init trigger needs an HImode
 * bitwise expression, which this function's semantics do not contain.
 */
void probe3(void) {
    /*
    u16 rx = *(u16 *)((u8 *)s2 + 0x10);
    u16 ry = *(u16 *)((u8 *)s2 + 0x12);
    u16 rz = *(u16 *)((u8 *)s2 + 0x14);
    local.rx = rx + (u16)D_800FF5D8;
    ...
    */
}

/* Probe 4 — named dist scalar (m2c's temp_a2/sp30 shape). DEAD: the scalar is
 * fully register-allocated ($a2), gets no expansion-time stack home; vars
 * unchanged. Codegen-neutral but frame-neutral too.
 */
void probe4(void) {
    /*
    s32 dist = *(s32 *)((u8 *)s2 + 0x18) + D_800FF5E0;
    local.dist = dist;
    func_80046BF4(p0, &local.rx, dist);
    */
}
