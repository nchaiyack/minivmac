# WORKLOG


## 05-23-25: Backing MMIO buffer; command processing loop skeleton; MessagePack chosen as wire messaging format; first pipeline defs

### Backing MMIO Buffer
- **Implemented a global memory buffer (`DGFXMDEV_MEM`)** to back the DGFXMDEV device's memory-mapped I/O (MMIO) window, covering the address range from `DGFXMDEV_WINDOW_BOTTOM` to `DGFXMDEV_WINDOW_TOP`.
- **Buffer Definition**: The buffer is defined as `ui5b DGFXMDEV_MEM[...]` in `DGFXMDEV.c` and declared as `extern` in `DGFXMDEV.h` for cross-module access.
- **Initialization**: In `DGFXMDEV_Reset()`, the first word is set to 0, and the rest are initialized to `0xF00F` for debugging visibility.
- **Access Logic**: All reads and writes to the MMIO window are routed through `DGFXMDEV_Access()`, which uses codebase-idiomatic types (`ui3b`, `ui4b`, `ui5b`) for byte, word, and long accesses, respectively. The function signature is:
  ```c
  ui5b DGFXMDEV_Access(ATTep p, ui5b Data, blnr WriteMem, blnr ByteSize, CPTR addr);
  ```

### Command Processing Loop Skeleton
- **Processing State Machine**: Introduced a state machine for DGFXMDEV, with states like `DGFX_IDLE` and `DGFX_PROCESSING`, to manage command processing.
- **Command Search Loop**: In `DGFXMDEV_Tick()`, implemented a loop that scans the MMIO buffer for command descriptors (address/length pairs), searching every first of two words for a nonzero address.
- **Command Buffer Handling**: When a command is found, a buffer is allocated, and the command data is copied from the MMIO buffer for further processing.
- **Skeleton for Message Parsing**: The loop sets up the infrastructure for parsing and executing commands, but actual command execution is deferred to future work.
- **Debug Display**: the OSD now displays the first 4 command buffer address/list pairs.

### MessagePack Chosen as Wire Messaging Format
- **Rationale**: Decided to use MessagePack for all over-the-wire communication between the Macintosh OS and DazzlerGFX, due to its compactness, speed, and cross-language support.
- **Integration**: Added `src/mpack.c` and `src/mpack.h` (amalgamated version) to the codebase. All command parsing in DGFXMDEV now uses MessagePack APIs.

### First Pipeline Definitions
- **Pipeline Header**: Created `DGFXMDEV_Pipeline.h` to house the blit spec and related pipeline definitions, separating them from the main device header for modularity.
- **Blit Spec Struct**: Defined the `DGFX_BlitSpec` struct to describe blit (block image transfer) operations, with fields for destination address, width, height, and stride:
  ```c
  typedef struct {
      ui5b destAddr;
      ui5b width;
      ui5b height;
      ui5b stride;
  } DGFX_BlitSpec;
  ```
- **Array Declaration**: Declared an array `DGFX_BLIT_SPEC[8]` for use as a pipeline of blit operations, with the definition to be provided in the implementation file.

## 05-23-25 First Control OSD

### Creation of DGFX Debug OSD
A new debug overlay, **DGFX Debug OSD**, was implemented to provide real-time tracing of accesses to the DGFXMDEV device in Control Mode.

### Behavior
- **Control Mode Overlay:** The DGFX OSD is only visible while Control Mode is active and toggled on with the 'G' key.
- **Key Handling:**
  - The OSD is toggled by pressing 'G' in Control Mode.
  - Exiting Control Mode automatically hides the DGFX OSD.
- **Debug Info:**
  - The OSD displays the arguments of the last call to `DGFXMDEV_Access()`, including address, data, write/read, byte size, and pointer.
  - The OSD is redrawn on every key event in Control Mode, ensuring up-to-date debug information.

#### Key Code Locations
- **OSD Toggle and State:**
  - `src/CONTROLM.h`: `DoControlModeKey()` (case `MKC_G`)
  - `src/CONTROLM.h`: `DoLeaveControlMode()` (clears OSD state)
- **OSD Display Logic:**
  - `src/CONTROLM.h`: `DrawSpclMode()` (prioritizes DGFX OSD when active)
  - `src/CONTROLM.h`: `DrawDGFXDebugMode()` and `DrawCellsDGFXDebugModeBody()` (renders OSD)
- **Debug Variable Updates:**
  - `src/DGFXMDEV.c`: `DGFXMDEV_Access()` (updates `DGFX_LAST_*` globals)

### Integration
- **Source Integration:**
  - OSD rendering and key handling are implemented in `src/CONTROLM.h`.
  - Debug variable storage and updates are in `src/DGFXMDEV.c` and declared as `extern` in both `src/DGFXMDEV.h` and via forward declarations in `src/CONTROLM.h`.
- **Overlay System:**
  - The DGFX OSD is integrated into the Control Mode overlay system, following the same precedence and display logic as other overlays (e.g., Magnify, Interrupt).
  - The display logic in `DrawSpclMode()` gives priority to the DGFX OSD if both `SpclModeDGFXDebug` and `SpclModeControl` are active:
    ```c
    if (SpecialModeTst(SpclModeDGFXDebug) && SpecialModeTst(SpclModeControl)) {
        DrawDGFXDebugMode();
    }
    ```
