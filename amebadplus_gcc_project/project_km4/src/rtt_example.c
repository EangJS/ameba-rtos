#include "ameba_soc.h"
#include "os_wrapper.h"
#include "SEGGER_RTT.h"

#define FLASH_APP_BASE 0x1EF000 // the last sector of IMG2
#define TOTAL_SIZE    (1024U * 32U)
static void flash_test_task(void *param)
{
    uint32_t address = FLASH_APP_BASE;

    void *buffer = rtos_mem_malloc(32);
    for (uint32_t i = 0; i < 32; i++)
    {
        ((uint8_t *)buffer)[i] = 0xA0 + i;
    }
    uint32_t val32_to_read;
    // int loop = 0;
    // int result = 0;

    UNUSED(param);

    rtos_time_delay_ms(10000);

    uint32_t x = 0;

    for (;;)
    {
        RTK_LOGI(NOTAG, "\r\nLoop %d\n", x++);

        uint32_t start_addr = address;
        uint32_t end_addr   = address + TOTAL_SIZE - 1;

        for (uint32_t addr = (start_addr & ~(4096 - 1));
            addr <= end_addr;
            addr += 4096)
        {
            FLASH_Write_Lock();
            FLASH_Erase(EraseSector, addr);
            FLASH_Write_Unlock();
        }

        for (uint32_t i = 0; i < 1024; i++)
        {
            uint32_t addr_to_write = address + i * 32;
            for (int j = 0; j < 32; j += 4) {
                val32_to_read = HAL_READ32(SPI_FLASH_BASE, addr_to_write + j);
                SEGGER_RTT_printf(0, "Initial Data 0x%x from 0x%08x\n", val32_to_read, addr_to_write + j);
            }

            SEGGER_RTT_printf(0, "%d: Write first byte 0x%x to 0x%08x\n", i, *((uint8_t *)buffer), addr_to_write);
            FLASH_Write_Lock();

            FLASH_TxData(addr_to_write, 32, (uint8_t *)buffer);
            WDG_Refresh(IWDG_DEV);

            FLASH_Write_Unlock();
            DCache_Invalidate(SPI_FLASH_BASE + addr_to_write, 32);

            for (int j = 0; j < 32; j += 4) {
                val32_to_read = HAL_READ32(SPI_FLASH_BASE, addr_to_write + j);
                SEGGER_RTT_printf(0, "Read Data 0x%x from 0x%08x\n", val32_to_read, addr_to_write + j);
            }


        }



        // verify result
        // result = (val32_to_write == val32_to_read) ? 1 : 0;
        // RTK_LOGI(NOTAG, "Result is %s\r\n", (result) ? "success" : "fail");
        // result = 0;
    }

    rtos_task_delete(NULL);
}

int example_raw_flash_read_write(void)
{
    if (rtos_task_create(NULL, ((const char *)"flash_test_task"), flash_test_task, NULL, 1024 * 4, 1) != RTK_SUCCESS)
    {
        RTK_LOGI(NOTAG, "\n\r%s rtos_task_create(flash_test_task) failed", __FUNCTION__);
    }

    rtos_sched_start();
    while (1)
    {
        rtos_time_delay_ms(1000);
    }

    return 0;
}
