Memory mapping and states of the DGFX (DazzlerGraphics SE) virtual PDS card
===========================================================================

## Purpose.

This document concerns the DGFX, an (emulated) PDS card for the Macintosh SE (as emulated by Mini vMac). As a PDS card for the Macintosh SE, communication with the PDS card
is done exclusively via memory-mapped IO with a memory window of the DGFX's choice. To aid in the implementation of MacOS applications that use the DGFX, this document describes the structure of the memory-mapped IO window, the command-processing states of the DGFX, and how clients send commands and receive results from the DGFX via reading and writing from the DGFX's memory window.

## States of the DGFX.

The DGFX has two states: `DGFX_IDLE = 1`, and `DGFX_PROCESSING = 2`.

-   When the DGFX is in state **DGFX_IDLE**, the client can access DGFX memory-mapped IO to prepare commands buffers to submit, or read off results buffers returned from the DGFX. Submission of commands is done by writing `DGFX_PROCESSING = 2` to the mailflag address (see below), at which point the DGFX transitions to state DGFX_PROCESSING.

-   When the DGFX is in state **DGFX_PROCESSING**, the DGFX is asynchronously processing commands. The client should not access DGFX memory-mapped IO, except to read the mailflag register which indicates the DGFX's state, to see whether the DGFX is again idle. When the DGFX is done processing commands, and has deposited results for the client to read, the DGFX will transition itself back to state DGFX_IDLE.

## Structure of the memory window.

Within the Macintosh SE's post-ROM overlay memory map, the DGFX reserves the area from 0x600000-0x60FFFF for memory-mapped IO (i.e., 64 KB, or ~16k 32-bit addresses). This memory window has 5 key sections, which we'll give both as a diagram and in list form for LLMs to read.

### Structure of the memory window: diagram form

    0x600000  ┌─────────────────────────────────────────────────────────────┐
              │ Mailflag Register (4 bytes)                                 │
              │ States: DGFX_IDLE=1, DGFX_PROCESSING=2                      │
    0x600004  ├─────────────────────────────────────────────────────────────┤
              │ Command/Result MessagePack Buffer Array (56 bytes)          │
              │ Up to 7 (address, length) pairs, terminated by (0,0)        │
              │ • IDLE: Client writes command buffer pointers               │
              │ • PROCESSING: DGFX writes result buffer pointers            │
    0x60003C  ├─────────────────────────────────────────────────────────────┤
              │ Unused Alignment Space (4 bytes)                            │
    0x600040  ├─────────────────────────────────────────────────────────────┤
              │                                                             │
              │                                                             │
              │ Client-Accessible On-DGFX Memory (65,456 bytes)             │
              │                                                             │
              │ • All MessagePack command/result buffers allocated here     │
              │ • Writable by client when DGFX_IDLE                         │
              │                                                             │
              │                                                             │
    0x60FFEF  ├─────────────────────────────────────────────────────────────┤
    0x60FFF0  │ Special Identification/Timestamp Window (16 bytes)          │
              │ ┌─────────────────────────────────────────────────────────┐ │
              │ │ +0x00: 0xEEEEEEEE (magic constant)                      │ │
              │ │ +0x04: 0xFFFFFFFF (magic constant)                      │ │  
              │ │ +0x08: 0xYYYYMMDD (BCD compilation date)                │ │
              │ │ +0x0C: 0xHHMMSS00 (BCD compilation time)                │ │
              │ └─────────────────────────────────────────────────────────┘ │
    0x60FFFF  └─────────────────────────────────────────────────────────────┘


### Structure of the memory window: list form


1.  **0x600000-0x600003: Mailflag register.** This stores the DGFX's state: either `DGFX_IDLE = 1`, or `DGFX_PROCESSING = 2`. The client can always read this mailflag register, but can only write to this register when = 1 (DGFX_IDLE), to set it = 2 (DGFX_PROCESSING) to signal the DGFX to begin processing.

2.  **0x600004-0x60003B: Command/result MessagePack buffer array.** This is a an array of pairs of (32-bit address, 32-bit length), that point to MessagePack buffers in the client-accessible on-DGFX memory (see item 4 below). A maximum of 7 (address, length) pairs can be stored here; fewer elements can be stored if the list is terminated with (0, 0). Depending on the state of the DGFX:

    a.  If the DGFX state is `DGFX_IDLE`, then the client can freely edit this array. After transitioning from `DGFX_PROCESSING`, the <= 7 pairs of (address, length) are <= 7 MessagePack result buffers that point into the client-accessible on-DGFX memory. The client can write <= 7 pairs of (address, length) pointing into the on-DGFX memory, to be interpreted as <= 7 MessagePack command buffers to be executed in order when the mailflag is set to `DGFX_PROCESSING = 2`.

    b. If the DGFX state is `DGFX_PROCESSING`, then the client should not view or edit this array: the DGFX is executing commands, and preparing the result buffer array.

3.  **0x60003C-0x60003F: Unused space for alignment.**

4.  **0x600040-0x60FFEF: Client-accessible on-DGFX memory.** All command/result buffers pointed to from the command/result buffer array should be allocated here.

5.  **0x60FFF0-0x60FFFF: Special identification/timestamp window.** These four 32-bit words contain metadata that help MacOS applications identify the DGFX:

    a.  **0x60FFF0-0x60FFF3**: magic constant 0xEEEEEEEE.

    b.  **0x60FFF4-0x60FFF7**: magic constant 0xFFFFFFFF.

    c.  **0x60FFF8-0x60FFFB**: BCD date of compilation in the form 0xYYYYMMDD.

    d.  **0x60FFFC-0x60FFFF**: BCD time of compilation in the form 0xHHMMSS00.