/* save_vc_ctrl -- pre-authorization C body, banked 2026-10-01 at de-authorization (owner ruling,
 * docs/grind/decisions.md "inline-asm audit"). Measured: sandbox save_vc_ctrl --disable all
 * --candidate <this file> -> honest floor in migration_pin.json. Volatile frame pads removed;
 * any asm barrier is stripped by the sandbox. Starting context only, not a vetted candidate. */
void save_vc_ctrl(s32 a0, s16 *a1, s32 a2) {
    s32 i = a2 - 1;
    if (a2 == 0) {
        return;
    }
    a2 = -1;
    a1 = (s16 *)((u8 *)a1 + 0xC);
    do {
        s32 val = *(s32 *)a1;
        if (val) {
            *(s32 *)a1 = val + a0;
        }
        i--;
        a1 = (s16 *)((u8 *)a1 + 0x68);
    } while (i != a2);
}
