
#include "ameba_soc.h"
#include "os_wrapper.h"
#include "main.h"
#if (defined CONFIG_WHC_HOST || defined CONFIG_WHC_NONE || defined CONFIG_WHC_WPA_SUPPLICANT_OFFLOAD)
#include "vfs.h"
#endif
#include "ameba_rtos_version.h"
#ifdef CONFIG_MBEDTLS_ENABLED
#include "ssl_rom_to_ram_map.h"
#include "threading_alt.h"
#endif
//#include "wifi_fast_connect.h"
#if defined(CONFIG_BT_COEXIST)
#include "rtw_coex_ipc.h"
#endif
#include "ameba_diagnose.h"

static const char *const TAG = "MAIN";

#if (defined(CONFIG_BT) && CONFIG_BT) && (defined(CONFIG_BT_INIC) && CONFIG_BT_INIC)
#include "bt_inic.h"
#endif

int ble_central_main(uint8_t enable);

void app_init_debug(void)
{
	u32 debug[LEVEL_NUMs];

	debug[LEVEL_ERROR] = 0xFFFFFFFF;
	debug[LEVEL_INFO]  = 0x0;
	debug[LEVEL_WARN]  = 0x0;
	debug[LEVEL_TRACE] = 0x0;

	LOG_MASK(LEVEL_ERROR, debug[LEVEL_ERROR]);
	LOG_MASK(LEVEL_WARN,  debug[LEVEL_WARN]);
	LOG_MASK(LEVEL_INFO,  debug[LEVEL_INFO]);
	LOG_MASK(LEVEL_TRACE, debug[LEVEL_TRACE]);
}

#ifdef CONFIG_MBEDTLS_ENABLED
void app_mbedtls_rom_init(void)
{
	CRYPTO_Init(NULL);
	CRYPTO_SHA_Init(NULL);
	ssl_function_map.ssl_calloc = (void *(*)(unsigned int, unsigned int))rtos_mem_calloc;
	ssl_function_map.ssl_free = (void (*)(void *))rtos_mem_free;
	ssl_function_map.ssl_printf = (long unsigned int (*)(const char *, ...))DiagPrintf;
	ssl_function_map.ssl_snprintf = (int (*)(char *s, size_t n, const char *format, ...))DiagSnPrintf;
#if defined(CONFIG_MBEDTLS_THREADING)
	mbedtls_threading_init();
#endif
}
#endif

void app_pmu_init(void)
{
	DBG_INFO_MSG_ON(MODULE_PMC);
	pmu_set_sleep_type(SLEEP_PG);
	pmu_init_wakeup_timer();
#ifndef CONFIG_MP_SHRINK
	SOCPS_SleepInit();
#endif
}

void app_calc_new_time(RTCIO_TimeInfo *pTimeInfoPre, RTC_TimeTypeDef *pTimeInfoPost)
{
	u8 IsLeapYear = 0;
	u32 CntTotal = 0;
	u32 modnum[4] = {60, 60, 24, 365};	//convert to: m h d y

	CntTotal = (pTimeInfoPre->Bkup_Year) + 1900;//total year
	IS_LEAP_YEAR_CHECK(CntTotal, IsLeapYear);
	modnum[3] += IsLeapYear;

	CntTotal = pTimeInfoPre->Pwd_Counter;

	TIME_STEP(pTimeInfoPre->Bkup_Seconds, CntTotal, modnum[0], pTimeInfoPost->RTC_Seconds);
	TIME_STEP(pTimeInfoPre->Bkup_Minutes, CntTotal, modnum[1], pTimeInfoPost->RTC_Minutes);
	TIME_STEP(pTimeInfoPre->Bkup_Hours, CntTotal, modnum[2], pTimeInfoPost->RTC_Hours);
	TIME_STEP(pTimeInfoPre->Bkup_Days, CntTotal, modnum[3], pTimeInfoPost->RTC_Days);
	TIME_STEP(((pTimeInfoPre->Bkup_Year) + 1900), CntTotal, 0xFFFF, pTimeInfoPost->RTC_Year);

	pTimeInfoPost->RTC_H12_PMAM = RTC_HourFormat_24;

}