- **Debug Variable Access:**
  - All DGFX debug variables are maintained in `DGFXMDEV` and accessed via forward `extern` declarations in `CONTROLM.h`, avoiding transitive include issues.
- **State Management:**
  - The DGFX OSD state (`SpclModeDGFXDebug`) is cleared automatically when leaving Control Mode (`DoLeaveControlMode()`), ensuring consistent overlay behavior.
- **Logging Removal:**
  - All logging and `dolog`-related code has been removed from `DGFXMDEV` for clarity and maintainability.
- **Implementation Notes:**
  - The implementation follows Mini vMac conventions for overlays and debug displays, making future maintenance and extension straightforward.

## 05-21-25: SE PDS device: reserved page is constant DEADBEEF

### Creation of DGFXMDEV
A new device, **DGFXMDEV**, was created to emulate a custom SE PDS (Processor Direct Slot) device for the Macintosh SE. The primary feature of this device is that it exposes a reserved memory page that always reads as the constant value `0xDEADBEEF`.

### Behavior
- **Constant Page:** The device maps a specific page in the Macintosh SE's address space. Any read operation to this page returns the 32-bit value `0xDEADBEEF`, regardless of the offset or access pattern.
- **Read-Only:** The page is strictly read-only. Any write attempts are either ignored or logged as errors, ensuring the value remains constant.
- **Device Emulation:** DGFXMDEV is implemented as a minimal device, focusing on this reserved page behavior for testing and demonstration purposes.

### Integration
- **Source Integration:** The DGFXMDEV source files were added to the `src/` directory, following the project's device abstraction conventions.
- **Build System:** The build scripts and project files were updated to include DGFXMDEV in the compilation process.
- **Device Registration:** During emulator initialization, DGFXMDEV is registered and mapped to its designated address range, making it available to the emulated system.
- **Testing:** Basic tests were performed to ensure that reads from the reserved page always return `0xDEADBEEF` and that writes have no effect.

---

**Integration (Expanded Details)**

- **Source Files:**
  - The device is implemented in `src/DGFXMDEV.c` and declared in `src/DGFXMDEV.h`.
  - The main integration points are in `src/GLOBGLUE.c` and `src/MINEM68K.c`.

- **Memory Mapping:**
  - In `src/GLOBGLUE.c`, the function responsible for setting up the memory map is `SetUpMemBanks()`, which calls `SetUp_address()`.
  - Within `SetUp_address()`, the following code block maps the address range `$600000–$60FFFF` to DGFXMDEV for the Macintosh SE:
    ```c
    #if (CurEmMd == kEmMd_SE)
        // Map $600000–$60FFFF to DGFXMDEV device for Macintosh SE
        r.cmpmask = 0xFF0000; // match $600000–$60FFFF
        r.cmpvalu = 0x600000;
        r.usemask = 0x00FFFF; // 64KB
        r.usebase = nullpr;
        r.Access = kATTA_mmdvmask;
        r.MMDV = kMMDV_DGFXMDEV;
        AddToATTList(&r);
    #endif
    ```
  - This ensures that any access to this address range is routed to the DGFXMDEV device.

- **Device Access Handling:**
  - In `src/GLOBGLUE.c`, the function `MMDV_Access()` dispatches memory accesses to the correct device handler based on the memory map. For DGFXMDEV:
    ```c
    #if (CurEmMd == kEmMd_SE)
        case kMMDV_DGFXMDEV:
            Data = DGFXMDEV_Access(p, Data, WriteMem, ByteSize, addr);
            break;
    #endif
    ```
  - The function `DGFXMDEV_Access()` in `src/DGFXMDEV.c` implements the logic for returning the constant value `0xDEADBEEF` for reads and ignoring writes.

- **Device Reset:**
  - The device is reset as part of the global reset routine in `src/GLOBGLUE.c`:
    ```c
    GLOBALPROC customreset(void)
    {
        // ... other device resets ...
    #if (CurEmMd == kEmMd_SE)
        DGFXMDEV_Reset();
    #endif
    }
    ```
  - The function `DGFXMDEV_Reset()` in `src/DGFXMDEV.c` is currently a stub, ready for future stateful extensions.

- **Per-Cycle Hooks:**
  - In `src/MINEM68K.c`, the function `DGFXMDEV_Tick()` is declared and can be called after each emulated instruction for per-cycle device behavior:
    ```c
    #if (CurEmMd == kEmMd_SE)
    IMPORTPROC DGFXMDEV_Tick(void);
    #endif
    ```
  - This provides a hook for future expansion if the device needs to emulate time-dependent behavior.

- **Build System:**
  - The build script (`build_macos.sh`) and the setup tool ensure that all source files in `src/`, including `DGFXMDEV.c`, are compiled and linked into the final binary.

- **Testing and Logging:**
  - The implementation of `DGFXMDEV_Access()` includes debug logging (guarded by the `DGFXMDEV_dolog` macro) to trace accesses and verify correct operation during development.

This device serves as a foundation for further SE PDS device emulation and provides a simple example of memory-mapped device integration in the Mini vMac codebase.
