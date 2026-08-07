/* s18 v01 rejected - masked 17, 179 build_insns.
 * Outer polling as for(;;) with break-on-timeout via sentinel v0=-2 and
 * structured if(v0==-2) do_timeout after the loop, replacing vT40's
 * goto do_timeout / goto success / goto check chain. Regressed +13
 * masked pts. Novel structural rewrite of outer flow, distinct from
 * s9v04 (masked 7 via inverted-vsync if/else + do_timeout inline). */