void app_rtc_init(void)
{
	RTCIO_TimeInfo RTCIO_TimeStruct;
	RTC_InitTypeDef RTC_InitStruct;
	RTC_TimeTypeDef RTC_TimeStruct;

	if (BOOT_Reason() == 0) {
		if (RTCIO_IsEnabled() == TRUE) {
			/* shift out bkup data */
			RTCIO_GetTimeInfo(&RTCIO_TimeStruct);

			/* update RTC Time */
			app_calc_new_time(&RTCIO_TimeStruct, &RTC_TimeStruct);
		} else {
			RTC_TimeStructInit(&RTC_TimeStruct);
			RTC_TimeStruct.RTC_Year = 2021;
			RTC_TimeStruct.RTC_Hours = 10;
			RTC_TimeStruct.RTC_Minutes = 20;
			RTC_TimeStruct.RTC_Seconds = 30;
		}

		/* Only Asic need OSC Calibration */
		if (SYSCFG_CHIPType_Get() == CHIP_TYPE_ASIC_POSTSIM) {
			/* reset rcal to avoid rvals conflict in aon and rtcio domain */
			RTCIO_SetRValue(RTCIO_RECV_RVAL_RST);
			OSC131K_Calibration(30000);
			RTCIO_SetRValue(RTCIO_RECV_RVAL_CAL);
		}

		//share process for both rtcio enabled/disabled
		RCC_PeriphClockCmd(APBPeriph_RTC, APBPeriph_RTC_CLOCK, ENABLE);
		RTC_StructInit(&RTC_InitStruct);
		RTC_Init(&RTC_InitStruct);

		RTC_SetTime(RTC_Format_BIN, &RTC_TimeStruct);
	}

}

#ifdef CONFIG_VFS_ENABLED
extern uint32_t vfs_ftl_init(void);
extern int vfs_kv_init(void);
void app_filesystem_init(void)
{
	int ret = 0;
	vfs_init();

	vfs_user_register(VFS_PREFIX, VFS_LITTLEFS, VFS_INF_FLASH, VFS_REGION_1, VFS_RW);
	ret = vfs_kv_init();
	if (ret == 0) {
		RTK_LOGI(TAG, "File System Init Success \n");
	} else {
		RTK_LOGE(TAG, "File System Init Fail \n");
	}

#ifdef CONFIG_FATFS_WITHIN_APP_IMG
	ret = vfs_user_register(VFS_R3_PREFIX, VFS_FATFS, VFS_INF_FLASH, VFS_REGION_3, VFS_RO);
	if (ret == 0) {
		RTK_LOGI(TAG, "VFS-FAT Init Success \n");
	} else {
		RTK_LOGI(TAG, "VFS-FAT Init Fail \n");
	}
#endif

#if defined(CONFIG_FTL_ENABLED) && CONFIG_FTL_ENABLED
	vfs_ftl_init();
#endif
}
#endif

/*
 * This function will be replaced when Sdk example is compiled using CMD "make EXAMPLE=xxx" or "make xip xxx"
 * To aviod compile error when example is not compiled
 */
_WEAK void app_pre_example(void)
{


}

_WEAK void app_example(void)
{


}

#define GPIO_SIGNAL_SOURCE		_PB_10

// void my_gpio_init(void)
// {
// 	GPIO_InitTypeDef GPIO_InitStruct_Source;

// 	printf("example_raw_gpio_level_irq \n");



// 	/* init gpio source pin */
// 	GPIO_InitStruct_Source.GPIO_Pin = GPIO_SIGNAL_SOURCE;
// 	GPIO_InitStruct_Source.GPIO_Mode = GPIO_Mode_OUT;
// 	GPIO_Init(&GPIO_InitStruct_Source);

// 	// while (1) {
// 	// 	GPIO_WriteBit(GPIO_SIGNAL_SOURCE, 1);
// 	// 	rtos_time_delay_ms(1000);

// 	// 	GPIO_WriteBit(GPIO_SIGNAL_SOURCE, 0);
// 	// 	rtos_time_delay_ms(1000);
// 	// }
// }
#include "device.h"
#include "platform_autoconf.h"

#define PWM_TIMER		8
#define PWM_PRESCALER	39
#define PWM_PERIOD		20000
#define PWM_STEP		(PWM_PERIOD / 200)  //Brightness change speed
#define PWM_CHANNEL_MAX		8
int pwms[PWM_CHANNEL_MAX] = {0, PWM_PERIOD / 8, PWM_PERIOD / 4, PWM_PERIOD / 8 * 3, PWM_PERIOD / 2, PWM_PERIOD / 8 * 5, \
							 PWM_PERIOD / 4 * 3, PWM_PERIOD / 8 * 7
							};
