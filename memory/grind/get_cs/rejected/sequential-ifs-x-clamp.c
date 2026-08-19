/* REJECTED s13 (driver session 8, 2026-08-10): sequential unchained ifs for
 * the X clamp (zero-clamp then independent upper-clamp re-test). sandbox
 * --disable all = 28 @ build 49 (vs floor 5 @ 50). GCC folds the re-tested
 * range; two insns short with heavy diffs. Only the X region shown — rest of
 * body identical to candidate.c. */
    if (arg0 < 0) {
        arg0 = 0;
    }
    if ((D_8009BE78 - 1) < arg0) {
        arg0 = D_8009BE78 - 1;
    }
