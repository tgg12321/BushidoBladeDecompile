/* REJECTED s1: volatile spelling — sandbox-INERT and mechanism-false.
 * `extern volatile u8 D_800F1159;` + `volatile u8 *flag` is stripped by the
 * cheat-invisible sandbox (D_800F1159 has NO volatile_extern_allowlist.txt
 * grant; cheat_asm_stripped went 169->170), so it cannot move the honest
 * floor. The raw cc1 probe (tmp/grind/func_80061250/s1/probe_volatile.s)
 * additionally shows volatile does NOT produce target's shape anyway: the +1
 * accesses still fold absolute and the 0x21 lui is still shared. The
 * chassis-era volatile decl was NOT load-bearing; do not re-propose a
 * volatile axis for this function. (Kill measured s1, 2026-08-26.) */
extern volatile u8 D_800F1159;
/* body identical to s1-ptr-local-plus1-folds-25.c with volatile u8 *flag */
