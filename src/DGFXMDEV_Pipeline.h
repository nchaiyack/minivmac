/*
    DGFXMDEV_Pipeline.h
    
    Blit specification and pipeline structures for DGFXMDEV.
    Extracted from DGFXMDEV.h.
*/

#ifndef DGFXMDEV_PIPELINE_H
#define DGFXMDEV_PIPELINE_H

/* Blitting */

typedef struct {
    ui5b destAddr;
    ui5b width;
    ui5b height;
    ui5b stride;
} DGFX_BlitSpec;

DGFX_BlitSpec DGFX_BLIT_SPEC[8];

#endif // DGFXMDEV_PIPELINE_H 