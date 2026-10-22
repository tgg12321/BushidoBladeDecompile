.include "macro.inc"

.section .data, "wa"
    .align 0

/* The .data of 4510C.c .. 51268.c (0x80099D88..0x8009BC04), still asm; it
 * moves to C one owning TU at a time (docs/foundation/backlog.tsv data-split). */

nonmatching D_80099D88

dlabel D_80099D88
    /* 8A588 80099D88 */ .short 0x0000
    /* 8A58A 80099D8A */ .byte 0x00
enddlabel D_80099D88

nonmatching D_80099D8B

dlabel D_80099D8B
    /* 8A58B 80099D8B */ .byte 0x04
enddlabel D_80099D8B

nonmatching D_80099D8C

dlabel D_80099D8C
    /* 8A58C 80099D8C */ .byte 0x00
enddlabel D_80099D8C

nonmatching D_80099D8D

dlabel D_80099D8D
    /* 8A58D 80099D8D */ .byte 0x40
enddlabel D_80099D8D

nonmatching D_80099D8E

dlabel D_80099D8E
    /* 8A58E 80099D8E */ .byte 0x04
enddlabel D_80099D8E

nonmatching D_80099D8F

dlabel D_80099D8F
    /* 8A58F 80099D8F */ .byte 0x80
enddlabel D_80099D8F

nonmatching D_80099D90

dlabel D_80099D90
    /* 8A590 80099D90 80FF0000 */ .word 0x0000FF80
enddlabel D_80099D90

nonmatching D_80099D94

dlabel D_80099D94
    /* 8A594 80099D94 */ .byte 0x00
    /* 8A595 80099D95 */ .byte 0x00
    /* 8A596 80099D96 */ .byte 0x00
enddlabel D_80099D94

nonmatching D_80099D97

dlabel D_80099D97
    /* 8A597 80099D97 */ .byte 0xFF
    /* 8A598 80099D98 */ .byte 0xC0
    /* 8A599 80099D99 */ .byte 0x80
    /* 8A59A 80099D9A */ .byte 0x40
    /* 8A59B 80099D9B */ .byte 0x40
enddlabel D_80099D97

nonmatching D_80099D9C

dlabel D_80099D9C
    /* 8A59C 80099D9C */ .byte 0x40
enddlabel D_80099D9C

nonmatching D_80099D9D

dlabel D_80099D9D
    /* 8A59D 80099D9D */ .byte 0x40
    /* 8A59E 80099D9E */ .byte 0x40
    /* 8A59F 80099D9F */ .byte 0x00
    /* 8A5A0 80099DA0 */ .byte 0x07
    /* 8A5A1 80099DA1 */ .byte 0x00
    /* 8A5A2 80099DA2 */ .byte 0x03
    /* 8A5A3 80099DA3 */ .byte 0x04
    /* 8A5A4 80099DA4 */ .byte 0x20
    /* 8A5A5 80099DA5 */ .byte 0x80
    /* 8A5A6 80099DA6 */ .byte 0x08
    /* 8A5A7 80099DA7 */ .byte 0x40
    /* 8A5A8 80099DA8 */ .byte 0x80
    /* 8A5A9 80099DA9 */ .byte 0x80
    /* 8A5AA 80099DAA */ .byte 0xFF
    /* 8A5AB 80099DAB */ .byte 0xFF
    /* 8A5AC 80099DAC */ .byte 0x60
    /* 8A5AD 80099DAD */ .byte 0x80
    /* 8A5AE 80099DAE */ .byte 0x00
    /* 8A5AF 80099DAF */ .byte 0x40
    /* 8A5B0 80099DB0 */ .byte 0xC0
    /* 8A5B1 80099DB1 */ .byte 0xC0
    /* 8A5B2 80099DB2 */ .byte 0x80
    /* 8A5B3 80099DB3 */ .byte 0x40
    /* 8A5B4 80099DB4 */ .byte 0x80
    /* 8A5B5 80099DB5 */ .byte 0x40
    /* 8A5B6 80099DB6 */ .byte 0x80
    /* 8A5B7 80099DB7 */ .byte 0x00
    /* 8A5B8 80099DB8 */ .byte 0x12
    /* 8A5B9 80099DB9 */ .byte 0x00
    /* 8A5BA 80099DBA */ .byte 0x02
    /* 8A5BB 80099DBB */ .byte 0x03
    /* 8A5BC 80099DBC */ .byte 0xFF
    /* 8A5BD 80099DBD */ .byte 0xC0
    /* 8A5BE 80099DBE */ .byte 0x10
    /* 8A5BF 80099DBF */ .byte 0xFF
    /* 8A5C0 80099DC0 */ .byte 0x20
    /* 8A5C1 80099DC1 */ .byte 0xFF
    /* 8A5C2 80099DC2 */ .byte 0x00
    /* 8A5C3 80099DC3 */ .byte 0x00
    /* 8A5C4 80099DC4 */ .byte 0x40
    /* 8A5C5 80099DC5 */ .byte 0x00
    /* 8A5C6 80099DC6 */ .byte 0x00
    /* 8A5C7 80099DC7 */ .byte 0x40
    /* 8A5C8 80099DC8 */ .byte 0x80
    /* 8A5C9 80099DC9 */ .byte 0x80
    /* 8A5CA 80099DCA */ .byte 0x80
    /* 8A5CB 80099DCB */ .byte 0x80
    /* 8A5CC 80099DCC */ .byte 0x40
    /* 8A5CD 80099DCD */ .byte 0xC0
    /* 8A5CE 80099DCE */ .byte 0xC0
    /* 8A5CF 80099DCF */ .byte 0x00
    /* 8A5D0 80099DD0 */ .byte 0x00
    /* 8A5D1 80099DD1 */ .byte 0x00
    /* 8A5D2 80099DD2 */ .byte 0x02
    /* 8A5D3 80099DD3 */ .byte 0x02
    /* 8A5D4 80099DD4 */ .byte 0x20
    /* 8A5D5 80099DD5 */ .byte 0x80
    /* 8A5D6 80099DD6 */ .byte 0x06
    /* 8A5D7 80099DD7 */ .byte 0xFF
    /* 8A5D8 80099DD8 */ .byte 0x80
    /* 8A5D9 80099DD9 */ .byte 0xFF
    /* 8A5DA 80099DDA */ .byte 0x00
    /* 8A5DB 80099DDB */ .byte 0x00
    /* 8A5DC 80099DDC */ .byte 0x00
    /* 8A5DD 80099DDD */ .byte 0x80
    /* 8A5DE 80099DDE */ .byte 0x00
    /* 8A5DF 80099DDF */ .byte 0xC0
    /* 8A5E0 80099DE0 */ .byte 0xC0
    /* 8A5E1 80099DE1 */ .byte 0x80
    /* 8A5E2 80099DE2 */ .byte 0x40
    /* 8A5E3 80099DE3 */ .byte 0x40
    /* 8A5E4 80099DE4 */ .byte 0x40
    /* 8A5E5 80099DE5 */ .byte 0x40
    /* 8A5E6 80099DE6 */ .byte 0x80
    /* 8A5E7 80099DE7 */ .byte 0x00
    /* 8A5E8 80099DE8 */ .byte 0x07
    /* 8A5E9 80099DE9 */ .byte 0x00
    /* 8A5EA 80099DEA */ .byte 0x01
    /* 8A5EB 80099DEB */ .byte 0x02
    /* 8A5EC 80099DEC */ .byte 0xFF
    /* 8A5ED 80099DED */ .byte 0xC0
    /* 8A5EE 80099DEE */ .byte 0x10
    /* 8A5EF 80099DEF */ .byte 0x40
    /* 8A5F0 80099DF0 */ .byte 0xFF
    /* 8A5F1 80099DF1 */ .byte 0x00
    /* 8A5F2 80099DF2 */ .byte 0xFF
    /* 8A5F3 80099DF3 */ .byte 0xFF
    /* 8A5F4 80099DF4 */ .byte 0x60
    /* 8A5F5 80099DF5 */ .byte 0x00
    /* 8A5F6 80099DF6 */ .byte 0x00
    /* 8A5F7 80099DF7 */ .byte 0x40
    /* 8A5F8 80099DF8 */ .byte 0x40
    /* 8A5F9 80099DF9 */ .byte 0xC0
    /* 8A5FA 80099DFA */ .byte 0x80
    /* 8A5FB 80099DFB */ .byte 0x80
    /* 8A5FC 80099DFC */ .byte 0xC0
    /* 8A5FD 80099DFD */ .byte 0xC0
    /* 8A5FE 80099DFE */ .byte 0xC0
    /* 8A5FF 80099DFF */ .byte 0x00
    /* 8A600 80099E00 */ .byte 0x05
    /* 8A601 80099E01 */ .byte 0x00
    /* 8A602 80099E02 */ .byte 0x01
    /* 8A603 80099E03 */ .byte 0x03
    /* 8A604 80099E04 */ .byte 0xFF
    /* 8A605 80099E05 */ .byte 0xFF
    /* 8A606 80099E06 */ .byte 0x20
    /* 8A607 80099E07 */ .byte 0x20
    /* 8A608 80099E08 */ .byte 0xFF
    /* 8A609 80099E09 */ .byte 0xFF
    /* 8A60A 80099E0A */ .byte 0x00
    /* 8A60B 80099E0B */ .byte 0xFF
    /* 8A60C 80099E0C */ .byte 0x00
    /* 8A60D 80099E0D */ .byte 0x00
    /* 8A60E 80099E0E */ .byte 0x00
    /* 8A60F 80099E0F */ .byte 0x00
    /* 8A610 80099E10 */ .byte 0xFF
    /* 8A611 80099E11 */ .byte 0x80
    /* 8A612 80099E12 */ .byte 0x40
    /* 8A613 80099E13 */ .byte 0x00
    /* 8A614 80099E14 */ .byte 0x00
    /* 8A615 80099E15 */ .byte 0x00
    /* 8A616 80099E16 */ .byte 0x00
    /* 8A617 80099E17 */ .byte 0x00
    /* 8A618 80099E18 */ .byte 0xB7
    /* 8A619 80099E19 */ .byte 0x00
    /* 8A61A 80099E1A */ .byte 0x01
    /* 8A61B 80099E1B */ .byte 0x02
    /* 8A61C 80099E1C */ .byte 0x00
    /* 8A61D 80099E1D */ .byte 0x40
    /* 8A61E 80099E1E */ .byte 0x10
    /* 8A61F 80099E1F */ .byte 0x40
    /* 8A620 80099E20 */ .byte 0xFF
    /* 8A621 80099E21 */ .byte 0xFF
    /* 8A622 80099E22 */ .byte 0xFF
    /* 8A623 80099E23 */ .byte 0xFF
    /* 8A624 80099E24 */ .byte 0x40
    /* 8A625 80099E25 */ .byte 0xFF
    /* 8A626 80099E26 */ .byte 0x00
    /* 8A627 80099E27 */ .byte 0xC0
    /* 8A628 80099E28 */ .byte 0x40
    /* 8A629 80099E29 */ .byte 0x80
    /* 8A62A 80099E2A */ .byte 0xC0
    /* 8A62B 80099E2B */ .byte 0x80
    /* 8A62C 80099E2C */ .byte 0xC0
    /* 8A62D 80099E2D */ .byte 0xC0
    /* 8A62E 80099E2E */ .byte 0x40
    /* 8A62F 80099E2F */ .byte 0x00
    /* 8A630 80099E30 */ .byte 0x00
    /* 8A631 80099E31 */ .byte 0x00
    /* 8A632 80099E32 */ .byte 0x00
    /* 8A633 80099E33 */ .byte 0x03
    /* 8A634 80099E34 */ .byte 0x00
    /* 8A635 80099E35 */ .byte 0x40
    /* 8A636 80099E36 */ .byte 0x04
    /* 8A637 80099E37 */ .byte 0xFF
    /* 8A638 80099E38 */ .byte 0xFF
    /* 8A639 80099E39 */ .byte 0x00
    /* 8A63A 80099E3A */ .byte 0x00
    /* 8A63B 80099E3B */ .byte 0x00
    /* 8A63C 80099E3C */ .byte 0x00
    /* 8A63D 80099E3D */ .byte 0x80
    /* 8A63E 80099E3E */ .byte 0x00
    /* 8A63F 80099E3F */ .byte 0xFF
    /* 8A640 80099E40 */ .byte 0xC0
    /* 8A641 80099E41 */ .byte 0x00
    /* 8A642 80099E42 */ .byte 0x40
    /* 8A643 80099E43 */ .byte 0x40
    /* 8A644 80099E44 */ .byte 0x40
    /* 8A645 80099E45 */ .byte 0x40
    /* 8A646 80099E46 */ .byte 0x80
    /* 8A647 80099E47 */ .byte 0x00
    /* 8A648 80099E48 */ .byte 0x53
    /* 8A649 80099E49 */ .byte 0x00
    /* 8A64A 80099E4A */ .byte 0x00
    /* 8A64B 80099E4B */ .byte 0x02
    /* 8A64C 80099E4C */ .byte 0xFF
    /* 8A64D 80099E4D */ .byte 0xC0
    /* 8A64E 80099E4E */ .byte 0x10
    /* 8A64F 80099E4F */ .byte 0x80
    /* 8A650 80099E50 */ .byte 0xFF
    /* 8A651 80099E51 */ .byte 0xFF
    /* 8A652 80099E52 */ .byte 0x80
    /* 8A653 80099E53 */ .byte 0x80
    /* 8A654 80099E54 */ .byte 0x80
    /* 8A655 80099E55 */ .byte 0xFF
    /* 8A656 80099E56 */ .byte 0x00
    /* 8A657 80099E57 */ .byte 0xFF
    /* 8A658 80099E58 */ .byte 0x40
    /* 8A659 80099E59 */ .byte 0xC0
    /* 8A65A 80099E5A */ .byte 0xC0
    /* 8A65B 80099E5B */ .byte 0x40
    /* 8A65C 80099E5C */ .byte 0x80
    /* 8A65D 80099E5D */ .byte 0x80
    /* 8A65E 80099E5E */ .byte 0xC0
    /* 8A65F 80099E5F */ .byte 0x00
    /* 8A660 80099E60 */ .byte 0x83
    /* 8A661 80099E61 */ .byte 0x00
    /* 8A662 80099E62 */ .byte 0x03
    /* 8A663 80099E63 */ .byte 0x03
    /* 8A664 80099E64 */ .byte 0xFF
    /* 8A665 80099E65 */ .byte 0x80
    /* 8A666 80099E66 */ .byte 0x08
    /* 8A667 80099E67 */ .byte 0xFF
    /* 8A668 80099E68 */ .byte 0x20
    /* 8A669 80099E69 */ .byte 0xFF
    /* 8A66A 80099E6A */ .byte 0x00
    /* 8A66B 80099E6B */ .byte 0x00
    /* 8A66C 80099E6C */ .byte 0x00
    /* 8A66D 80099E6D */ .byte 0xFF
    /* 8A66E 80099E6E */ .byte 0x00
    /* 8A66F 80099E6F */ .byte 0x40
    /* 8A670 80099E70 */ .byte 0x80
    /* 8A671 80099E71 */ .byte 0xC0
    /* 8A672 80099E72 */ .byte 0x40
    /* 8A673 80099E73 */ .byte 0x40
    /* 8A674 80099E74 */ .byte 0x80
    /* 8A675 80099E75 */ .byte 0x80
    /* 8A676 80099E76 */ .byte 0x80
    /* 8A677 80099E77 */ .byte 0x00
    /* 8A678 80099E78 */ .byte 0x02
    /* 8A679 80099E79 */ .byte 0x00
    /* 8A67A 80099E7A */ .byte 0x02
    /* 8A67B 80099E7B */ .byte 0x03
    /* 8A67C 80099E7C */ .byte 0x20
    /* 8A67D 80099E7D */ .byte 0x00
    /* 8A67E 80099E7E */ .byte 0x08
    /* 8A67F 80099E7F */ .byte 0xFF
    /* 8A680 80099E80 */ .byte 0x80
    /* 8A681 80099E81 */ .byte 0x80
    /* 8A682 80099E82 */ .byte 0x80
    /* 8A683 80099E83 */ .byte 0x80
    /* 8A684 80099E84 */ .byte 0x00
    /* 8A685 80099E85 */ .byte 0xFF
    /* 8A686 80099E86 */ .byte 0x00
    /* 8A687 80099E87 */ .byte 0xC0
    /* 8A688 80099E88 */ .byte 0x80
    /* 8A689 80099E89 */ .byte 0x80
    /* 8A68A 80099E8A */ .byte 0x40
    /* 8A68B 80099E8B */ .byte 0x40
    /* 8A68C 80099E8C */ .byte 0x40
    /* 8A68D 80099E8D */ .byte 0x40
    /* 8A68E 80099E8E */ .byte 0xC0
    /* 8A68F 80099E8F */ .byte 0x00
    /* 8A690 80099E90 */ .byte 0xC1
    /* 8A691 80099E91 */ .byte 0x01
    /* 8A692 80099E92 */ .byte 0x09
    /* 8A693 80099E93 */ .byte 0xFF
    /* 8A694 80099E94 */ .byte 0x00
    /* 8A695 80099E95 */ .byte 0x80
    /* 8A696 80099E96 */ .byte 0x20
    /* 8A697 80099E97 */ .byte 0x30
    /* 8A698 80099E98 */ .byte 0x00
    /* 8A699 80099E99 */ .byte 0x00
    /* 8A69A 80099E9A */ .byte 0xFF
    /* 8A69B 80099E9B */ .byte 0x80
    /* 8A69C 80099E9C */ .byte 0x00
    /* 8A69D 80099E9D */ .byte 0x00
    /* 8A69E 80099E9E */ .byte 0x00
    /* 8A69F 80099E9F */ .byte 0x00
    /* 8A6A0 80099EA0 */ .byte 0xFF
    /* 8A6A1 80099EA1 */ .byte 0xFF
    /* 8A6A2 80099EA2 */ .byte 0xFF
    /* 8A6A3 80099EA3 */ .byte 0xFF
    /* 8A6A4 80099EA4 */ .byte 0x00
    /* 8A6A5 80099EA5 */ .byte 0x40
    /* 8A6A6 80099EA6 */ .byte 0xFF
    /* 8A6A7 80099EA7 */ .byte 0x00
    /* 8A6A8 80099EA8 */ .byte 0x11
    /* 8A6A9 80099EA9 */ .byte 0x00
    /* 8A6AA 80099EAA */ .byte 0x02
    /* 8A6AB 80099EAB */ .byte 0x04
    /* 8A6AC 80099EAC */ .byte 0x00
    /* 8A6AD 80099EAD */ .byte 0x80
    /* 8A6AE 80099EAE */ .byte 0x10
    /* 8A6AF 80099EAF */ .byte 0xFF
    /* 8A6B0 80099EB0 */ .byte 0x20
    /* 8A6B1 80099EB1 */ .byte 0xFF
    /* 8A6B2 80099EB2 */ .byte 0x00
    /* 8A6B3 80099EB3 */ .byte 0x00
    /* 8A6B4 80099EB4 */ .byte 0x00
    /* 8A6B5 80099EB5 */ .byte 0x80
    /* 8A6B6 80099EB6 */ .byte 0x00
    /* 8A6B7 80099EB7 */ .byte 0xC0
    /* 8A6B8 80099EB8 */ .byte 0x80
    /* 8A6B9 80099EB9 */ .byte 0xC0
    /* 8A6BA 80099EBA */ .byte 0x40
    /* 8A6BB 80099EBB */ .byte 0x40
    /* 8A6BC 80099EBC */ .byte 0x40
    /* 8A6BD 80099EBD */ .byte 0x40
    /* 8A6BE 80099EBE */ .byte 0x40
    /* 8A6BF 80099EBF */ .byte 0x00
    /* 8A6C0 80099EC0 */ .byte 0x01
    /* 8A6C1 80099EC1 */ .byte 0x00
    /* 8A6C2 80099EC2 */ .byte 0x04
    /* 8A6C3 80099EC3 */ .byte 0x03
    /* 8A6C4 80099EC4 */ .byte 0xFF
    /* 8A6C5 80099EC5 */ .byte 0x80
    /* 8A6C6 80099EC6 */ .byte 0x08
    /* 8A6C7 80099EC7 */ .byte 0x80
    /* 8A6C8 80099EC8 */ .byte 0x80
    /* 8A6C9 80099EC9 */ .byte 0xFF
    /* 8A6CA 80099ECA */ .byte 0x00
    /* 8A6CB 80099ECB */ .byte 0x00
    /* 8A6CC 80099ECC */ .byte 0x00
    /* 8A6CD 80099ECD */ .byte 0x00
    /* 8A6CE 80099ECE */ .byte 0x00
    /* 8A6CF 80099ECF */ .byte 0xC0
    /* 8A6D0 80099ED0 */ .byte 0xC0
    /* 8A6D1 80099ED1 */ .byte 0x80
    /* 8A6D2 80099ED2 */ .byte 0x40
    /* 8A6D3 80099ED3 */ .byte 0x40
    /* 8A6D4 80099ED4 */ .byte 0x80
    /* 8A6D5 80099ED5 */ .byte 0x40
    /* 8A6D6 80099ED6 */ .byte 0x80
    /* 8A6D7 80099ED7 */ .byte 0x00
    /* 8A6D8 80099ED8 */ .byte 0x87
    /* 8A6D9 80099ED9 */ .byte 0x00
    /* 8A6DA 80099EDA */ .byte 0x00
    /* 8A6DB 80099EDB */ .byte 0x02
    /* 8A6DC 80099EDC */ .byte 0xFF
    /* 8A6DD 80099EDD */ .byte 0xC0
    /* 8A6DE 80099EDE */ .byte 0x18
    /* 8A6DF 80099EDF */ .byte 0x40
    /* 8A6E0 80099EE0 */ .byte 0xFF
    /* 8A6E1 80099EE1 */ .byte 0x80
    /* 8A6E2 80099EE2 */ .byte 0x80
    /* 8A6E3 80099EE3 */ .byte 0x80
    /* 8A6E4 80099EE4 */ .byte 0x40
    /* 8A6E5 80099EE5 */ .byte 0x00
    /* 8A6E6 80099EE6 */ .byte 0x00
    /* 8A6E7 80099EE7 */ .byte 0xC0
    /* 8A6E8 80099EE8 */ .byte 0x40
    /* 8A6E9 80099EE9 */ .byte 0x40
    /* 8A6EA 80099EEA */ .byte 0xC0
    /* 8A6EB 80099EEB */ .byte 0x80
    /* 8A6EC 80099EEC */ .byte 0xC0
    /* 8A6ED 80099EED */ .byte 0xC0
    /* 8A6EE 80099EEE */ .byte 0xC0
    /* 8A6EF 80099EEF */ .byte 0x00
    /* 8A6F0 80099EF0 */ .byte 0x03
    /* 8A6F1 80099EF1 */ .byte 0x00
    /* 8A6F2 80099EF2 */ .byte 0x02
    /* 8A6F3 80099EF3 */ .byte 0x04
    /* 8A6F4 80099EF4 */ .byte 0x00
    /* 8A6F5 80099EF5 */ .byte 0x40
    /* 8A6F6 80099EF6 */ .byte 0x08
    /* 8A6F7 80099EF7 */ .byte 0x80
    /* 8A6F8 80099EF8 */ .byte 0xFF
    /* 8A6F9 80099EF9 */ .byte 0x80
    /* 8A6FA 80099EFA */ .byte 0x00
    /* 8A6FB 80099EFB */ .byte 0x00
    /* 8A6FC 80099EFC */ .byte 0x00
    /* 8A6FD 80099EFD */ .byte 0xFF
    /* 8A6FE 80099EFE */ .byte 0x00
    /* 8A6FF 80099EFF */ .byte 0xFF
    /* 8A700 80099F00 */ .byte 0x40
    /* 8A701 80099F01 */ .byte 0xC0
    /* 8A702 80099F02 */ .byte 0x80
    /* 8A703 80099F03 */ .byte 0x40
    /* 8A704 80099F04 */ .byte 0x40
    /* 8A705 80099F05 */ .byte 0x40
    /* 8A706 80099F06 */ .byte 0x80
    /* 8A707 80099F07 */ .byte 0x00
    /* 8A708 80099F08 */ .byte 0x27
    /* 8A709 80099F09 */ .byte 0x00
    /* 8A70A 80099F0A */ .byte 0x00
    /* 8A70B 80099F0B */ .byte 0x00
    /* 8A70C 80099F0C */ .byte 0x00
    /* 8A70D 80099F0D */ .byte 0xFF
    /* 8A70E 80099F0E */ .byte 0x40
    /* 8A70F 80099F0F */ .byte 0x00
    /* 8A710 80099F10 */ .byte 0x20
    /* 8A711 80099F11 */ .byte 0x00
    /* 8A712 80099F12 */ .byte 0xFF
    /* 8A713 80099F13 */ .byte 0xFF
    /* 8A714 80099F14 */ .byte 0x00
    /* 8A715 80099F15 */ .byte 0x00
    /* 8A716 80099F16 */ .byte 0x00
    /* 8A717 80099F17 */ .byte 0x00
    /* 8A718 80099F18 */ .byte 0x40
    /* 8A719 80099F19 */ .byte 0x80
    /* 8A71A 80099F1A */ .byte 0xFF
    /* 8A71B 80099F1B */ .byte 0x00
    /* 8A71C 80099F1C */ .byte 0xFF
    /* 8A71D 80099F1D */ .byte 0x00
    /* 8A71E 80099F1E */ .byte 0x00
    /* 8A71F 80099F1F */ .byte 0x00
    /* 8A720 80099F20 */ .byte 0x02
    /* 8A721 80099F21 */ .byte 0x00
    /* 8A722 80099F22 */ .byte 0x02
    /* 8A723 80099F23 */ .byte 0x03
    /* 8A724 80099F24 */ .byte 0x20
    /* 8A725 80099F25 */ .byte 0x80
    /* 8A726 80099F26 */ .byte 0x10
    /* 8A727 80099F27 */ .byte 0xFF
    /* 8A728 80099F28 */ .byte 0x20
    /* 8A729 80099F29 */ .byte 0xFF
    /* 8A72A 80099F2A */ .byte 0x00
    /* 8A72B 80099F2B */ .byte 0x00
    /* 8A72C 80099F2C */ .byte 0x20
    /* 8A72D 80099F2D */ .byte 0x80
    /* 8A72E 80099F2E */ .byte 0x00
    /* 8A72F 80099F2F */ .byte 0x40
    /* 8A730 80099F30 */ .byte 0x80
    /* 8A731 80099F31 */ .byte 0xC0
    /* 8A732 80099F32 */ .byte 0x80
    /* 8A733 80099F33 */ .byte 0x40
    /* 8A734 80099F34 */ .byte 0x40
    /* 8A735 80099F35 */ .byte 0x40
    /* 8A736 80099F36 */ .byte 0xC0
    /* 8A737 80099F37 */ .byte 0x00
    /* 8A738 80099F38 */ .byte 0x27
    /* 8A739 80099F39 */ .byte 0x00
    /* 8A73A 80099F3A */ .byte 0x01
    /* 8A73B 80099F3B */ .byte 0x02
    /* 8A73C 80099F3C */ .byte 0xFF
    /* 8A73D 80099F3D */ .byte 0xC0
    /* 8A73E 80099F3E */ .byte 0x18
    /* 8A73F 80099F3F */ .byte 0x80
    /* 8A740 80099F40 */ .byte 0x80
    /* 8A741 80099F41 */ .byte 0x80
    /* 8A742 80099F42 */ .byte 0x00
    /* 8A743 80099F43 */ .byte 0x00
    /* 8A744 80099F44 */ .byte 0x20
    /* 8A745 80099F45 */ .byte 0x80
    /* 8A746 80099F46 */ .byte 0x00
    /* 8A747 80099F47 */ .byte 0x40
    /* 8A748 80099F48 */ .byte 0xC0
    /* 8A749 80099F49 */ .byte 0x80
    /* 8A74A 80099F4A */ .byte 0x40
    /* 8A74B 80099F4B */ .byte 0x40
    /* 8A74C 80099F4C */ .byte 0xC0
    /* 8A74D 80099F4D */ .byte 0xC0
    /* 8A74E 80099F4E */ .byte 0xC0
    /* 8A74F 80099F4F */ .byte 0x00
    /* 8A750 80099F50 */ .byte 0x13
    /* 8A751 80099F51 */ .byte 0x00
    /* 8A752 80099F52 */ .byte 0x04
    /* 8A753 80099F53 */ .byte 0x03
    /* 8A754 80099F54 */ .byte 0x20
    /* 8A755 80099F55 */ .byte 0x80
    /* 8A756 80099F56 */ .byte 0x18
    /* 8A757 80099F57 */ .byte 0xFF
    /* 8A758 80099F58 */ .byte 0x20
    /* 8A759 80099F59 */ .byte 0xFF
    /* 8A75A 80099F5A */ .byte 0x80
    /* 8A75B 80099F5B */ .byte 0x80
    /* 8A75C 80099F5C */ .byte 0x20
    /* 8A75D 80099F5D */ .byte 0x00
    /* 8A75E 80099F5E */ .byte 0x00
    /* 8A75F 80099F5F */ .byte 0x40
    /* 8A760 80099F60 */ .byte 0x40
    /* 8A761 80099F61 */ .byte 0x80
    /* 8A762 80099F62 */ .byte 0xC0
    /* 8A763 80099F63 */ .byte 0x40
    /* 8A764 80099F64 */ .byte 0x80
    /* 8A765 80099F65 */ .byte 0x80
    /* 8A766 80099F66 */ .byte 0x80
    /* 8A767 80099F67 */ .byte 0x00
    /* 8A768 80099F68 */ .byte 0xE7
    /* 8A769 80099F69 */ .byte 0x00
    /* 8A76A 80099F6A */ .byte 0x00
    /* 8A76B 80099F6B */ .byte 0x01
    /* 8A76C 80099F6C */ .byte 0x20
    /* 8A76D 80099F6D */ .byte 0xC0
    /* 8A76E 80099F6E */ .byte 0x20
    /* 8A76F 80099F6F */ .byte 0x30
    /* 8A770 80099F70 */ .byte 0x00
    /* 8A771 80099F71 */ .byte 0x00
    /* 8A772 80099F72 */ .byte 0x80
    /* 8A773 80099F73 */ .byte 0xFF
    /* 8A774 80099F74 */ .byte 0x80
    /* 8A775 80099F75 */ .byte 0x00
    /* 8A776 80099F76 */ .byte 0x00
    /* 8A777 80099F77 */ .byte 0xC0
    /* 8A778 80099F78 */ .byte 0x80
    /* 8A779 80099F79 */ .byte 0xC0
    /* 8A77A 80099F7A */ .byte 0x40
    /* 8A77B 80099F7B */ .byte 0x40
    /* 8A77C 80099F7C */ .byte 0xC0
    /* 8A77D 80099F7D */ .byte 0xC0
    /* 8A77E 80099F7E */ .byte 0xC0
    /* 8A77F 80099F7F */ .byte 0x00
    /* 8A780 80099F80 */ .byte 0x01
    /* 8A781 80099F81 */ .byte 0x00
    /* 8A782 80099F82 */ .byte 0x00
    /* 8A783 80099F83 */ .byte 0x02
    /* 8A784 80099F84 */ .byte 0x00
    /* 8A785 80099F85 */ .byte 0x40
    /* 8A786 80099F86 */ .byte 0x04
    /* 8A787 80099F87 */ .byte 0xFF
    /* 8A788 80099F88 */ .byte 0x20
    /* 8A789 80099F89 */ .byte 0xFF
    /* 8A78A 80099F8A */ .byte 0x80
    /* 8A78B 80099F8B */ .byte 0x80
    /* 8A78C 80099F8C */ .byte 0x20
    /* 8A78D 80099F8D */ .byte 0xFF
    /* 8A78E 80099F8E */ .byte 0x00
    /* 8A78F 80099F8F */ .byte 0xFF
    /* 8A790 80099F90 */ .byte 0xC0
    /* 8A791 80099F91 */ .byte 0xC0
    /* 8A792 80099F92 */ .byte 0x40
    /* 8A793 80099F93 */ .byte 0x40
    /* 8A794 80099F94 */ .byte 0x80
    /* 8A795 80099F95 */ .byte 0xC0
    /* 8A796 80099F96 */ .byte 0x40
    /* 8A797 80099F97 */ .byte 0x00
    /* 8A798 80099F98 */ .byte 0x01
    /* 8A799 80099F99 */ .byte 0x00
    /* 8A79A 80099F9A */ .byte 0x00
    /* 8A79B 80099F9B */ .byte 0x04
    /* 8A79C 80099F9C */ .byte 0x00
    /* 8A79D 80099F9D */ .byte 0x40
    /* 8A79E 80099F9E */ .byte 0x02
    /* 8A79F 80099F9F */ .byte 0x80
    /* 8A7A0 80099FA0 */ .byte 0x80
    /* 8A7A1 80099FA1 */ .byte 0x80
    /* 8A7A2 80099FA2 */ .byte 0x80
    /* 8A7A3 80099FA3 */ .byte 0x80
    /* 8A7A4 80099FA4 */ .byte 0x00
    /* 8A7A5 80099FA5 */ .byte 0x80
    /* 8A7A6 80099FA6 */ .byte 0x00
    /* 8A7A7 80099FA7 */ .byte 0x80
    /* 8A7A8 80099FA8 */ .byte 0xC0
    /* 8A7A9 80099FA9 */ .byte 0xC0
    /* 8A7AA 80099FAA */ .byte 0x80
    /* 8A7AB 80099FAB */ .byte 0x80
    /* 8A7AC 80099FAC */ .byte 0x80
    /* 8A7AD 80099FAD */ .byte 0x40
    /* 8A7AE 80099FAE */ .byte 0x40
    /* 8A7AF 80099FAF */ .byte 0x00
    /* 8A7B0 80099FB0 */ .byte 0xC1
    /* 8A7B1 80099FB1 */ .byte 0x02
    /* 8A7B2 80099FB2 */ .byte 0x09
    /* 8A7B3 80099FB3 */ .byte 0x00
    /* 8A7B4 80099FB4 */ .byte 0x00
    /* 8A7B5 80099FB5 */ .byte 0x80
    /* 8A7B6 80099FB6 */ .byte 0x20
    /* 8A7B7 80099FB7 */ .byte 0x30
    /* 8A7B8 80099FB8 */ .byte 0x80
    /* 8A7B9 80099FB9 */ .byte 0x40
    /* 8A7BA 80099FBA */ .byte 0x00
    /* 8A7BB 80099FBB */ .byte 0x00
    /* 8A7BC 80099FBC */ .byte 0xFF
    /* 8A7BD 80099FBD */ .byte 0x00
    /* 8A7BE 80099FBE */ .byte 0x00
    /* 8A7BF 80099FBF */ .byte 0x00
    /* 8A7C0 80099FC0 */ .byte 0xFF
    /* 8A7C1 80099FC1 */ .byte 0xFF
    /* 8A7C2 80099FC2 */ .byte 0xFF
    /* 8A7C3 80099FC3 */ .byte 0xFF
    /* 8A7C4 80099FC4 */ .byte 0x80
    /* 8A7C5 80099FC5 */ .byte 0x30
    /* 8A7C6 80099FC6 */ .byte 0xFF
    /* 8A7C7 80099FC7 */ .byte 0x00
    /* 8A7C8 80099FC8 */ .byte 0x00
    /* 8A7C9 80099FC9 */ .byte 0x00
    /* 8A7CA 80099FCA */ .byte 0x09
    /* 8A7CB 80099FCB */ .byte 0x01
    /* 8A7CC 80099FCC */ .byte 0xFF
    /* 8A7CD 80099FCD */ .byte 0x80
    /* 8A7CE 80099FCE */ .byte 0x30
    /* 8A7CF 80099FCF */ .byte 0x10
    /* 8A7D0 80099FD0 */ .byte 0x00
    /* 8A7D1 80099FD1 */ .byte 0x00
    /* 8A7D2 80099FD2 */ .byte 0xFF
    /* 8A7D3 80099FD3 */ .byte 0xFF
    /* 8A7D4 80099FD4 */ .byte 0xFF
    /* 8A7D5 80099FD5 */ .byte 0x00
    /* 8A7D6 80099FD6 */ .byte 0x00
    /* 8A7D7 80099FD7 */ .byte 0x00
    /* 8A7D8 80099FD8 */ .byte 0xFF
    /* 8A7D9 80099FD9 */ .byte 0xFF
    /* 8A7DA 80099FDA */ .byte 0xFF
    /* 8A7DB 80099FDB */ .byte 0xFF
    /* 8A7DC 80099FDC */ .byte 0xFF
    /* 8A7DD 80099FDD */ .byte 0xFF
    /* 8A7DE 80099FDE */ .byte 0xFF
    /* 8A7DF 80099FDF */ .byte 0x00
    /* 8A7E0 80099FE0 */ .byte 0x80
    /* 8A7E1 80099FE1 */ .byte 0x80
    /* 8A7E2 80099FE2 */ .byte 0x09
    /* 8A7E3 80099FE3 */ .byte 0xFF
    /* 8A7E4 80099FE4 */ .byte 0x00
    /* 8A7E5 80099FE5 */ .byte 0x80
    /* 8A7E6 80099FE6 */ .byte 0x00
    /* 8A7E7 80099FE7 */ .byte 0x00
    /* 8A7E8 80099FE8 */ .byte 0x00
    /* 8A7E9 80099FE9 */ .byte 0x00
    /* 8A7EA 80099FEA */ .byte 0x00
    /* 8A7EB 80099FEB */ .byte 0x00
    /* 8A7EC 80099FEC */ .byte 0x00
    /* 8A7ED 80099FED */ .byte 0x00
    /* 8A7EE 80099FEE */ .byte 0x00
    /* 8A7EF 80099FEF */ .byte 0x00
    /* 8A7F0 80099FF0 */ .byte 0xFF
    /* 8A7F1 80099FF1 */ .byte 0xFF
    /* 8A7F2 80099FF2 */ .byte 0xFF
    /* 8A7F3 80099FF3 */ .byte 0xFF
    /* 8A7F4 80099FF4 */ .byte 0x00
    /* 8A7F5 80099FF5 */ .byte 0x00
    /* 8A7F6 80099FF6 */ .byte 0xFF
    /* 8A7F7 80099FF7 */ .byte 0x00
    /* 8A7F8 80099FF8 */ .byte 0x00
    /* 8A7F9 80099FF9 */ .byte 0x00
    /* 8A7FA 80099FFA */ .byte 0x09
    /* 8A7FB 80099FFB */ .byte 0x01
    /* 8A7FC 80099FFC */ .byte 0xFF
    /* 8A7FD 80099FFD */ .byte 0x80
    /* 8A7FE 80099FFE */ .byte 0x30
    /* 8A7FF 80099FFF */ .byte 0x10
    /* 8A800 8009A000 */ .byte 0x00
    /* 8A801 8009A001 */ .byte 0x00
    /* 8A802 8009A002 */ .byte 0xFF
    /* 8A803 8009A003 */ .byte 0xFF
    /* 8A804 8009A004 */ .byte 0xFF
    /* 8A805 8009A005 */ .byte 0x00
    /* 8A806 8009A006 */ .byte 0x00
    /* 8A807 8009A007 */ .byte 0x00
    /* 8A808 8009A008 */ .byte 0xFF
    /* 8A809 8009A009 */ .byte 0xFF
    /* 8A80A 8009A00A */ .byte 0xFF
    /* 8A80B 8009A00B */ .byte 0xFF
    /* 8A80C 8009A00C */ .byte 0xFF
    /* 8A80D 8009A00D */ .byte 0xFF
    /* 8A80E 8009A00E */ .byte 0xFF
    /* 8A80F 8009A00F */ .byte 0x00
    /* 8A810 8009A010 */ .byte 0x81
    /* 8A811 8009A011 */ .byte 0x04
    /* 8A812 8009A012 */ .byte 0x09
    /* 8A813 8009A013 */ .byte 0x01
    /* 8A814 8009A014 */ .byte 0x00
    /* 8A815 8009A015 */ .byte 0x80
    /* 8A816 8009A016 */ .byte 0x18
    /* 8A817 8009A017 */ .byte 0x80
    /* 8A818 8009A018 */ .byte 0xFF
    /* 8A819 8009A019 */ .byte 0x00
    /* 8A81A 8009A01A */ .byte 0x00
    /* 8A81B 8009A01B */ .byte 0x00
    /* 8A81C 8009A01C */ .byte 0x00
    /* 8A81D 8009A01D */ .byte 0x00
    /* 8A81E 8009A01E */ .byte 0x00
    /* 8A81F 8009A01F */ .byte 0x00
    /* 8A820 8009A020 */ .byte 0xFF
    /* 8A821 8009A021 */ .byte 0xFF
    /* 8A822 8009A022 */ .byte 0xFF
    /* 8A823 8009A023 */ .byte 0x00
    /* 8A824 8009A024 */ .byte 0x00
    /* 8A825 8009A025 */ .byte 0x00
    /* 8A826 8009A026 */ .byte 0xFF
    /* 8A827 8009A027 */ .byte 0x00
    /* 8A828 8009A028 */ .byte 0x81
    /* 8A829 8009A029 */ .byte 0x08
    /* 8A82A 8009A02A */ .byte 0x09
    /* 8A82B 8009A02B */ .byte 0x01
    /* 8A82C 8009A02C */ .byte 0x00
    /* 8A82D 8009A02D */ .byte 0x80
    /* 8A82E 8009A02E */ .byte 0x10
    /* 8A82F 8009A02F */ .byte 0xFF
    /* 8A830 8009A030 */ .byte 0xFF
    /* 8A831 8009A031 */ .byte 0x00
    /* 8A832 8009A032 */ .byte 0x80
    /* 8A833 8009A033 */ .byte 0x00
    /* 8A834 8009A034 */ .byte 0x40
    /* 8A835 8009A035 */ .byte 0x00
    /* 8A836 8009A036 */ .byte 0x00
    /* 8A837 8009A037 */ .byte 0x00
    /* 8A838 8009A038 */ .byte 0xFF
    /* 8A839 8009A039 */ .byte 0xFF
    /* 8A83A 8009A03A */ .byte 0xFF
    /* 8A83B 8009A03B */ .byte 0x00
    /* 8A83C 8009A03C */ .byte 0x00
    /* 8A83D 8009A03D */ .byte 0x00
    /* 8A83E 8009A03E */ .byte 0xFF
    /* 8A83F 8009A03F */ .byte 0x00
    /* 8A840 8009A040 */ .byte 0x81
    /* 8A841 8009A041 */ .byte 0x10
    /* 8A842 8009A042 */ .byte 0x09
    /* 8A843 8009A043 */ .byte 0x01
    /* 8A844 8009A044 */ .byte 0xFF
    /* 8A845 8009A045 */ .byte 0x80
    /* 8A846 8009A046 */ .byte 0x08
    /* 8A847 8009A047 */ .byte 0x40
    /* 8A848 8009A048 */ .byte 0xFF
    /* 8A849 8009A049 */ .byte 0xFF
    /* 8A84A 8009A04A */ .byte 0x00
    /* 8A84B 8009A04B */ .byte 0x00
    /* 8A84C 8009A04C */ .byte 0x40
    /* 8A84D 8009A04D */ .byte 0x80
    /* 8A84E 8009A04E */ .byte 0x00
    /* 8A84F 8009A04F */ .byte 0x00
    /* 8A850 8009A050 */ .byte 0xFF
    /* 8A851 8009A051 */ .byte 0xFF
    /* 8A852 8009A052 */ .byte 0xFF
    /* 8A853 8009A053 */ .byte 0x00
    /* 8A854 8009A054 */ .byte 0x00
    /* 8A855 8009A055 */ .byte 0x00
    /* 8A856 8009A056 */ .byte 0xFF
    /* 8A857 8009A057 */ .byte 0x00
    /* 8A858 8009A058 */ .byte 0x81
    /* 8A859 8009A059 */ .byte 0x20
    /* 8A85A 8009A05A */ .byte 0x09
    /* 8A85B 8009A05B */ .byte 0x01
    /* 8A85C 8009A05C */ .byte 0x00
    /* 8A85D 8009A05D */ .byte 0x80
    /* 8A85E 8009A05E */ .byte 0x10
    /* 8A85F 8009A05F */ .byte 0xFF
    /* 8A860 8009A060 */ .byte 0x80
    /* 8A861 8009A061 */ .byte 0xFF
    /* 8A862 8009A062 */ .byte 0x00
    /* 8A863 8009A063 */ .byte 0x80
    /* 8A864 8009A064 */ .byte 0x00
    /* 8A865 8009A065 */ .byte 0x80
    /* 8A866 8009A066 */ .byte 0x00
    /* 8A867 8009A067 */ .byte 0x00
    /* 8A868 8009A068 */ .byte 0xFF
    /* 8A869 8009A069 */ .byte 0xFF
    /* 8A86A 8009A06A */ .byte 0xFF
    /* 8A86B 8009A06B */ .byte 0x00
    /* 8A86C 8009A06C */ .byte 0x00
    /* 8A86D 8009A06D */ .byte 0x00
    /* 8A86E 8009A06E */ .byte 0xFF
    /* 8A86F 8009A06F */ .byte 0x00
    /* 8A870 8009A070 */ .byte 0x83
    /* 8A871 8009A071 */ .byte 0x40
    /* 8A872 8009A072 */ .byte 0x09
    /* 8A873 8009A073 */ .byte 0x01
    /* 8A874 8009A074 */ .byte 0xFF
    /* 8A875 8009A075 */ .byte 0x80
    /* 8A876 8009A076 */ .byte 0x30
    /* 8A877 8009A077 */ .byte 0x10
    /* 8A878 8009A078 */ .byte 0x00
    /* 8A879 8009A079 */ .byte 0x00
    /* 8A87A 8009A07A */ .byte 0xFF
    /* 8A87B 8009A07B */ .byte 0xFF
    /* 8A87C 8009A07C */ .byte 0xFF
    /* 8A87D 8009A07D */ .byte 0x00
    /* 8A87E 8009A07E */ .byte 0x00
    /* 8A87F 8009A07F */ .byte 0xFF
    /* 8A880 8009A080 */ .byte 0xFF
    /* 8A881 8009A081 */ .byte 0xFF
    /* 8A882 8009A082 */ .byte 0xFF
    /* 8A883 8009A083 */ .byte 0xFF
    /* 8A884 8009A084 */ .byte 0xFF
    /* 8A885 8009A085 */ .byte 0xFF
    /* 8A886 8009A086 */ .byte 0xFF
    /* 8A887 8009A087 */ .byte 0x00
