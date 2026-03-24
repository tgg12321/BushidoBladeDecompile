glabel func_8001CD68
    /* D568 8001CD68 0200023C */  lui        $v0, (0x2BF1F >> 16)
    /* D56C 8001CD6C 0A80033C */  lui        $v1, %hi(D_800A3858)
    /* D570 8001CD70 5838638C */  lw         $v1, %lo(D_800A3858)($v1)
    /* D574 8001CD74 1FBF4234 */  ori        $v0, $v0, (0x2BF1F & 0xFFFF)
    /* D578 8001CD78 2A104300 */  slt        $v0, $v0, $v1
    /* D57C 8001CD7C 08004010 */  beqz       $v0, .L8001CDA0
    /* D580 8001CD80 21388000 */   addu      $a3, $a0, $zero
    /* D584 8001CD84 63000224 */  addiu      $v0, $zero, 0x63
    /* D588 8001CD88 0000E2A4 */  sh         $v0, 0x0($a3)
    /* D58C 8001CD8C 3B000224 */  addiu      $v0, $zero, 0x3B
    /* D590 8001CD90 0200E2A0 */  sb         $v0, 0x2($a3)
    /* D594 8001CD94 63000224 */  addiu      $v0, $zero, 0x63
    /* D598 8001CD98 96730008 */  j          .L8001CE58
    /* D59C 8001CD9C 0300E2A0 */   sb        $v0, 0x3($a3)
  .L8001CDA0:
    /* D5A0 8001CDA0 A291023C */  lui        $v0, (0x91A2B3C5 >> 16)
    /* D5A4 8001CDA4 C5B34234 */  ori        $v0, $v0, (0x91A2B3C5 & 0xFFFF)
    /* D5A8 8001CDA8 18006200 */  mult       $v1, $v0
    /* D5AC 8001CDAC 10100000 */  mfhi       $v0
    /* D5B0 8001CDB0 8888063C */  lui        $a2, (0x88888889 >> 16)
    /* D5B4 8001CDB4 8988C634 */  ori        $a2, $a2, (0x88888889 & 0xFFFF)
    /* D5B8 8001CDB8 18006600 */  mult       $v1, $a2
    /* D5BC 8001CDBC 21284300 */  addu       $a1, $v0, $v1
    /* D5C0 8001CDC0 832A0500 */  sra        $a1, $a1, 10
    /* D5C4 8001CDC4 C3170300 */  sra        $v0, $v1, 31
    /* D5C8 8001CDC8 2328A200 */  subu       $a1, $a1, $v0
    /* D5CC 8001CDCC 10200000 */  mfhi       $a0
    /* D5D0 8001CDD0 21188300 */  addu       $v1, $a0, $v1
    /* D5D4 8001CDD4 03190300 */  sra        $v1, $v1, 4
    /* D5D8 8001CDD8 23186200 */  subu       $v1, $v1, $v0
    /* D5DC 8001CDDC 00110500 */  sll        $v0, $a1, 4
    /* D5E0 8001CDE0 23104500 */  subu       $v0, $v0, $a1
    /* D5E4 8001CDE4 80100200 */  sll        $v0, $v0, 2
    /* D5E8 8001CDE8 23186200 */  subu       $v1, $v1, $v0
    /* D5EC 8001CDEC 0200E3A0 */  sb         $v1, 0x2($a3)
    /* D5F0 8001CDF0 0A80043C */  lui        $a0, %hi(D_800A3858)
    /* D5F4 8001CDF4 5838848C */  lw         $a0, %lo(D_800A3858)($a0)
    /* D5F8 8001CDF8 00000000 */  nop
    /* D5FC 8001CDFC 18008600 */  mult       $a0, $a2
    /* D600 8001CE00 10100000 */  mfhi       $v0
    /* D604 8001CE04 21184400 */  addu       $v1, $v0, $a0
    /* D608 8001CE08 03190300 */  sra        $v1, $v1, 4
    /* D60C 8001CE0C C3170400 */  sra        $v0, $a0, 31
    /* D610 8001CE10 23186200 */  subu       $v1, $v1, $v0
    /* D614 8001CE14 00110300 */  sll        $v0, $v1, 4
    /* D618 8001CE18 23104300 */  subu       $v0, $v0, $v1
    /* D61C 8001CE1C 40100200 */  sll        $v0, $v0, 1
    /* D620 8001CE20 23208200 */  subu       $a0, $a0, $v0
    /* D624 8001CE24 40100400 */  sll        $v0, $a0, 1
    /* D628 8001CE28 21104400 */  addu       $v0, $v0, $a0
    /* D62C 8001CE2C C0100200 */  sll        $v0, $v0, 3
    /* D630 8001CE30 21104400 */  addu       $v0, $v0, $a0
    /* D634 8001CE34 80100200 */  sll        $v0, $v0, 2
    /* D638 8001CE38 18004600 */  mult       $v0, $a2
    /* D63C 8001CE3C 0000E5A4 */  sh         $a1, 0x0($a3)
    /* D640 8001CE40 10180000 */  mfhi       $v1
    /* D644 8001CE44 21186200 */  addu       $v1, $v1, $v0
    /* D648 8001CE48 03190300 */  sra        $v1, $v1, 4
    /* D64C 8001CE4C C3170200 */  sra        $v0, $v0, 31
    /* D650 8001CE50 23186200 */  subu       $v1, $v1, $v0
    /* D654 8001CE54 0300E3A0 */  sb         $v1, 0x3($a3)
  .L8001CE58:
    /* D658 8001CE58 0800E003 */  jr         $ra
    /* D65C 8001CE5C 00000000 */   nop
endlabel func_8001CD68
