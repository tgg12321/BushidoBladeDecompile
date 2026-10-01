# plp_decode.py: decode the gp-relative load offsets in the psylink_probe.sh output binary (out.bin).
import struct
b = open("/dev/shm/plp/out.bin", "rb").read()
names = ["fa: a_st1", "fa: a_com", "fa: a_st2", "fa: shared_com", "fa: a_sd", None, None, None,
         "fb: b_st1", "fb: b_com", "fb: shared_com", "fb: b_sd"]
for i in range(len(b) // 4):
    w = struct.unpack_from("<I", b, i * 4)[0]
    op, rs, rt, imm = w >> 26, (w >> 21) & 31, (w >> 16) & 31, w & 0xFFFF
    if op == 0x23:
        print(f"{0x80010000 + 4 * i:08x}: lw ${rt},{imm - 0x10000 if imm & 0x8000 else imm}(${rs})")
    else:
        print(f"{0x80010000 + 4 * i:08x}: {w:08x}")