enddlabel D_80099D9D

nonmatching D_8009A088

dlabel D_8009A088
    /* 8A888 8009A088 */ .byte 0x0A
    /* 8A889 8009A089 */ .byte 0x02
    /* 8A88A 8009A08A */ .byte 0x02
    /* 8A88B 8009A08B */ .byte 0x02
    /* 8A88C 8009A08C */ .byte 0x04
    /* 8A88D 8009A08D */ .byte 0x07
    /* 8A88E 8009A08E */ .byte 0x00
    /* 8A88F 8009A08F */ .byte 0x00
enddlabel D_8009A088

nonmatching D_8009A090

dlabel D_8009A090
    /* 8A890 8009A090 401FF811 */ .word 0x11F81F40
    /* 8A894 8009A094 6621AE10 */ .word 0x10AE2166
    /* 8A898 8009A098 6621880E */ .word 0x0E882166
    /* 8A89C 8009A09C 401F700D */ .word 0x0D701F40
    /* 8A8A0 8009A0A0 101D880E */ .word 0x0E881D10
    /* 8A8A4 8009A0A4 101DAE10 */ .word 0x10AE1D10
enddlabel D_8009A090

nonmatching D_8009A0A8

dlabel D_8009A0A8
    /* 8A8A8 8009A0A8 401FC2F2 */ .word 0xF2C21F40
    /* 8A8AC 8009A0AC 662178F1 */ .word 0xF1782166
    /* 8A8B0 8009A0B0 662148EF */ .word 0xEF482166
    /* 8A8B4 8009A0B4 401F3AEE */ .word 0xEE3A1F40
    /* 8A8B8 8009A0B8 101D48EF */ .word 0xEF481D10
    /* 8A8BC 8009A0BC 101D78F1 */ .word 0xF1781D10
enddlabel D_8009A0A8

nonmatching D_8009A0C0

dlabel D_8009A0C0
    /* 8A8C0 8009A0C0 F0D83011 */ .word 0x1130D8F0
    /* 8A8C4 8009A0C4 4BDA6810 */ .word 0x1068DA4B
    /* 8A8C8 8009A0C8 4BDAD80E */ .word 0x0ED8DA4B
    /* 8A8CC 8009A0CC F0D8100E */ .word 0x0E10D8F0
enddlabel D_8009A0C0

nonmatching D_8009A0D0

dlabel D_8009A0D0
    /* 8A8D0 8009A0D0 F0D8F0F1 */ .word 0xF1F0D8F0
    /* 8A8D4 8009A0D4 4BDA28F1 */ .word 0xF128DA4B
    /* 8A8D8 8009A0D8 4BDA98EF */ .word 0xEF98DA4B
    /* 8A8DC 8009A0DC F0D8D0EE */ .word 0xEED0D8F0
enddlabel D_8009A0D0

nonmatching D_8009A0E0

dlabel D_8009A0E0
    /* 8A8E0 8009A0E0 00062506 */ .word 0x06250600
    /* 8A8E4 8009A0E4 90A00980 */ .word D_8009A090
    /* 8A8E8 8009A0E8 00062506 */ .word 0x06250600
    /* 8A8EC 8009A0EC A8A00980 */ .word D_8009A0A8
    /* 8A8F0 8009A0F0 80062504 */ .word 0x04250680
    /* 8A8F4 8009A0F4 C0A00980 */ .word D_8009A0C0
    /* 8A8F8 8009A0F8 80062504 */ .word 0x04250680
    /* 8A8FC 8009A0FC D0A00980 */ .word D_8009A0D0
enddlabel D_8009A0E0

nonmatching D_8009A100

dlabel D_8009A100
    /* 8A900 8009A100 60F0B036 */ .word 0x36B0F060
    /* 8A904 8009A104 60F01027 */ .word 0x2710F060
    /* 8A908 8009A108 20D11027 */ .word 0x2710D120
enddlabel D_8009A100

nonmatching D_8009A10C

dlabel D_8009A10C
    /* 8A90C 8009A10C 60F0C409 */ .word 0x09C4F060
    /* 8A910 8009A110 54F2D007 */ .word 0x07D0F254
    /* 8A914 8009A114 60F0DC05 */ .word 0x05DCF060
    /* 8A918 8009A118 6CEED007 */ .word 0x07D0EE6C
enddlabel D_8009A10C

nonmatching D_8009A11C

dlabel D_8009A11C
    /* 8A91C 8009A11C 80032503 */ .word 0x03250380
    /* 8A920 8009A120 00A10980 */ .word D_8009A100
    /* 8A924 8009A124 00062504 */ .word 0x04250600
    /* 8A928 8009A128 0CA10980 */ .word D_8009A10C
enddlabel D_8009A11C

nonmatching D_8009A12C

dlabel D_8009A12C
    /* 8A92C 8009A12C 10AA30F8 */ .word 0xF830AA10
    /* 8A930 8009A130 E0B130F8 */ .word 0xF830B1E0
    /* 8A934 8009A134 80C1E803 */ .word 0x03E8C180
    /* 8A938 8009A138 50C9E803 */ .word 0x03E8C950
    /* 8A93C 8009A13C C0E048F4 */ .word 0xF448E0C0
    /* 8A940 8009A140 90E848F4 */ .word 0xF448E890
    /* 8A944 8009A144 30F878EC */ .word 0xEC78F830
    /* 8A948 8009A148 401F78EC */ .word 0xEC781F40
    /* 8A94C 8009A14C B036D8DC */ .word 0xDCD836B0
    /* 8A950 8009A150 204ED8DC */ .word 0xDCD84E20
enddlabel D_8009A12C

nonmatching D_8009A154

dlabel D_8009A154
    /* 8A954 8009A154 204EE803 */ .word 0x03E84E20
    /* 8A958 8009A158 B036E803 */ .word 0x03E836B0
    /* 8A95C 8009A15C 401F8813 */ .word 0x13881F40
    /* 8A960 8009A160 30F88813 */ .word 0x1388F830
    /* 8A964 8009A164 90E8581B */ .word 0x1B58E890
    /* 8A968 8009A168 C0E0581B */ .word 0x1B58E0C0
    /* 8A96C 8009A16C 50C9F82A */ .word 0x2AF8C950
    /* 8A970 8009A170 80C1F82A */ .word 0x2AF8C180
    /* 8A974 8009A174 E0B1401F */ .word 0x1F40B1E0
    /* 8A978 8009A178 10AA401F */ .word 0x1F40AA10
enddlabel D_8009A154

nonmatching D_8009A17C

dlabel D_8009A17C
    /* 8A97C 8009A17C 8003250A */ .word 0x0A250380
    /* 8A980 8009A180 2CA10980 */ .word D_8009A12C
    /* 8A984 8009A184 8003250A */ .word 0x0A250380
    /* 8A988 8009A188 54A10980 */ .word D_8009A154
enddlabel D_8009A17C

nonmatching D_8009A18C

dlabel D_8009A18C
    /* 8A98C 8009A18C 9E07F609 */ .word 0x09F6079E
    /* 8A990 8009A190 1A04F609 */ .word 0x09F6041A
    /* 8A994 8009A194 1A04D40D */ .word 0x0DD4041A
    /* 8A998 8009A198 9E07D40D */ .word 0x0DD4079E
enddlabel D_8009A18C

nonmatching D_8009A19C

dlabel D_8009A19C
    /* 8A99C 8009A19C E02EC0E0 */ .word 0xE0C02EE0
    /* 8A9A0 8009A1A0 B03690E8 */ .word 0xE89036B0
    /* 8A9A4 8009A1A4 B03630F8 */ .word 0xF83036B0
    /* 8A9A8 8009A1A8 C8320000 */ .word 0x000032C8
    /* 8A9AC 8009A1AC C8327017 */ .word 0x177032C8
    /* 8A9B0 8009A1B0 E02E401F */ .word 0x1F402EE0
    /* 8A9B4 8009A1B4 E02E1027 */ .word 0x27102EE0
