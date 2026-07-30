/* REJECTED — the pre-session src form. Dead because it is a cheat AND because
 * a pure-C construct now does the same job.
 *
 * It bought the two D_80101E60 reloads with a scheduling/CSE barrier:
 *
 *     D_80101E60 = arg0;
 *     D_80101E6C = entry[0];
 *     D_80101E70 = entry[1];
 *     asm volatile("" ::: "memory");          <-- the cheat
 *     D_80101E74 = cdrom_BcdToFrames(... D_80101E60 ...) ...;
 *
 * The barrier hashes as a volatile ASM_OPERANDS in cse.c, which flushes the
 * memory hash table so the read of D_80101E60 is not forwarded from the `sh`.
 * The sandbox strips it, so it never moved the honest floor: 17.
 *
 * Also present in that form and equally dead for this function:
 *   extern volatile s32 D_80101E70;    (code6cac_b2_post.c:45)
 * a volatile store does NOT call invalidate_memory in GCC 2.7.2 cse — it only
 * goes unrecorded itself — so it never defeated the fold in the first place.
 *
 * Superseded by the aggregate-assignment lever (see ../candidate.c and
 * ../evidence.md): a BLKmode destination sets note_mem_written's `writes->all`
 * and flushes cse's memory table in pure C. Floor 17 -> 9.
 */
