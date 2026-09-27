/* REJECTED — func_800174F4, session 1.
 * WHY DEAD: floor 18 -> 32, build_insns 136 -> 133. This is the isolated
 * version of the K2 damage: keep the H2 counter split (which is good), but
 * additionally give the D_800A37A8[] table value its own local instead of
 * reusing `h`. Three instructions are lost and the whole case-20 tail
 * diverges. The `h` reuse for the table value is load-bearing.
 * Do not re-propose.
 */
        } else {
            u8 new_val = a2_val + 1;
            unsigned short tv;          /* <-- the killer: must be `h` */
            D_800A38F8 = new_val;
            D_800A37C0 = 0;
            tv = D_800A37A8[a0_val];
            if ((new_val & 0xFF) == a1_val) {
                tv |= 0x8000;
            }
            func_80060414(tv, (u8 *)prim, 0);
        }