enddlabel D_8009A19C

nonmatching D_8009A1B8

dlabel D_8009A1B8
    /* 8A9B8 8009A1B8 C0E0401F */ .word 0x1F40E0C0
    /* 8A9BC 8009A1BC C0E07017 */ .word 0x1770E0C0
    /* 8A9C0 8009A1C0 F0D8A00F */ .word 0x0FA0D8F0
    /* 8A9C4 8009A1C4 F0D80000 */ .word 0x0000D8F0
    /* 8A9C8 8009A1C8 C0E030F8 */ .word 0xF830E0C0
    /* 8A9CC 8009A1CC C0E0C0E0 */ .word 0xE0C0E0C0
enddlabel D_8009A1B8

nonmatching D_8009A1D0

dlabel D_8009A1D0
    /* 8A9D0 8009A1D0 00032504 */ .word 0x04250300
    /* 8A9D4 8009A1D4 8CA10980 */ .word D_8009A18C
    /* 8A9D8 8009A1D8 80032507 */ .word 0x07250380
    /* 8A9DC 8009A1DC 9CA10980 */ .word D_8009A19C
    /* 8A9E0 8009A1E0 80032506 */ .word 0x06250380
    /* 8A9E4 8009A1E4 B8A10980 */ .word D_8009A1B8
enddlabel D_8009A1D0

nonmatching D_8009A1E8

dlabel D_8009A1E8
    /* 8A9E8 8009A1E8 A00FE0B1 */ .word 0xB1E00FA0
    /* 8A9EC 8009A1EC A00F60F0 */ .word 0xF0600FA0
    /* 8A9F0 8009A1F0 E02E60F0 */ .word 0xF0602EE0
enddlabel D_8009A1E8

nonmatching D_8009A1F4

dlabel D_8009A1F4
    /* 8A9F4 8009A1F4 50C960F0 */ .word 0xF060C950
    /* 8A9F8 8009A1F8 90E860F0 */ .word 0xF060E890
    /* 8A9FC 8009A1FC 90E8E0B1 */ .word 0xB1E0E890
enddlabel D_8009A1F4

nonmatching D_8009A200

dlabel D_8009A200
    /* 8AA00 8009A200 151CD52E */ .word 0x2ED51C15
    /* 8AA04 8009A204 151C7A17 */ .word 0x177A1C15
    /* 8AA08 8009A208 9A1A7A17 */ .word 0x177A1A9A
    /* 8AA0C 8009A20C 9A1AD52E */ .word 0x2ED51A9A
enddlabel D_8009A200

nonmatching D_8009A210

dlabel D_8009A210
    /* 8AA10 8009A210 B80B204E */ .word 0x4E200BB8
    /* 8AA14 8009A214 B80B6842 */ .word 0x42680BB8
    /* 8AA18 8009A218 78EC6842 */ .word 0x4268EC78
    /* 8AA1C 8009A21C 78EC204E */ .word 0x4E20EC78
enddlabel D_8009A210

nonmatching D_8009A220

dlabel D_8009A220
    /* 8AA20 8009A220 80032503 */ .word 0x03250380
    /* 8AA24 8009A224 E8A10980 */ .word D_8009A1E8
    /* 8AA28 8009A228 80032503 */ .word 0x03250380
    /* 8AA2C 8009A22C F4A10980 */ .word D_8009A1F4
    /* 8AA30 8009A230 40032504 */ .word 0x04250340
    /* 8AA34 8009A234 00A20980 */ .word D_8009A200
    /* 8AA38 8009A238 80032504 */ .word 0x04250380
    /* 8AA3C 8009A23C 10A20980 */ .word D_8009A210
enddlabel D_8009A220

nonmatching D_8009A240

dlabel D_8009A240
    /* 8AA40 8009A240 5046E0B1 */ .word 0xB1E04650
    /* 8AA44 8009A244 F0550000 */ .word 0x000055F0
    /* 8AA48 8009A248 3075401F */ .word 0x1F407530
enddlabel D_8009A240

nonmatching D_8009A24C

dlabel D_8009A24C
    /* 8AA4C 8009A24C A00FC05D */ .word 0x5DC00FA0
    /* 8AA50 8009A250 C0E0E02E */ .word 0x2EE0E0C0
    /* 8AA54 8009A254 80C1E02E */ .word 0x2EE0C180
enddlabel D_8009A24C

nonmatching D_8009A258

dlabel D_8009A258
    /* 8AA58 8009A258 4C1D8C3C */ .word 0x3C8C1D4C
    /* 8AA5C 8009A25C 4C1DD430 */ .word 0x30D41D4C
    /* 8AA60 8009A260 9411D430 */ .word 0x30D41194
    /* 8AA64 8009A264 94118C3C */ .word 0x3C8C1194
enddlabel D_8009A258

nonmatching D_8009A268

dlabel D_8009A268
    /* 8AA68 8009A268 80032503 */ .word 0x03250380
    /* 8AA6C 8009A26C 40A20980 */ .word D_8009A240
    /* 8AA70 8009A270 80032503 */ .word 0x03250380
    /* 8AA74 8009A274 4CA20980 */ .word D_8009A24C
    /* 8AA78 8009A278 00032504 */ .word 0x04250300
    /* 8AA7C 8009A27C 58A20980 */ .word D_8009A258
enddlabel D_8009A268

nonmatching D_8009A280

dlabel D_8009A280
    /* 8AA80 8009A280 4F06A9C0 */ .word 0xC0A9064F
    /* 8AA84 8009A284 F8F034E0 */ .word 0xE034F0F8
    /* 8AA88 8009A288 0000EFED */ .word 0xEDEF0000
    /* 8AA8C 8009A28C D80EEFED */ .word 0xEDEF0ED8
    /* 8AA90 8009A290 D80E84C2 */ .word 0xC2840ED8
enddlabel D_8009A280

nonmatching D_8009A294

dlabel D_8009A294
    /* 8AA94 8009A294 D80E893F */ .word 0x3F890ED8
    /* 8AA98 8009A298 D80E5816 */ .word 0x16580ED8
    /* 8AA9C 8009A29C 60F0E218 */ .word 0x18E2F060
    /* 8AAA0 8009A2A0 8FEE3822 */ .word 0x2238EE8F
    /* 8AAA4 8009A2A4 D3046A3E */ .word 0x3E6A04D3
enddlabel D_8009A294

nonmatching D_8009A2A8

dlabel D_8009A2A8
    /* 8AAA8 8009A2A8 80032505 */ .word 0x05250380
    /* 8AAAC 8009A2AC 80A20980 */ .word D_8009A280
    /* 8AAB0 8009A2B0 80032505 */ .word 0x05250380
    /* 8AAB4 8009A2B4 94A20980 */ .word D_8009A294
enddlabel D_8009A2A8

nonmatching D_8009A2B8

dlabel D_8009A2B8
    /* 8AAB8 8009A2B8 803EA00F */ .word 0x0FA03E80
    /* 8AABC 8009A2BC 10271027 */ .word 0x27102710
    /* 8AAC0 8009A2C0 1027E02E */ .word 0x2EE02710
enddlabel D_8009A2B8

nonmatching D_8009A2C4

dlabel D_8009A2C4
    /* 8AAC4 8009A2C4 C832C0E0 */ .word 0xE0C032C8
    /* 8AAC8 8009A2C8 C83290E8 */ .word 0xE89032C8
    /* 8AACC 8009A2CC 803E30F8 */ .word 0xF8303E80
enddlabel D_8009A2C4

nonmatching D_8009A2D0

dlabel D_8009A2D0
    /* 8AAD0 8009A2D0 80062503 */ .word 0x03250680
    /* 8AAD4 8009A2D4 B8A20980 */ .word D_8009A2B8
    /* 8AAD8 8009A2D8 80062503 */ .word 0x03250680
    /* 8AADC 8009A2DC C4A20980 */ .word D_8009A2C4
enddlabel D_8009A2D0

nonmatching D_8009A2E0

dlabel D_8009A2E0
    /* 8AAE0 8009A2E0 530FA13F */ .word 0x3FA10F53
    /* 8AAE4 8009A2E4 C110313E */ .word 0x3E3110C1
    /* 8AAE8 8009A2E8 CB0EAC3D */ .word 0x3DAC0ECB
enddlabel D_8009A2E0

nonmatching D_8009A2EC

dlabel D_8009A2EC
    /* 8AAEC 8009A2EC 14F0B93B */ .word 0x3BB9F014
    /* 8AAF0 8009A2F0 82F1493A */ .word 0x3A49F182
    /* 8AAF4 8009A2F4 8CEFC439 */ .word 0x39C4EF8C
enddlabel D_8009A2EC

nonmatching D_8009A2F8

dlabel D_8009A2F8
    /* 8AAF8 8009A2F8 23179118 */ .word 0x18911723
    /* 8AAFC 8009A2FC 91182117 */ .word 0x17211891
    /* 8AB00 8009A300 9B169C16 */ .word 0x169C169B
enddlabel D_8009A2F8

nonmatching D_8009A304

dlabel D_8009A304
    /* 8AB04 8009A304 E4F79118 */ .word 0x1891F7E4
    /* 8AB08 8009A308 52F92117 */ .word 0x1721F952
    /* 8AB0C 8009A30C 5CF79C16 */ .word 0x169CF75C
enddlabel D_8009A304

nonmatching D_8009A310

dlabel D_8009A310
    /* 8AB10 8009A310 E4F752F9 */ .word 0xF952F7E4
    /* 8AB14 8009A314 52F9E2F7 */ .word 0xF7E2F952
    /* 8AB18 8009A318 5CF75DF7 */ .word 0xF75DF75C
enddlabel D_8009A310

nonmatching D_8009A31C

dlabel D_8009A31C
    /* 8AB1C 8009A31C 00062503 */ .word 0x03250600
    /* 8AB20 8009A320 E0A20980 */ .word D_8009A2E0
    /* 8AB24 8009A324 00062503 */ .word 0x03250600
    /* 8AB28 8009A328 ECA20980 */ .word D_8009A2EC
    /* 8AB2C 8009A32C 00062503 */ .word 0x03250600
    /* 8AB30 8009A330 F8A20980 */ .word D_8009A2F8
    /* 8AB34 8009A334 00062503 */ .word 0x03250600
    /* 8AB38 8009A338 04A30980 */ .word D_8009A304
    /* 8AB3C 8009A33C 00062503 */ .word 0x03250600
    /* 8AB40 8009A340 10A30980 */ .word D_8009A310
enddlabel D_8009A31C

nonmatching D_8009A344

dlabel D_8009A344
    /* 8AB44 8009A344 4C180604 */ .word 0x0406184C
    /* 8AB48 8009A348 4C188601 */ .word 0x0186184C
    /* 8AB4C 8009A34C D6068601 */ .word 0x018606D6
    /* 8AB50 8009A350 D6060604 */ .word 0x040606D6
enddlabel D_8009A344

nonmatching D_8009A354

dlabel D_8009A354
    /* 8AB54 8009A354 36F7C0E0 */ .word 0xE0C0F736
    /* 8AB58 8009A358 36F734DB */ .word 0xDB34F736
    /* 8AB5C 8009A35C C0E534DB */ .word 0xDB34E5C0
    /* 8AB60 8009A360 C0E5C0E0 */ .word 0xE0C0E5C0
enddlabel D_8009A354

nonmatching D_8009A364

dlabel D_8009A364
    /* 8AB64 8009A364 00032504 */ .word 0x04250300
    /* 8AB68 8009A368 44A30980 */ .word D_8009A344
    /* 8AB6C 8009A36C 00032504 */ .word 0x04250300
    /* 8AB70 8009A370 54A30980 */ .word D_8009A354
enddlabel D_8009A364

nonmatching D_8009A374

dlabel D_8009A374
    /* 8AB74 8009A374 204E60F0 */ .word 0xF0604E20
    /* 8AB78 8009A378 803E60F0 */ .word 0xF0603E80
    /* 8AB7C 8009A37C 803EA00F */ .word 0x0FA03E80
    /* 8AB80 8009A380 204EA00F */ .word 0x0FA04E20
enddlabel D_8009A374

nonmatching D_8009A384

dlabel D_8009A384
    /* 8AB84 8009A384 80032504 */ .word 0x04250380
    /* 8AB88 8009A388 74A30980 */ .word D_8009A374
enddlabel D_8009A384

nonmatching D_8009A38C

dlabel D_8009A38C
    /* 8AB8C 8009A38C 90E87017 */ .word 0x1770E890
    /* 8AB90 8009A390 90E8A00F */ .word 0x0FA0E890
    /* 8AB94 8009A394 C0E0A00F */ .word 0x0FA0E0C0
enddlabel D_8009A38C

nonmatching D_8009A398

dlabel D_8009A398
    /* 8AB98 8009A398 C0E030F8 */ .word 0xF830E0C0
    /* 8AB9C 8009A39C 90E830F8 */ .word 0xF830E890
    /* 8ABA0 8009A3A0 90E890E8 */ .word 0xE890E890
    /* 8ABA4 8009A3A4 C0E090E8 */ .word 0xE890E0C0
enddlabel D_8009A398

nonmatching D_8009A3A8

dlabel D_8009A3A8
    /* 8ABA8 8009A3A8 401F90E8 */ .word 0xE8901F40
    /* 8ABAC 8009A3AC 701790E8 */ .word 0xE8901770
    /* 8ABB0 8009A3B0 701730F8 */ .word 0xF8301770
    /* 8ABB4 8009A3B4 401F30F8 */ .word 0xF8301F40
enddlabel D_8009A3A8

nonmatching D_8009A3B8

dlabel D_8009A3B8
    /* 8ABB8 8009A3B8 80032503 */ .word 0x03250380
    /* 8ABBC 8009A3BC 8CA30980 */ .word D_8009A38C
    /* 8ABC0 8009A3C0 80032504 */ .word 0x04250380
    /* 8ABC4 8009A3C4 98A30980 */ .word D_8009A398
    /* 8ABC8 8009A3C8 80032504 */ .word 0x04250380
    /* 8ABCC 8009A3CC A8A30980 */ .word D_8009A3A8
enddlabel D_8009A3B8

nonmatching D_8009A3D0

dlabel D_8009A3D0
    /* 8ABD0 8009A3D0 0000B036 */ .word 0x36B00000
    /* 8ABD4 8009A3D4 0000E02E */ .word 0x2EE00000
    /* 8ABD8 8009A3D8 C0E0A00F */ .word 0x0FA0E0C0
    /* 8ABDC 8009A3DC 20D1A00F */ .word 0x0FA0D120
enddlabel D_8009A3D0

nonmatching D_8009A3E0

dlabel D_8009A3E0
    /* 8ABE0 8009A3E0 80032504 */ .word 0x04250380
    /* 8ABE4 8009A3E4 D0A30980 */ .word D_8009A3D0
enddlabel D_8009A3E0

nonmatching D_8009A3E8

dlabel D_8009A3E8
    /* 8ABE8 8009A3E8 78ECD8DC */ .word 0xDCD8EC78
    /* 8ABEC 8009A3EC 78ECACF4 */ .word 0xF4ACEC78
    /* 8ABF0 8009A3F0 A00FACF4 */ .word 0xF4AC0FA0
enddlabel D_8009A3E8

nonmatching D_8009A3F4

dlabel D_8009A3F4
    /* 8ABF4 8009A3F4 80032503 */ .word 0x03250380
    /* 8ABF8 8009A3F8 E8A30980 */ .word D_8009A3E8
enddlabel D_8009A3F4

nonmatching D_8009A3FC

dlabel D_8009A3FC
    /* 8ABFC 8009A3FC 84EABCEE */ .word 0xEEBCEA84
    /* 8AC00 8009A400 F4EDBCEE */ .word 0xEEBCEDF4
    /* 8AC04 8009A404 F4ED20EA */ .word 0xEA20EDF4
    /* 8AC08 8009A408 84EA20EA */ .word 0xEA20EA84
enddlabel D_8009A3FC

nonmatching D_8009A40C

dlabel D_8009A40C
    /* 8AC0C 8009A40C 7C1520EA */ .word 0xEA20157C
    /* 8AC10 8009A410 0C1220EA */ .word 0xEA20120C
    /* 8AC14 8009A414 0C12BCEE */ .word 0xEEBC120C
    /* 8AC18 8009A418 7C15BCEE */ .word 0xEEBC157C
enddlabel D_8009A40C

nonmatching D_8009A41C

dlabel D_8009A41C
    /* 8AC1C 8009A41C 7C158CD8 */ .word 0xD88C157C
    /* 8AC20 8009A420 0C128CD8 */ .word 0xD88C120C
    /* 8AC24 8009A424 0C1232DD */ .word 0xDD32120C
    /* 8AC28 8009A428 7C1532DD */ .word 0xDD32157C
enddlabel D_8009A41C

nonmatching D_8009A42C

dlabel D_8009A42C
    /* 8AC2C 8009A42C 84EA32DD */ .word 0xDD32EA84
    /* 8AC30 8009A430 F4ED32DD */ .word 0xDD32EDF4
    /* 8AC34 8009A434 F4ED8CD8 */ .word 0xD88CEDF4
    /* 8AC38 8009A438 84EA8CD8 */ .word 0xD88CEA84
enddlabel D_8009A42C

nonmatching D_8009A43C

dlabel D_8009A43C
    /* 8AC3C 8009A43C C0032504 */ .word 0x042503C0
    /* 8AC40 8009A440 FCA30980 */ .word D_8009A3FC
    /* 8AC44 8009A444 C0032504 */ .word 0x042503C0
    /* 8AC48 8009A448 0CA40980 */ .word D_8009A40C
    /* 8AC4C 8009A44C C0032504 */ .word 0x042503C0
    /* 8AC50 8009A450 1CA40980 */ .word D_8009A41C
    /* 8AC54 8009A454 C0032504 */ .word 0x042503C0
    /* 8AC58 8009A458 2CA40980 */ .word D_8009A42C
enddlabel D_8009A43C

nonmatching D_8009A45C

dlabel D_8009A45C
    /* 8AC5C 8009A45C 94116203 */ .word 0x03621194
    /* 8AC60 8009A460 88130000 */ .word 0x00001388
    /* 8AC64 8009A464 94119EFC */ .word 0xFC9E1194
    /* 8AC68 8009A468 AC0D9EFC */ .word 0xFC9E0DAC
    /* 8AC6C 8009A46C B80B0000 */ .word 0x00000BB8
    /* 8AC70 8009A470 AC0D6203 */ .word 0x03620DAC
enddlabel D_8009A45C

nonmatching D_8009A474

dlabel D_8009A474
    /* 8AC74 8009A474 84EA6203 */ .word 0x0362EA84
    /* 8AC78 8009A478 78EC0000 */ .word 0x0000EC78
    /* 8AC7C 8009A47C 84EA9EFC */ .word 0xFC9EEA84
    /* 8AC80 8009A480 9CE69EFC */ .word 0xFC9EE69C
    /* 8AC84 8009A484 A8E40000 */ .word 0x0000E4A8
    /* 8AC88 8009A488 9CE66203 */ .word 0x0362E69C
enddlabel D_8009A474

nonmatching D_8009A48C

dlabel D_8009A48C
    /* 8AC8C 8009A48C A4386203 */ .word 0x036238A4
    /* 8AC90 8009A490 BC340000 */ .word 0x000034BC
    /* 8AC94 8009A494 C8320000 */ .word 0x000032C8
    /* 8AC98 8009A498 BC346203 */ .word 0x036234BC
    /* 8AC9C 8009A49C A4386203 */ .word 0x036238A4
enddlabel D_8009A48C

nonmatching D_8009A4A0

dlabel D_8009A4A0
    /* 8ACA0 8009A4A0 8CBF6203 */ .word 0x0362BF8C
    /* 8ACA4 8009A4A4 74C36203 */ .word 0x0362C374
    /* 8ACA8 8009A4A8 68C50000 */ .word 0x0000C568
    /* 8ACAC 8009A4AC 74C39EFC */ .word 0xFC9EC374
    /* 8ACB0 8009A4B0 8CBF9EFC */ .word 0xFC9EBF8C
enddlabel D_8009A4A0

nonmatching D_8009A4B4

dlabel D_8009A4B4
    /* 8ACB4 8009A4B4 00032506 */ .word 0x06250300
    /* 8ACB8 8009A4B8 5CA40980 */ .word D_8009A45C
    /* 8ACBC 8009A4BC 00032506 */ .word 0x06250300
    /* 8ACC0 8009A4C0 74A40980 */ .word D_8009A474
    /* 8ACC4 8009A4C4 80032505 */ .word 0x05250380
    /* 8ACC8 8009A4C8 8CA40980 */ .word D_8009A48C
    /* 8ACCC 8009A4CC 80032505 */ .word 0x05250380
    /* 8ACD0 8009A4D0 A0A40980 */ .word D_8009A4A0
enddlabel D_8009A4B4

nonmatching D_8009A4D4

dlabel D_8009A4D4
    /* 8ACD4 8009A4D4 C8231810 */ .word 0x101823C8
    /* 8ACD8 8009A4D8 2814D007 */ .word 0x07D01428
    /* 8ACDC 8009A4DC 8804B81F */ .word 0x1FB80488
enddlabel D_8009A4D4

nonmatching D_8009A4E0

dlabel D_8009A4E0
    /* 8ACE0 8009A4E0 48E5E817 */ .word 0x17E8E548
    /* 8ACE4 8009A4E4 78DD1810 */ .word 0x1018DD78
    /* 8ACE8 8009A4E8 A8D51810 */ .word 0x1018D5A8
enddlabel D_8009A4E0

nonmatching D_8009A4EC

dlabel D_8009A4EC
    /* 8ACEC 8009A4EC 580C98D1 */ .word 0xD1980C58
    /* 8ACF0 8009A4F0 281468D9 */ .word 0xD9681428
    /* 8ACF4 8009A4F4 C82368D9 */ .word 0xD96823C8
enddlabel D_8009A4EC

nonmatching D_8009A4F8

dlabel D_8009A4F8
    /* 8ACF8 8009A4F8 80032503 */ .word 0x03250380
    /* 8ACFC 8009A4FC D4A40980 */ .word D_8009A4D4
    /* 8AD00 8009A500 80032503 */ .word 0x03250380
    /* 8AD04 8009A504 E0A40980 */ .word D_8009A4E0
    /* 8AD08 8009A508 80032503 */ .word 0x03250380
    /* 8AD0C 8009A50C ECA40980 */ .word D_8009A4EC
enddlabel D_8009A4F8

nonmatching D_8009A510

dlabel D_8009A510
    /* 8AD10 8009A510 E9EE7A3F */ .word 0x3F7AEEE9
    /* 8AD14 8009A514 E9EE863D */ .word 0x3D86EEE9
    /* 8AD18 8009A518 EFED863D */ .word 0x3D86EDEF
    /* 8AD1C 8009A51C EFED7A3F */ .word 0x3F7AEDEF
enddlabel D_8009A510

nonmatching D_8009A520

dlabel D_8009A520
    /* 8AD20 8009A520 20D160F0 */ .word 0xF060D120
    /* 8AD24 8009A524 C0E060F0 */ .word 0xF060E0C0
    /* 8AD28 8009A528 C0E040A2 */ .word 0xA240E0C0
enddlabel D_8009A520

nonmatching D_8009A52C

dlabel D_8009A52C
    /* 8AD2C 8009A52C 00062504 */ .word 0x04250600
    /* 8AD30 8009A530 10A50980 */ .word D_8009A510
    /* 8AD34 8009A534 80032503 */ .word 0x03250380
    /* 8AD38 8009A538 20A50980 */ .word D_8009A520
enddlabel D_8009A52C

nonmatching D_8009A53C

dlabel D_8009A53C
    /* 8AD3C 8009A53C 204EA00F */ .word 0x0FA04E20
    /* 8AD40 8009A540 20D1A00F */ .word 0x0FA0D120
    /* 8AD44 8009A544 20D1803E */ .word 0x3E80D120
enddlabel D_8009A53C

nonmatching D_8009A548

dlabel D_8009A548
    /* 8AD48 8009A548 20D180C1 */ .word 0xC180D120
    /* 8AD4C 8009A54C 20D160F0 */ .word 0xF060D120
    /* 8AD50 8009A550 204E60F0 */ .word 0xF0604E20
enddlabel D_8009A548

nonmatching D_8009A554

dlabel D_8009A554
    /* 8AD54 8009A554 80033203 */ .word 0x03320380
    /* 8AD58 8009A558 3CA50980 */ .word D_8009A53C
    /* 8AD5C 8009A55C 80033203 */ .word 0x03320380
    /* 8AD60 8009A560 48A50980 */ .word D_8009A548
enddlabel D_8009A554

nonmatching D_8009A564

dlabel D_8009A564
    /* 8AD64 8009A564 E02ED007 */ .word 0x07D02EE0
    /* 8AD68 8009A568 1027D007 */ .word 0x07D02710
    /* 8AD6C 8009A56C 10275046 */ .word 0x46502710
enddlabel D_8009A564

nonmatching D_8009A570

dlabel D_8009A570
    /* 8AD70 8009A570 F0D85046 */ .word 0x4650D8F0
    /* 8AD74 8009A574 F0D8B036 */ .word 0x36B0D8F0
    /* 8AD78 8009A578 20D1B036 */ .word 0x36B0D120
enddlabel D_8009A570

nonmatching D_8009A57C

dlabel D_8009A57C
    /* 8AD7C 8009A57C 80032503 */ .word 0x03250380
    /* 8AD80 8009A580 64A50980 */ .word D_8009A564
    /* 8AD84 8009A584 80032503 */ .word 0x03250380
    /* 8AD88 8009A588 70A50980 */ .word D_8009A570
enddlabel D_8009A57C

nonmatching D_8009A58C

dlabel D_8009A58C
    /* 8AD8C 8009A58C E02EA00F */ .word 0x0FA02EE0
    /* 8AD90 8009A590 A00FA00F */ .word 0x0FA00FA0
    /* 8AD94 8009A594 A00FE02E */ .word 0x2EE00FA0
enddlabel D_8009A58C

nonmatching D_8009A598

dlabel D_8009A598
    /* 8AD98 8009A598 80032503 */ .word 0x03250380
    /* 8AD9C 8009A59C 8CA50980 */ .word D_8009A58C
enddlabel D_8009A598

nonmatching D_8009A5A0

dlabel D_8009A5A0
    /* 8ADA0 8009A5A0 102730F8 */ .word 0xF8302710
    /* 8ADA4 8009A5A4 000030F8 */ .word 0xF8300000
    /* 8ADA8 8009A5A8 0000D007 */ .word 0x07D00000
    /* 8ADAC 8009A5AC 1027D007 */ .word 0x07D02710
enddlabel D_8009A5A0

nonmatching D_8009A5B0

dlabel D_8009A5B0
    /* 8ADB0 8009A5B0 F0D8A00F */ .word 0x0FA0D8F0
    /* 8ADB4 8009A5B4 D8DCA00F */ .word 0x0FA0DCD8
    /* 8ADB8 8009A5B8 D8DC60F0 */ .word 0xF060DCD8
    /* 8ADBC 8009A5BC F0D860F0 */ .word 0xF060D8F0
enddlabel D_8009A5B0

nonmatching D_8009A5C0

