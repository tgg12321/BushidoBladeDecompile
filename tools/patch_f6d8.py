import re

# Attempt 6: precise variable ordering to get right stack layout
# Target: sp10@sp+0x10, sp28@sp+0x28 => 5 vars between them
# Variables s0-s4 (temp_s0,temp_s1,var_s2,var_s3,temp_s4) go in slots 1-5
# Then sp28 (volatile to force stack), then s5-s7,fp vars

func_c = r"""void func_8003F6D8(s16 *arg0) {
    s16 *sp10;
    s32 temp_s0;
    void *temp_s1;
    void *var_s2;
    void *var_s3;
    void *temp_s4;
    volatile s32 sp28;
    s32 var_s5;
    s32 var_s6;
    s32 var_s7;
    s32 var_fp;
    void *temp_v0;

    sp10 = arg0;
    var_fp = 0;
    if (*arg0 > 0) {
        sp28 = 8;
        do {
            var_s7 = 0;
            temp_v0 = (void *)((u8 *)sp10 + sp28);
            temp_s4 = (void *)((u8 *)temp_v0 + 0x1C);
            if (*(s32 *)((u8 *)temp_v0 + 0x1C) > 0) {
                var_s6 = 0x18;
                var_s5 = 0x84;
                var_s3 = temp_s4;
                var_s2 = (void *)((u8 *)temp_s4 + 0x84);
                do {
                    temp_s1 = (void *)((u8 *)temp_s4 + var_s6);
                    var_s6 += 0x20;
                    var_s5 += 0x10;
                    temp_s0 = *(s32 *)((u8 *)var_s3 + 4);
                    var_s3 = (void *)((u8 *)var_s3 + 4);
                    temp_s0 += 0x18;
                    func_80052A20((s32 *)temp_s0, (s32 *)var_s2, (s16 *)temp_s1);
                    func_80052A20((s32 *)temp_s0, (s32 *)((u8 *)var_s2 + 8), (s16 *)((u8 *)temp_s1 + 0x10));
                    var_s7 += 1;
                    var_s2 = (void *)((u8 *)temp_s4 + var_s5);
                } while (var_s7 < *(s32 *)temp_s4);
            }
            sp28 += 0xD0;
            var_fp += 1;
        } while (var_fp < *sp10);
    }
}
"""

with open("src/config.c", "r", newline="\n") as f:
    content = f.read()

func_start = "void func_8003F6D8"
func_end_marker = "void func_8003F7F4"

start_idx = content.find(func_start)
end_idx = content.find(func_end_marker)

assert start_idx != -1, "func_8003F6D8 not found"
assert end_idx != -1, "func_8003F7F4 not found"

content = content[:start_idx] + func_c + content[end_idx:]

with open("src/config.c", "w", newline="\n") as f:
    f.write(content)

print("Done - patched func_8003F6D8 (attempt 6: precise ordering with volatile sp28)")
