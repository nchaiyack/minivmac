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
ui5b DGFX_LAST_ADDR = 0;
const char* DGFX_LAST_MESSAGE = "No message yet";

/* Implementation. */

void DGFXMDEV_Reset(void) {
    // Reset any internal state here if needed in the future
    DGFX_STATE = DGFX_IDLE;
    DGFXMDEV_MEM[0] = 0;
    for (size_t i = 1; i < (sizeof(DGFXMDEV_MEM)/sizeof(DGFXMDEV_MEM[0])); ++i) {
        DGFXMDEV_MEM[i] = 0xF00F;
    }
    DGFX_LAST_MESSAGE = "Device reset complete";
}

void DGFXMDEV_Tick(void) {
    // If we're in processing mode, search every first of two words from
    // DGFXMDEV_WINDOW_BOTTOM + 1 to DGFXMDEV_COMMANDLIST_TOP for the first nonzero address.
    if (DGFX_STATE == DGFX_PROCESSING) {
        DGFX_LAST_MESSAGE = "Processing commands";
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
            DGFX_LAST_MESSAGE = "Command processing complete";
        }
    }
} 

/* Forward decl of a helper function we'll need below... */
static void DGFXMDEV_ComputeBCDTimestamp(ui5b *date_out, ui5b *time_out);