dlabel D_8009A5C0
    /* 8ADC0 8009A5C0 D8DC204E */ .word 0x4E20DCD8
    /* 8ADC4 8009A5C4 D8DCB036 */ .word 0x36B0DCD8
    /* 8ADC8 8009A5C8 F0D8B036 */ .word 0x36B0D8F0
enddlabel D_8009A5C0

nonmatching D_8009A5CC

dlabel D_8009A5CC
    /* 8ADCC 8009A5CC F0D850C9 */ .word 0xC950D8F0
    /* 8ADD0 8009A5D0 D8DC50C9 */ .word 0xC950DCD8
    /* 8ADD4 8009A5D4 D8DCE0B1 */ .word 0xB1E0DCD8
enddlabel D_8009A5CC

nonmatching D_8009A5D8

dlabel D_8009A5D8
    /* 8ADD8 8009A5D8 80032504 */ .word 0x04250380
    /* 8ADDC 8009A5DC A0A50980 */ .word D_8009A5A0
    /* 8ADE0 8009A5E0 80032504 */ .word 0x04250380
    /* 8ADE4 8009A5E4 B0A50980 */ .word D_8009A5B0
    /* 8ADE8 8009A5E8 80032503 */ .word 0x03250380
    /* 8ADEC 8009A5EC C0A50980 */ .word D_8009A5C0
    /* 8ADF0 8009A5F0 80032503 */ .word 0x03250380
    /* 8ADF4 8009A5F4 CCA50980 */ .word D_8009A5CC
enddlabel D_8009A5D8

nonmatching D_8009A5F8

dlabel D_8009A5F8
    /* 8ADF8 8009A5F8 44CBF0D8 */ .word 0xD8F0CB44
    /* 8ADFC 8009A5FC 30F8F0D8 */ .word 0xD8F0F830
    /* 8AE00 8009A600 60F060F0 */ .word 0xF060F060
    /* 8AE04 8009A604 803E60F0 */ .word 0xF0603E80
enddlabel D_8009A5F8

nonmatching D_8009A608

dlabel D_8009A608
    /* 8AE08 8009A608 54DE6806 */ .word 0x0668DE54
    /* 8AE0C 8009A60C 54DE5E01 */ .word 0x015EDE54
    /* 8AE10 8009A610 F0D85E01 */ .word 0x015ED8F0
    /* 8AE14 8009A614 F0D86806 */ .word 0x0668D8F0
enddlabel D_8009A608

nonmatching D_8009A618

dlabel D_8009A618
    /* 8AE18 8009A618 ACFE6806 */ .word 0x0668FEAC
    /* 8AE1C 8009A61C ACFE5E01 */ .word 0x015EFEAC
    /* 8AE20 8009A620 EEE95E01 */ .word 0x015EE9EE
    /* 8AE24 8009A624 EEE96806 */ .word 0x0668E9EE
enddlabel D_8009A618

nonmatching D_8009A628

dlabel D_8009A628
    /* 8AE28 8009A628 AC2B6806 */ .word 0x06682BAC
    /* 8AE2C 8009A62C AC2B5E01 */ .word 0x015E2BAC
    /* 8AE30 8009A630 12115E01 */ .word 0x015E1112
    /* 8AE34 8009A634 12116806 */ .word 0x06681112
enddlabel D_8009A628

nonmatching D_8009A638

dlabel D_8009A638
    /* 8AE38 8009A638 C0032504 */ .word 0x042503C0
    /* 8AE3C 8009A63C F8A50980 */ .word D_8009A5F8
    /* 8AE40 8009A640 40032504 */ .word 0x04250340
    /* 8AE44 8009A644 08A60980 */ .word D_8009A608
    /* 8AE48 8009A648 40032504 */ .word 0x04250340
    /* 8AE4C 8009A64C 18A60980 */ .word D_8009A618
    /* 8AE50 8009A650 40032504 */ .word 0x04250340
    /* 8AE54 8009A654 28A60980 */ .word D_8009A628
enddlabel D_8009A638

nonmatching D_8009A658

dlabel D_8009A658
    /* 8AE58 8009A658 00000000 */ .word 0x00000000
    /* 8AE5C 8009A65C 00000000 */ .word 0x00000000
    /* 8AE60 8009A660 00000000 */ .word 0x00000000
    /* 8AE64 8009A664 04000000 */ .word 0x00000004
    /* 8AE68 8009A668 E0A00980 */ .word D_8009A0E0
    /* 8AE6C 8009A66C 00000000 */ .word 0x00000000
    /* 8AE70 8009A670 00000000 */ .word 0x00000000
    /* 8AE74 8009A674 00000000 */ .word 0x00000000
    /* 8AE78 8009A678 00000000 */ .word 0x00000000
    /* 8AE7C 8009A67C 00000000 */ .word 0x00000000
    /* 8AE80 8009A680 00000000 */ .word 0x00000000
    /* 8AE84 8009A684 00000000 */ .word 0x00000000
    /* 8AE88 8009A688 02000000 */ .word 0x00000002
    /* 8AE8C 8009A68C 1CA10980 */ .word D_8009A11C
    /* 8AE90 8009A690 00000000 */ .word 0x00000000
    /* 8AE94 8009A694 00000000 */ .word 0x00000000
    /* 8AE98 8009A698 00000000 */ .word 0x00000000
    /* 8AE9C 8009A69C 00000000 */ .word 0x00000000
    /* 8AEA0 8009A6A0 02000000 */ .word 0x00000002
    /* 8AEA4 8009A6A4 7CA10980 */ .word D_8009A17C
    /* 8AEA8 8009A6A8 00000000 */ .word 0x00000000
    /* 8AEAC 8009A6AC 03000000 */ .word 0x00000003
    /* 8AEB0 8009A6B0 D0A10980 */ .word D_8009A1D0
    /* 8AEB4 8009A6B4 00000000 */ .word 0x00000000
    /* 8AEB8 8009A6B8 04000000 */ .word 0x00000004
    /* 8AEBC 8009A6BC 20A20980 */ .word D_8009A220
    /* 8AEC0 8009A6C0 00000000 */ .word 0x00000000
    /* 8AEC4 8009A6C4 03000000 */ .word 0x00000003
    /* 8AEC8 8009A6C8 68A20980 */ .word D_8009A268
    /* 8AECC 8009A6CC 00000000 */ .word 0x00000000
    /* 8AED0 8009A6D0 02000000 */ .word 0x00000002
    /* 8AED4 8009A6D4 A8A20980 */ .word D_8009A2A8
    /* 8AED8 8009A6D8 00000000 */ .word 0x00000000
    /* 8AEDC 8009A6DC 02000000 */ .word 0x00000002
    /* 8AEE0 8009A6E0 D0A20980 */ .word D_8009A2D0
    /* 8AEE4 8009A6E4 00000000 */ .word 0x00000000
    /* 8AEE8 8009A6E8 05000000 */ .word 0x00000005
    /* 8AEEC 8009A6EC 1CA30980 */ .word D_8009A31C
    /* 8AEF0 8009A6F0 00000000 */ .word 0x00000000
    /* 8AEF4 8009A6F4 00000000 */ .word 0x00000000
    /* 8AEF8 8009A6F8 00000000 */ .word 0x00000000
    /* 8AEFC 8009A6FC 00000000 */ .word 0x00000000
    /* 8AF00 8009A700 00000000 */ .word 0x00000000
    /* 8AF04 8009A704 00000000 */ .word 0x00000000
    /* 8AF08 8009A708 00000000 */ .word 0x00000000
    /* 8AF0C 8009A70C 02000000 */ .word 0x00000002
    /* 8AF10 8009A710 64A30980 */ .word D_8009A364
    /* 8AF14 8009A714 00000000 */ .word 0x00000000
    /* 8AF18 8009A718 01000000 */ .word 0x00000001
    /* 8AF1C 8009A71C 84A30980 */ .word D_8009A384
    /* 8AF20 8009A720 00000000 */ .word 0x00000000
    /* 8AF24 8009A724 03000000 */ .word 0x00000003
    /* 8AF28 8009A728 B8A30980 */ .word D_8009A3B8
    /* 8AF2C 8009A72C 00000000 */ .word 0x00000000
    /* 8AF30 8009A730 00000000 */ .word 0x00000000
    /* 8AF34 8009A734 00000000 */ .word 0x00000000
    /* 8AF38 8009A738 00000000 */ .word 0x00000000
    /* 8AF3C 8009A73C 01000000 */ .word 0x00000001
    /* 8AF40 8009A740 E0A30980 */ .word D_8009A3E0
    /* 8AF44 8009A744 00000000 */ .word 0x00000000
    /* 8AF48 8009A748 01000000 */ .word 0x00000001
    /* 8AF4C 8009A74C F4A30980 */ .word D_8009A3F4
    /* 8AF50 8009A750 00000000 */ .word 0x00000000
    /* 8AF54 8009A754 00000000 */ .word 0x00000000
    /* 8AF58 8009A758 00000000 */ .word 0x00000000
    /* 8AF5C 8009A75C 00000000 */ .word 0x00000000
    /* 8AF60 8009A760 04000000 */ .word 0x00000004
    /* 8AF64 8009A764 3CA40980 */ .word D_8009A43C
    /* 8AF68 8009A768 00000000 */ .word 0x00000000
    /* 8AF6C 8009A76C 04000000 */ .word 0x00000004
    /* 8AF70 8009A770 B4A40980 */ .word D_8009A4B4
    /* 8AF74 8009A774 00000000 */ .word 0x00000000
    /* 8AF78 8009A778 03000000 */ .word 0x00000003
    /* 8AF7C 8009A77C F8A40980 */ .word D_8009A4F8
    /* 8AF80 8009A780 00000000 */ .word 0x00000000
    /* 8AF84 8009A784 02000000 */ .word 0x00000002
    /* 8AF88 8009A788 2CA50980 */ .word D_8009A52C
    /* 8AF8C 8009A78C 00000000 */ .word 0x00000000
    /* 8AF90 8009A790 02000000 */ .word 0x00000002
enddlabel D_8009A658

nonmatching D_8009A794

dlabel D_8009A794
    /* 8AF94 8009A794 */ .short 0xA554
    /* 8AF96 8009A796 */ .short 0x8009
    /* 8AF98 8009A798 */ .short 0x0000
    /* 8AF9A 8009A79A */ .short 0x0000
    /* 8AF9C 8009A79C */ .short 0x0000
    /* 8AF9E 8009A79E */ .short 0x0000
    /* 8AFA0 8009A7A0 */ .short 0x0000
    /* 8AFA2 8009A7A2 */ .short 0x0000
    /* 8AFA4 8009A7A4 */ .short 0x0000
    /* 8AFA6 8009A7A6 */ .short 0x0000
    /* 8AFA8 8009A7A8 */ .short 0x0002
    /* 8AFAA 8009A7AA */ .short 0x0000
    /* 8AFAC 8009A7AC */ .short 0xA57C
    /* 8AFAE 8009A7AE */ .short 0x8009
    /* 8AFB0 8009A7B0 */ .short 0x0000
    /* 8AFB2 8009A7B2 */ .short 0x0000
    /* 8AFB4 8009A7B4 */ .short 0x0001
    /* 8AFB6 8009A7B6 */ .short 0x0000
    /* 8AFB8 8009A7B8 */ .short 0xA598
    /* 8AFBA 8009A7BA */ .short 0x8009
    /* 8AFBC 8009A7BC */ .short 0x0000
    /* 8AFBE 8009A7BE */ .short 0x0000
    /* 8AFC0 8009A7C0 */ .short 0x0004
    /* 8AFC2 8009A7C2 */ .short 0x0000
    /* 8AFC4 8009A7C4 */ .short 0xA5D8
    /* 8AFC6 8009A7C6 */ .short 0x8009
    /* 8AFC8 8009A7C8 */ .short 0x0000
    /* 8AFCA 8009A7CA */ .short 0x0000
    /* 8AFCC 8009A7CC */ .short 0x0004
    /* 8AFCE 8009A7CE */ .short 0x0000
    /* 8AFD0 8009A7D0 */ .short 0xA638
    /* 8AFD2 8009A7D2 */ .short 0x8009
    /* 8AFD4 8009A7D4 */ .short 0x0000
    /* 8AFD6 8009A7D6 */ .short 0x0000
    /* 8AFD8 8009A7D8 */ .short 0x0000
    /* 8AFDA 8009A7DA */ .short 0x0000
    /* 8AFDC 8009A7DC */ .short 0x0000
    /* 8AFDE 8009A7DE */ .short 0x0000
    /* 8AFE0 8009A7E0 */ .short 0x0000
    /* 8AFE2 8009A7E2 */ .short 0x0000
    /* 8AFE4 8009A7E4 */ .short 0x0000
    /* 8AFE6 8009A7E6 */ .short 0x0000
    /* 8AFE8 8009A7E8 */ .short 0x0000
    /* 8AFEA 8009A7EA */ .short 0x0000
    /* 8AFEC 8009A7EC */ .short 0x0000
    /* 8AFEE 8009A7EE */ .short 0x0000
    /* 8AFF0 8009A7F0 */ .short 0x0000
    /* 8AFF2 8009A7F2 */ .short 0x0000
    /* 8AFF4 8009A7F4 */ .short 0x0000
    /* 8AFF6 8009A7F6 */ .short 0x0000
    /* 8AFF8 8009A7F8 */ .short 0x0000
    /* 8AFFA 8009A7FA */ .short 0x0000
    /* 8AFFC 8009A7FC */ .short 0x0000
    /* 8AFFE 8009A7FE */ .short 0x0000
    /* 8B000 8009A800 */ .short 0x0000
    /* 8B002 8009A802 */ .short 0x0000
    /* 8B004 8009A804 */ .short 0x0000
    /* 8B006 8009A806 */ .short 0x0000
    /* 8B008 8009A808 */ .short 0x0000
    /* 8B00A 8009A80A */ .short 0x0000
    /* 8B00C 8009A80C */ .short 0x0000
    /* 8B00E 8009A80E */ .short 0x0000
    /* 8B010 8009A810 */ .short 0x0000
    /* 8B012 8009A812 */ .short 0x0000
    /* 8B014 8009A814 */ .short 0x0000
    /* 8B016 8009A816 */ .short 0x0000
    /* 8B018 8009A818 */ .short 0x0000
    /* 8B01A 8009A81A */ .short 0x0000
    /* 8B01C 8009A81C */ .short 0x0000
    /* 8B01E 8009A81E */ .short 0x0000
enddlabel D_8009A794

nonmatching g_fighter_cpu_probes

dlabel g_fighter_cpu_probes
    /* 8B020 8009A820 */ .byte 0x0D
    /* 8B021 8009A821 */ .byte 0x00
    /* 8B022 8009A822 */ .byte 0x09
    /* 8B023 8009A823 */ .byte 0x04
    /* 8B024 8009A824 */ .byte 0x0B
    /* 8B025 8009A825 */ .byte 0x06
    /* 8B026 8009A826 */ .byte 0x0D
    /* 8B027 8009A827 */ .byte 0x08
    /* 8B028 8009A828 */ .byte 0x0B
    /* 8B029 8009A829 */ .byte 0x0A
    /* 8B02A 8009A82A */ .byte 0x09
    /* 8B02B 8009A82B */ .byte 0x0C
    /* 8B02C 8009A82C */ .byte 0x2E
    /* 8B02D 8009A82D */ .byte 0x00
    /* 8B02E 8009A82E */ .byte 0x0D
    /* 8B02F 8009A82F */ .byte 0x18
enddlabel g_fighter_cpu_probes

nonmatching D_8009A830

dlabel D_8009A830
    /* 8B030 8009A830 */ .byte 0x00
    /* 8B031 8009A831 */ .byte 0xD1
    /* 8B032 8009A832 */ .byte 0x7C
    /* 8B033 8009A833 */ .byte 0x62
    /* 8B034 8009A834 */ .byte 0xFB
    /* 8B035 8009A835 */ .byte 0x12
    /* 8B036 8009A836 */ .byte 0x00
    /* 8B037 8009A837 */ .byte 0x00
enddlabel D_8009A830

nonmatching D_8009A838

dlabel D_8009A838
    /* 8B038 8009A838 */ .byte 0x00
    /* 8B039 8009A839 */ .byte 0xF5
    /* 8B03A 8009A83A */ .byte 0x1F
    /* 8B03B 8009A83B */ .byte 0x18
    /* 8B03C 8009A83C */ .byte 0x41
    /* 8B03D 8009A83D */ .byte 0x75
    /* 8B03E 8009A83E */ .byte 0x00
    /* 8B03F 8009A83F */ .byte 0x00
enddlabel D_8009A838

nonmatching D_8009A840

dlabel D_8009A840
    /* 8B040 8009A840 */ .byte 0x00
    /* 8B041 8009A841 */ .byte 0x00
    /* 8B042 8009A842 */ .byte 0x91
    /* 8B043 8009A843 */ .byte 0x00
    /* 8B044 8009A844 */ .byte 0x54
    /* 8B045 8009A845 */ .byte 0x51
    /* 8B046 8009A846 */ .byte 0x00
    /* 8B047 8009A847 */ .byte 0x00
    /* 8B048 8009A848 */ .byte 0x29
    /* 8B049 8009A849 */ .byte 0x00
    /* 8B04A 8009A84A */ .byte 0x00
    /* 8B04B 8009A84B */ .byte 0x00
    /* 8B04C 8009A84C */ .byte 0x00
    /* 8B04D 8009A84D */ .byte 0x97
    /* 8B04E 8009A84E */ .byte 0x00
    /* 8B04F 8009A84F */ .byte 0x00
enddlabel D_8009A840

nonmatching D_8009A850

dlabel D_8009A850
    /* 8B050 8009A850 */ .byte 0x0E
enddlabel D_8009A850

nonmatching D_8009A851

dlabel D_8009A851
    /* 8B051 8009A851 */ .byte 0xFF
enddlabel D_8009A851

nonmatching D_8009A852

dlabel D_8009A852
    /* 8B052 8009A852 */ .byte 0x7D
enddlabel D_8009A852

nonmatching D_8009A853

dlabel D_8009A853
    /* 8B053 8009A853 */ .byte 0x03
    /* 8B054 8009A854 */ .byte 0x2C
    /* 8B055 8009A855 */ .byte 0xFF
    /* 8B056 8009A856 */ .byte 0x7D
    /* 8B057 8009A857 */ .byte 0x03
    /* 8B058 8009A858 */ .byte 0x09
    /* 8B059 8009A859 */ .byte 0xFF
    /* 8B05A 8009A85A */ .byte 0x9C
    /* 8B05B 8009A85B */ .byte 0x0B
    /* 8B05C 8009A85C */ .byte 0x08
    /* 8B05D 8009A85D */ .byte 0xFF
    /* 8B05E 8009A85E */ .byte 0xBB
    /* 8B05F 8009A85F */ .byte 0x0B
    /* 8B060 8009A860 */ .byte 0x18
    /* 8B061 8009A861 */ .byte 0xFF
    /* 8B062 8009A862 */ .byte 0x7D
    /* 8B063 8009A863 */ .byte 0x03
    /* 8B064 8009A864 */ .byte 0x17
    /* 8B065 8009A865 */ .byte 0xFF
    /* 8B066 8009A866 */ .byte 0x7D
    /* 8B067 8009A867 */ .byte 0x03
    /* 8B068 8009A868 */ .byte 0x13
    /* 8B069 8009A869 */ .byte 0xFF
    /* 8B06A 8009A86A */ .byte 0x9C
    /* 8B06B 8009A86B */ .byte 0x0A
    /* 8B06C 8009A86C */ .byte 0x15
    /* 8B06D 8009A86D */ .byte 0x18
    /* 8B06E 8009A86E */ .byte 0xFA
    /* 8B06F 8009A86F */ .byte 0x00
enddlabel D_8009A853

nonmatching D_8009A870

dlabel D_8009A870
    /* 8B070 8009A870 05800000 */ .word 0x00008005
enddlabel D_8009A870

nonmatching D_8009A874

dlabel D_8009A874
    /* 8B074 8009A874 06800000 */ .word 0x00008006
enddlabel D_8009A874

nonmatching D_8009A878

dlabel D_8009A878
    /* 8B078 8009A878 0E811E81 */ .word 0x811E810E
    /* 8B07C 8009A87C 0E800000 */ .word 0x0000800E
enddlabel D_8009A878

nonmatching D_8009A880

dlabel D_8009A880
    /* 8B080 8009A880 0C811C81 */ .word 0x811C810C
    /* 8B084 8009A884 0C800000 */ .word 0x0000800C
enddlabel D_8009A880

nonmatching D_8009A888

dlabel D_8009A888
    /* 8B088 8009A888 0D811D81 */ .word 0x811D810D
    /* 8B08C 8009A88C 0D800000 */ .word 0x0000800D
enddlabel D_8009A888

nonmatching D_8009A890

dlabel D_8009A890
    /* 8B090 8009A890 0F811F81 */ .word 0x811F810F
    /* 8B094 8009A894 0F800000 */ .word 0x0000800F
enddlabel D_8009A890

nonmatching D_8009A898

dlabel D_8009A898
    /* 8B098 8009A898 03800000 */ .word 0x00008003
enddlabel D_8009A898

nonmatching D_8009A89C

dlabel D_8009A89C
    /* 8B09C 8009A89C 0D850184 */ .word 0x8401850D
    /* 8B0A0 8009A8A0 03800000 */ .word 0x00008003
enddlabel D_8009A89C

nonmatching D_8009A8A4

dlabel D_8009A8A4
    /* 8B0A4 8009A8A4 02810184 */ .word 0x84018102
    /* 8B0A8 8009A8A8 03800000 */ .word 0x00008003
enddlabel D_8009A8A4

nonmatching D_8009A8AC

dlabel D_8009A8AC
    /* 8B0AC 8009A8AC 01850580 */ .word 0x80058501
    /* 8B0B0 8009A8B0 80000000 */ .word 0x00000080
enddlabel D_8009A8AC

nonmatching D_8009A8B4

dlabel D_8009A8B4
    /* 8B0B4 8009A8B4 01850581 */ .word 0x81058501
    /* 8B0B8 8009A8B8 15850580 */ .word 0x80058515
    /* 8B0BC 8009A8BC 80000000 */ .word 0x00000080
enddlabel D_8009A8B4

nonmatching D_8009A8C0

dlabel D_8009A8C0
    /* 8B0C0 8009A8C0 01850680 */ .word 0x80068501
enddlabel D_8009A8C0

nonmatching D_8009A8C4

dlabel D_8009A8C4
    /* 8B0C4 8009A8C4 80000000 */ .word 0x00000080
dlabel D_8009A8C8
    /* 8B0C8 8009A8C8 */ .short 0x3200
enddlabel D_8009A8C4

nonmatching D_8009A8CA

dlabel D_8009A8CA
    /* 8B0CA 8009A8CA */ .short 0x0083
    /* 8B0CC 8009A8CC 00328700 */ .word 0x00873200
    /* 8B0D0 8009A8D0 10288F00 */ .word 0x008F2810
    /* 8B0D4 8009A8D4 1028CF43 */ .word 0x43CF2810
    /* 8B0D8 8009A8D8 1028CF43 */ .word 0x43CF2810
    /* 8B0DC 8009A8DC 101EBF41 */ .word 0x41BF1E10
    /* 8B0E0 8009A8E0 0C1E7F42 */ .word 0x427F1E0C
    /* 8B0E4 8009A8E4 00000000 */ .word 0x00000000
    /* 8B0E8 8009A8E8 00320000 */ .word 0x00003200
    /* 8B0EC 8009A8EC 00328320 */ .word 0x20833200
    /* 8B0F0 8009A8F0 0C288320 */ .word 0x2083280C
    /* 8B0F4 8009A8F4 0C286B22 */ .word 0x226B280C
    /* 8B0F8 8009A8F8 0C286B22 */ .word 0x226B280C
    /* 8B0FC 8009A8FC 0C1EAF21 */ .word 0x21AF1E0C
    /* 8B100 8009A900 081EAF21 */ .word 0x21AF1E08
    /* 8B104 8009A904 00000000 */ .word 0x00000000
    /* 8B108 8009A908 00320000 */ .word 0x00003200
    /* 8B10C 8009A90C 00320710 */ .word 0x10073200
    /* 8B110 8009A910 0C280F10 */ .word 0x100F280C
    /* 8B114 8009A914 0C285F10 */ .word 0x105F280C
    /* 8B118 8009A918 0C285F10 */ .word 0x105F280C
    /* 8B11C 8009A91C 0C1EAF10 */ .word 0x10AF1E0C
    /* 8B120 8009A920 081EAF10 */ .word 0x10AF1E08
    /* 8B124 8009A924 00000000 */ .word 0x00000000
enddlabel D_8009A8CA

nonmatching D_8009A928

dlabel D_8009A928
    /* 8B128 8009A928 01000500 */ .word 0x00050001
    /* 8B12C 8009A92C 02000E00 */ .word 0x000E0002
    /* 8B130 8009A930 82001500 */ .word 0x00150082
    /* 8B134 8009A934 41002A00 */ .word 0x002A0041
    /* 8B138 8009A938 28003000 */ .word 0x00300028
    /* 8B13C 8009A93C 04020112 */ .word 0x12010204
    /* 8B140 8009A940 08010221 */ .word 0x21020108
    /* 8B144 8009A944 0C033C00 */ .word 0x003C030C
    /* 8B148 8009A948 0F002C01 */ .word 0x012C000F
    /* 8B14C 8009A94C 21022C00 */ .word 0x002C0221
    /* 8B150 8009A950 1F002411 */ .word 0x1124001F
    /* 8B154 8009A954 3C300100 */ .word 0x0001303C
    /* 8B158 8009A958 05000200 */ .word 0x00020005
    /* 8B15C 8009A95C 0A008200 */ .word 0x0082000A
    /* 8B160 8009A960 0B004100 */ .word 0x0041000B
    /* 8B164 8009A964 2A002800 */ .word 0x0028002A
    /* 8B168 8009A968 30000402 */ .word 0x02040030
    /* 8B16C 8009A96C 01060401 */ .word 0x01040601
    /* 8B170 8009A970 02090903 */ .word 0x03090902
    /* 8B174 8009A974 3C000F00 */ .word 0x000F003C
    /* 8B178 8009A978 2C010901 */ .word 0x0109012C
    /* 8B17C 8009A97C 1C001F00 */ .word 0x001F001C
    /* 8B180 8009A980 0C063C0C */ .word 0x0C3C060C
    /* 8B184 8009A984 01000500 */ .word 0x00050001
    /* 8B188 8009A988 02000E00 */ .word 0x000E0002
    /* 8B18C 8009A98C 22000500 */ .word 0x00050022
    /* 8B190 8009A990 21000D00 */ .word 0x000D0021
    /* 8B194 8009A994 0C002C00 */ .word 0x002C000C
    /* 8B198 8009A998 44004102 */ .word 0x02410044
    /* 8B19C 8009A99C 88008802 */ .word 0x02880088
    /* 8B1A0 8009A9A0 CC008F00 */ .word 0x008F00CC
    /* 8B1A4 8009A9A4 0F008C04 */ .word 0x048C000F
    /* 8B1A8 8009A9A8 09010F00 */ .word 0x000F0109
    /* 8B1AC 8009A9AC 4F008C02 */ .word 0x028C004F
    /* 8B1B0 8009A9B0 0F060000 */ .word 0x0000060F
