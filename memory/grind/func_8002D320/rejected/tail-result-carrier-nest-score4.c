/* REJECTED (s1, 2026-08-31): result-carrier nested tail -- score 4, 119 insns
 * (vs the plain early-return control's 3/118 and target's 120).  The carrier
 * adds an instruction and does NOT keep the unfolded 0/1 diamond; the old
 * regfix rules 3106-3109 existed precisely to hand-erase this carrier from the
 * retired chassis, so this confirms the brief's LEVER 2 in the forward
 * direction as well.  Tail fragment (rest of body = candidate.c): */
        y = *(s32 *)(obj + 0x108);
        {
            s32 result = 0;
            if (max_y >= y - x) {
                if (y + x >= min_y) result = 1;
            }
            return result;
        }
/* Also measured dead this session (byte-identical to the folding control,
 * score 3/118 -- jump threading re-folds both):
 *   if (y + x < min_y) goto reject; return 1; reject: return 0;
 *   if (y + x >= min_y) return 1; return 0;
 */
