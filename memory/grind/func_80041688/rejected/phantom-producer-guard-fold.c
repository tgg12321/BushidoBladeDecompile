/* s17 REJECTED (measurement probes, phantom-slot honest-producer search):
 * three loop1-guard respellings attempting to strand an orphan pseudo for
 * the 0x20 leading frame region. ALL measured vars=0 (.frame gradient) —
 * GCC 2.7.2 either DCEs the folded intermediate outright (zero refs, no
 * limbo pseudo) or keeps it genuinely allocated; alter_reg never pays a
 * slot. Do not re-propose.
 *
 * P1a (vars=0, score unchanged): reuse of multi-set local v —
 *     i++; v = i - 18; if (v < 0) goto loop1;
 * P1b (vars=0, score unchanged): fresh single-set intermediate —
 *     i++; lim = i - 18; if (lim < 0) goto loop1;
 * P1c (vars=0, score REGRESSES 6->7): branch-consumed intermediate —
 *     i++; lim = i - 18; if (lim != 0) goto loop1;
 *     (emits addiu+bnez where target has slti+bnez)
 *
 * Also measured in s17 with the same instruments, same verdict:
 * P2 (vars=0): lim = *(s16 *)(p + 2); b = lim >= 0;   [single narrow
 *     consumer folds clean; target has exactly ONE lh -> no second
 *     required HImode use exists in the algorithm]
 * P3 (vars=0): lim = *(s32 *)(q + 0x57); if (lim == 0) goto after2;
 *     [no multi-read field exists anywhere in this function]
 *
 * Orphan detector on the honest baseline: every pseudo (73..116, 31 live)
 * hard-reg allocated; 0 stack slots; 0 bare (use (reg)) orphans.
 * Full log: memory/grind/func_80041688/evidence.md [s17],
 * tmp/grind/func_80041688/s17/.
 */
