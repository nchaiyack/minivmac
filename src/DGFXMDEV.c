/*
    DGFXMDEV.c

    Copyright (C) 2025 Nick Chaiyachakorn

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#include "DGFXMDEV.h"

#define DGFXMDEV_MAGIC_TOP    0x60FFFF
#define DGFXMDEV_MAGIC_BOTTOM 0x600000
#define DGFXMDEV_MAGIC_VALUE  0xDEADBEEF

/* OSD debug variables that we'll define here. */
ui5b DGFX_LAST_DATA = 0;
blnr DGFX_LAST_WRITEMEM = 0;
blnr DGFX_LAST_BYTESIZE = 0;
ui4r DGFX_LAST_ADDR = 0;

/* Implementation. */

void DGFXMDEV_Reset(void) {
    // Reset any internal state here if needed in the future
}

void DGFXMDEV_Tick(void) {
    // TODO: Implement bus master tick logic here
} 

ui5b DGFXMDEV_Access(ATTep p, ui5b Data, blnr WriteMem, blnr ByteSize, CPTR addr) {
    /* Set OSD debug globals. */
    DGFX_LAST_DATA = Data;
    DGFX_LAST_WRITEMEM = WriteMem;
    DGFX_LAST_BYTESIZE = ByteSize;
    DGFX_LAST_ADDR = addr;

    if (addr < DGFXMDEV_MAGIC_BOTTOM || addr > DGFXMDEV_MAGIC_TOP) {
        return 0;
    }
    if (WriteMem) {
        return 0;
    }
    // Calculate 16-bit word index
    ui5b word16_index = (addr - DGFXMDEV_MAGIC_BOTTOM) >> 1;
    ui5b val16 = (word16_index & 1) ? 0xBEEF : 0xDEAD;
    // 32-bit aligned read (long, big-endian)
    if (!ByteSize && (addr & 3) == 0) {
        ui5b next_val16 = ((word16_index + 1) & 1) ? 0xBEEF : 0xDEAD;
        ui5b ret = (val16 << 16) | (next_val16 & 0xFFFF);
        return ret;
    }
    // 16-bit aligned read (word, big-endian)
    if (!ByteSize && (addr & 1) == 0) {
        return val16;
    }
    // 8-bit read (byte, big-endian)
    if (ByteSize) {
        int byte_in_word = 1 - (addr & 1); // 0 = high, 1 = low
        ui5b ret = (val16 >> (8 * byte_in_word)) & 0xFF;
        return ret;
    }
    return 0;
} 