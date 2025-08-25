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