enddlabel D_8009A928

nonmatching D_8009A9B4

dlabel D_8009A9B4
    /* 8B1B4 8009A9B4 001E001E */ .word 0x1E001E00
    /* 8B1B8 8009A9B8 001E001E */ .word 0x1E001E00
    /* 8B1BC 8009A9BC 001E001E */ .word 0x1E001E00
    /* 8B1C0 8009A9C0 181E1014 */ .word 0x14101E18
    /* 8B1C4 8009A9C4 181E1014 */ .word 0x14101E18
    /* 8B1C8 8009A9C8 141E0C14 */ .word 0x140C1E14
    /* 8B1CC 8009A9CC 141E0C14 */ .word 0x140C1E14
    /* 8B1D0 8009A9D0 101E0814 */ .word 0x14081E10
    /* 8B1D4 8009A9D4 101E0814 */ .word 0x14081E10
    /* 8B1D8 8009A9D8 0C14040F */ .word 0x0F04140C
enddlabel D_8009A9B4

nonmatching D_8009A9DC

dlabel D_8009A9DC
    /* 8B1DC 8009A9DC 00000200 */ .word 0x00020000
    /* 8B1E0 8009A9E0 00020202 */ .word 0x02020200
    /* 8B1E4 8009A9E4 02020202 */ .word 0x02020202
    /* 8B1E8 8009A9E8 00020001 */ .word 0x01000200
    /* 8B1EC 8009A9EC 00010000 */ .word 0x00000100
enddlabel D_8009A9DC

nonmatching D_8009A9F0

dlabel D_8009A9F0
    /* 8B1F0 8009A9F0 00000000 */ .word 0x00000000
    /* 8B1F4 8009A9F4 12030000 */ .word 0x00000312
    /* 8B1F8 8009A9F8 22040000 */ .word 0x00000422
    /* 8B1FC 8009A9FC 143204F0 */ .word 0xF0043214
    /* 8B200 8009AA00 52060000 */ .word 0x00000652
    /* 8B204 8009AA04 13530000 */ .word 0x00005313
    /* 8B208 8009AA08 23640000 */ .word 0x00006423
    /* 8B20C 8009AA0C 163254F6 */ .word 0xF6543216
    /* 8B210 8009AA10 00000000 */ .word 0x00000000
    /* 8B214 8009AA14 11000000 */ .word 0x00000011
    /* 8B218 8009AA18 21000000 */ .word 0x00000021
    /* 8B21C 8009AA1C 120200F0 */ .word 0xF0000212
    /* 8B220 8009AA20 52060000 */ .word 0x00000652
    /* 8B224 8009AA24 12050000 */ .word 0x00000512
    /* 8B228 8009AA28 22060000 */ .word 0x00000622
    /* 8B22C 8009AA2C 145206F0 */ .word 0xF0065214
    /* 8B230 8009AA30 00000000 */ .word 0x00000000
    /* 8B234 8009AA34 12030000 */ .word 0x00000312
    /* 8B238 8009AA38 22040000 */ .word 0x00000422
    /* 8B23C 8009AA3C 143204F0 */ .word 0xF0043214
    /* 8B240 8009AA40 32040000 */ .word 0x00000432
    /* 8B244 8009AA44 12030000 */ .word 0x00000312
    /* 8B248 8009AA48 22040000 */ .word 0x00000422
    /* 8B24C 8009AA4C 143204F0 */ .word 0xF0043214
enddlabel D_8009A9F0

nonmatching D_8009AA50

dlabel D_8009AA50
    /* 8B250 8009AA50 00300000 */ .word 0x00003000
    /* 8B254 8009AA54 00100000 */ .word 0x00001000
    /* 8B258 8009AA58 00900000 */ .word 0x00009000
    /* 8B25C 8009AA5C 00800000 */ .word 0x00008000
    /* 8B260 8009AA60 00C00000 */ .word 0x0000C000
    /* 8B264 8009AA64 00400000 */ .word 0x00004000
    /* 8B268 8009AA68 00600000 */ .word 0x00006000
    /* 8B26C 8009AA6C 00200000 */ .word 0x00002000
enddlabel D_8009AA50

nonmatching D_8009AA70

dlabel D_8009AA70
    /* 8B270 8009AA70 00000000 */ .word 0x00000000
    /* 8B274 8009AA74 00000100 */ .word 0x00010000
    /* 8B278 8009AA78 00000200 */ .word 0x00020000
    /* 8B27C 8009AA7C 00000300 */ .word 0x00030000
    /* 8B280 8009AA80 00000400 */ .word 0x00040000
    /* 8B284 8009AA84 01000000 */ .word 0x00000001
    /* 8B288 8009AA88 01000100 */ .word 0x00010001
    /* 8B28C 8009AA8C 01000200 */ .word 0x00020001
    /* 8B290 8009AA90 01000300 */ .word 0x00030001
    /* 8B294 8009AA94 01000400 */ .word 0x00040001
    /* 8B298 8009AA98 01000500 */ .word 0x00050001
    /* 8B29C 8009AA9C 01000600 */ .word 0x00060001
    /* 8B2A0 8009AAA0 01000700 */ .word 0x00070001
    /* 8B2A4 8009AAA4 01000800 */ .word 0x00080001
    /* 8B2A8 8009AAA8 01000900 */ .word 0x00090001
    /* 8B2AC 8009AAAC 01000A00 */ .word 0x000A0001
    /* 8B2B0 8009AAB0 01000B00 */ .word 0x000B0001
    /* 8B2B4 8009AAB4 01000C00 */ .word 0x000C0001
    /* 8B2B8 8009AAB8 01000D00 */ .word 0x000D0001
    /* 8B2BC 8009AABC 01000E00 */ .word 0x000E0001
    /* 8B2C0 8009AAC0 01000F00 */ .word 0x000F0001
    /* 8B2C4 8009AAC4 01001000 */ .word 0x00100001
    /* 8B2C8 8009AAC8 01001100 */ .word 0x00110001
    /* 8B2CC 8009AACC 01001200 */ .word 0x00120001
    /* 8B2D0 8009AAD0 01001300 */ .word 0x00130001
    /* 8B2D4 8009AAD4 01001400 */ .word 0x00140001
    /* 8B2D8 8009AAD8 01001500 */ .word 0x00150001
    /* 8B2DC 8009AADC 01001600 */ .word 0x00160001
    /* 8B2E0 8009AAE0 01001700 */ .word 0x00170001
    /* 8B2E4 8009AAE4 01001800 */ .word 0x00180001
    /* 8B2E8 8009AAE8 01001900 */ .word 0x00190001
    /* 8B2EC 8009AAEC 01001A00 */ .word 0x001A0001
    /* 8B2F0 8009AAF0 01001B00 */ .word 0x001B0001
    /* 8B2F4 8009AAF4 01000000 */ .word 0x00000001
    /* 8B2F8 8009AAF8 01000100 */ .word 0x00010001
    /* 8B2FC 8009AAFC 01000200 */ .word 0x00020001
    /* 8B300 8009AB00 01000300 */ .word 0x00030001
    /* 8B304 8009AB04 01000400 */ .word 0x00040001
    /* 8B308 8009AB08 01000500 */ .word 0x00050001
    /* 8B30C 8009AB0C 01000600 */ .word 0x00060001
    /* 8B310 8009AB10 01000700 */ .word 0x00070001
    /* 8B314 8009AB14 01000800 */ .word 0x00080001
    /* 8B318 8009AB18 01000900 */ .word 0x00090001
    /* 8B31C 8009AB1C 01000A00 */ .word 0x000A0001
    /* 8B320 8009AB20 01000B00 */ .word 0x000B0001
    /* 8B324 8009AB24 01000C00 */ .word 0x000C0001
    /* 8B328 8009AB28 01000D00 */ .word 0x000D0001
    /* 8B32C 8009AB2C 01000E00 */ .word 0x000E0001
    /* 8B330 8009AB30 01000F00 */ .word 0x000F0001
    /* 8B334 8009AB34 02000000 */ .word 0x00000002
    /* 8B338 8009AB38 02000100 */ .word 0x00010002
    /* 8B33C 8009AB3C 02000200 */ .word 0x00020002
    /* 8B340 8009AB40 02000300 */ .word 0x00030002
    /* 8B344 8009AB44 02000400 */ .word 0x00040002
    /* 8B348 8009AB48 02000500 */ .word 0x00050002
    /* 8B34C 8009AB4C 02000600 */ .word 0x00060002
    /* 8B350 8009AB50 02000700 */ .word 0x00070002
    /* 8B354 8009AB54 02000800 */ .word 0x00080002
    /* 8B358 8009AB58 02000900 */ .word 0x00090002
    /* 8B35C 8009AB5C 02000A00 */ .word 0x000A0002
    /* 8B360 8009AB60 02000B00 */ .word 0x000B0002
    /* 8B364 8009AB64 02000C00 */ .word 0x000C0002
    /* 8B368 8009AB68 02000D00 */ .word 0x000D0002
    /* 8B36C 8009AB6C 02000E00 */ .word 0x000E0002
    /* 8B370 8009AB70 02000F00 */ .word 0x000F0002
    /* 8B374 8009AB74 03000000 */ .word 0x00000003
    /* 8B378 8009AB78 03000100 */ .word 0x00010003
    /* 8B37C 8009AB7C 03000200 */ .word 0x00020003
    /* 8B380 8009AB80 03000300 */ .word 0x00030003
    /* 8B384 8009AB84 03000400 */ .word 0x00040003
    /* 8B388 8009AB88 03000500 */ .word 0x00050003
    /* 8B38C 8009AB8C 03000600 */ .word 0x00060003
    /* 8B390 8009AB90 03000700 */ .word 0x00070003
    /* 8B394 8009AB94 04000000 */ .word 0x00000004
    /* 8B398 8009AB98 04000100 */ .word 0x00010004
    /* 8B39C 8009AB9C 04000200 */ .word 0x00020004
    /* 8B3A0 8009ABA0 04000300 */ .word 0x00030004
    /* 8B3A4 8009ABA4 04000400 */ .word 0x00040004
    /* 8B3A8 8009ABA8 04000500 */ .word 0x00050004
    /* 8B3AC 8009ABAC 04000600 */ .word 0x00060004
    /* 8B3B0 8009ABB0 04000700 */ .word 0x00070004
    /* 8B3B4 8009ABB4 04000800 */ .word 0x00080004
    /* 8B3B8 8009ABB8 04000900 */ .word 0x00090004
    /* 8B3BC 8009ABBC 04000A00 */ .word 0x000A0004
    /* 8B3C0 8009ABC0 04000B00 */ .word 0x000B0004
    /* 8B3C4 8009ABC4 04000C00 */ .word 0x000C0004
    /* 8B3C8 8009ABC8 04000D00 */ .word 0x000D0004
    /* 8B3CC 8009ABCC 04000E00 */ .word 0x000E0004
    /* 8B3D0 8009ABD0 04000F00 */ .word 0x000F0004
    /* 8B3D4 8009ABD4 05000000 */ .word 0x00000005
    /* 8B3D8 8009ABD8 05000100 */ .word 0x00010005
    /* 8B3DC 8009ABDC 05000200 */ .word 0x00020005
    /* 8B3E0 8009ABE0 05000300 */ .word 0x00030005
    /* 8B3E4 8009ABE4 05000400 */ .word 0x00040005
    /* 8B3E8 8009ABE8 05000500 */ .word 0x00050005
    /* 8B3EC 8009ABEC 05000600 */ .word 0x00060005
    /* 8B3F0 8009ABF0 05000700 */ .word 0x00070005
    /* 8B3F4 8009ABF4 05000800 */ .word 0x00080005
    /* 8B3F8 8009ABF8 05000900 */ .word 0x00090005
    /* 8B3FC 8009ABFC 05000A00 */ .word 0x000A0005
    /* 8B400 8009AC00 05000B00 */ .word 0x000B0005
    /* 8B404 8009AC04 05000C00 */ .word 0x000C0005
    /* 8B408 8009AC08 05000D00 */ .word 0x000D0005
    /* 8B40C 8009AC0C 05000E00 */ .word 0x000E0005
    /* 8B410 8009AC10 05000F00 */ .word 0x000F0005
    /* 8B414 8009AC14 06000000 */ .word 0x00000006
    /* 8B418 8009AC18 06000100 */ .word 0x00010006
    /* 8B41C 8009AC1C 06000200 */ .word 0x00020006
    /* 8B420 8009AC20 06000300 */ .word 0x00030006
    /* 8B424 8009AC24 06000400 */ .word 0x00040006
    /* 8B428 8009AC28 06000500 */ .word 0x00050006
    /* 8B42C 8009AC2C 06000600 */ .word 0x00060006
    /* 8B430 8009AC30 06000700 */ .word 0x00070006
    /* 8B434 8009AC34 07000000 */ .word 0x00000007
    /* 8B438 8009AC38 07000100 */ .word 0x00010007
    /* 8B43C 8009AC3C 07000200 */ .word 0x00020007
    /* 8B440 8009AC40 07000300 */ .word 0x00030007
    /* 8B444 8009AC44 07000400 */ .word 0x00040007
    /* 8B448 8009AC48 07000500 */ .word 0x00050007
    /* 8B44C 8009AC4C 07000600 */ .word 0x00060007
    /* 8B450 8009AC50 07000700 */ .word 0x00070007
    /* 8B454 8009AC54 07000800 */ .word 0x00080007
    /* 8B458 8009AC58 07000900 */ .word 0x00090007
    /* 8B45C 8009AC5C 07000A00 */ .word 0x000A0007
    /* 8B460 8009AC60 07000B00 */ .word 0x000B0007
    /* 8B464 8009AC64 07000C00 */ .word 0x000C0007
    /* 8B468 8009AC68 07000D00 */ .word 0x000D0007
    /* 8B46C 8009AC6C 07000E00 */ .word 0x000E0007
    /* 8B470 8009AC70 08000000 */ .word 0x00000008
    /* 8B474 8009AC74 08000100 */ .word 0x00010008
    /* 8B478 8009AC78 08000200 */ .word 0x00020008
    /* 8B47C 8009AC7C 08000300 */ .word 0x00030008
    /* 8B480 8009AC80 08000400 */ .word 0x00040008
    /* 8B484 8009AC84 08000500 */ .word 0x00050008
    /* 8B488 8009AC88 08000600 */ .word 0x00060008
    /* 8B48C 8009AC8C 08000700 */ .word 0x00070008
    /* 8B490 8009AC90 08000800 */ .word 0x00080008
    /* 8B494 8009AC94 08000900 */ .word 0x00090008
    /* 8B498 8009AC98 08000A00 */ .word 0x000A0008
    /* 8B49C 8009AC9C 08000B00 */ .word 0x000B0008
    /* 8B4A0 8009ACA0 08000C00 */ .word 0x000C0008
    /* 8B4A4 8009ACA4 08000D00 */ .word 0x000D0008
    /* 8B4A8 8009ACA8 08000E00 */ .word 0x000E0008
    /* 8B4AC 8009ACAC 08000F00 */ .word 0x000F0008
    /* 8B4B0 8009ACB0 08001000 */ .word 0x00100008
    /* 8B4B4 8009ACB4 08001100 */ .word 0x00110008
    /* 8B4B8 8009ACB8 08001200 */ .word 0x00120008
    /* 8B4BC 8009ACBC 08001300 */ .word 0x00130008
    /* 8B4C0 8009ACC0 08001400 */ .word 0x00140008
    /* 8B4C4 8009ACC4 08001500 */ .word 0x00150008
    /* 8B4C8 8009ACC8 08001600 */ .word 0x00160008
    /* 8B4CC 8009ACCC 08001700 */ .word 0x00170008
    /* 8B4D0 8009ACD0 08001800 */ .word 0x00180008
    /* 8B4D4 8009ACD4 08001900 */ .word 0x00190008
    /* 8B4D8 8009ACD8 08001A00 */ .word 0x001A0008
    /* 8B4DC 8009ACDC 08001B00 */ .word 0x001B0008
    /* 8B4E0 8009ACE0 09000000 */ .word 0x00000009
    /* 8B4E4 8009ACE4 09000100 */ .word 0x00010009
    /* 8B4E8 8009ACE8 09000200 */ .word 0x00020009
    /* 8B4EC 8009ACEC 09000300 */ .word 0x00030009
    /* 8B4F0 8009ACF0 09000400 */ .word 0x00040009
    /* 8B4F4 8009ACF4 09000500 */ .word 0x00050009
    /* 8B4F8 8009ACF8 09000600 */ .word 0x00060009
    /* 8B4FC 8009ACFC 09000700 */ .word 0x00070009
    /* 8B500 8009AD00 09000800 */ .word 0x00080009
    /* 8B504 8009AD04 09000900 */ .word 0x00090009
    /* 8B508 8009AD08 09000A00 */ .word 0x000A0009
    /* 8B50C 8009AD0C 09000B00 */ .word 0x000B0009
    /* 8B510 8009AD10 09000C00 */ .word 0x000C0009
    /* 8B514 8009AD14 09000D00 */ .word 0x000D0009
enddlabel D_8009AA70

nonmatching g_vab_id_list

dlabel g_vab_id_list
    /* 8B518 8009AD18 */ .byte 0x07
    /* 8B519 8009AD19 */ .byte 0x03
    /* 8B51A 8009AD1A */ .byte 0x06
    /* 8B51B 8009AD1B */ .byte 0x00
enddlabel g_vab_id_list

nonmatching D_8009AD1C

dlabel D_8009AD1C
    /* 8B51C 8009AD1C 02000200 */ .word 0x00020002
    /* 8B520 8009AD20 01000200 */ .word 0x00020001
    /* 8B524 8009AD24 01000200 */ .word 0x00020001
    /* 8B528 8009AD28 FFFF0000 */ .word 0x0000FFFF
    /* 8B52C 8009AD2C FFFF0000 */ .word 0x0000FFFF
    /* 8B530 8009AD30 FFFF0000 */ .word 0x0000FFFF
    /* 8B534 8009AD34 FFFF0000 */ .word 0x0000FFFF
    /* 8B538 8009AD38 FFFF0000 */ .word 0x0000FFFF
    /* 8B53C 8009AD3C FFFF0000 */ .word 0x0000FFFF
    /* 8B540 8009AD40 FFFF0000 */ .word 0x0000FFFF
    /* 8B544 8009AD44 FFFF0000 */ .word 0x0000FFFF
    /* 8B548 8009AD48 FFFF0000 */ .word 0x0000FFFF
    /* 8B54C 8009AD4C FFFF0000 */ .word 0x0000FFFF
    /* 8B550 8009AD50 FFFF0000 */ .word 0x0000FFFF
    /* 8B554 8009AD54 FFFF0000 */ .word 0x0000FFFF
    /* 8B558 8009AD58 FFFF0000 */ .word 0x0000FFFF
    /* 8B55C 8009AD5C FFFF0000 */ .word 0x0000FFFF
    /* 8B560 8009AD60 FFFF0000 */ .word 0x0000FFFF
    /* 8B564 8009AD64 FFFF0000 */ .word 0x0000FFFF
    /* 8B568 8009AD68 FFFF0000 */ .word 0x0000FFFF
    /* 8B56C 8009AD6C FFFF0000 */ .word 0x0000FFFF
    /* 8B570 8009AD70 FFFF0000 */ .word 0x0000FFFF
    /* 8B574 8009AD74 FFFF0000 */ .word 0x0000FFFF
    /* 8B578 8009AD78 FFFF0000 */ .word 0x0000FFFF
    /* 8B57C 8009AD7C 04002000 */ .word 0x00200004
    /* 8B580 8009AD80 FFFF0000 */ .word 0x0000FFFF
    /* 8B584 8009AD84 FFFF0000 */ .word 0x0000FFFF
    /* 8B588 8009AD88 FFFF0000 */ .word 0x0000FFFF
    /* 8B58C 8009AD8C FFFF0800 */ .word 0x0008FFFF
    /* 8B590 8009AD90 FFFF0000 */ .word 0x0000FFFF
    /* 8B594 8009AD94 01000800 */ .word 0x00080001
    /* 8B598 8009AD98 FFFF0200 */ .word 0x0002FFFF
    /* 8B59C 8009AD9C 01000200 */ .word 0x00020001
    /* 8B5A0 8009ADA0 01000200 */ .word 0x00020001
    /* 8B5A4 8009ADA4 FFFF0200 */ .word 0x0002FFFF
    /* 8B5A8 8009ADA8 01000200 */ .word 0x00020001
    /* 8B5AC 8009ADAC 01000200 */ .word 0x00020001
    /* 8B5B0 8009ADB0 FFFF0000 */ .word 0x0000FFFF
enddlabel D_8009AD1C

nonmatching D_8009ADB4

dlabel D_8009ADB4
    /* 8B5B4 8009ADB4 1F000300 */ .word 0x0003001F
    /* 8B5B8 8009ADB8 C003FF01 */ .word 0x01FF03C0
    /* 8B5BC 8009ADBC 00007700 */ .word 0x00770000
enddlabel D_8009ADB4

nonmatching D_8009ADC0

dlabel D_8009ADC0
    /* 8B5C0 8009ADC0 30005000 */ .word 0x00500030
    /* 8B5C4 8009ADC4 B000100C */ .word 0x0C1000B0
    /* 8B5C8 8009ADC8 40005000 */ .word 0x00500040
    /* 8B5CC 8009ADCC C000100C */ .word 0x0C1000C0
    /* 8B5D0 8009ADD0 50005000 */ .word 0x00500050
    /* 8B5D4 8009ADD4 D000300C */ .word 0x0C3000D0
enddlabel D_8009ADC0

nonmatching D_8009ADD8

dlabel D_8009ADD8
    /* 8B5D8 8009ADD8 00005000 */ .word 0x00500000
    /* 8B5DC 8009ADDC 0000200C */ .word 0x0C200000
    /* 8B5E0 8009ADE0 20005000 */ .word 0x00500020
    /* 8B5E4 8009ADE4 2000200C */ .word 0x0C200020
    /* 8B5E8 8009ADE8 40005000 */ .word 0x00500040
    /* 8B5EC 8009ADEC 40000B0C */ .word 0x0C0B0040
    /* 8B5F0 8009ADF0 00005000 */ .word 0x00500000
    /* 8B5F4 8009ADF4 0018200C */ .word 0x0C201800
    /* 8B5F8 8009ADF8 20005000 */ .word 0x00500020
    /* 8B5FC 8009ADFC 2018200C */ .word 0x0C201820
    /* 8B600 8009AE00 40005000 */ .word 0x00500040
    /* 8B604 8009AE04 40180B0C */ .word 0x0C0B1840
    /* 8B608 8009AE08 00005000 */ .word 0x00500000
    /* 8B60C 8009AE0C 000C200C */ .word 0x0C200C00
    /* 8B610 8009AE10 20005000 */ .word 0x00500020
    /* 8B614 8009AE14 200C200C */ .word 0x0C200C20
    /* 8B618 8009AE18 40005000 */ .word 0x00500040
    /* 8B61C 8009AE1C 400C020C */ .word 0x0C020C40
    /* 8B620 8009AE20 00005000 */ .word 0x00500000
    /* 8B624 8009AE24 0024200C */ .word 0x0C202400
    /* 8B628 8009AE28 20005000 */ .word 0x00500020
    /* 8B62C 8009AE2C 2024200C */ .word 0x0C202420
    /* 8B630 8009AE30 40005000 */ .word 0x00500040
    /* 8B634 8009AE34 4024160C */ .word 0x0C162440
    /* 8B638 8009AE38 00005000 */ .word 0x00500000
    /* 8B63C 8009AE3C 0030200C */ .word 0x0C203000
    /* 8B640 8009AE40 20005000 */ .word 0x00500020
    /* 8B644 8009AE44 2030200C */ .word 0x0C203020
    /* 8B648 8009AE48 40005000 */ .word 0x00500040
    /* 8B64C 8009AE4C 4030350C */ .word 0x0C353040
    /* 8B650 8009AE50 00005000 */ .word 0x00500000
    /* 8B654 8009AE54 4E54200C */ .word 0x0C20544E
    /* 8B658 8009AE58 20005000 */ .word 0x00500020
    /* 8B65C 8009AE5C 6E54200C */ .word 0x0C20546E
    /* 8B660 8009AE60 40005000 */ .word 0x00500040
    /* 8B664 8009AE64 8E54120C */ .word 0x0C12548E
    /* 8B668 8009AE68 00005000 */ .word 0x00500000
    /* 8B66C 8009AE6C 8030200C */ .word 0x0C203080
    /* 8B670 8009AE70 20005000 */ .word 0x00500020
    /* 8B674 8009AE74 A030200C */ .word 0x0C2030A0
    /* 8B678 8009AE78 40005000 */ .word 0x00500040
    /* 8B67C 8009AE7C C030100C */ .word 0x0C1030C0
    /* 8B680 8009AE80 00005000 */ .word 0x00500000
    /* 8B684 8009AE84 003C200C */ .word 0x0C203C00
    /* 8B688 8009AE88 20005000 */ .word 0x00500020
    /* 8B68C 8009AE8C 203C200C */ .word 0x0C203C20
    /* 8B690 8009AE90 40005000 */ .word 0x00500040
    /* 8B694 8009AE94 403C370C */ .word 0x0C373C40
    /* 8B698 8009AE98 00005000 */ .word 0x00500000
    /* 8B69C 8009AE9C 8000100C */ .word 0x0C100080
    /* 8B6A0 8009AEA0 10005000 */ .word 0x00500010
    /* 8B6A4 8009AEA4 9000100C */ .word 0x0C100090
    /* 8B6A8 8009AEA8 20005000 */ .word 0x00500020
    /* 8B6AC 8009AEAC A000100C */ .word 0x0C1000A0
    /* 8B6B0 8009AEB0 00005000 */ .word 0x00500000
    /* 8B6B4 8009AEB4 0048200C */ .word 0x0C204800
    /* 8B6B8 8009AEB8 20005000 */ .word 0x00500020
    /* 8B6BC 8009AEBC 2048200C */ .word 0x0C204820
    /* 8B6C0 8009AEC0 40005000 */ .word 0x00500040
    /* 8B6C4 8009AEC4 4048330C */ .word 0x0C334840
    /* 8B6C8 8009AEC8 00005000 */ .word 0x00500000
    /* 8B6CC 8009AECC 4E60200C */ .word 0x0C20604E
    /* 8B6D0 8009AED0 20005000 */ .word 0x00500020
    /* 8B6D4 8009AED4 6E60200C */ .word 0x0C20606E
    /* 8B6D8 8009AED8 40005000 */ .word 0x00500040
    /* 8B6DC 8009AEDC 8E60180C */ .word 0x0C18608E
    /* 8B6E0 8009AEE0 00005000 */ .word 0x00500000
    /* 8B6E4 8009AEE4 0054100C */ .word 0x0C105400
    /* 8B6E8 8009AEE8 10005000 */ .word 0x00500010
    /* 8B6EC 8009AEEC 1054100C */ .word 0x0C105410
    /* 8B6F0 8009AEF0 20005000 */ .word 0x00500020
    /* 8B6F4 8009AEF4 2054100C */ .word 0x0C105420
    /* 8B6F8 8009AEF8 00005000 */ .word 0x00500000
    /* 8B6FC 8009AEFC 8048070C */ .word 0x0C074880
    /* 8B700 8009AF00 07005000 */ .word 0x00500007
    /* 8B704 8009AF04 8748070C */ .word 0x0C074887
    /* 8B708 8009AF08 0E005000 */ .word 0x0050000E
    /* 8B70C 8009AF0C 8E48090C */ .word 0x0C09488E
    /* 8B710 8009AF10 00005000 */ .word 0x00500000
    /* 8B714 8009AF14 006C200C */ .word 0x0C206C00
    /* 8B718 8009AF18 20005000 */ .word 0x00500020
    /* 8B71C 8009AF1C 206C200C */ .word 0x0C206C20
    /* 8B720 8009AF20 40005000 */ .word 0x00500040
    /* 8B724 8009AF24 406C0E0C */ .word 0x0C0E6C40
    /* 8B728 8009AF28 00005000 */ .word 0x00500000
    /* 8B72C 8009AF2C 8024100C */ .word 0x0C102480
    /* 8B730 8009AF30 10005000 */ .word 0x00500010
    /* 8B734 8009AF34 9024100C */ .word 0x0C102490
    /* 8B738 8009AF38 20005000 */ .word 0x00500020
    /* 8B73C 8009AF3C A024150C */ .word 0x0C1524A0
    /* 8B740 8009AF40 00005000 */ .word 0x00500000
    /* 8B744 8009AF44 800C200C */ .word 0x0C200C80
    /* 8B748 8009AF48 20005000 */ .word 0x00500020
    /* 8B74C 8009AF4C A00C200C */ .word 0x0C200CA0
    /* 8B750 8009AF50 40005000 */ .word 0x00500040
    /* 8B754 8009AF54 C00C3A0C */ .word 0x0C3A0CC0
    /* 8B758 8009AF58 00005000 */ .word 0x00500000
    /* 8B75C 8009AF5C 0060200C */ .word 0x0C206000
    /* 8B760 8009AF60 20005000 */ .word 0x00500020
    /* 8B764 8009AF64 2060200C */ .word 0x0C206020
    /* 8B768 8009AF68 40005000 */ .word 0x00500040
    /* 8B76C 8009AF6C 4060080C */ .word 0x0C086040
    /* 8B770 8009AF70 00005000 */ .word 0x00500000
    /* 8B774 8009AF74 803C200C */ .word 0x0C203C80
    /* 8B778 8009AF78 20005000 */ .word 0x00500020
    /* 8B77C 8009AF7C A03C200C */ .word 0x0C203CA0
    /* 8B780 8009AF80 40005000 */ .word 0x00500040
    /* 8B784 8009AF84 C03C160C */ .word 0x0C163CC0
    /* 8B788 8009AF88 00005000 */ .word 0x00500000
    /* 8B78C 8009AF8C 8018200C */ .word 0x0C201880
    /* 8B790 8009AF90 20005000 */ .word 0x00500020
