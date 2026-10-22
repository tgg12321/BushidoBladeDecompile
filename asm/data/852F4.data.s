.include "macro.inc"

.section .data, "wa"
    .align 0

/* The .data from 0x80094AF4 (after d_80DF8.c) to 0x80094C68 (309CC.c's
 * D_80094C68), still asm: 309CC.c's head (g_gpu_draw_move_table ..
 * D_80094B96) (docs/foundation/backlog.tsv data-split). */

nonmatching g_gpu_draw_move_table

dlabel g_gpu_draw_move_table
    /* 852F4 80094AF4 90020000 */ .word 0x00000290
    /* 852F8 80094AF8 10002000 */ .word 0x00200010
    /* 852FC 80094AFC 80020000 */ .word 0x00000280
    /* 85300 80094B00 A0020000 */ .word 0x000002A0
    /* 85304 80094B04 10002000 */ .word 0x00200010
    /* 85308 80094B08 80020000 */ .word 0x00000280
    /* 8530C 80094B0C B0020000 */ .word 0x000002B0
    /* 85310 80094B10 10002000 */ .word 0x00200010
    /* 85314 80094B14 80020000 */ .word 0x00000280
    /* 85318 80094B18 90022000 */ .word 0x00200290
    /* 8531C 80094B1C 10002800 */ .word 0x00280010
    /* 85320 80094B20 80022000 */ .word 0x00200280
    /* 85324 80094B24 A0022000 */ .word 0x002002A0
    /* 85328 80094B28 10002800 */ .word 0x00280010
    /* 8532C 80094B2C 80022000 */ .word 0x00200280
    /* 85330 80094B30 C0024000 */ .word 0x004002C0
    /* 85334 80094B34 10002000 */ .word 0x00200010
    /* 85338 80094B38 80020000 */ .word 0x00000280
    /* 8533C 80094B3C D0024000 */ .word 0x004002D0
    /* 85340 80094B40 10002800 */ .word 0x00280010
    /* 85344 80094B44 80022000 */ .word 0x00200280
enddlabel g_gpu_draw_move_table

nonmatching D_80094B48

dlabel D_80094B48
    /* 85348 80094B48 */ .byte 0x1F
    /* 85349 80094B49 */ .byte 0x1F
    /* 8534A 80094B4A */ .byte 0x1F
    /* 8534B 80094B4B */ .byte 0x1F
    /* 8534C 80094B4C */ .byte 0x1F
    /* 8534D 80094B4D */ .byte 0x1F
    /* 8534E 80094B4E */ .byte 0x1F
    /* 8534F 80094B4F */ .byte 0x1F
    /* 85350 80094B50 */ .byte 0x1F
    /* 85351 80094B51 */ .byte 0x1F
    /* 85352 80094B52 */ .byte 0x1F
    /* 85353 80094B53 */ .byte 0x1C
    /* 85354 80094B54 */ .byte 0x1F
    /* 85355 80094B55 */ .byte 0x1F
    /* 85356 80094B56 */ .byte 0x00
    /* 85357 80094B57 */ .byte 0x1F
    /* 85358 80094B58 */ .byte 0x1F
    /* 85359 80094B59 */ .byte 0x1F
    /* 8535A 80094B5A */ .byte 0x1F
    /* 8535B 80094B5B */ .byte 0x1F
    /* 8535C 80094B5C */ .byte 0x1F
    /* 8535D 80094B5D */ .byte 0x1F
    /* 8535E 80094B5E */ .byte 0x1F
    /* 8535F 80094B5F */ .byte 0x1F
    /* 85360 80094B60 */ .byte 0x1F
    /* 85361 80094B61 */ .byte 0x1F
    /* 85362 80094B62 */ .byte 0x1F
    /* 85363 80094B63 */ .byte 0x1F
    /* 85364 80094B64 */ .byte 0x1F
    /* 85365 80094B65 */ .byte 0x00
    /* 85366 80094B66 */ .byte 0x1F
    /* 85367 80094B67 */ .byte 0x00
    /* 85368 80094B68 */ .byte 0x00
    /* 85369 80094B69 */ .byte 0x00
    /* 8536A 80094B6A */ .byte 0x00
    /* 8536B 80094B6B */ .byte 0x00
    /* 8536C 80094B6C */ .byte 0x00
    /* 8536D 80094B6D */ .byte 0x00
    /* 8536E 80094B6E */ .byte 0x00
    /* 8536F 80094B6F */ .byte 0x00
    /* 85370 80094B70 */ .byte 0x00
    /* 85371 80094B71 */ .byte 0x00
    /* 85372 80094B72 */ .byte 0x00
    /* 85373 80094B73 */ .byte 0x00
    /* 85374 80094B74 */ .byte 0x00
    /* 85375 80094B75 */ .byte 0x00
    /* 85376 80094B76 */ .byte 0x00
    /* 85377 80094B77 */ .byte 0x00
    /* 85378 80094B78 */ .byte 0x00
    /* 85379 80094B79 */ .byte 0x00
    /* 8537A 80094B7A */ .byte 0x00
    /* 8537B 80094B7B */ .byte 0x00
    /* 8537C 80094B7C */ .byte 0x00
    /* 8537D 80094B7D */ .byte 0x00
    /* 8537E 80094B7E */ .byte 0x00
    /* 8537F 80094B7F */ .byte 0x00
    /* 85380 80094B80 */ .byte 0x00
    /* 85381 80094B81 */ .byte 0x00
    /* 85382 80094B82 */ .byte 0x1C
    /* 85383 80094B83 */ .byte 0x1F
    /* 85384 80094B84 */ .byte 0x1F
    /* 85385 80094B85 */ .byte 0x1F
    /* 85386 80094B86 */ .byte 0x00
    /* 85387 80094B87 */ .byte 0x1F
