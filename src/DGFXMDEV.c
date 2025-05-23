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

/* Our command-passing schema of choice... */
#include "mpack.h"

/* Backing memory.*/
#define DGFXMDEV_WINDOW_SIZE ((DGFXMDEV_WINDOW_TOP - DGFXMDEV_WINDOW_BOTTOM) + 1)
ui5b DGFXMDEV_MEM[(DGFXMDEV_WINDOW_SIZE + 1) / sizeof(ui5b)] = {0};

/* State machine. */
ui5b DGFX_STATE = DGFX_IDLE;



/* OSD debug variables that we'll define here. */
ui5b DGFX_LAST_DATA = 0;
blnr DGFX_LAST_WRITEMEM = 0;
blnr DGFX_LAST_BYTESIZE = 0;
ui4r DGFX_LAST_ADDR = 0;

/* Implementation. */

void DGFXMDEV_Reset(void) {
    // Reset any internal state here if needed in the future
    DGFX_STATE = DGFX_IDLE;
    DGFXMDEV_MEM[0] = 0;
    for (size_t i = 1; i < (sizeof(DGFXMDEV_MEM)/sizeof(DGFXMDEV_MEM[0])); ++i) {
        DGFXMDEV_MEM[i] = 0xF00F;
    }
}

void DGFXMDEV_Tick(void) {
    // If we're in processing mode, search every first of two words from
    // DGFXMDEV_WINDOW_BOTTOM + 1 to DGFXMDEV_COMMANDLIST_TOP for the first nonzero address.
    if (DGFX_STATE == DGFX_PROCESSING) {
        ui5b data = 0;
        for (ui5b i = 1; i <= (DGFXMDEV_COMMANDLIST_TOP - DGFXMDEV_WINDOW_BOTTOM); i += 2) {
            data = DGFXMDEV_MEM[i];
            if (data != 0) {
                // If so, the second of two words is the length of the command buffer.
                ui5b length = DGFXMDEV_MEM[i + 1];
                // Malloc a buffer of size length, and copy [data, ..., data + length - 1].
                ui5b *cmdBuffer = (ui5b *)malloc(length);
                for (ui5b j = 0; j < length; j++) {
                    cmdBuffer[j] = DGFXMDEV_MEM[i + 2 + j];
                }

                // Don't forget to free the buffer.
                free(cmdBuffer);
            }
        }
        // If there are no more commands, we're done.
        // Reset the mailflag.
        if (data != 0) {
            DGFXMDEV_ClearMailflag();
            DGFX_STATE = DGFX_IDLE;
        }
    }
} 

ui5b DGFXMDEV_Access(ATTep p, ui5b Data, blnr WriteMem, blnr ByteSize, CPTR addr) {
    /* Set OSD debug globals. */
    DGFX_LAST_DATA = Data;
    DGFX_LAST_WRITEMEM = WriteMem;
    DGFX_LAST_BYTESIZE = ByteSize;
    DGFX_LAST_ADDR = addr;

    /* Check the mailflag. If it's set, we're in PROCESSING mode. 
        DGFXMDEV_Tick will handle the rest.  */
    if (DGFXMDEV_CheckMailflag()) {
        DGFX_STATE = DGFX_PROCESSING;
    }

    if (addr < DGFXMDEV_WINDOW_BOTTOM || addr > DGFXMDEV_WINDOW_TOP) {
        return 0;
    }
    /* Update the backing memory DGFXMDEV_MEM. */

    size_t offset = addr - DGFXMDEV_WINDOW_BOTTOM;
    size_t word_index = offset / sizeof(ui5b);
    size_t byte_index = offset % sizeof(ui5b);

    if (WriteMem) {
        if (ByteSize) {
            // 8-bit write
            ((ui3b *)&DGFXMDEV_MEM[word_index])[byte_index] = (ui3b)Data;
        } else if ((addr & 1) == 0) {
            // 16-bit write (word, big-endian)
            ((ui4b *)&DGFXMDEV_MEM[word_index])[byte_index / 2] = (ui4b)Data;
        } else if ((addr & 3) == 0) {
            // 32-bit write (long, big-endian)
            DGFXMDEV_MEM[word_index] = Data;
        }
        return 0;
    }

    // Read
    if (ByteSize) {
        // 8-bit read
        return ((ui3b *)&DGFXMDEV_MEM[word_index])[byte_index];
    } else if ((addr & 1) == 0) {
        // 16-bit read (word, big-endian)
        return ((ui4b *)&DGFXMDEV_MEM[word_index])[byte_index / 2];
    } else if ((addr & 3) == 0) {
        // 32-bit read (long, big-endian)
        return DGFXMDEV_MEM[word_index];
    }
    return 0;
} 

/* Command-passing. */

ui5b DGFXMDEV_CheckMailflag(void) {
    // Check to see whether the byte at DGFXMDEV_WINDOW_TOP is 0x01.
    ui5b addr = DGFXMDEV_WINDOW_BOTTOM;
    ui5b data = DGFXMDEV_Access(nullpr, 0, falseblnr, trueblnr, addr);
    return data == 0x01;
}

void DGFXMDEV_ClearMailflag(void) {
    // Clear the byte at DGFXMDEV_WINDOW_TOP to 0x00.
    ui5b addr = DGFXMDEV_WINDOW_BOTTOM;
    DGFXMDEV_Access(nullpr, 0, trueblnr, trueblnr, addr);
}