enddlabel D_8009ADD8

nonmatching D_8009AF94

dlabel D_8009AF94
    /* 8B794 8009AF94 */ .short 0x18A0
    /* 8B796 8009AF96 */ .short 0x0C20
    /* 8B798 8009AF98 */ .short 0x0040
    /* 8B79A 8009AF9A */ .short 0x0050
    /* 8B79C 8009AF9C */ .short 0x18C0
    /* 8B79E 8009AF9E */ .short 0x0C36
    /* 8B7A0 8009AFA0 */ .short 0x0000
    /* 8B7A2 8009AFA2 */ .short 0x0050
    /* 8B7A4 8009AFA4 */ .short 0x6C4E
    /* 8B7A6 8009AFA6 */ .short 0x0C20
    /* 8B7A8 8009AFA8 */ .short 0x0020
    /* 8B7AA 8009AFAA */ .short 0x0050
    /* 8B7AC 8009AFAC */ .short 0x6C6E
    /* 8B7AE 8009AFAE */ .short 0x0C20
    /* 8B7B0 8009AFB0 */ .short 0x0040
    /* 8B7B2 8009AFB2 */ .short 0x0050
    /* 8B7B4 8009AFB4 */ .short 0x6C8E
    /* 8B7B6 8009AFB6 */ .short 0x0C12
enddlabel D_8009AF94

nonmatching D_8009AFB8

dlabel D_8009AFB8
    /* 8B7B8 8009AFB8 B001B001 */ .word 0x01B001B0
    /* 8B7BC 8009AFBC B401AA01 */ .word 0x01AA01B4
    /* 8B7C0 8009AFC0 9B01AC01 */ .word 0x01AC019B
    /* 8B7C4 8009AFC4 AD019A01 */ .word 0x019A01AD
    /* 8B7C8 8009AFC8 95019C01 */ .word 0x019C0195
    /* 8B7CC 8009AFCC A901BD01 */ .word 0x01BD01A9
    /* 8B7D0 8009AFD0 CB01AE01 */ .word 0x01AE01CB
    /* 8B7D4 8009AFD4 BB019801 */ .word 0x019801BB
    /* 8B7D8 8009AFD8 B101AA01 */ .word 0x01AA01B1
    /* 8B7DC 8009AFDC 9B01AC01 */ .word 0x01AC019B
enddlabel D_8009AFB8

nonmatching D_8009AFE0

dlabel D_8009AFE0
    /* 8B7E0 8009AFE0 1E0006FF */ .word 0xFF06001E
    /* 8B7E4 8009AFE4 B003FE01 */ .word 0x01FE03B0
    /* 8B7E8 8009AFE8 00000000 */ .word 0x00000000
    /* 8B7EC 8009AFEC 91003A00 */ .word 0x003A0091
    /* 8B7F0 8009AFF0 0000400C */ .word 0x0C400000
    /* 8B7F4 8009AFF4 D1003A00 */ .word 0x003A00D1
    /* 8B7F8 8009AFF8 4000400C */ .word 0x0C400040
    /* 8B7FC 8009AFFC 11013A00 */ .word 0x003A0111
    /* 8B800 8009B000 8000400C */ .word 0x0C400080
    /* 8B804 8009B004 51013A00 */ .word 0x003A0151
    /* 8B808 8009B008 C000400C */ .word 0x0C4000C0
    /* 8B80C 8009B00C 91013A00 */ .word 0x003A0191
    /* 8B810 8009B010 000C400C */ .word 0x0C400C00
    /* 8B814 8009B014 D1013A00 */ .word 0x003A01D1
    /* 8B818 8009B018 400C1E0C */ .word 0x0C1E0C40
    /* 8B81C 8009B01C 00000000 */ .word 0x00000000
    /* 8B820 8009B020 00000000 */ .word 0x00000000
enddlabel D_8009AFE0

nonmatching D_8009B024

dlabel D_8009B024
    /* 8B824 8009B024 1E0003FF */ .word 0xFF03001E
    /* 8B828 8009B028 9003FF01 */ .word 0x01FF0390
    /* 8B82C 8009B02C 00002800 */ .word 0x00280000
    /* 8B830 8009B030 6B005000 */ .word 0x0050006B
    /* 8B834 8009B034 0000150C */ .word 0x0C150000
    /* 8B838 8009B038 6B006A00 */ .word 0x006A006B
    /* 8B83C 8009B03C 1500150C */ .word 0x0C150015
    /* 8B840 8009B040 6B008400 */ .word 0x0084006B
    /* 8B844 8009B044 2A00150C */ .word 0x0C15002A
enddlabel D_8009B024

nonmatching D_8009B048

dlabel D_8009B048
    /* 8B848 8009B048 1E000CFF */ .word 0xFF0C001E
    /* 8B84C 8009B04C 8003FF01 */ .word 0x01FF0380
    /* 8B850 8009B050 FA001A00 */ .word 0x001A00FA
    /* 8B854 8009B054 E6004E00 */ .word 0x004E00E6
    /* 8B858 8009B058 00000304 */ .word 0x04030000
    /* 8B85C 8009B05C 1E014E00 */ .word 0x004E011E
    /* 8B860 8009B060 00000304 */ .word 0x04030000
    /* 8B864 8009B064 23014E00 */ .word 0x004E0123
    /* 8B868 8009B068 00000304 */ .word 0x04030000
    /* 8B86C 8009B06C E6006800 */ .word 0x006800E6
    /* 8B870 8009B070 00000304 */ .word 0x04030000
    /* 8B874 8009B074 1E016800 */ .word 0x0068011E
    /* 8B878 8009B078 00000304 */ .word 0x04030000
    /* 8B87C 8009B07C 23016800 */ .word 0x00680123
    /* 8B880 8009B080 00000304 */ .word 0x04030000
    /* 8B884 8009B084 E6008200 */ .word 0x008200E6
    /* 8B888 8009B088 00000304 */ .word 0x04030000
    /* 8B88C 8009B08C 1E018200 */ .word 0x0082011E
    /* 8B890 8009B090 00000304 */ .word 0x04030000
    /* 8B894 8009B094 23018200 */ .word 0x00820123
    /* 8B898 8009B098 00000304 */ .word 0x04030000
    /* 8B89C 8009B09C D100B000 */ .word 0x00B000D1
    /* 8B8A0 8009B0A0 00000304 */ .word 0x04030000
    /* 8B8A4 8009B0A4 0901B000 */ .word 0x00B00109
    /* 8B8A8 8009B0A8 00000304 */ .word 0x04030000
    /* 8B8AC 8009B0AC 0E01B000 */ .word 0x00B0010E
    /* 8B8B0 8009B0B0 00000304 */ .word 0x04030000
enddlabel D_8009B048

nonmatching D_8009B0B4

dlabel D_8009B0B4
    /* 8B8B4 8009B0B4 1E0001FF */ .word 0xFF01001E
    /* 8B8B8 8009B0B8 8003FF01 */ .word 0x01FF0380
    /* 8B8BC 8009B0BC 00001A00 */ .word 0x001A0000
enddlabel D_8009B0B4

nonmatching D_8009B0C0

dlabel D_8009B0C0
    /* 8B8C0 8009B0C0 E0AF0980 */ .word D_8009AFE0
    /* 8B8C4 8009B0C4 24B00980 */ .word D_8009B024
    /* 8B8C8 8009B0C8 48B00980 */ .word D_8009B048
    /* 8B8CC 8009B0CC B4AD0980 */ .word D_8009ADB4
    /* 8B8D0 8009B0D0 C0AD0980 */ .word D_8009ADC0
    /* 8B8D4 8009B0D4 B4B00980 */ .word D_8009B0B4
    /* 8B8D8 8009B0D8 64320A80 */ .word D_800A3264
    /* 8B8DC 8009B0DC B8AF0980 */ .word D_8009AFB8
enddlabel D_8009B0C0

nonmatching D_8009B0E0

dlabel D_8009B0E0
    /* 8B8E0 8009B0E0 1F000200 */ .word 0x0002001F
    /* 8B8E4 8009B0E4 D003FD01 */ .word 0x01FD03D0
    /* 8B8E8 8009B0E8 A0001C00 */ .word 0x001C00A0
    /* 8B8EC 8009B0EC 1F000200 */ .word 0x0002001F
    /* 8B8F0 8009B0F0 D003FD01 */ .word 0x01FD03D0
    /* 8B8F4 8009B0F4 A0002200 */ .word 0x002200A0
enddlabel D_8009B0E0

nonmatching D_8009B0F8

dlabel D_8009B0F8
    /* 8B8F8 8009B0F8 1F000200 */ .word 0x0002001F
    /* 8B8FC 8009B0FC D003FD01 */ .word 0x01FD03D0
    /* 8B900 8009B100 A0002A00 */ .word 0x002A00A0
    /* 8B904 8009B104 1F000300 */ .word 0x0003001F
    /* 8B908 8009B108 D003FC01 */ .word 0x01FC03D0
    /* 8B90C 8009B10C 00005000 */ .word 0x00500000
enddlabel D_8009B0F8

nonmatching D_8009B110

dlabel D_8009B110
    /* 8B910 8009B110 1F000200 */ .word 0x0002001F
    /* 8B914 8009B114 E003FC01 */ .word 0x01FC03E0
    /* 8B918 8009B118 38006A00 */ .word 0x006A0038
enddlabel D_8009B110

nonmatching D_8009B11C

dlabel D_8009B11C
    /* 8B91C 8009B11C 1F000200 */ .word 0x0002001F
    /* 8B920 8009B120 E003FC01 */ .word 0x01FC03E0
    /* 8B924 8009B124 95005000 */ .word 0x00500095
    /* 8B928 8009B128 1F000400 */ .word 0x0004001F
    /* 8B92C 8009B12C E003FC01 */ .word 0x01FC03E0
    /* 8B930 8009B130 00000000 */ .word 0x00000000
    /* 8B934 8009B134 1F000400 */ .word 0x0004001F
    /* 8B938 8009B138 E003FC01 */ .word 0x01FC03E0
    /* 8B93C 8009B13C 00000000 */ .word 0x00000000
    /* 8B940 8009B140 1F000600 */ .word 0x0006001F
    /* 8B944 8009B144 E003FC01 */ .word 0x01FC03E0
    /* 8B948 8009B148 00000000 */ .word 0x00000000
enddlabel D_8009B11C

nonmatching D_8009B14C

dlabel D_8009B14C
    /* 8B94C 8009B14C */ .short 0x001F
enddlabel D_8009B14C

nonmatching D_8009B14E

dlabel D_8009B14E
    /* 8B94E 8009B14E */ .byte 0x04
    /* 8B94F 8009B14F */ .byte 0x00
    /* 8B950 8009B150 E003FC01 */ .word 0x01FC03E0
    /* 8B954 8009B154 00000000 */ .word 0x00000000
enddlabel D_8009B14E

nonmatching D_8009B158

dlabel D_8009B158
    /* 8B958 8009B158 1F000100 */ .word 0x0001001F
    /* 8B95C 8009B15C C003FF01 */ .word 0x01FF03C0
    /* 8B960 8009B160 00000000 */ .word 0x00000000
enddlabel D_8009B158

nonmatching D_8009B164

dlabel D_8009B164
    /* 8B964 8009B164 00000000 */ .word 0x00000000
    /* 8B968 8009B168 00001006 */ .word 0x06100000
enddlabel D_8009B164

nonmatching D_8009B16C

dlabel D_8009B16C
    /* 8B96C 8009B16C */ .short 0x0000
    /* 8B96E 8009B16E */ .short 0x0000
    /* 8B970 8009B170 */ .short 0x0010
    /* 8B972 8009B172 */ .short 0x0610
    /* 8B974 8009B174 */ .short 0x0000
    /* 8B976 8009B176 */ .short 0xFFFA
    /* 8B978 8009B178 */ .short 0x0000
    /* 8B97A 8009B17A */ .short 0x0810
enddlabel D_8009B16C

nonmatching D_8009B17C

dlabel D_8009B17C
    /* 8B97C 8009B17C */ .short 0x0000
    /* 8B97E 8009B17E */ .short 0xFFFA
    /* 8B980 8009B180 */ .short 0x0010
    /* 8B982 8009B182 */ .short 0x0810
enddlabel D_8009B17C

nonmatching D_8009B184

dlabel D_8009B184
    /* 8B984 8009B184 0000FCFF */ .word 0xFFFC0000
    /* 8B988 8009B188 0000100A */ .word 0x0A100000
enddlabel D_8009B184

nonmatching D_8009B18C

dlabel D_8009B18C
    /* 8B98C 8009B18C */ .short 0x0000
    /* 8B98E 8009B18E */ .short 0xFFFC
    /* 8B990 8009B190 */ .short 0x0010
    /* 8B992 8009B192 */ .short 0x0A10
enddlabel D_8009B18C

nonmatching D_8009B194

dlabel D_8009B194
    /* 8B994 8009B194 F7000300 */ .word 0x000300F7
    /* 8B998 8009B198 0000400C */ .word 0x0C400000
    /* 8B99C 8009B19C 37010300 */ .word 0x00030137
    /* 8B9A0 8009B1A0 4000400C */ .word 0x0C400040
    /* 8B9A4 8009B1A4 77010300 */ .word 0x00030177
    /* 8B9A8 8009B1A8 8000140C */ .word 0x0C140080
enddlabel D_8009B194

nonmatching D_8009B1AC

dlabel D_8009B1AC
    /* 8B9AC 8009B1AC 09011200 */ .word 0x00120109
    /* 8B9B0 8009B1B0 0000400C */ .word 0x0C400000
    /* 8B9B4 8009B1B4 49011200 */ .word 0x00120149
    /* 8B9B8 8009B1B8 40002D0C */ .word 0x0C2D0040
enddlabel D_8009B1AC

nonmatching D_8009B1BC

dlabel D_8009B1BC
    /* 8B9BC 8009B1BC 0A010400 */ .word 0x0004010A
    /* 8B9C0 8009B1C0 0000400C */ .word 0x0C400000
    /* 8B9C4 8009B1C4 4A010400 */ .word 0x0004014A
    /* 8B9C8 8009B1C8 40002B0C */ .word 0x0C2B0040
enddlabel D_8009B1BC

nonmatching D_8009B1CC

dlabel D_8009B1CC
    /* 8B9CC 8009B1CC C8001300 */ .word 0x001300C8
    /* 8B9D0 8009B1D0 005D400C */ .word 0x0C405D00
    /* 8B9D4 8009B1D4 08011300 */ .word 0x00130108
    /* 8B9D8 8009B1D8 405D350C */ .word 0x0C355D40
    /* 8B9DC 8009B1DC 3D011300 */ .word 0x0013013D
    /* 8B9E0 8009B1E0 006A390C */ .word 0x0C396A00
    /* 8B9E4 8009B1E4 76011300 */ .word 0x00130176
    /* 8B9E8 8009B1E8 A95D410C */ .word 0x0C415DA9
enddlabel D_8009B1CC

nonmatching D_8009B1EC

dlabel D_8009B1EC
    /* 8B9EC 8009B1EC CB002200 */ .word 0x002200CB
    /* 8B9F0 8009B1F0 005D400C */ .word 0x0C405D00
    /* 8B9F4 8009B1F4 0B012200 */ .word 0x0022010B
    /* 8B9F8 8009B1F8 405D400C */ .word 0x0C405D40
    /* 8B9FC 8009B1FC 4B012200 */ .word 0x0022014B
    /* 8BA00 8009B200 805D400C */ .word 0x0C405D80
    /* 8BA04 8009B204 8B012200 */ .word 0x0022018B
    /* 8BA08 8009B208 C05D2A0C */ .word 0x0C2A5DC0
enddlabel D_8009B1EC

nonmatching D_8009B20C

dlabel D_8009B20C
    /* 8BA0C 8009B20C 9F00A500 */ .word 0x00A5009F
    /* 8BA10 8009B210 009CC00D */ .word 0x0DC09C00
    /* 8BA14 8009B214 5F01A500 */ .word 0x00A5015F
    /* 8BA18 8009B218 C09C400D */ .word 0x0D409CC0
    /* 8BA1C 8009B21C 9F01A500 */ .word 0x00A5019F
    /* 8BA20 8009B220 00A9420D */ .word 0x0D42A900
    /* 8BA24 8009B224 B600B200 */ .word 0x00B200B6
    /* 8BA28 8009B228 00B6C00D */ .word 0x0DC0B600
    /* 8BA2C 8009B22C 7601B200 */ .word 0x00B20176
    /* 8BA30 8009B230 C0B6400D */ .word 0x0D40B6C0
    /* 8BA34 8009B234 B601B200 */ .word 0x00B201B6
    /* 8BA38 8009B238 00C3130D */ .word 0x0D13C300
enddlabel D_8009B20C

nonmatching D_8009B23C

dlabel D_8009B23C
    /* 8BA3C 8009B23C B4004000 */ .word 0x004000B4
    /* 8BA40 8009B240 008F400C */ .word 0x0C408F00
    /* 8BA44 8009B244 F4004000 */ .word 0x004000F4
    /* 8BA48 8009B248 408F180C */ .word 0x0C188F40
    /* 8BA4C 8009B24C 1C014000 */ .word 0x0040011C
    /* 8BA50 8009B250 4EA9320C */ .word 0x0C32A94E
    /* 8BA54 8009B254 4E014000 */ .word 0x0040014E
    /* 8BA58 8009B258 80A91A0C */ .word 0x0C1AA980
    /* 8BA5C 8009B25C B4004F00 */ .word 0x004F00B4
    /* 8BA60 8009B260 5B8F400C */ .word 0x0C408F5B
    /* 8BA64 8009B264 F4004F00 */ .word 0x004F00F4
    /* 8BA68 8009B268 9B8F280C */ .word 0x0C288F9B
    /* 8BA6C 8009B26C 1C014F00 */ .word 0x004F011C
    /* 8BA70 8009B270 4EA9320C */ .word 0x0C32A94E
    /* 8BA74 8009B274 4E014F00 */ .word 0x004F014E
    /* 8BA78 8009B278 80A91A0C */ .word 0x0C1AA980
    /* 8BA7C 8009B27C B4005E00 */ .word 0x005E00B4
    /* 8BA80 8009B280 9CA90C0D */ .word 0x0D0CA99C
    /* 8BA84 8009B284 C0005E00 */ .word 0x005E00C0
    /* 8BA88 8009B288 A8A90C0D */ .word 0x0D0CA9A8
    /* 8BA8C 8009B28C CC005E00 */ .word 0x005E00CC
    /* 8BA90 8009B290 B4A90C0D */ .word 0x0D0CA9B4
    /* 8BA94 8009B294 D8005E00 */ .word 0x005E00D8
    /* 8BA98 8009B298 C0A91D0D */ .word 0x0D1DA9C0
enddlabel D_8009B23C

nonmatching D_8009B29C

dlabel D_8009B29C
    /* 8BA9C 8009B29C AE013F00 */ .word 0x003F01AE
    /* 8BAA0 8009B2A0 DC2A1E0C */ .word 0x0C1E2ADC
    /* 8BAA4 8009B2A4 7C013F00 */ .word 0x003F017C
    /* 8BAA8 8009B2A8 C02A1C0C */ .word 0x0C1C2AC0
enddlabel D_8009B29C

nonmatching D_8009B2AC

dlabel D_8009B2AC
    /* 8BAAC 8009B2AC ACB10980 */ .word D_8009B1AC
    /* 8BAB0 8009B2B0 ACB10980 */ .word D_8009B1AC
    /* 8BAB4 8009B2B4 CCB10980 */ .word D_8009B1CC
    /* 8BAB8 8009B2B8 ECB10980 */ .word D_8009B1EC
enddlabel D_8009B2AC

nonmatching D_8009B2BC

dlabel D_8009B2BC
    /* 8BABC 8009B2BC */ .short 0x0102
enddlabel D_8009B2BC

nonmatching D_8009B2BE

dlabel D_8009B2BE
    /* 8BABE 8009B2BE */ .short 0x0041
    /* 8BAC0 8009B2C0 02013300 */ .word 0x00330102
enddlabel D_8009B2BE

nonmatching D_8009B2C4

dlabel D_8009B2C4
    /* 8BAC4 8009B2C4 */ .short 0x0148
    /* 8BAC6 8009B2C6 */ .short 0x009B
enddlabel D_8009B2C4

nonmatching D_8009B2C8

dlabel D_8009B2C8
    /* 8BAC8 8009B2C8 1F000300 */ .word 0x0003001F
    /* 8BACC 8009B2CC C003FE01 */ .word 0x01FE03C0
    /* 8BAD0 8009B2D0 00006400 */ .word 0x00640000
    /* 8BAD4 8009B2D4 1F000600 */ .word 0x0006001F
    /* 8BAD8 8009B2D8 D003FE01 */ .word 0x01FE03D0
    /* 8BADC 8009B2DC 00009200 */ .word 0x00920000
enddlabel D_8009B2C8

nonmatching D_8009B2E0

dlabel D_8009B2E0
    /* 8BAE0 8009B2E0 1F000100 */ .word 0x0001001F
    /* 8BAE4 8009B2E4 E003FE01 */ .word 0x01FE03E0
    /* 8BAE8 8009B2E8 90009200 */ .word 0x00920090
    /* 8BAEC 8009B2EC 1F000100 */ .word 0x0001001F
    /* 8BAF0 8009B2F0 E003FE01 */ .word 0x01FE03E0
    /* 8BAF4 8009B2F4 9000A600 */ .word 0x00A60090
    /* 8BAF8 8009B2F8 1F000100 */ .word 0x0001001F
    /* 8BAFC 8009B2FC E003FE01 */ .word 0x01FE03E0
    /* 8BB00 8009B300 9E00A600 */ .word 0x00A6009E
    /* 8BB04 8009B304 1E000300 */ .word 0x0003001E
    /* 8BB08 8009B308 8003FE01 */ .word 0x01FE0380
    /* 8BB0C 8009B30C 00006400 */ .word 0x00640000
    /* 8BB10 8009B310 1E000600 */ .word 0x0006001E
    /* 8BB14 8009B314 9003FE01 */ .word 0x01FE0390
    /* 8BB18 8009B318 00009200 */ .word 0x00920000
    /* 8BB1C 8009B31C 1E000100 */ .word 0x0001001E
    /* 8BB20 8009B320 A003FE01 */ .word 0x01FE03A0
    /* 8BB24 8009B324 90009200 */ .word 0x00920090
    /* 8BB28 8009B328 1E000100 */ .word 0x0001001E
    /* 8BB2C 8009B32C A003FE01 */ .word 0x01FE03A0
    /* 8BB30 8009B330 9000A600 */ .word 0x00A60090
    /* 8BB34 8009B334 1E000100 */ .word 0x0001001E
    /* 8BB38 8009B338 A003FE01 */ .word 0x01FE03A0
    /* 8BB3C 8009B33C 9E00A600 */ .word 0x00A6009E
enddlabel D_8009B2E0

nonmatching D_8009B340

dlabel D_8009B340
    /* 8BB40 8009B340 4901AA00 */ .word 0x00AA0149
    /* 8BB44 8009B344 0000402E */ .word 0x2E400000
    /* 8BB48 8009B348 8901AA00 */ .word 0x00AA0189
    /* 8BB4C 8009B34C 4000402E */ .word 0x2E400040
    /* 8BB50 8009B350 C901AA00 */ .word 0x00AA01C9
    /* 8BB54 8009B354 8000342E */ .word 0x2E340080
enddlabel D_8009B340

nonmatching D_8009B358

dlabel D_8009B358
    /* 8BB58 8009B358 DC018600 */ .word 0x008601DC
    /* 8BB5C 8009B35C 00001040 */ .word 0x40100000
    /* 8BB60 8009B360 EC018600 */ .word 0x008601EC
    /* 8BB64 8009B364 10004040 */ .word 0x40400010
    /* 8BB68 8009B368 2C028600 */ .word 0x0086022C
    /* 8BB6C 8009B36C 50004040 */ .word 0x40400050
    /* 8BB70 8009B370 DC01C600 */ .word 0x00C601DC
    /* 8BB74 8009B374 00401018 */ .word 0x18104000
    /* 8BB78 8009B378 EC01C600 */ .word 0x00C601EC
    /* 8BB7C 8009B37C 10404018 */ .word 0x18404010
    /* 8BB80 8009B380 2C02C600 */ .word 0x00C6022C
    /* 8BB84 8009B384 50404018 */ .word 0x18404050
enddlabel D_8009B358

nonmatching D_8009B388