enddlabel D_80094B48

nonmatching D_80094B88

dlabel D_80094B88
    /* 85388 80094B88 FFFFFFFF */ .word 0xFFFFFFFF
    /* 8538C 80094B8C FFFFFFFF */ .word 0xFFFFFFFF
    /* 85390 80094B90 FFFFFFFF */ .word 0xFFFFFFFF
    /* 85394 80094B94 */ .short 0xFFFF
enddlabel D_80094B88

nonmatching D_80094B96

dlabel D_80094B96
    /* 85396 80094B96 */ .short 0xFFFF
enddlabel D_80094B96

nonmatching D_80094B98

dlabel D_80094B98
    /* 85398 80094B98 */ .short 0x0000
enddlabel D_80094B98

nonmatching D_80094B9A

dlabel D_80094B9A
    /* 8539A 80094B9A */ .short 0x0000
enddlabel D_80094B9A

nonmatching D_80094B9C

dlabel D_80094B9C
    /* 8539C 80094B9C */ .short 0x0000
enddlabel D_80094B9C

nonmatching D_80094B9E

dlabel D_80094B9E
    /* 8539E 80094B9E */ .short 0xFFFF
    /* 853A0 80094BA0 */ .short 0x0000
    /* 853A2 80094BA2 */ .short 0x0000
    /* 853A4 80094BA4 */ .short 0x0000
    /* 853A6 80094BA6 */ .short 0x0000
    /* 853A8 80094BA8 */ .short 0x0000
    /* 853AA 80094BAA */ .short 0x0001
    /* 853AC 80094BAC */ .short 0x0000
    /* 853AE 80094BAE */ .short 0x000A
    /* 853B0 80094BB0 */ .short 0x0000
    /* 853B2 80094BB2 */ .short 0x0001
    /* 853B4 80094BB4 */ .short 0x0002
    /* 853B6 80094BB6 */ .short 0x01D4
    /* 853B8 80094BB8 */ .short 0x0000
    /* 853BA 80094BBA */ .short 0x0000
    /* 853BC 80094BBC */ .short 0xFFFF
    /* 853BE 80094BBE */ .short 0x0002
    /* 853C0 80094BC0 */ .short 0x0121
    /* 853C2 80094BC2 */ .short 0x0000
    /* 853C4 80094BC4 */ .short 0xFF9C
    /* 853C6 80094BC6 */ .short 0x0002
    /* 853C8 80094BC8 */ .short 0x0004
    /* 853CA 80094BCA */ .short 0x008C
    /* 853CC 80094BCC */ .short 0x0000
    /* 853CE 80094BCE */ .short 0x0000
    /* 853D0 80094BD0 */ .short 0x0003
    /* 853D2 80094BD2 */ .short 0x0005
    /* 853D4 80094BD4 */ .short 0x0106
    /* 853D6 80094BD6 */ .short 0x0000
    /* 853D8 80094BD8 */ .short 0x0000
    /* 853DA 80094BDA */ .short 0x0004
    /* 853DC 80094BDC */ .short 0x0006
    /* 853DE 80094BDE */ .short 0x00D3
    /* 853E0 80094BE0 */ .short 0x0000
    /* 853E2 80094BE2 */ .short 0xFFFD
    /* 853E4 80094BE4 */ .short 0xFFFF
    /* 853E6 80094BE6 */ .short 0x0002
    /* 853E8 80094BE8 */ .short 0x0121
    /* 853EA 80094BEA */ .short 0x0000
    /* 853EC 80094BEC */ .short 0x0064
    /* 853EE 80094BEE */ .short 0x0005
    /* 853F0 80094BF0 */ .short 0x0008
    /* 853F2 80094BF2 */ .short 0x008C
    /* 853F4 80094BF4 */ .short 0x0000
    /* 853F6 80094BF6 */ .short 0x0000
    /* 853F8 80094BF8 */ .short 0x0006
    /* 853FA 80094BFA */ .short 0x0009
    /* 853FC 80094BFC */ .short 0x0106
    /* 853FE 80094BFE */ .short 0x0000
    /* 85400 80094C00 */ .short 0x0000
    /* 85402 80094C02 */ .short 0x0007
    /* 85404 80094C04 */ .short 0x000A
    /* 85406 80094C06 */ .short 0x00D3
    /* 85408 80094C08 */ .short 0x0000
    /* 8540A 80094C0A */ .short 0xFFFE
    /* 8540C 80094C0C */ .short 0x0008
    /* 8540E 80094C0E */ .short 0x0001
    /* 85410 80094C10 */ .short 0x0000
    /* 85412 80094C12 */ .short 0x000A
    /* 85414 80094C14 */ .short 0x0000
    /* 85416 80094C16 */ .short 0x0009
    /* 85418 80094C18 */ .short 0x000C
    /* 8541A 80094C1A */ .short 0x00B4
    /* 8541C 80094C1C */ .short 0x0000
    /* 8541E 80094C1E */ .short 0x006E
    /* 85420 80094C20 */ .short 0x000A
    /* 85422 80094C22 */ .short 0x000D
    /* 85424 80094C24 */ .short 0x0186
    /* 85426 80094C26 */ .short 0x0000
    /* 85428 80094C28 */ .short 0x0000
    /* 8542A 80094C2A */ .short 0x000B
    /* 8542C 80094C2C */ .short 0x000E
    /* 8542E 80094C2E */ .short 0x01B9
    /* 85430 80094C30 */ .short 0x0000
    /* 85432 80094C32 */ .short 0x0000
    /* 85434 80094C34 */ .short 0x000C
    /* 85436 80094C36 */ .short 0x000C
    /* 85438 80094C38 */ .short 0x00B4
    /* 8543A 80094C3A */ .short 0x0000
    /* 8543C 80094C3C */ .short 0xFF93
    /* 8543E 80094C3E */ .short 0x000D
    /* 85440 80094C40 */ .short 0x0010
    /* 85442 80094C42 */ .short 0x0186
    /* 85444 80094C44 */ .short 0x0000
    /* 85446 80094C46 */ .short 0x0000
    /* 85448 80094C48 */ .short 0x000E
    /* 8544A 80094C4A */ .short 0x0011
    /* 8544C 80094C4C */ .short 0x01BC
    /* 8544E 80094C4E */ .short 0x0000
    /* 85450 80094C50 */ .short 0x0000
    /* 85452 80094C52 */ .short 0xFFFF
    /* 85454 80094C54 */ .short 0x0001
    /* 85456 80094C56 */ .short 0x0000
    /* 85458 80094C58 */ .short 0x0000
    /* 8545A 80094C5A */ .short 0x0000
    /* 8545C 80094C5C */ .short 0xFFFF
    /* 8545E 80094C5E */ .short 0x0001
    /* 85460 80094C60 */ .short 0x0000
    /* 85462 80094C62 */ .short 0x0000
    /* 85464 80094C64 */ .short 0x0000
    /* 85466 80094C66 */ .short 0x0000
enddlabel D_80094B9E
