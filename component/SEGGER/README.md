# Usage guide

## Prerequisites

1. SEGGER JLINK Debug Probe
2. SystemView Licence

## Enabling Systemview

1. Include directories for SEGGER/inc and SEGGER/src to your Makefile
2. Call `SEGGER_SYSVIEW_Conf();` and `SEGGER_SYSVIEW_Start();` in your `main()`

### Configure RAM base address

In `component/common/SEGGER/src/SEGGER_SYSVIEW_Config_FreeRTOS.c`
Set:
```
#define SYSVIEW_RAM_BASE        (0x10000500) <-- Your RAM base address
```

### Configure heap tracking

1. Define heap regions for Systemview in SEGGER_SYSVIEW_heaptrace.h

2. To malloc with tracking, call `void* HeapTrace_Malloc(size_t size)`
e.g.
```c
void* ptr = HeapTrace_Malloc(1024);
```

3. Then to subsequently free the block:
```c
HeapTrace_Free(ptr);
```

## Running Systemview

1. Open SystemView on PC
2. Ensure JLINK debug probe is connected via SWD
3. In systemview top menu, select: **Target** -> Systemview Recorder: **J-Link**
4. Use the following settings:
    * USB
    * CORTEX-M33
    * SWD @ 4000 KHz
    * RTT address can be found using:
    `arm-none-eabi-objdump -t amebadplus_gcc_project/project_km4/asdk/image/target_img2.axf | grep SEGGER_RTT`
5. Start Recording

## Configuring SEGGER RTT
Add `SEGGER_RTT.c`, `SEGGER_RTT_printf.c` and `SEGGER_RTT_ASM_ARMv7M.S` to CMAKE
RTT address can be found using: `arm-none-eabi-objdump -t amebadplus_gcc_project/project_km4/asdk/image/target_img2.axf | grep SEGGER_RTT`
The RTT address is needed if you are using J-Link RTT Viewer. Ozone automatically detects the RTT address using the .axf file provided.

### Defining RTT buffer

By default, there is no configuration needed to define the RTT buffer. You may simply call the SEGGER_RTT_printf() functions directly.

#### Custom buffer size
In `SEGGER_RTT_Conf.h` configure the `BUFFER_SIZE_UP` for output buffer size (MCU -> PC) and `BUFFER_SIZE_DOWN` for input buffer size (PC -> MCU).
You may encounter overflow errors during linking. See the next section to resolve this.

#### Defining specific region
Sometimes, the RTT buffer may be too large to be automatically placed by the linker. You can define the RTT buffer region yourself.
For example, in amebadplus, for KM4 only. In `ameba_img2_all.ld`, you can place the RTT buffer in PSRAM using:
```
	.rtt (NOLOAD) : ALIGN(8) {
    	KEEP(*(.rtt))
  	} > KM4_BD_PSRAM
```

Then in `SEGGER_RTT_Conf.h` add `#define SEGGER_RTT_SECTION ".rtt"`