dlabel D_8009B388
    /* 8BB88 8009B388 00000000 */ .word 0x00000000
    /* 8BB8C 8009B38C 00002414 */ .word 0x14240000
enddlabel D_8009B388

nonmatching D_8009B390

dlabel D_8009B390
    /* 8BB90 8009B390 00000000 */ .word 0x00000000
    /* 8BB94 8009B394 00000E06 */ .word 0x060E0000
enddlabel D_8009B390

nonmatching D_8009B398

dlabel D_8009B398
    /* 8BB98 8009B398 1F000100 */ .word 0x0001001F
    /* 8BB9C 8009B39C C003FD01 */ .word 0x01FD03C0
    /* 8BBA0 8009B3A0 00000000 */ .word 0x00000000
enddlabel D_8009B398

nonmatching D_8009B3A4

dlabel D_8009B3A4
    /* 8BBA4 8009B3A4 1F000100 */ .word 0x0001001F
    /* 8BBA8 8009B3A8 E003FD01 */ .word 0x01FD03E0
    /* 8BBAC 8009B3AC 00000000 */ .word 0x00000000
enddlabel D_8009B3A4

nonmatching D_8009B3B0

dlabel D_8009B3B0
    /* 8BBB0 8009B3B0 1F000100 */ .word 0x0001001F
    /* 8BBB4 8009B3B4 C003FE01 */ .word 0x01FE03C0
    /* 8BBB8 8009B3B8 00000000 */ .word 0x00000000
enddlabel D_8009B3B0

nonmatching D_8009B3BC

dlabel D_8009B3BC
    /* 8BBBC 8009B3BC 1F000100 */ .word 0x0001001F
    /* 8BBC0 8009B3C0 D003FE01 */ .word 0x01FE03D0
    /* 8BBC4 8009B3C4 00000000 */ .word 0x00000000
enddlabel D_8009B3BC

nonmatching D_8009B3C8

dlabel D_8009B3C8
    /* 8BBC8 8009B3C8 CB011600 */ .word 0x001601CB
    /* 8BBCC 8009B3CC FC000405 */ .word 0x050400FC
    /* 8BBD0 8009B3D0 01021600 */ .word 0x00160201
    /* 8BBD4 8009B3D4 FC000405 */ .word 0x050400FC
    /* 8BBD8 8009B3D8 FC011600 */ .word 0x001601FC
    /* 8BBDC 8009B3DC FC000405 */ .word 0x050400FC
    /* 8BBE0 8009B3E0 35021700 */ .word 0x00170235
    /* 8BBE4 8009B3E4 C01C3C0E */ .word 0x0E3C1CC0
    /* 8BBE8 8009B3E8 35022A00 */ .word 0x002A0235
    /* 8BBEC 8009B3EC C02A3C0E */ .word 0x0E3C2AC0
    /* 8BBF0 8009B3F0 38020000 */ .word 0x00000238
    /* 8BBF4 8009B3F4 C0DB2301 */ .word 0x0123DBC0
    /* 8BBF8 8009B3F8 5B020000 */ .word 0x0000025B
    /* 8BBFC 8009B3FC E3DB1D01 */ .word 0x011DDBE3
enddlabel D_8009B3C8

nonmatching D_8009B400

dlabel D_8009B400
    /* 8BC00 8009B400 00000000 */ .word 0x00000000
    /* 8BC04 8009B404 A000130E */ .word 0x0E1300A0
    /* 8BC08 8009B408 00000000 */ .word 0x00000000
    /* 8BC0C 8009B40C B3000F0E */ .word 0x0E0F00B3
    /* 8BC10 8009B410 00000000 */ .word 0x00000000
    /* 8BC14 8009B414 C200130E */ .word 0x0E1300C2
    /* 8BC18 8009B418 00000000 */ .word 0x00000000
    /* 8BC1C 8009B41C D500130E */ .word 0x0E1300D5
    /* 8BC20 8009B420 00000000 */ .word 0x00000000
    /* 8BC24 8009B424 E800130E */ .word 0x0E1300E8
    /* 8BC28 8009B428 00000000 */ .word 0x00000000
    /* 8BC2C 8009B42C A00E130E */ .word 0x0E130EA0
    /* 8BC30 8009B430 00000000 */ .word 0x00000000
    /* 8BC34 8009B434 B30E130E */ .word 0x0E130EB3
    /* 8BC38 8009B438 00000000 */ .word 0x00000000
    /* 8BC3C 8009B43C C60E130E */ .word 0x0E130EC6
    /* 8BC40 8009B440 00000000 */ .word 0x00000000
    /* 8BC44 8009B444 D90E130E */ .word 0x0E130ED9
    /* 8BC48 8009B448 00000000 */ .word 0x00000000
    /* 8BC4C 8009B44C EC0E130E */ .word 0x0E130EEC
enddlabel D_8009B400

nonmatching D_8009B450

dlabel D_8009B450
    /* 8BC50 8009B450 */ .short 0x01A2
enddlabel D_8009B450

nonmatching D_8009B452

dlabel D_8009B452
    /* 8BC52 8009B452 */ .short 0x0024
    /* 8BC54 8009B454 */ .short 0x01F7
    /* 8BC56 8009B456 */ .short 0x0037
enddlabel D_8009B452

nonmatching D_8009B458

dlabel D_8009B458
    /* 8BC58 8009B458 35021700 */ .word 0x00170235
    /* 8BC5C 8009B45C C01C3C0E */ .word 0x0E3C1CC0
    /* 8BC60 8009B460 0F001700 */ .word 0x0017000F
    /* 8BC64 8009B464 C01C3C0E */ .word 0x0E3C1CC0
enddlabel D_8009B458

nonmatching D_8009B468

dlabel D_8009B468
    /* 8BC68 8009B468 38020000 */ .word 0x00000238
    /* 8BC6C 8009B46C C0DB2301 */ .word 0x0123DBC0
enddlabel D_8009B468

nonmatching D_8009B470

dlabel D_8009B470
    /* 8BC70 8009B470 5B020000 */ .word 0x0000025B
    /* 8BC74 8009B474 E3DB1D01 */ .word 0x011DDBE3
    /* 8BC78 8009B478 26000000 */ .word 0x00000026
    /* 8BC7C 8009B47C DEDA2201 */ .word 0x0122DADE
    /* 8BC80 8009B480 08000000 */ .word 0x00000008
    /* 8BC84 8009B484 C0DA1E01 */ .word 0x011EDAC0
enddlabel D_8009B470

nonmatching D_8009B488

dlabel D_8009B488
    /* 8BC88 8009B488 35021700 */ .word 0x00170235
    /* 8BC8C 8009B48C */ .short 0x2AC0
enddlabel D_8009B488

nonmatching D_8009B48E

dlabel D_8009B48E
    /* 8BC8E 8009B48E */ .byte 0x3C
    /* 8BC8F 8009B48F */ .byte 0x0E
enddlabel D_8009B48E

nonmatching D_8009B490

dlabel D_8009B490
    /* 8BC90 8009B490 F2000000 */ .word 0x000000F2
    /* 8BC94 8009B494 C0DB2301 */ .word 0x0123DBC0
enddlabel D_8009B490

nonmatching D_8009B498

dlabel D_8009B498
    /* 8BC98 8009B498 6D010000 */ .word 0x0000016D
    /* 8BC9C 8009B49C DEDA2101 */ .word 0x0121DADE
    /* 8BCA0 8009B4A0 15010000 */ .word 0x00000115
    /* 8BCA4 8009B4A4 E3DB1C01 */ .word 0x011CDBE3
    /* 8BCA8 8009B4A8 4F010000 */ .word 0x0000014F
    /* 8BCAC 8009B4AC C0DA1E01 */ .word 0x011EDAC0
enddlabel D_8009B498

nonmatching D_8009B4B0

dlabel D_8009B4B0
    /* 8BCB0 8009B4B0 1E000100 */ .word 0x0001001E
    /* 8BCB4 8009B4B4 8003FE01 */ .word 0x01FE0380
    /* 8BCB8 8009B4B8 00000000 */ .word 0x00000000
enddlabel D_8009B4B0

nonmatching D_8009B4BC

dlabel D_8009B4BC
    /* 8BCBC 8009B4BC 29010000 */ .word 0x00000129
    /* 8BCC0 8009B4C0 A0002E0C */ .word 0x0C2E00A0
    /* 8BCC4 8009B4C4 29010000 */ .word 0x00000129
    /* 8BCC8 8009B4C8 CE002E0C */ .word 0x0C2E00CE
    /* 8BCCC 8009B4CC 29010000 */ .word 0x00000129
    /* 8BCD0 8009B4D0 A00C2E0C */ .word 0x0C2E0CA0
    /* 8BCD4 8009B4D4 29010000 */ .word 0x00000129
    /* 8BCD8 8009B4D8 CE0C2E0C */ .word 0x0C2E0CCE
    /* 8BCDC 8009B4DC 29010000 */ .word 0x00000129
    /* 8BCE0 8009B4E0 A0182E0C */ .word 0x0C2E18A0
enddlabel D_8009B4BC

nonmatching D_8009B4E4

dlabel D_8009B4E4
    /* 8BCE4 8009B4E4 1E000100 */ .word 0x0001001E
    /* 8BCE8 8009B4E8 8003FE01 */ .word 0x01FE0380
    /* 8BCEC 8009B4EC 00000000 */ .word 0x00000000
enddlabel D_8009B4E4

nonmatching D_8009B4F0

dlabel D_8009B4F0
    /* 8BCF0 8009B4F0 1E000100 */ .word 0x0001001E
    /* 8BCF4 8009B4F4 8003FF01 */ .word 0x01FF0380
    /* 8BCF8 8009B4F8 00000000 */ .word 0x00000000
enddlabel D_8009B4F0

nonmatching D_8009B4FC

dlabel D_8009B4FC
    /* 8BCFC 8009B4FC 10010000 */ .word 0x00000110
    /* 8BD00 8009B500 A0511A10 */ .word 0x101A51A0
enddlabel D_8009B4FC

nonmatching D_8009B504

dlabel D_8009B504
    /* 8BD04 8009B504 10010000 */ .word 0x00000110
    /* 8BD08 8009B508 A0611A10 */ .word 0x101A61A0
enddlabel D_8009B504

nonmatching D_8009B50C

dlabel D_8009B50C
    /* 8BD0C 8009B50C 10010000 */ .word 0x00000110
    /* 8BD10 8009B510 A0711A10 */ .word 0x101A71A0
enddlabel D_8009B50C

nonmatching D_8009B514

dlabel D_8009B514
    /* 8BD14 8009B514 38010000 */ .word 0x00000138
    /* 8BD18 8009B518 A0811010 */ .word 0x101081A0
enddlabel D_8009B514

nonmatching D_8009B51C

dlabel D_8009B51C
    /* 8BD1C 8009B51C 00000000 */ .word 0x00000000
    /* 8BD20 8009B520 BA3C140A */ .word 0x0A143CBA
enddlabel D_8009B51C

nonmatching D_8009B524

dlabel D_8009B524
    /* 8BD24 8009B524 1E000300 */ .word 0x0003001E
    /* 8BD28 8009B528 8003FE01 */ .word 0x01FE0380
    /* 8BD2C 8009B52C 00000000 */ .word 0x00000000
enddlabel D_8009B524

nonmatching D_8009B530

dlabel D_8009B530
    /* 8BD30 8009B530 1E000200 */ .word 0x0002001E
    /* 8BD34 8009B534 B003FF01 */ .word 0x01FF03B0
    /* 8BD38 8009B538 8600A000 */ .word 0x00A00086
enddlabel D_8009B530

nonmatching D_8009B53C

dlabel D_8009B53C
    /* 8BD3C 8009B53C 1E000300 */ .word 0x0003001E
    /* 8BD40 8009B540 8003FE01 */ .word 0x01FE0380
    /* 8BD44 8009B544 00002000 */ .word 0x00200000
enddlabel D_8009B53C

nonmatching D_8009B548

dlabel D_8009B548
    /* 8BD48 8009B548 1E000200 */ .word 0x0002001E
    /* 8BD4C 8009B54C B003FF01 */ .word 0x01FF03B0
    /* 8BD50 8009B550 8600AA00 */ .word 0x00AA0086
enddlabel D_8009B548

nonmatching D_8009B554

dlabel D_8009B554
    /* 8BD54 8009B554 5D001C00 */ .word 0x001C005D
    /* 8BD58 8009B558 00A04020 */ .word 0x2040A000
    /* 8BD5C 8009B55C 9D001C00 */ .word 0x001C009D
    /* 8BD60 8009B560 40A04020 */ .word 0x2040A040
    /* 8BD64 8009B564 DD001C00 */ .word 0x001C00DD
    /* 8BD68 8009B568 80A00620 */ .word 0x2006A080
enddlabel D_8009B554

nonmatching D_8009B56C

dlabel D_8009B56C
    /* 8BD6C 8009B56C 65001200 */ .word 0x00120065
    /* 8BD70 8009B570 0000400A */ .word 0x0A400000
    /* 8BD74 8009B574 A5001200 */ .word 0x001200A5
    /* 8BD78 8009B578 4000350A */ .word 0x0A350040
enddlabel D_8009B56C

nonmatching D_8009B57C

dlabel D_8009B57C
    /* 8BD7C 8009B57C 7B001200 */ .word 0x0012007B
    /* 8BD80 8009B580 0000400A */ .word 0x0A400000
    /* 8BD84 8009B584 BB001200 */ .word 0x001200BB
    /* 8BD88 8009B588 4000090A */ .word 0x0A090040
enddlabel D_8009B57C

nonmatching D_8009B58C

dlabel D_8009B58C
    /* 8BD8C 8009B58C */ .byte 0x7B
    /* 8BD8D 8009B58D */ .byte 0x7B
    /* 8BD8E 8009B58E */ .byte 0x7F
    /* 8BD8F 8009B58F */ .byte 0x75
    /* 8BD90 8009B590 */ .byte 0x66
    /* 8BD91 8009B591 */ .byte 0x77
    /* 8BD92 8009B592 */ .byte 0x78
    /* 8BD93 8009B593 */ .byte 0x64
    /* 8BD94 8009B594 */ .byte 0x60
    /* 8BD95 8009B595 */ .byte 0x67
    /* 8BD96 8009B596 */ .byte 0x74
    /* 8BD97 8009B597 */ .byte 0x88
    /* 8BD98 8009B598 */ .byte 0x95
    /* 8BD99 8009B599 */ .byte 0x79
    /* 8BD9A 8009B59A */ .byte 0x86
    /* 8BD9B 8009B59B */ .byte 0x63
    /* 8BD9C 8009B59C */ .byte 0x7C
    /* 8BD9D 8009B59D */ .byte 0x76
    /* 8BD9E 8009B59E */ .byte 0x66
    /* 8BD9F 8009B59F */ .byte 0x77
enddlabel D_8009B58C

nonmatching D_8009B5A0

dlabel D_8009B5A0
    /* 8BDA0 8009B5A0 1E000100 */ .word 0x0001001E
    /* 8BDA4 8009B5A4 A003FE01 */ .word 0x01FE03A0
    /* 8BDA8 8009B5A8 00000000 */ .word 0x00000000
    /* 8BDAC 8009B5AC 1E000100 */ .word 0x0001001E
    /* 8BDB0 8009B5B0 B003FE01 */ .word 0x01FE03B0
    /* 8BDB4 8009B5B4 00000000 */ .word 0x00000000
enddlabel D_8009B5A0

nonmatching D_8009B5B8

dlabel D_8009B5B8
    /* 8BDB8 8009B5B8 51001600 */ .word 0x00160051
    /* 8BDBC 8009B5BC BA241C0E */ .word 0x0E1C24BA
    /* 8BDC0 8009B5C0 51001600 */ .word 0x00160051
    /* 8BDC4 8009B5C4 D6240E0E */ .word 0x0E0E24D6
    /* 8BDC8 8009B5C8 51002E00 */ .word 0x002E0051
    /* 8BDCC 8009B5CC BA32140A */ .word 0x0A1432BA
    /* 8BDD0 8009B5D0 51002E00 */ .word 0x002E0051
    /* 8BDD4 8009B5D4 CE320C0A */ .word 0x0A0C32CE
enddlabel D_8009B5B8

nonmatching D_8009B5D8

dlabel D_8009B5D8
    /* 8BDD8 8009B5D8 0F001700 */ .word 0x0017000F
    /* 8BDDC 8009B5DC C01C3C0E */ .word 0x0E3C1CC0
    /* 8BDE0 8009B5E0 0F002A00 */ .word 0x002A000F
    /* 8BDE4 8009B5E4 C02A400E */ .word 0x0E402AC0
enddlabel D_8009B5D8

nonmatching D_8009B5E8

dlabel D_8009B5E8
    /* 8BDE8 8009B5E8 00000000 */ .word 0x00000000
    /* 8BDEC 8009B5EC FC000405 */ .word 0x050400FC
enddlabel D_8009B5E8

nonmatching D_8009B5F0

dlabel D_8009B5F0
    /* 8BDF0 8009B5F0 38020000 */ .word 0x00000238
    /* 8BDF4 8009B5F4 C0DB2301 */ .word 0x0123DBC0
enddlabel D_8009B5F0

nonmatching D_8009B5F8

dlabel D_8009B5F8
    /* 8BDF8 8009B5F8 5B020000 */ .word 0x0000025B
    /* 8BDFC 8009B5FC E3DB1D01 */ .word 0x011DDBE3
    /* 8BE00 8009B600 26000000 */ .word 0x00000026
    /* 8BE04 8009B604 DEDA2201 */ .word 0x0122DADE
    /* 8BE08 8009B608 08000000 */ .word 0x00000008
    /* 8BE0C 8009B60C C0DA1E01 */ .word 0x011EDAC0
enddlabel D_8009B5F8

nonmatching D_8009B610

dlabel D_8009B610
    /* 8BE10 8009B610 1F000100 */ .word 0x0001001F
    /* 8BE14 8009B614 C003FC01 */ .word 0x01FC03C0
    /* 8BE18 8009B618 00000000 */ .word 0x00000000
    /* 8BE1C 8009B61C 1E000100 */ .word 0x0001001E
    /* 8BE20 8009B620 9003FE01 */ .word 0x01FE0390
    /* 8BE24 8009B624 00000000 */ .word 0x00000000
    /* 8BE28 8009B628 1E000100 */ .word 0x0001001E
    /* 8BE2C 8009B62C 9003FF01 */ .word 0x01FF0390
    /* 8BE30 8009B630 00005000 */ .word 0x00500000
enddlabel D_8009B610

nonmatching D_8009B634

dlabel D_8009B634
    /* 8BE34 8009B634 A0004B00 */ .word 0x004B00A0
    /* 8BE38 8009B638 00009F50 */ .word 0x509F0000
enddlabel D_8009B634

nonmatching D_8009B63C

dlabel D_8009B63C
    /* 8BE3C 8009B63C 1F000200 */ .word 0x0002001F
    /* 8BE40 8009B640 E003FE01 */ .word 0x01FE03E0
    /* 8BE44 8009B644 A0003800 */ .word 0x003800A0
    /* 8BE48 8009B648 1E000100 */ .word 0x0001001E
    /* 8BE4C 8009B64C A003FF01 */ .word 0x01FF03A0
    /* 8BE50 8009B650 00000000 */ .word 0x00000000
    /* 8BE54 8009B654 1E000400 */ .word 0x0004001E
    /* 8BE58 8009B658 A003FF01 */ .word 0x01FF03A0
    /* 8BE5C 8009B65C 00000000 */ .word 0x00000000
enddlabel D_8009B63C

nonmatching D_8009B660

dlabel D_8009B660
    /* 8BE60 8009B660 E6009B00 */ .word 0x009B00E6
    /* 8BE64 8009B664 0000400A */ .word 0x0A400000
    /* 8BE68 8009B668 26019B00 */ .word 0x009B0126
    /* 8BE6C 8009B66C 40001A0A */ .word 0x0A1A0040
enddlabel D_8009B660

nonmatching D_8009B670

dlabel D_8009B670
    /* 8BE70 8009B670 07019B00 */ .word 0x009B0107
    /* 8BE74 8009B674 86BE380A */ .word 0x0A38BE86
enddlabel D_8009B670

nonmatching D_8009B678

dlabel D_8009B678
    /* 8BE78 8009B678 A5009B00 */ .word 0x009B00A5
    /* 8BE7C 8009B67C 86C8400A */ .word 0x0A40C886
    /* 8BE80 8009B680 E5009B00 */ .word 0x009B00E5
    /* 8BE84 8009B684 C6C8340A */ .word 0x0A34C8C6
    /* 8BE88 8009B688 19019B00 */ .word 0x009B0119
    /* 8BE8C 8009B68C B2BE0C0A */ .word 0x0A0CBEB2
    /* 8BE90 8009B690 25019B00 */ .word 0x009B0125
    /* 8BE94 8009B694 C7BE1B0A */ .word 0x0A1BBEC7
enddlabel D_8009B678

nonmatching D_8009B698

dlabel D_8009B698
    /* 8BE98 8009B698 0E000800 */ .word 0x0008000E
    /* 8BE9C 8009B69C E003FF01 */ .word 0x01FF03E0
    /* 8BEA0 8009B6A0 00000000 */ .word 0x00000000
    /* 8BEA4 8009B6A4 0E000800 */ .word 0x0008000E
    /* 8BEA8 8009B6A8 E003FF01 */ .word 0x01FF03E0
    /* 8BEAC 8009B6AC 00008000 */ .word 0x00800000
enddlabel D_8009B698

nonmatching D_8009B6B0

dlabel D_8009B6B0
    /* 8BEB0 8009B6B0 40003800 */ .word 0x00380040
    /* 8BEB4 8009B6B4 00004040 */ .word 0x40400000
    /* 8BEB8 8009B6B8 80003800 */ .word 0x00380080
    /* 8BEBC 8009B6BC 40004040 */ .word 0x40400040
    /* 8BEC0 8009B6C0 C0003800 */ .word 0x003800C0
    /* 8BEC4 8009B6C4 80004040 */ .word 0x40400080
    /* 8BEC8 8009B6C8 00013800 */ .word 0x00380100
    /* 8BECC 8009B6CC C0004040 */ .word 0x404000C0
    /* 8BED0 8009B6D0 40013800 */ .word 0x00380140
    /* 8BED4 8009B6D4 00404040 */ .word 0x40404000
    /* 8BED8 8009B6D8 80013800 */ .word 0x00380180
    /* 8BEDC 8009B6DC 40404040 */ .word 0x40404040
    /* 8BEE0 8009B6E0 C0013800 */ .word 0x003801C0
    /* 8BEE4 8009B6E4 80404040 */ .word 0x40404080
    /* 8BEE8 8009B6E8 00023800 */ .word 0x00380200
    /* 8BEEC 8009B6EC C0404040 */ .word 0x404040C0
enddlabel D_8009B6B0

nonmatching D_8009B6F0

dlabel D_8009B6F0
    /* 8BEF0 8009B6F0 1E000300 */ .word 0x0003001E
    /* 8BEF4 8009B6F4 B003FE01 */ .word 0x01FE03B0
    /* 8BEF8 8009B6F8 00000000 */ .word 0x00000000
enddlabel D_8009B6F0

nonmatching D_8009B6FC

dlabel D_8009B6FC
    /* 8BEFC 8009B6FC 1E000100 */ .word 0x0001001E
    /* 8BF00 8009B700 B003FE01 */ .word 0x01FE03B0
    /* 8BF04 8009B704 C0002800 */ .word 0x002800C0
enddlabel D_8009B6FC

nonmatching D_8009B708

dlabel D_8009B708
    /* 8BF08 8009B708 00006400 */ .word 0x00640000
    /* 8BF0C 8009B70C 00002028 */ .word 0x28200000
    /* 8BF10 8009B710 00006400 */ .word 0x00640000
    /* 8BF14 8009B714 20002028 */ .word 0x28200020
    /* 8BF18 8009B718 00006400 */ .word 0x00640000
    /* 8BF1C 8009B71C 00282028 */ .word 0x28202800
    /* 8BF20 8009B720 00006400 */ .word 0x00640000
    /* 8BF24 8009B724 20282028 */ .word 0x28202820
    /* 8BF28 8009B728 00006400 */ .word 0x00640000
    /* 8BF2C 8009B72C 00502028 */ .word 0x28205000
    /* 8BF30 8009B730 00006400 */ .word 0x00640000
    /* 8BF34 8009B734 20502028 */ .word 0x28205020
    /* 8BF38 8009B738 00006400 */ .word 0x00640000
    /* 8BF3C 8009B73C 00782028 */ .word 0x28207800
    /* 8BF40 8009B740 00006400 */ .word 0x00640000
    /* 8BF44 8009B744 20782028 */ .word 0x28207820
    /* 8BF48 8009B748 00006400 */ .word 0x00640000
    /* 8BF4C 8009B74C 00A02028 */ .word 0x2820A000
    /* 8BF50 8009B750 00006400 */ .word 0x00640000
    /* 8BF54 8009B754 20A02028 */ .word 0x2820A020
enddlabel D_8009B708

nonmatching D_8009B758

dlabel D_8009B758
    /* 8BF58 8009B758 00006800 */ .word 0x00680000
    /* 8BF5C 8009B75C 0000C028 */ .word 0x28C00000
    /* 8BF60 8009B760 C0006800 */ .word 0x006800C0
    /* 8BF64 8009B764 C0004028 */ .word 0x284000C0
    /* 8BF68 8009B768 00016800 */ .word 0x00680100
    /* 8BF6C 8009B76C 00288828 */ .word 0x28882800
enddlabel D_8009B758

nonmatching D_8009B770

dlabel D_8009B770
    /* 8BF70 8009B770 1E000500 */ .word 0x0005001E
    /* 8BF74 8009B774 9003FE01 */ .word 0x01FE0390
    /* 8BF78 8009B778 B4000000 */ .word 0x000000B4
    /* 8BF7C 8009B77C 1E000400 */ .word 0x0004001E
    /* 8BF80 8009B780 9003FF01 */ .word 0x01FF0390
    /* 8BF84 8009B784 80007F00 */ .word 0x007F0080
    /* 8BF88 8009B788 1E000400 */ .word 0x0004001E
    /* 8BF8C 8009B78C 8003FF01 */ .word 0x01FF0380
    /* 8BF90 8009B790 00007F00 */ .word 0x007F0000
dlabel D_8009B794
    /* 8BF94 8009B794 */ .short 0x001E
    /* 8BF96 8009B796 */ .short 0x0001
    /* 8BF98 8009B798 */ .short 0x0380
    /* 8BF9A 8009B79A */ .short 0x01FE
    /* 8BF9C 8009B79C */ .short 0x0000
    /* 8BF9E 8009B79E */ .short 0x0000
enddlabel D_8009B770

nonmatching D_8009B7A0

dlabel D_8009B7A0
    /* 8BFA0 8009B7A0 1F000400 */ .word 0x0004001F
    /* 8BFA4 8009B7A4 C003FF01 */ .word 0x01FF03C0
    /* 8BFA8 8009B7A8 00008000 */ .word 0x00800000
enddlabel D_8009B7A0

nonmatching D_8009B7AC

dlabel D_8009B7AC
    /* 8BFAC 8009B7AC 1E000100 */ .word 0x0001001E
    /* 8BFB0 8009B7B0 A003FE01 */ .word 0x01FE03A0
    /* 8BFB4 8009B7B4 B4007100 */ .word 0x007100B4