ui5b DGFXMDEV_Access(ATTep p, ui5b Data, blnr WriteMem, blnr ByteSize, ui5b addr) {
    DGFX_LAST_MESSAGE = "Access called";

    /* Set OSD debug globals. */
    DGFX_LAST_DATA = Data;
    DGFX_LAST_WRITEMEM = WriteMem;
    DGFX_LAST_BYTESIZE = ByteSize;
    DGFX_LAST_ADDR = addr;


    /* Special handling: trying to access outside of our window? This should
        be a programmer error, but here we are. */
    if (addr < DGFXMDEV_WINDOW_BOTTOM || addr > DGFXMDEV_WINDOW_TOP) {
        DGFX_LAST_MESSAGE = "Access outside window";
        ReportAbnormalID(0xD00D, "DGFX: called with addr outside of window");  
        return 0;
    }

    // --- Special handling for last 4 words before and at DGFXMDEV_WINDOW_TOP ---
    // These are constants: 0xEEEEEEEE, 0xFFFFFFFF, and a 64-bit compilation timestamp.
    // Writing to this area triggers a bus error.
    // Each word is sizeof(ui5b) bytes, so we need 4 * sizeof(ui5b) bytes total.
    // The 4-word region occupies the last 16 bytes of the address space.
    const ui5b special_base = DGFXMDEV_WINDOW_TOP - 4*sizeof(ui5b) + 1;
    const ui5b special_end  = DGFXMDEV_WINDOW_TOP;
    if (addr >= special_base && addr <= special_end) {
        // Can't write to this area...
        if (WriteMem) {
            DGFX_LAST_MESSAGE = "DGFX: attempted write to read-only ID/timestamp zone";
            ReportAbnormalID(0xD00D, "DGFX: write to identification + compile timestamp area");  
            return 0;
        }
        // Read: return constants.
        // Compute which word this is (0, 1, 2, 3)
        /*
         * Layout of the 4-word identification/timestamp area:
         * Word 0: 0xEEEEEEEE (identification marker)
         * Word 1: 0xFFFFFFFF (identification marker) 
         * Word 2: ts_hi: Date in format 0xYYYYMMDD (BCD)
         * Word 3: ts_lo: Time in format 0xHHMMSS00 (BCD)
         */
        ui5b word_offset = (addr - special_base) / sizeof(ui5b);
        ui5b byte_offset = (addr - special_base) % sizeof(ui5b);
        
        // Debug: Update message to show which word we're accessing
        if (word_offset == 0) {
            DGFX_LAST_MESSAGE = "DGFX special: word 0 (EEE)";
        } else if (word_offset == 1) {
            DGFX_LAST_MESSAGE = "DGFX special: word 1 (FFF)";
        } else if (word_offset == 2) {
            DGFX_LAST_MESSAGE = "DGFX special: word 2 (date)";
        } else if (word_offset == 3) {
            DGFX_LAST_MESSAGE = "DGFX special: word 3 (time)";
        } else {
            DGFX_LAST_MESSAGE = "DGFX special: unknown word";
        }
        
        // Get the 32-bit value for this word
        ui5b word_value;
        switch (word_offset) {
            case 0:
                word_value = 0xEEEEEEEE;
                break;
            case 1:
                word_value = 0xFFFFFFFF;
                break;
            case 2:
            case 3:
            {
                static ui5b cached_date = 0;
                static ui5b cached_time = 0;
                static int initialized = 0;
                if (!initialized) {
                    DGFXMDEV_ComputeBCDTimestamp(&cached_date, &cached_time);
                    initialized = 1;
                }
                word_value = (word_offset == 2) ? cached_date : cached_time;
                break;
            }
            default:
                word_value = 0;
                break;
        }
        
        // For byte reads, return the appropriate byte (big-endian)
        if (ByteSize) {
            return (word_value >> (8 * (3 - byte_offset))) & 0xFF;
        } else {
            return word_value;
        }
    }
    // --- End special handling ---

    /* Check the mailflag. If it's set, we're in PROCESSING mode. 
        DGFXMDEV_Tick will handle the rest.  */
    if (DGFXMDEV_CheckMailflag()) {
        DGFX_STATE = DGFX_PROCESSING;
        DGFX_LAST_MESSAGE = "Mailflag detected, starting processing";
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
    return ((ui3b *)DGFXMDEV_MEM)[0] == 0x01;
}

void DGFXMDEV_ClearMailflag(void) {
    // Clear the byte at DGFXMDEV_WINDOW_TOP to 0x00.
    ((ui3b *)DGFXMDEV_MEM)[0] = 0x00;
}

/* Helper function to compute BCD-encoded timestamp from __DATE__ and __TIME__. */
static void DGFXMDEV_ComputeBCDTimestamp(ui5b *date_out, ui5b *time_out) {
    // Convert a 2-digit decimal number to BCD (e.g., 23 -> 0x23)
    #define TO_BCD(val) (((val / 10) << 4) | (val % 10))
    
    // Parse __DATE__ string "MMM DD YYYY" (e.g., "Jun  7 2024")
    const char *date_str = __DATE__;
    
    // Extract month from month name
    int month = 1; // default fallback
    if (date_str[0] == 'J' && date_str[1] == 'a' && date_str[2] == 'n') month = 1;  // Jan
    else if (date_str[0] == 'F' && date_str[1] == 'e' && date_str[2] == 'b') month = 2;  // Feb
    else if (date_str[0] == 'M' && date_str[1] == 'a' && date_str[2] == 'r') month = 3;  // Mar
    else if (date_str[0] == 'A' && date_str[1] == 'p' && date_str[2] == 'r') month = 4;  // Apr
    else if (date_str[0] == 'M' && date_str[1] == 'a' && date_str[2] == 'y') month = 5;  // May
    else if (date_str[0] == 'J' && date_str[1] == 'u' && date_str[2] == 'n') month = 6;  // Jun
    else if (date_str[0] == 'J' && date_str[1] == 'u' && date_str[2] == 'l') month = 7;  // Jul
    else if (date_str[0] == 'A' && date_str[1] == 'u' && date_str[2] == 'g') month = 8;  // Aug
    else if (date_str[0] == 'S' && date_str[1] == 'e' && date_str[2] == 'p') month = 9;  // Sep
    else if (date_str[0] == 'O' && date_str[1] == 'c' && date_str[2] == 't') month = 10; // Oct
    else if (date_str[0] == 'N' && date_str[1] == 'o' && date_str[2] == 'v') month = 11; // Nov
    else if (date_str[0] == 'D' && date_str[1] == 'e' && date_str[2] == 'c') month = 12; // Dec
    
    // Extract day (handle single digit days with leading space)
    int day = 0;
    if (date_str[4] != ' ') {
        day = (date_str[4] - '0') * 10;
    }
    day += (date_str[5] - '0');
    
    // Extract year from positions 7-10 (e.g., "2024")
    int year = (date_str[7] - '0') * 1000 + 
               (date_str[8] - '0') * 100 + 
               (date_str[9] - '0') * 10 + 
               (date_str[10] - '0');
    
    // Parse __TIME__ string "HH:MM:SS" (e.g., "12:34:56")
    const char *time_str = __TIME__;
    int hour = (time_str[0] - '0') * 10 + (time_str[1] - '0');
    int minute = (time_str[3] - '0') * 10 + (time_str[4] - '0');
    int second = (time_str[6] - '0') * 10 + (time_str[7] - '0');
    
    // Build BCD date: 0xYYYYMMDD
    // Year: convert 4-digit year to two BCD bytes
    ui5b year_high = TO_BCD(year / 100);  // centuries and decades (e.g., 20 -> 0x20)
    ui5b year_low = TO_BCD(year % 100);   // decades and units (e.g., 24 -> 0x24)
    ui5b month_bcd = TO_BCD(month);       // month (e.g., 6 -> 0x06)
    ui5b day_bcd = TO_BCD(day);           // day (e.g., 7 -> 0x07)
    
    *date_out = (year_high << 24) | (year_low << 16) | (month_bcd << 8) | day_bcd;
    
    // Build BCD time: 0xHHMMSS00
    ui5b hour_bcd = TO_BCD(hour);         // hour (e.g., 12 -> 0x12)
    ui5b minute_bcd = TO_BCD(minute);     // minute (e.g., 34 -> 0x34)
    ui5b second_bcd = TO_BCD(second);     // second (e.g., 56 -> 0x56)
    
    *time_out = (hour_bcd << 24) | (minute_bcd << 16) | (second_bcd << 8) | 0x00;
    
    #undef TO_BCD
}