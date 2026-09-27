extern u8 g_match_p1_score[2];
extern u8 g_match_p1_tiebreaker[2];
void func_800340A0(void) {
    u8 p1, p2, round;

    p1 = g_match_p1_score[0];
    if ((u8)p1 == D_800A37F8) {
        round = D_800A3874;
        D_800A377C[round] = 0;
    } else {
        p2 = g_match_p1_score[1];
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
            if ((u8)g_match_p1_tiebreaker[0] < (u8)g_match_p1_tiebreaker[1]) {
                g_match_p1_score[0]++;
                round = D_800A3874;
                D_800A377C[round] = 0;
            } else if ((u8)g_match_p1_tiebreaker[1] < (u8)g_match_p1_tiebreaker[0]) {
                g_match_p1_score[1] = p2 + 1;
                round = D_800A3874;
                D_800A377C[round] = 1;
            } else {
                round = D_800A3874;
                D_800A377C[round] = 2;
            }
        }
    }
    *(&D_800F65F8 + (D_800A3874 * 2)) = g_match_p1_score[0];
    *(&D_800F65F9 + (D_800A3874 * 2)) = g_match_p1_score[1];
    D_800A3874 = D_800A3874 + 1;
}
