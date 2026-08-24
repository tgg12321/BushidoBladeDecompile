s32 _spu_pitch2note(s32 arg0, s32 arg1, s32 arg2) {
    register s32 saved_arg0 asm("$24");
    register s32 search asm("$6");
    register s32 saved_arg2 asm("$4");
    s32 scan_bit;
    register s32 shift asm("$10");
    s32 scan_result;

    saved_arg0 = arg0;
    search = arg2;
    saved_arg2 = search;
    search = ~search;
    scan_bit = 0;
    shift = 0xF;
    search &= 0xFFFF;
    scan_result = search >> shift;

scan_loop:
    if ((scan_result & 1) == 0) {
        do {
            scan_bit = shift;
            goto have_bit;
        } while (0);
    }

    shift--;
    scan_result = search >> shift;
    if (shift >= 0) {
        goto scan_loop;
    }

have_bit:
    {
        register s32 bit asm("$3") = scan_bit;
        register s32 bit_offset asm("$15");
        register s32 scale asm("$14");
        register u32 curve asm("$8");
        register s32 outer asm("$10");
        register u32 target asm("$6");
        register s32 result asm("$2");

        bit_offset = bit - 12;
        result = 1;
        scale = result << bit;
        curve = 0x1000;
        outer = 0;
        target = saved_arg2 & 0xFFFF;

        for (;;) {
            register s32 old_prod asm("$12");
            register s32 next_prod asm("$3");
            register u32 step asm("$9");
            register u32 next asm("$7");
            register u32 acc asm("$11");
            register s32 inner asm("$4");
            register s32 base asm("$13");

            old_prod = scale * curve;
            result = curve << 6;
            result += curve;
            result <<= 4;
            result -= curve;
            result <<= 2;
            curve = result - curve;
            curve >>= 12;
            next_prod = scale * curve;
            inner = 0;
            base = outer << 5;
            acc = 0;
            result = next_prod - old_prod;
            step = (u32)result >> 5;
            next = step;

            do {
                result = old_prod + acc;
                next_prod = old_prod + next;
                next_prod = (u32)next_prod >> 12;
                result = (u32)result >> 12;

                if (target < (u32)result) {
                    goto inner_next;
                }
                result = target < (u32)next_prod;
                if (result != 0) {
                    result = base + inner;
                    goto found;
                }

            inner_next:
                next += step;
                inner++;
                acc += step;
            } while (inner < 0x20);

            outer++;
            if (outer >= 0x30) {
                result = 0x600;
                goto found;
            }
        }

    found:
        {
            register s32 quot asm("$3") = result;
            register s32 rem asm("$4");

            if (result < 0) {
                quot = result + 0x7F;
            }
            quot >>= 7;
            rem = result - (quot << 7);
            result = (saved_arg0 & 0xFFFF) + quot;
            quot = bit_offset * 3;
            quot <<= 2;
            result += quot;
            quot = (arg1 & 0xFFFF) + rem;
            result <<= 8;
            return result | quot;
        }
    }
}