int steps[PWM_CHANNEL_MAX] = {PWM_STEP, PWM_STEP, PWM_STEP, PWM_STEP, PWM_STEP, PWM_STEP, PWM_STEP, PWM_STEP};
void raw_pwm_demo(void)
{
	RTIM_TimeBaseInitTypeDef RTIM_InitStruct;
	TIM_CCInitTypeDef TIM_CCInitStruct;
	/* close swdclk/swdio*/
	Pinmux_Swdoff();
	/* Enable TIM_PWM function & clock */
	RCC_PeriphClockCmd(APBPeriph_TIMx[PWM_TIMER], APBPeriph_TIMx_CLOCK[PWM_TIMER], ENABLE);

	RTIM_TimeBaseStructInit(&RTIM_InitStruct);
	RTIM_InitStruct.TIM_Idx = PWM_TIMER;
	RTIM_InitStruct.TIM_Prescaler = PWM_PRESCALER;
	RTIM_InitStruct.TIM_Period = PWM_PERIOD - 1;
	RTIM_TimeBaseInit(TIMx[PWM_TIMER], (&RTIM_InitStruct), TIMx_irq[PWM_TIMER], NULL, NULL);

	for (int i = 0; i < 2; i++) {
		RTIM_CCStructInit(&TIM_CCInitStruct);
		TIM_CCInitStruct.TIM_OCPulse = pwms[i];
		RTIM_CCxInit(TIMx[PWM_TIMER], &TIM_CCInitStruct, i);
		RTIM_CCxCmd(TIMx[PWM_TIMER], i, TIM_CCx_Enable);
	}
	Pinmux_Config(_PB_18, (PINMUX_FUNCTION_PWM0 + 0));
	Pinmux_Config(_PB_10, (PINMUX_FUNCTION_PWM0 + 1));

	RTIM_Cmd(TIMx[PWM_TIMER], ENABLE);
}
#include <rtk_bt_def.h>
#include <rtk_bt_common.h>
#include <rtk_bt_le_gap.h>
#include <bt_utils.h>
static rtk_bt_le_create_conn_param_t conn_param = {
	.peer_addr = {
		.type = (rtk_bt_le_addr_type_t)1,
		.addr_val = {0},
	},
	.scan_interval = 0x60,
	.scan_window = 0x30,
	.filter_policy = RTK_BT_LE_CONN_FILTER_WITHOUT_WHITELIST,
	.conn_interval_max = 0x60,
	.conn_interval_min = 0x60,
	.conn_latency      = 0,
	.supv_timeout      = 0x100,
	.scan_timeout      = 1000,
};

void main_task(void *param)
{
	UNUSED(param);
	ble_central_main(1);
	// int argc = 3;
	// char *argv[] = {"conn", "1", "d8c2822d6ff1"};
	// atcmd_ble_gap_connect(argc, argv);

	hexdata_str_to_bd_addr("d8c2822d6ff1", conn_param.peer_addr.addr_val, 6);
	rtk_bt_le_gap_connect(&conn_param);
	while (1) 
	{
		rtos_time_delay_ms(1000);
	}
}

//default main
int main(void)
{
	RTK_LOGI(TAG, "KM4 MAIN \n");
	ameba_rtos_get_version();
	/* Debug log control */
	app_init_debug();

	InterruptRegister(IPC_INTHandler, IPC_KM4_IRQ, (u32)IPCKM4_DEV, INT_PRI5);
	InterruptEn(IPC_KM4_IRQ, INT_PRI5);

	/*IPC table initialization*/
	ipc_table_init(IPCKM4_DEV);

#ifdef CONFIG_VFS_ENABLED
	app_filesystem_init();
#endif


#ifdef CONFIG_MBEDTLS_ENABLED
	app_mbedtls_rom_init();
#endif
	/* pre-processor of application example */
	app_pre_example();

#if defined(CONFIG_WIFI_FW_EN) && CONFIG_WIFI_FW_EN
	wififw_task_create();
#endif

#if defined(CONFIG_BT_COEXIST)
	/* init coex ipc */
	coex_ipc_entry();
#endif

#if defined(CONFIG_WLAN)
	wifi_init();
#endif

	/* initialize BT iNIC */
#if (defined(CONFIG_BT) && CONFIG_BT) && (defined(CONFIG_BT_INIC) && CONFIG_BT_INIC)
	bt_inic_init();
#endif

#ifdef CONFIG_SHELL
	/* init console */
	shell_init_rom(0, 0);
	shell_init_ram();
#endif

	app_pmu_init();
#if defined(CONFIG_RTCIO_ENABLED) && CONFIG_RTCIO_ENABLED
	app_rtc_init();
#endif

	rtk_diag_init(RTK_DIAG_HEAP_SIZE, RTK_DIAG_SEND_BUFFER_SIZE);

	/* Execute application example */
	raw_pwm_demo();
	rtos_task_create(NULL, ((const char *)"main_task"), main_task, NULL, 8192, 5);
	app_example();
	IPC_patch_function(&rtos_critical_enter, &rtos_critical_exit);
	IPC_SEMDelay(rtos_time_delay_ms);


	RTK_LOGI(TAG, "KM4 START SCHEDULER \n");

	/* Enable Schedule, Start Kernel */
	rtos_sched_start();
}


