/* REJECTED: statement-reorder (b first in source) — score unchanged at 2.
 * sched1 INSN_PRIORITY orders longer-chain (r,g) loads before shorter-chain b regardless of LUID.
 * Session 1 (recon), 2026-07-20.
 */
    } else {
        b = *((u8 *)player + 0x1A);
        r = *((u8 *)player + 0x18);
        g = *((u8 *)player + 0x19);
        gnd_load_tex(b | ((r << 16) | (g << 8)));
    }
