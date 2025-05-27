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

/* Forward declarations to avoid including PICOMMON.h in header */
/* Basic integer types - sized types for cross-platform consistency */
typedef unsigned char ui3b;
typedef signed char si3b;
typedef unsigned short ui4b;
typedef signed short si4b;
typedef unsigned int ui5b;
typedef signed int si5b;

/* Register-sized types (may vary by platform) */
typedef unsigned char ui3r;
typedef signed char si3r;
typedef unsigned short ui4r;
typedef signed short si4r;
typedef unsigned int ui5r;
typedef signed int si5r;

/* Pointer types */
typedef ui3b *ui3p;
typedef ui4b *ui4p;
typedef ui5b *ui5p;

/* Common derived types */
typedef ui5r uimr;      /* Largest efficiently supported unsigned type */
typedef si5r simr;      /* Largest efficiently supported signed type */
typedef ui5b CPTR;      /* CPU pointer type */
typedef ui3p anyp;      /* Generic pointer type */

/* Boolean type */
typedef ui3r blnr;
#define trueblnr 1
#define falseblnr 0

/* Null pointer */
#define nullpr ((void *) 0)

/* ATTer structure forward declaration */
struct ATTer;
typedef struct ATTer *ATTep;

#include "DGFXMDEV_Pipeline.h"


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

/* State machine. */
#define DGFX_IDLE 1
#define DGFX_PROCESSING 2
#define DGFX_STATE (*(ui5b *)(DGFXMDEV_MEM))

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
extern ui5b DGFXMDEV_MEM[];

#endif // DGFXMDEV_H 