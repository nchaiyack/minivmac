# WORKLOG

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