enddlabel D_8009B7AC

nonmatching D_8009B7B8

dlabel D_8009B7B8
    /* 8BFB8 8009B7B8 1E000100 */ .word 0x0001001E
    /* 8BFBC 8009B7BC B003FE01 */ .word 0x01FE03B0
    /* 8BFC0 8009B7C0 C8007100 */ .word 0x007100C8
enddlabel D_8009B7B8

nonmatching D_8009B7C4

dlabel D_8009B7C4
    /* 8BFC4 8009B7C4 1E000100 */ .word 0x0001001E
    /* 8BFC8 8009B7C8 A003FF01 */ .word 0x01FF03A0
    /* 8BFCC 8009B7CC DC007100 */ .word 0x007100DC
enddlabel D_8009B7C4

nonmatching D_8009B7D0

dlabel D_8009B7D0
    /* 8BFD0 8009B7D0 41003400 */ .word 0x00340041
    /* 8BFD4 8009B7D4 0000B37F */ .word 0x7FB30000
enddlabel D_8009B7D0

nonmatching D_8009B7D8

dlabel D_8009B7D8
    /* 8BFD8 8009B7D8 41001E00 */ .word 0x001E0041
    /* 8BFDC 8009B7DC 00004010 */ .word 0x10400000
    /* 8BFE0 8009B7E0 81001E00 */ .word 0x001E0081
    /* 8BFE4 8009B7E4 40000C10 */ .word 0x100C0040
    /* 8BFE8 8009B7E8 8D001E00 */ .word 0x001E008D
    /* 8BFEC 8009B7EC 00104010 */ .word 0x10401000
    /* 8BFF0 8009B7F0 CD001E00 */ .word 0x001E00CD
    /* 8BFF4 8009B7F4 40100C10 */ .word 0x100C1040
    /* 8BFF8 8009B7F8 D9001E00 */ .word 0x001E00D9
    /* 8BFFC 8009B7FC 00204010 */ .word 0x10402000
enddlabel D_8009B7D8

nonmatching D_8009B800

dlabel D_8009B800
    /* 8C000 8009B800 BF013400 */ .word 0x003401BF
    /* 8C004 8009B804 00004040 */ .word 0x40400000
    /* 8C008 8009B808 FF013400 */ .word 0x003401FF
    /* 8C00C 8009B80C 40004040 */ .word 0x40400040
    /* 8C010 8009B810 BF017400 */ .word 0x007401BF
    /* 8C014 8009B814 0040403F */ .word 0x3F404000
    /* 8C018 8009B818 FF017400 */ .word 0x007401FF
    /* 8C01C 8009B81C 4040403F */ .word 0x3F404040
enddlabel D_8009B800

nonmatching D_8009B820

dlabel D_8009B820
    /* 8C020 8009B820 3F01C100 */ .word 0x00C1013F
    /* 8C024 8009B824 00004016 */ .word 0x16400000
    /* 8C028 8009B828 7F01C100 */ .word 0x00C1017F
    /* 8C02C 8009B82C 40004016 */ .word 0x16400040
    /* 8C030 8009B830 BF01C100 */ .word 0x00C101BF
    /* 8C034 8009B834 80004016 */ .word 0x16400080
    /* 8C038 8009B838 FF01C100 */ .word 0x00C101FF
    /* 8C03C 8009B83C C0004016 */ .word 0x164000C0
enddlabel D_8009B820

nonmatching D_8009B840

dlabel D_8009B840
    /* 8C040 8009B840 3F02D600 */ .word 0x00D6023F
    /* 8C044 8009B844 C0DB2301 */ .word 0x0123DBC0
    /* 8C048 8009B848 6202D600 */ .word 0x00D60262
    /* 8C04C 8009B84C E3DB1D01 */ .word 0x011DDBE3
enddlabel D_8009B840

nonmatching D_8009B850

dlabel D_8009B850
    /* 8C050 8009B850 B8A1D2A8 */ .word 0xA8D2A1B8
    /* 8C054 8009B854 3811B6A5 */ .word 0xA5B61138
    /* 8C058 8009B858 64964593 */ .word 0x93459664
    /* 8C05C 8009B85C 429FB581 */ .word 0x81B59F42
    /* 8C060 8009B860 57A39367 */ .word 0x6793A357
    /* 8C064 8009B864 33479951 */ .word 0x51994733
    /* 8C068 8009B868 5517BE8C */ .word 0x8CBE1755
    /* 8C06C 8009B86C CC8EE678 */ .word 0x78E68ECC
    /* 8C070 8009B870 50773C67 */ .word 0x673C7750
    /* 8C074 8009B874 D9504649 */ .word 0x494650D9
    /* 8C078 8009B878 8C48EA2F */ .word 0x2FEA488C
    /* 8C07C 8009B87C 46213E13 */ .word 0x133E2146
    /* 8C080 8009B880 99A49483 */ .word 0x8394A499
    /* 8C084 8009B884 477A5A69 */ .word 0x695A7A47
    /* 8C088 8009B888 F447AB5A */ .word 0x5AAB47F4
    /* 8C08C 8009B88C 3458C60C */ .word 0x0CC65834
enddlabel D_8009B850

nonmatching D_8009B890

dlabel D_8009B890
    /* 8C090 8009B890 8003FF00 */ .word 0x00FF0380
    /* 8C094 8009B894 A000E000 */ .word 0x00E000A0
    /* 8C098 8009B898 8003FF00 */ .word 0x00FF0380
    /* 8C09C 8009B89C A000E800 */ .word 0x00E800A0
    /* 8C0A0 8009B8A0 8003FF00 */ .word 0x00FF0380
    /* 8C0A4 8009B8A4 A000E800 */ .word 0x00E800A0
    /* 8C0A8 8009B8A8 8003FF00 */ .word 0x00FF0380
    /* 8C0AC 8009B8AC A000E800 */ .word 0x00E800A0
enddlabel D_8009B890

nonmatching D_8009B8B0

dlabel D_8009B8B0
    /* 8C0B0 8009B8B0 8003FF00 */ .word 0x00FF0380
    /* 8C0B4 8009B8B4 A000E800 */ .word 0x00E800A0
    /* 8C0B8 8009B8B8 8003FF00 */ .word 0x00FF0380
    /* 8C0BC 8009B8BC A000E800 */ .word 0x00E800A0
    /* 8C0C0 8009B8C0 8003FF00 */ .word 0x00FF0380
    /* 8C0C4 8009B8C4 A000E800 */ .word 0x00E800A0
enddlabel D_8009B8B0

nonmatching D_8009B8C8

dlabel D_8009B8C8
    /* 8C0C8 8009B8C8 8003FC00 */ .word 0x00FC0380
    /* 8C0CC 8009B8CC 00000000 */ .word 0x00000000
enddlabel D_8009B8C8

nonmatching D_8009B8D0

dlabel D_8009B8D0
    /* 8C0D0 8009B8D0 9003FC00 */ .word 0x00FC0390
    /* 8C0D4 8009B8D4 40000000 */ .word 0x00000040
enddlabel D_8009B8D0

nonmatching D_8009B8D8

dlabel D_8009B8D8
    /* 8C0D8 8009B8D8 A003FC00 */ .word 0x00FC03A0
    /* 8C0DC 8009B8DC 40004000 */ .word 0x00400040
enddlabel D_8009B8D8

nonmatching D_8009B8E0

dlabel D_8009B8E0
    /* 8C0E0 8009B8E0 B003FC00 */ .word 0x00FC03B0
    /* 8C0E4 8009B8E4 40008000 */ .word 0x00800040
enddlabel D_8009B8E0

nonmatching D_8009B8E8

dlabel D_8009B8E8
    /* 8C0E8 8009B8E8 9003FF00 */ .word 0x00FF0390
    /* 8C0EC 8009B8EC C0000000 */ .word 0x000000C0
    /* 8C0F0 8009B8F0 9003FF00 */ .word 0x00FF0390
    /* 8C0F4 8009B8F4 C0002000 */ .word 0x002000C0
    /* 8C0F8 8009B8F8 9003FF00 */ .word 0x00FF0390
    /* 8C0FC 8009B8FC C0004000 */ .word 0x004000C0
    /* 8C100 8009B900 9003FF00 */ .word 0x00FF0390
    /* 8C104 8009B904 C0006000 */ .word 0x006000C0
    /* 8C108 8009B908 9003FF00 */ .word 0x00FF0390
    /* 8C10C 8009B90C C0008000 */ .word 0x008000C0
    /* 8C110 8009B910 9003FF00 */ .word 0x00FF0390
    /* 8C114 8009B914 C000A000 */ .word 0x00A000C0
    /* 8C118 8009B918 9003FF00 */ .word 0x00FF0390
    /* 8C11C 8009B91C C000C000 */ .word 0x00C000C0
enddlabel D_8009B8E8

nonmatching D_8009B920

dlabel D_8009B920
    /* 8C120 8009B920 9003FE00 */ .word 0x00FE0390
    /* 8C124 8009B924 8000E000 */ .word 0x00E00080
    /* 8C128 8009B928 9003FE00 */ .word 0x00FE0390
    /* 8C12C 8009B92C 9000E000 */ .word 0x00E00090
    /* 8C130 8009B930 A003FE00 */ .word 0x00FE03A0
    /* 8C134 8009B934 9800E000 */ .word 0x00E00098
    /* 8C138 8009B938 B003FE00 */ .word 0x00FE03B0
    /* 8C13C 8009B93C 8800E000 */ .word 0x00E00088
enddlabel D_8009B920

nonmatching D_8009B940

dlabel D_8009B940
    /* 8C140 8009B940 A003FF00 */ .word 0x00FF03A0
    /* 8C144 8009B944 80000000 */ .word 0x00000080
enddlabel D_8009B940

nonmatching D_8009B948

dlabel D_8009B948
    /* 8C148 8009B948 A003FF00 */ .word 0x00FF03A0
    /* 8C14C 8009B94C 80004000 */ .word 0x00400080
enddlabel D_8009B948

nonmatching D_8009B950

dlabel D_8009B950
    /* 8C150 8009B950 A003FF00 */ .word 0x00FF03A0
    /* 8C154 8009B954 80008000 */ .word 0x00800080
enddlabel D_8009B950

nonmatching D_8009B958

dlabel D_8009B958
    /* 8C158 8009B958 A003FD00 */ .word 0x00FD03A0
    /* 8C15C 8009B95C 0000C000 */ .word 0x00C00000
enddlabel D_8009B958

nonmatching D_8009B960

dlabel D_8009B960
    /* 8C160 8009B960 A003FD00 */ .word 0x00FD03A0
    /* 8C164 8009B964 4000C000 */ .word 0x00C00040
enddlabel D_8009B960

nonmatching D_8009B968

dlabel D_8009B968
    /* 8C168 8009B968 A003FD00 */ .word 0x00FD03A0
    /* 8C16C 8009B96C 2000C000 */ .word 0x00C00020
enddlabel D_8009B968

nonmatching D_8009B970

dlabel D_8009B970
    /* 8C170 8009B970 A003FD00 */ .word 0x00FD03A0
    /* 8C174 8009B974 8000C000 */ .word 0x00C00080
enddlabel D_8009B970

nonmatching D_8009B978

dlabel D_8009B978
    /* 8C178 8009B978 8003FD00 */ .word 0x00FD0380
    /* 8C17C 8009B97C 00004000 */ .word 0x00400000
enddlabel D_8009B978

nonmatching D_8009B980

dlabel D_8009B980
    /* 8C180 8009B980 9003FD00 */ .word 0x00FD0390
    /* 8C184 8009B984 00008000 */ .word 0x00800000
enddlabel D_8009B980

nonmatching D_8009B988

dlabel D_8009B988
    /* 8C188 8009B988 B003FD00 */ .word 0x00FD03B0
    /* 8C18C 8009B98C 0000E000 */ .word 0x00E00000
enddlabel D_8009B988

nonmatching D_8009B990

dlabel D_8009B990
    /* 8C190 8009B990 8003FE00 */ .word 0x00FE0380
    /* 8C194 8009B994 4000E000 */ .word 0x00E00040
enddlabel D_8009B990

nonmatching D_8009B998

dlabel D_8009B998
    /* 8C198 8009B998 E003FC00 */ .word 0x00FC03E0
    /* 8C19C 8009B99C C0004000 */ .word 0x004000C0
    /* 8C1A0 8009B9A0 E003FC00 */ .word 0x00FC03E0
    /* 8C1A4 8009B9A4 E0004000 */ .word 0x004000E0
    /* 8C1A8 8009B9A8 E003FC00 */ .word 0x00FC03E0
    /* 8C1AC 8009B9AC C0006000 */ .word 0x006000C0
    /* 8C1B0 8009B9B0 E003FC00 */ .word 0x00FC03E0
    /* 8C1B4 8009B9B4 E0006000 */ .word 0x006000E0
enddlabel D_8009B998

nonmatching D_8009B9B8

dlabel D_8009B9B8
    /* 8C1B8 8009B9B8 D003FC00 */ .word 0x00FC03D0
    /* 8C1BC 8009B9BC C0000000 */ .word 0x000000C0
    /* 8C1C0 8009B9C0 D003FC00 */ .word 0x00FC03D0
    /* 8C1C4 8009B9C4 E0000000 */ .word 0x000000E0
    /* 8C1C8 8009B9C8 D003FC00 */ .word 0x00FC03D0
    /* 8C1CC 8009B9CC C0002000 */ .word 0x002000C0
    /* 8C1D0 8009B9D0 D003FC00 */ .word 0x00FC03D0
    /* 8C1D4 8009B9D4 E0002000 */ .word 0x002000E0
enddlabel D_8009B9B8

nonmatching D_8009B9D8

dlabel D_8009B9D8
    /* 8C1D8 8009B9D8 C003FC00 */ .word 0x00FC03C0
    /* 8C1DC 8009B9DC 00000000 */ .word 0x00000000
enddlabel D_8009B9D8

nonmatching D_8009B9E0

dlabel D_8009B9E0
    /* 8C1E0 8009B9E0 C003FC00 */ .word 0x00FC03C0
    /* 8C1E4 8009B9E4 40000000 */ .word 0x00000040
enddlabel D_8009B9E0

nonmatching D_8009B9E8

dlabel D_8009B9E8
    /* 8C1E8 8009B9E8 E003FD00 */ .word 0x00FD03E0
    /* 8C1EC 8009B9EC 4000B400 */ .word 0x00B40040
enddlabel D_8009B9E8

nonmatching D_8009B9F0

dlabel D_8009B9F0
    /* 8C1F0 8009B9F0 E003FD00 */ .word 0x00FD03E0
    /* 8C1F4 8009B9F4 0000B400 */ .word 0x00B40000
    /* 8C1F8 8009B9F8 C003FC00 */ .word 0x00FC03C0
    /* 8C1FC 8009B9FC 00000000 */ .word 0x00000000
enddlabel D_8009B9F0

nonmatching D_8009BA00

dlabel D_8009BA00
    /* 8C200 8009BA00 C003FD00 */ .word 0x00FD03C0
    /* 8C204 8009BA04 00009400 */ .word 0x00940000
    /* 8C208 8009BA08 C003FD00 */ .word 0x00FD03C0
    /* 8C20C 8009BA0C 20009400 */ .word 0x00940020
    /* 8C210 8009BA10 C003FD00 */ .word 0x00FD03C0
    /* 8C214 8009BA14 40009400 */ .word 0x00940040
    /* 8C218 8009BA18 C003FD00 */ .word 0x00FD03C0
    /* 8C21C 8009BA1C 60009400 */ .word 0x00940060
    /* 8C220 8009BA20 C003FD00 */ .word 0x00FD03C0
    /* 8C224 8009BA24 80009400 */ .word 0x00940080
    /* 8C228 8009BA28 C003FD00 */ .word 0x00FD03C0
    /* 8C22C 8009BA2C A0009400 */ .word 0x009400A0
enddlabel D_8009BA00

nonmatching D_8009BA30

dlabel D_8009BA30
    /* 8C230 8009BA30 F003FC00 */ .word 0x00FC03F0
    /* 8C234 8009BA34 00008000 */ .word 0x00800000
    /* 8C238 8009BA38 F003FC00 */ .word 0x00FC03F0
    /* 8C23C 8009BA3C 10008000 */ .word 0x00800010
    /* 8C240 8009BA40 F003FC00 */ .word 0x00FC03F0
    /* 8C244 8009BA44 20008000 */ .word 0x00800020
    /* 8C248 8009BA48 F003FC00 */ .word 0x00FC03F0
    /* 8C24C 8009BA4C 30008000 */ .word 0x00800030
enddlabel D_8009BA30

nonmatching D_8009BA50

dlabel D_8009BA50
    /* 8C250 8009BA50 C003FE00 */ .word 0x00FE03C0
    /* 8C254 8009BA54 00009400 */ .word 0x00940000
enddlabel D_8009BA50

nonmatching D_8009BA58

dlabel D_8009BA58
    /* 8C258 8009BA58 F003FD00 */ .word 0x00FD03F0
    /* 8C25C 8009BA5C 30008000 */ .word 0x00800030
enddlabel D_8009BA58

nonmatching D_8009BA60

dlabel D_8009BA60
    /* 8C260 8009BA60 */ .byte 0x00
    /* 8C261 8009BA61 */ .byte 0x02
    /* 8C262 8009BA62 */ .byte 0x04
    /* 8C263 8009BA63 */ .byte 0x07
    /* 8C264 8009BA64 */ .byte 0x0A
    /* 8C265 8009BA65 */ .byte 0x0C
    /* 8C266 8009BA66 */ .byte 0x0E
    /* 8C267 8009BA67 */ .byte 0x10
    /* 8C268 8009BA68 */ .byte 0x12
    /* 8C269 8009BA69 */ .byte 0x14
    /* 8C26A 8009BA6A */ .byte 0x16
    /* 8C26B 8009BA6B */ .byte 0x18
    /* 8C26C 8009BA6C */ .byte 0x1A
    /* 8C26D 8009BA6D */ .byte 0x1C
    /* 8C26E 8009BA6E */ .byte 0x1E
    /* 8C26F 8009BA6F */ .byte 0x20
    /* 8C270 8009BA70 */ .byte 0x22
    /* 8C271 8009BA71 */ .byte 0x24
    /* 8C272 8009BA72 */ .byte 0x26
    /* 8C273 8009BA73 */ .byte 0x28
    /* 8C274 8009BA74 */ .byte 0x2A
    /* 8C275 8009BA75 */ .byte 0x2D
    /* 8C276 8009BA76 */ .byte 0x30
    /* 8C277 8009BA77 */ .byte 0x33
    /* 8C278 8009BA78 */ .byte 0x36
    /* 8C279 8009BA79 */ .byte 0x39
    /* 8C27A 8009BA7A */ .byte 0x3C
    /* 8C27B 8009BA7B */ .byte 0x00
enddlabel D_8009BA60

nonmatching D_8009BA7C

dlabel D_8009BA7C
    /* 8C27C 8009BA7C F03A0680 */ .word func_80063AF0
    /* 8C280 8009BA80 783B0680 */ .word func_80063B78
    /* 8C284 8009BA84 343B0680 */ .word func_80063B34
    /* 8C288 8009BA88 A43B0680 */ .word func_80063BA4
    /* 8C28C 8009BA8C C06E0680 */ .word func_80066EC0
    /* 8C290 8009BA90 F46E0680 */ .word func_80066EF4
    /* 8C294 8009BA94 C8760680 */ .word func_800676C8
    /* 8C298 8009BA98 286F0680 */ .word func_80066F28
    /* 8C29C 8009BA9C 5C6F0680 */ .word func_80066F5C
    /* 8C2A0 8009BAA0 04770680 */ .word func_80067704
    /* 8C2A4 8009BAA4 904E0680 */ .word func_80064E90
    /* 8C2A8 8009BAA8 F4520680 */ .word func_800652F4
    /* 8C2AC 8009BAAC D84E0680 */ .word func_80064ED8
    /* 8C2B0 8009BAB0 44530680 */ .word func_80065344
    /* 8C2B4 8009BAB4 204F0680 */ .word func_80064F20
    /* 8C2B8 8009BAB8 94530680 */ .word func_80065394
    /* 8C2BC 8009BABC B0450680 */ .word eff_SpawnJitteredBillboards
    /* 8C2C0 8009BAC0 E8460680 */ .word func_800646E8
    /* 8C2C4 8009BAC4 EC2F0680 */ .word eff_SpawnBillboardPair
    /* 8C2C8 8009BAC8 84300680 */ .word func_80063084
    /* 8C2CC 8009BACC 684F0680 */ .word func_80064F68
    /* 8C2D0 8009BAD0 E4530680 */ .word func_800653E4
    /* 8C2D4 8009BAD4 B44F0680 */ .word func_80064FB4
    /* 8C2D8 8009BAD8 34540680 */ .word func_80065434
    /* 8C2DC 8009BADC 00500680 */ .word func_80065000
    /* 8C2E0 8009BAE0 84540680 */ .word func_80065484
    /* 8C2E4 8009BAE4 5C500680 */ .word func_8006505C
    /* 8C2E8 8009BAE8 40550680 */ .word func_80065540
    /* 8C2EC 8009BAEC A4500680 */ .word func_800650A4
    /* 8C2F0 8009BAF0 90550680 */ .word func_80065590
    /* 8C2F4 8009BAF4 EC500680 */ .word func_800650EC
    /* 8C2F8 8009BAF8 E0550680 */ .word func_800655E0
    /* 8C2FC 8009BAFC 34510680 */ .word func_80065134
    /* 8C300 8009BB00 30560680 */ .word func_80065630
    /* 8C304 8009BB04 7C510680 */ .word func_8006517C
    /* 8C308 8009BB08 80560680 */ .word func_80065680
    /* 8C30C 8009BB0C F0510680 */ .word func_800651F0
    /* 8C310 8009BB10 EC560680 */ .word func_800656EC
    /* 8C314 8009BB14 64520680 */ .word func_80065264
    /* 8C318 8009BB18 60570680 */ .word func_80065760
    /* 8C31C 8009BB1C AC520680 */ .word func_800652AC
    /* 8C320 8009BB20 B0570680 */ .word func_800657B0
    /* 8C324 8009BB24 906F0680 */ .word func_80066F90
    /* 8C328 8009BB28 C46F0680 */ .word func_80066FC4
    /* 8C32C 8009BB2C 40770680 */ .word func_80067740
    /* 8C330 8009BB30 F86F0680 */ .word func_80066FF8
    /* 8C334 8009BB34 2C700680 */ .word func_8006702C
    /* 8C338 8009BB38 7C770680 */ .word func_8006777C
    /* 8C33C 8009BB3C 60700680 */ .word func_80067060
    /* 8C340 8009BB40 94700680 */ .word func_80067094
    /* 8C344 8009BB44 B8770680 */ .word func_800677B8
    /* 8C348 8009BB48 C8700680 */ .word func_800670C8
    /* 8C34C 8009BB4C FC700680 */ .word func_800670FC
    /* 8C350 8009BB50 F4770680 */ .word func_800677F4
    /* 8C354 8009BB54 30710680 */ .word func_80067130
    /* 8C358 8009BB58 64710680 */ .word func_80067164
    /* 8C35C 8009BB5C 30780680 */ .word func_80067830
    /* 8C360 8009BB60 98710680 */ .word func_80067198
    /* 8C364 8009BB64 CC710680 */ .word func_800671CC
    /* 8C368 8009BB68 6C780680 */ .word func_8006786C
    /* 8C36C 8009BB6C 8C280680 */ .word func_8006288C
    /* 8C370 8009BB70 5C290680 */ .word func_8006295C
enddlabel D_8009BA7C

nonmatching D_8009BB74

dlabel D_8009BB74
    /* 8C374 8009BB74 00100000 */ .word 0x00001000
    /* 8C378 8009BB78 00080000 */ .word 0x00000800
    /* 8C37C 8009BB7C 00100000 */ .word 0x00001000
    /* 8C380 8009BB80 00000000 */ .word 0x00000000
enddlabel D_8009BB74

nonmatching D_8009BB84

dlabel D_8009BB84
    /* 8C384 8009BB84 83FF83FF */ .word 0xFF83FF83
    /* 8C388 8009BB88 00000000 */ .word 0x00000000
    /* 8C38C 8009BB8C 7D0083FF */ .word 0xFF83007D
    /* 8C390 8009BB90 00000000 */ .word 0x00000000
    /* 8C394 8009BB94 83FF7D00 */ .word 0x007DFF83
    /* 8C398 8009BB98 00000000 */ .word 0x00000000
    /* 8C39C 8009BB9C 7D007D00 */ .word 0x007D007D
    /* 8C3A0 8009BBA0 00000000 */ .word 0x00000000
    /* 8C3A4 8009BBA4 83FF0000 */ .word 0x0000FF83
    /* 8C3A8 8009BBA8 00000000 */ .word 0x00000000
    /* 8C3AC 8009BBAC 83FF0000 */ .word 0x0000FF83
    /* 8C3B0 8009BBB0 F4010000 */ .word 0x000001F4
    /* 8C3B4 8009BBB4 7D000000 */ .word 0x0000007D
    /* 8C3B8 8009BBB8 00000000 */ .word 0x00000000
    /* 8C3BC 8009BBBC 7D000000 */ .word 0x0000007D
    /* 8C3C0 8009BBC0 F4010000 */ .word 0x000001F4
    /* 8C3C4 8009BBC4 000083FF */ .word 0xFF830000
    /* 8C3C8 8009BBC8 00000000 */ .word 0x00000000
    /* 8C3CC 8009BBCC 000083FF */ .word 0xFF830000
    /* 8C3D0 8009BBD0 F4010000 */ .word 0x000001F4
    /* 8C3D4 8009BBD4 00007D00 */ .word 0x007D0000
    /* 8C3D8 8009BBD8 00000000 */ .word 0x00000000
    /* 8C3DC 8009BBDC 00007D00 */ .word 0x007D0000
    /* 8C3E0 8009BBE0 F4010000 */ .word 0x000001F4
enddlabel D_8009BB84

nonmatching D_8009BBE4

dlabel D_8009BBE4
    /* 8C3E4 8009BBE4 AEFF0000 */ .word 0x0000FFAE
    /* 8C3E8 8009BBE8 24FF0000 */ .word 0x0000FF24
enddlabel D_8009BBE4

.align 2
nonmatching D_8009BBEC

dlabel D_8009BBEC
    /* 8C3EC 8009BBEC */ .asciz "R"
    /* 52000000 */
.align 2
    /* 8C3F0 8009BBF0 24FF0000 */ .word 0x0000FF24
enddlabel D_8009BBEC

nonmatching D_8009BBF4

dlabel D_8009BBF4
    /* 8C3F4 8009BBF4 AEFF0000 */ .word 0x0000FFAE
    /* 8C3F8 8009BBF8 4E000000 */ .word 0x0000004E
enddlabel D_8009BBF4

.align 2
nonmatching D_8009BBFC

dlabel D_8009BBFC
    /* 8C3FC 8009BBFC */ .asciz "R"
    /* 52000000 */
.align 2
.align 2
    /* 8C400 8009BC00 */ .asciz "N"
    /* 4E000000 */
.align 2
enddlabel D_8009BBFC
