glabel func_8001C444
    /* CC44 8001C444 00080224 */  addiu      $v0, $zero, 0x800
    /* CC48 8001C448 1080013C */  lui        $at, %hi(D_8010277A)
    /* CC4C 8001C44C 7A2722A4 */  sh         $v0, %lo(D_8010277A)($at)
    /* CC50 8001C450 1080013C */  lui        $at, %hi(D_80102778)
    /* CC54 8001C454 782722A4 */  sh         $v0, %lo(D_80102778)($at)
    /* CC58 8001C458 01000224 */  addiu      $v0, $zero, 0x1
    /* CC5C 8001C45C 1080013C */  lui        $at, %hi(D_8010277C)
    /* CC60 8001C460 7C2722A0 */  sb         $v0, %lo(D_8010277C)($at)
    /* CC64 8001C464 10000224 */  addiu      $v0, $zero, 0x10
    /* CC68 8001C468 1080013C */  lui        $at, %hi(D_8010277D)
    /* CC6C 8001C46C 7D2722A0 */  sb         $v0, %lo(D_8010277D)($at)
    /* CC70 8001C470 0C000224 */  addiu      $v0, $zero, 0xC
    /* CC74 8001C474 1080013C */  lui        $at, %hi(D_80102784)
    /* CC78 8001C478 842722A0 */  sb         $v0, %lo(D_80102784)($at)
    /* CC7C 8001C47C 04000224 */  addiu      $v0, $zero, 0x4
    /* CC80 8001C480 1080013C */  lui        $at, %hi(D_8010277E)
    /* CC84 8001C484 7E2720A0 */  sb         $zero, %lo(D_8010277E)($at)
    /* CC88 8001C488 1080013C */  lui        $at, %hi(D_8010277F)
    /* CC8C 8001C48C 7F2720A0 */  sb         $zero, %lo(D_8010277F)($at)
    /* CC90 8001C490 1080013C */  lui        $at, %hi(D_80102781)
    /* CC94 8001C494 812720A0 */  sb         $zero, %lo(D_80102781)($at)
    /* CC98 8001C498 1080013C */  lui        $at, %hi(D_80102780)
    /* CC9C 8001C49C 802720A0 */  sb         $zero, %lo(D_80102780)($at)
    /* CCA0 8001C4A0 1080013C */  lui        $at, %hi(D_80102785)
    /* CCA4 8001C4A4 852722A0 */  sb         $v0, %lo(D_80102785)($at)
    /* CCA8 8001C4A8 1080013C */  lui        $at, %hi(D_80102786)
    /* CCAC 8001C4AC 862720A0 */  sb         $zero, %lo(D_80102786)($at)
    /* CCB0 8001C4B0 1080013C */  lui        $at, %hi(D_80102787)
    /* CCB4 8001C4B4 872720A0 */  sb         $zero, %lo(D_80102787)($at)
    /* CCB8 8001C4B8 0800E003 */  jr         $ra
    /* CCBC 8001C4BC 00000000 */   nop
endlabel func_8001C444
