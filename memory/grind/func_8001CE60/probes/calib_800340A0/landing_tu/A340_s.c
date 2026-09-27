/* struct form: one object per pair, members as named bytes */
struct score_pair { u8 p1; u8 p2; };
extern struct score_pair g_match_score;
extern struct score_pair g_match_tiebreaker;
void func_800340A0(void) {
    u8 p1, p2, round;

    p1 = g_match_score.p1;
    if ((u8)p1 == D_800A37F8) {
        round = D_800A3874;
        D_800A377C[round] = 0;
    } else {
        p2 = g_match_score.p2;
        if ((u8)p2 == D_800A37F8) {
            round = D_800A3874;
            D_800A377C[round] = 1;
        } else if ((u8)p2 < (u8)p1) {
            round = D_800A3874;
            D_800A377C[round] = 0;
        } else if ((u8)p1 < (u8)p2) {
            round = D_800A3874;
            D_800A377C[round] = 1;
        } else {
            if ((u8)g_match_tiebreaker.p1 < (u8)g_match_tiebreaker.p2) {
                g_match_score.p1 = p1 + 1;
                round = D_800A3874;
                D_800A377C[round] = 0;
            } else if ((u8)g_match_tiebreaker.p2 < (u8)g_match_tiebreaker.p1) {
                g_match_score.p2 = p2 + 1;
                round = D_800A3874;
                D_800A377C[round] = 1;
            } else {
                round = D_800A3874;
                D_800A377C[round] = 2;
            }
        }
    }
    *(&D_800F65F8 + (D_800A3874 * 2)) = g_match_score.p1;
    *(&D_800F65F9 + (D_800A3874 * 2)) = g_match_score.p2;
    D_800A3874 = D_800A3874 + 1;
}
