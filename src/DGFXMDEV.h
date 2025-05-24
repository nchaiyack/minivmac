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

/* Addresses of interest. */
#define DGFXMDEV_WINDOW_TOP    0x60FFFF
#define DGFXMDEV_WINDOW_BOTTOM 0x600000
#define DGFXMDEV_COMMANDLIST_TOP 0x600000 + 63
#define DGFXMDEV_MAGIC_VALUE  0xDEADBEEF

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