/* REJECTED — permuter score-0 find (s2 campaign s2-score1-random, 89 s,
 * tmp/perm_48AD0/output-0-1). CHEAT: the closing mutation is
 * `extern s32 snd_LoadBgm(volatile int);` — the permuter's
 * perm_randomize_external_type pass rewrote the PROTOTYPE from u8 to
 * (volatile) int. Widening the parameter removes the caller-side
 * `andi $a0,$a0,0xff` truncation: this is the twice-layer-2-REFUSED
 * declaration edit (A) reached by yet another syntactic route (int instead
 * of s32, plus a semantically inert `volatile` qualifier on a by-value
 * param). Same intent, same effect, banned by the s2 Judge constraint
 * ("respelling of the twice-refused declaration-type-correction (A)").
 * Body below the decls is byte-for-byte the honest score-1 form — the find
 * contains NO body-side lever. Recorded per no-new-park-categories
 * §Auto-search tools: permuter finds are proposals; this one is vetted and
 * rejected. Campaign follow-up: perm_randomize_external_type,
 * perm_randomize_function_type, perm_pad_var_decl zeroed in settings.toml
 * so the search is confined to body-side spellings. */
typedef int s32;
typedef unsigned char u8;
extern s32 snd_LoadBgm(volatile int); /* <-- the cheat mutation */
/* ... body identical to memory/grind/func_80048AD0/candidate.c (score-1
 * form with snd_LoadBgm(sound)); omitted — see tmp/perm_48AD0/output-0-1/source.c */
