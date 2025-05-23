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

#include "PICOMMON.h"

#ifndef DGFXMDEV_H
#define DGFXMDEV_H


ui5b DGFXMDEV_Access(ATTep p, ui5b Data, blnr WriteMem, blnr ByteSize, CPTR addr);
void DGFXMDEV_Reset(void);
void DGFXMDEV_Tick(void);

/* Debug variables to be displayed in DGFX debug OSD. */
extern ui5b DGFX_LAST_DATA;
extern blnr DGFX_LAST_WRITEMEM;
extern blnr DGFX_LAST_BYTESIZE;
extern ui4r DGFX_LAST_ADDR;

#endif // DGFXMDEV_H 