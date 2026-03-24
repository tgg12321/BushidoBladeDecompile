glabel func_8002E6B0
    /* 1EEB0 8002E6B0 E8FFBD27 */  addiu      $sp, $sp, -0x18
    /* 1EEB4 8002E6B4 5555083C */  lui        $t0, (0x55555556 >> 16)
    /* 1EEB8 8002E6B8 56550835 */  ori        $t0, $t0, (0x55555556 & 0xFFFF)
    /* 1EEBC 8002E6BC 1400B5AF */  sw         $s5, 0x14($sp)
    /* 1EEC0 8002E6C0 1000B4AF */  sw         $s4, 0x10($sp)
    /* 1EEC4 8002E6C4 0C00B3AF */  sw         $s3, 0xC($sp)
    /* 1EEC8 8002E6C8 0800B2AF */  sw         $s2, 0x8($sp)
    /* 1EECC 8002E6CC 0400B1AF */  sw         $s1, 0x4($sp)
    /* 1EED0 8002E6D0 0000B0AF */  sw         $s0, 0x0($sp)
    /* 1EED4 8002E6D4 00008A8C */  lw         $t2, 0x0($a0)
    /* 1EED8 8002E6D8 0000AC8C */  lw         $t4, 0x0($a1)
    /* 1EEDC 8002E6DC 0000CE8C */  lw         $t6, 0x0($a2)
    /* 1EEE0 8002E6E0 21104C01 */  addu       $v0, $t2, $t4
    /* 1EEE4 8002E6E4 21104E00 */  addu       $v0, $v0, $t6
    /* 1EEE8 8002E6E8 18004800 */  mult       $v0, $t0
    /* 1EEEC 8002E6EC 0800898C */  lw         $t1, 0x8($a0)
    /* 1EEF0 8002E6F0 0800AB8C */  lw         $t3, 0x8($a1)
    /* 1EEF4 8002E6F4 0800CD8C */  lw         $t5, 0x8($a2)
    /* 1EEF8 8002E6F8 10780000 */  mfhi       $t7
    /* 1EEFC 8002E6FC 21182B01 */  addu       $v1, $t1, $t3
    /* 1EF00 8002E700 21186D00 */  addu       $v1, $v1, $t5
    /* 1EF04 8002E704 18006800 */  mult       $v1, $t0
    /* 1EF08 8002E708 C3170200 */  sra        $v0, $v0, 31
    /* 1EF0C 8002E70C 2388E201 */  subu       $s1, $t7, $v0
    /* 1EF10 8002E710 10280000 */  mfhi       $a1
    /* 1EF14 8002E714 23106901 */  subu       $v0, $t3, $t1
    /* 1EF18 8002E718 23782A02 */  subu       $t7, $s1, $t2
    /* 1EF1C 8002E71C 18004F00 */  mult       $v0, $t7
    /* 1EF20 8002E720 0000F38C */  lw         $s3, 0x0($a3)
    /* 1EF24 8002E724 12C80000 */  mflo       $t9
    /* 1EF28 8002E728 23C06A02 */  subu       $t8, $s3, $t2
    /* 1EF2C 8002E72C 00000000 */  nop
    /* 1EF30 8002E730 18005800 */  mult       $v0, $t8
    /* 1EF34 8002E734 0800F28C */  lw         $s2, 0x8($a3)
    /* 1EF38 8002E738 12300000 */  mflo       $a2
    /* 1EF3C 8002E73C 23404902 */  subu       $t0, $s2, $t1
    /* 1EF40 8002E740 23108A01 */  subu       $v0, $t4, $t2
    /* 1EF44 8002E744 18004800 */  mult       $v0, $t0
    /* 1EF48 8002E748 C31F0300 */  sra        $v1, $v1, 31
    /* 1EF4C 8002E74C 12200000 */  mflo       $a0
    /* 1EF50 8002E750 2380A300 */  subu       $s0, $a1, $v1
    /* 1EF54 8002E754 23280902 */  subu       $a1, $s0, $t1
    /* 1EF58 8002E758 18004500 */  mult       $v0, $a1
    /* 1EF5C 8002E75C 2318C400 */  subu       $v1, $a2, $a0
    /* 1EF60 8002E760 12A80000 */  mflo       $s5
    /* 1EF64 8002E764 23203503 */  subu       $a0, $t9, $s5
    /* 1EF68 8002E768 26188300 */  xor        $v1, $a0, $v1
    /* 1EF6C 8002E76C 29006004 */  bltz       $v1, .L8002E814
    /* 1EF70 8002E770 21100000 */   addu      $v0, $zero, $zero
    /* 1EF74 8002E774 2320A901 */  subu       $a0, $t5, $t1
    /* 1EF78 8002E778 18008F00 */  mult       $a0, $t7
    /* 1EF7C 8002E77C 12380000 */  mflo       $a3
    /* 1EF80 8002E780 2318CA01 */  subu       $v1, $t6, $t2
    /* 1EF84 8002E784 00000000 */  nop
    /* 1EF88 8002E788 18006500 */  mult       $v1, $a1
    /* 1EF8C 8002E78C 12280000 */  mflo       $a1
    /* 1EF90 8002E790 00000000 */  nop
    /* 1EF94 8002E794 00000000 */  nop
    /* 1EF98 8002E798 18009800 */  mult       $a0, $t8
    /* 1EF9C 8002E79C 12300000 */  mflo       $a2
    /* 1EFA0 8002E7A0 00000000 */  nop
    /* 1EFA4 8002E7A4 00000000 */  nop
    /* 1EFA8 8002E7A8 18006800 */  mult       $v1, $t0
    /* 1EFAC 8002E7AC 2320E500 */  subu       $a0, $a3, $a1
    /* 1EFB0 8002E7B0 12180000 */  mflo       $v1
    /* 1EFB4 8002E7B4 2318C300 */  subu       $v1, $a2, $v1
    /* 1EFB8 8002E7B8 26188300 */  xor        $v1, $a0, $v1
    /* 1EFBC 8002E7BC 15006004 */  bltz       $v1, .L8002E814
    /* 1EFC0 8002E7C0 2320AB01 */   subu      $a0, $t5, $t3
    /* 1EFC4 8002E7C4 23102C02 */  subu       $v0, $s1, $t4
    /* 1EFC8 8002E7C8 18008200 */  mult       $a0, $v0
    /* 1EFCC 8002E7CC 12380000 */  mflo       $a3
    /* 1EFD0 8002E7D0 2318CC01 */  subu       $v1, $t6, $t4
    /* 1EFD4 8002E7D4 23100B02 */  subu       $v0, $s0, $t3
    /* 1EFD8 8002E7D8 18006200 */  mult       $v1, $v0
    /* 1EFDC 8002E7DC 12300000 */  mflo       $a2
    /* 1EFE0 8002E7E0 23106C02 */  subu       $v0, $s3, $t4
    /* 1EFE4 8002E7E4 00000000 */  nop
    /* 1EFE8 8002E7E8 18008200 */  mult       $a0, $v0
    /* 1EFEC 8002E7EC 12280000 */  mflo       $a1
    /* 1EFF0 8002E7F0 23104B02 */  subu       $v0, $s2, $t3
    /* 1EFF4 8002E7F4 00000000 */  nop
    /* 1EFF8 8002E7F8 18006200 */  mult       $v1, $v0
    /* 1EFFC 8002E7FC 2320E600 */  subu       $a0, $a3, $a2
    /* 1F000 8002E800 12180000 */  mflo       $v1
    /* 1F004 8002E804 2318A300 */  subu       $v1, $a1, $v1
    /* 1F008 8002E808 26108300 */  xor        $v0, $a0, $v1
    /* 1F00C 8002E80C 27100200 */  nor        $v0, $zero, $v0
    /* 1F010 8002E810 C2170200 */  srl        $v0, $v0, 31
  .L8002E814:
    /* 1F014 8002E814 1400B58F */  lw         $s5, 0x14($sp)
    /* 1F018 8002E818 1000B48F */  lw         $s4, 0x10($sp)
    /* 1F01C 8002E81C 0C00B38F */  lw         $s3, 0xC($sp)
    /* 1F020 8002E820 0800B28F */  lw         $s2, 0x8($sp)
    /* 1F024 8002E824 0400B18F */  lw         $s1, 0x4($sp)
    /* 1F028 8002E828 0000B08F */  lw         $s0, 0x0($sp)
    /* 1F02C 8002E82C 1800BD27 */  addiu      $sp, $sp, 0x18
    /* 1F030 8002E830 0800E003 */  jr         $ra
    /* 1F034 8002E834 00000000 */   nop
endlabel func_8002E6B0
