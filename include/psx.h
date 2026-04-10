#ifndef PSX_H
#define PSX_H

/* ===== GPU Command Constants (GP0/GP1) ===== */

/* GP0 Commands (sent to GPU data port) */
#define GP0_DRAW_MODE       0xE1000000  /* GP0(E1h) - Set draw mode / texpage */
#define GP0_TEXWINDOW       0xE2000000  /* GP0(E2h) - Texture window */
#define GP0_DRAW_AREA_TL    0xE3000000  /* GP0(E3h) - Drawing area top-left */
#define GP0_DRAW_AREA_BR    0xE4000000  /* GP0(E4h) - Drawing area bottom-right */
#define GP0_DRAW_OFFSET     0xE5000000  /* GP0(E5h) - Drawing offset */
#define GP0_MASK_BIT        0xE6000000  /* GP0(E6h) - Mask bit setting */
#define GP0_COPY_RECT_C2V   0xA0000000  /* GP0(A0h) - Copy rectangle CPU -> VRAM */
#define GP0_COPY_RECT_V2C   0xC0000000  /* GP0(C0h) - Copy rectangle VRAM -> CPU */

/* GP1 Commands (sent to GPU status/control port) */
#define GP1_RESET_GPU       0x00000000  /* GP1(00h) - Reset GPU */
#define GP1_DISP_ENABLE     0x03000001  /* GP1(03h) - Display enable (mask=1) */
#define GP1_DMA_DIR         0x04000000  /* GP1(04h) - DMA direction / data request */
#define GP1_DMA_DIR_FIFO    0x04000002  /* GP1(04h) - DMA direction: FIFO (linked list) */
#define GP1_GPU_INFO        0x10000000  /* GP1(10h) - Get GPU info */

/* Ordering Table constants */
#define OT_TERMINATOR       0x01000000  /* OT chain terminator */
#define OT_TAG_MASK         0xFF000000  /* OT tag (packet size) mask */
#define OT_ADDR_MASK        0x00FFFFFF  /* OT address (24-bit) mask */
#define OT_TAG_BASE         0x80000000  /* OT tag with KSEG0 high bit */

/* GPU Dither pattern */
#define GPU_DITHER_PATTERN   0x55555555  /* 50% dither pattern for draw env */

/* Draw mode flag mask */
#define GPU_DRAW_MODE_MASK   0x9FF       /* Valid bits for GP0(E1h) mode */
#define GPU_DRAW_MODE_DITHER 0x200       /* Dither enable bit in draw mode */
#define GPU_DRAW_MODE_TEXOFF 0x400       /* Texture disable bit */

/* ===== DMA Constants ===== */

/* GPU DMA (channel 2) */
#define DMA_GPU_LINKED_LIST  0x01000401  /* GPU DMA: from RAM, linked list mode */

/* CD-ROM DMA (channel 3) */
#define DMA_CD_TO_RAM        0x11000000  /* CD DMA: normal transfer */
#define DMA_CD_TO_RAM_CHAIN  0x11400100  /* CD DMA: chained transfer */
#define DMA_CD_ENABLE        0x00008000  /* CD DMA enable bit */

/* SPU DMA (channel 4) */
#define DMA_CHAN_MASK         0xF0FFFFFF  /* Mask to clear channel bits */
#define DMA_SPU_FROM_RAM     0x20000000  /* SPU DMA: RAM -> SPU */
#define DMA_SPU_TO_RAM       0x22000000  /* SPU DMA: SPU -> RAM */
#define DMA_BUSY             0x01000000  /* DMA busy/active bit */

/* CD-ROM register flags */
#define CD_IRQ_DATA_READY    0x80        /* CD data ready interrupt flag */
#define CD_STAT_DATA_REQ     0x40        /* CD status: data request */

/* ===== Controller / Pad Button Constants ===== */

#define PAD_L2       0x0001
#define PAD_R2       0x0002
#define PAD_L1       0x0004
#define PAD_R1       0x0008
#define PAD_TRIANGLE 0x0010
#define PAD_CIRCLE   0x0020
#define PAD_CROSS    0x0040
#define PAD_SQUARE   0x0080
#define PAD_SELECT   0x0100
#define PAD_L3       0x0200
#define PAD_R3       0x0400
#define PAD_START    0x0800
#define PAD_UP       0x1000
#define PAD_RIGHT    0x2000
#define PAD_DOWN     0x4000
#define PAD_LEFT     0x8000

#endif /* PSX_H */
