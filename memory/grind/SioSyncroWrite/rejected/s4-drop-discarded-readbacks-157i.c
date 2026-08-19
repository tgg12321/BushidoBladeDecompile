/* REJECTED (s4) — 157 insns vs target 159 (two `lw` read-backs missing).
 * Spelling `st[1] = st[1] + 1;` / `st[2] = st[2] - 1;` (explicit read-modify-
 * write) does NOT emit the discarded re-load that target has at
 * asm/funcs/SioSyncroWrite.s:107 and :111.  Only the increment/decrement
 * OPERATORS on a volatile lvalue with a discarded result do
 * (`st[1]++;` / `st[2]--;` — 159i, exact match; `++st[1];` / `--st[2];` also
 * 159i exact).  Removing the read-backs altogether is 157i, and keeping only
 * one of the two is 158i — both re-loads are load-bearing and semantically
 * real, so they must NOT be treated as padding.                              */
        st[1] = st[1] + 1;
        i += 1;
        st[2] = st[2] - 1;
