#ifndef BB2_CONST_H
#define BB2_CONST_H

/* Values of D_800A3768, a five-valued mode (0xFF, 1, 2, 10, 0x14) that
   selects what func_800174F4 draws each frame */
#define DISP_DISABLED 0xFF /* Display off / loading */
#define DISP_ACTIVE 0x14   /* Normal active display */
#define DISP_LOADING 10    /* Loading state */

/* Sound channels */
#define SND_CHANNEL_BGM 8  /* Background music channel */
#define SND_CHANNEL_SE 9   /* Sound effects channel */
#define SND_CHANNEL_UI 0xA /* UI/selection sounds channel */

#endif /* BB2_CONST_H */
