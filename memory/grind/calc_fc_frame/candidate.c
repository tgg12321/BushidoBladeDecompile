extern void func_800520B8(s32, s32, s32);
s32 calc_fc_frame(s32 src_base, s32 *dest_arr, s16 *frame_offsets) {
    s16 *fp;
    s16 *scan;
    s32 *orig_dest;
    s32 val;
    s32 data_ptr;
    s32 src_orig;
    s32 sentinel;
    s32 count;
    s32 *src_ptr;
    s32 start;
    s32 size;

    fp = frame_offsets;
    count = 0;
    scan = fp + 1;
    src_orig = src_base;
    orig_dest = dest_arr;

    val = *fp;
    if (val != -2) {
        do {
            if (val >= 0) {
                count++;
            }
            val = *scan;
            scan++;
        } while (val != -2);
    }

    *dest_arr = count;
    dest_arr++;
    src_base += 4;
    data_ptr = (s32)orig_dest + (count + 2) * 4;

    val = *fp;
    fp++;
    if (val != -2) {
        sentinel = -2;
        src_ptr = (s32 *)src_base;
        do {
            if (val >= 0) {
                size = src_ptr[1];
                start = *src_ptr;
                *dest_arr = data_ptr - (s32)orig_dest;
                dest_arr++;
                size = size - start;
                func_800520B8(src_orig + start, data_ptr, size);
                size = (u32)size >> 2;
                size = size << 2;
                data_ptr += size;
            }
            src_ptr++;
            val = *fp;
            fp++;
        } while (val != sentinel);
    }

    *dest_arr = data_ptr - (s32)orig_dest;
    return data_ptr;
}
