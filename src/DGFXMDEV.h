/*
    DGFXMDEV.h

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


#ifndef DGFXMDEV_H
#define DGFXMDEV_H

#include "PICOMMON.h"
#include "DGFXMDEV_Pipeline.h"

/* State machine. */
#define DGFX_IDLE 1
#define DGFX_PROCESSING 2
extern ui5b DGFX_STATE;

/* Memory window boundaries (matches memory_map.md). */
#define DGFXMDEV_WINDOW_BOTTOM        0x600000
#define DGFXMDEV_WINDOW_TOP           0x60FFFF

/* Section boundaries within the memory window. */
#define DGFXMDEV_MAILFLAG_START       0x600000
#define DGFXMDEV_MAILFLAG_END         0x600003
#define DGFXMDEV_COMMANDLIST_START    0x600004
#define DGFXMDEV_COMMANDLIST_END      0x60003B
#define DGFXMDEV_UNUSED_START         0x60003C
#define DGFXMDEV_UNUSED_END           0x60003F
#define DGFXMDEV_CLIENT_MEM_START     0x600040
#define DGFXMDEV_CLIENT_MEM_END       0x60FFEF
#define DGFXMDEV_SPECIAL_START        0x60FFF0
#define DGFXMDEV_SPECIAL_END          0x60FFFF

ui5b DGFXMDEV_Access(ATTep p, ui5b Data, blnr WriteMem, blnr ByteSize, ui5b addr);
void DGFXMDEV_Reset(void);
void DGFXMDEV_Tick(void);

/* Command-passing. */
extern ui5b DGFX_MAILFLAG_STATUS;
ui5b DGFXMDEV_CheckMailflag(void);
void DGFXMDEV_ClearMailflag(void);

/* Debug variables to be displayed in DGFX debug OSD. */
extern ui5b DGFX_LAST_DATA;
extern blnr DGFX_LAST_WRITEMEM;
extern blnr DGFX_LAST_BYTESIZE;
extern ui5b DGFX_LAST_ADDR;
extern const char* DGFX_LAST_MESSAGE;


#endif // DGFXMDEV_H 