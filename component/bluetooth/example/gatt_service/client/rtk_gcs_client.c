/*
 *******************************************************************************
 * Copyright(c) 2022, Realtek Semiconductor Corporation. All rights reserved.
 *******************************************************************************
 */

#include <stdio.h>
#include <string.h>

#include <rtk_bt_def.h>
#include <rtk_bt_common.h>
#include <rtk_bt_att_defs.h>
#include <rtk_bt_gattc.h>
#include <rtk_client_config.h>
#include <bt_utils.h>

#if !defined(RTK_BLE_MGR_LIB) || !RTK_BLE_MGR_LIB
void _print_uuid(uint8_t type, uint8_t *uuid)
{
	if (type == BT_UUID_TYPE_16) {
		BT_LOGA("%04x\r\n", LE_TO_U16(uuid));
		BT_AT_PRINT("%04x\r\n", LE_TO_U16(uuid));
	} else if (type == BT_UUID_TYPE_128) {
		BT_LOGA("%08x-%04x-%04x-%04x-%04x%08x\r\n",
				LE_TO_U32(uuid + 12), LE_TO_U16(uuid + 10),
				LE_TO_U16(uuid + 8), LE_TO_U16(uuid + 6),
				LE_TO_U16(uuid + 4), LE_TO_U32(uuid));
		BT_AT_PRINT("%08x-%04x-%04x-%04x-%04x%08x\r\n",
					LE_TO_U32(uuid + 12), LE_TO_U16(uuid + 10),
					LE_TO_U16(uuid + 8), LE_TO_U16(uuid + 6),
					LE_TO_U16(uuid + 4), LE_TO_U32(uuid));
	}
}

static void prop_str_joint(uint8_t order, char *dst_str, char *src_str)
{
	if (order) {
		strcat(dst_str, "/");
	}
	strcat(dst_str, src_str);
}

static char *gatt_charac_prop_str(uint8_t prop)
{
	static char str[90] = {0};
	uint8_t order = 0;

	memset(str, 0, sizeof(str));
	if (prop & RTK_BT_GATT_CHRC_BROADCAST) {
		prop_str_joint(order, str, "broadcast");
		order++;
	}
	if (prop & RTK_BT_GATT_CHRC_READ) {
		prop_str_joint(order, str, "read");
		order++;
	}
	if (prop & RTK_BT_GATT_CHRC_WRITE_WITHOUT_RESP) {
		prop_str_joint(order, str, "write_cmd");
		order++;
	}
	if (prop & RTK_BT_GATT_CHRC_WRITE) {
		prop_str_joint(order, str, "write_req");
		order++;
	}
	if (prop & RTK_BT_GATT_CHRC_NOTIFY) {
		prop_str_joint(order, str, "notify");
		order++;
	}
	if (prop & RTK_BT_GATT_CHRC_INDICATE) {
		prop_str_joint(order, str, "indicate");
		order++;
	}
	if (prop & RTK_BT_GATT_CHRC_AUTH) {
		prop_str_joint(order, str, "write_auth");
		order++;
	}
	if (prop & RTK_BT_GATT_CHRC_EXT_PROP) {
		prop_str_joint(order, str, "ext_prop");
		order++;
	}

	return str;
}

void general_client_discover_res_hdl(void *data)
{
	rtk_bt_gattc_discover_ind_t *disc_res = (rtk_bt_gattc_discover_ind_t *)data;
	rtk_bt_status_t disc_status = disc_res->status;

	if (RTK_BT_STATUS_CONTINUE == disc_status) {
		switch (disc_res->type) {
		case RTK_BT_GATT_DISCOVER_PRIMARY_ALL:
			BT_LOGA("[APP] GATTC discover primary result: start_handle: 0x%04x, end_handle: 0x%04x, UUID: ",
					disc_res->disc_primary_all_per.start_handle, disc_res->disc_primary_all_per.end_handle);
			BT_AT_PRINT("+BLEGATTC:disc,%d,%d,%u,0x%04x,0x%04x,",
						disc_status, disc_res->type, disc_res->conn_handle,
						disc_res->disc_primary_all_per.start_handle, disc_res->disc_primary_all_per.end_handle);
			_print_uuid(disc_res->disc_primary_all_per.uuid_type, disc_res->disc_primary_all_per.uuid);
			break;

		case RTK_BT_GATT_DISCOVER_PRIMARY_BY_UUID:
			BT_LOGA("[APP] GATTC discover primary by uuid result: start_handle: 0x%04x, end_handle: 0x%04x\r\n",
					disc_res->disc_primary_by_uuid_per.start_handle, disc_res->disc_primary_by_uuid_per.end_handle);
			BT_AT_PRINT("+BLEGATTC:disc,%d,%d,%u,0x%04x,0x%04x\r\n",
						disc_status, disc_res->type, disc_res->conn_handle,
						disc_res->disc_primary_by_uuid_per.start_handle, disc_res->disc_primary_by_uuid_per.end_handle);
			break;

		case RTK_BT_GATT_DISCOVER_INCLUDE:
			BT_LOGA("[APP] GATTC discover include result: handle: 0x%04x, start_handle: 0x%04x, end_handle: 0x%04x, UUID: ",
					disc_res->disc_inc_per.handle, disc_res->disc_inc_per.start_handle, disc_res->disc_inc_per.end_handle);
			BT_AT_PRINT("+BLEGATTC:disc,%d,%d,%u,0x%04x,0x%04x,0x%04x,",
						disc_status, disc_res->type, disc_res->conn_handle,
						disc_res->disc_inc_per.handle, disc_res->disc_inc_per.start_handle,
						disc_res->disc_inc_per.end_handle);
			_print_uuid(disc_res->disc_inc_per.uuid_type, disc_res->disc_inc_per.uuid);
			break;

		case RTK_BT_GATT_DISCOVER_CHARACTERISTIC_ALL:
		case RTK_BT_GATT_DISCOVER_CHARACTERISTIC_BY_UUID:
			BT_LOGA("[APP] GATTC discover characteristic of a service result: handle: 0x%04x, "
					"properties: 0x%02x(%s), value_handle: 0x%04x, UUID: ", disc_res->disc_char_all_per.handle,
					disc_res->disc_char_all_per.properties,
					gatt_charac_prop_str(disc_res->disc_char_all_per.properties),
					disc_res->disc_char_all_per.value_handle);
			BT_AT_PRINT("+BLEGATTC:disc,%d,%d,%u,0x%04x,0x%02x,0x%04x,",
						disc_status, disc_res->type, disc_res->conn_handle,
						disc_res->disc_char_all_per.handle, disc_res->disc_char_all_per.properties,
						disc_res->disc_char_all_per.value_handle);
			_print_uuid(disc_res->disc_char_all_per.uuid_type, disc_res->disc_char_all_per.uuid);
			break;

		case RTK_BT_GATT_DISCOVER_DESCRIPTORS_ALL:
			BT_LOGA("[APP] GATTC discover descriptor all result: handle: 0x%04x, UUID: ",
					disc_res->disc_descriptor_per.handle);
			BT_AT_PRINT("+BLEGATTC:disc,%d,%d,%u,0x%04x,",
						disc_status, disc_res->type, disc_res->conn_handle,
						disc_res->disc_descriptor_per.handle);
			_print_uuid(disc_res->disc_descriptor_per.uuid_type, disc_res->disc_descriptor_per.uuid);
			break;

		default:
			BT_LOGE("[APP] GATTC discover result event, wrong event type (%d)!\r\n", disc_res->type);
			break;
		}
	} else if (RTK_BT_STATUS_DONE == disc_status) {
		BT_LOGA("[APP] GATTC discover completed. profile_id: %d, conn_handle: %d, type: %d\r\n",
				disc_res->profile_id, disc_res->conn_handle, disc_res->type);
		BT_AT_PRINT("+BLEGATTC:disc,%d,%d,%d\r\n",
					disc_status, disc_res->conn_handle, disc_res->type);
		rtk_bt_gattc_write_param_t write_param = {0};
		write_param.conn_handle = 16;
		write_param.profile_id = GCS_CLIENT_PROFILE_ID;
		write_param.type = 1;
		write_param.handle = 41;
		write_param.length = 2;
		write_param.data = (void *)osif_mem_alloc(RAM_TYPE_DATA_ON, write_param.length);
		((uint8_t *)write_param.data)[0] = 0x01;
		((uint8_t *)write_param.data)[1] = 0x00;
		rtk_bt_gattc_write(&write_param);					
		return;
	} else {
		BT_LOGE("[APP] GATTC discover failed! profile_id: %d, conn_handle: %d, type: %d, err: 0x%x\r\n",
				disc_res->profile_id, disc_res->conn_handle, disc_res->type, disc_res->err_code);
		BT_AT_PRINT("+BLEGATTC:disc,%d,%d,%d,0x%x\r\n",
					disc_status, disc_res->conn_handle, disc_res->type,
					disc_res->err_code);
		return;
	}
}

void general_client_read_res_hdl(void *data)
{
	rtk_bt_gattc_read_ind_t *read_res = (rtk_bt_gattc_read_ind_t *)data;
	rtk_bt_status_t read_status = read_res->status;
	uint16_t handle = 1;

	if (RTK_BT_STATUS_FAIL == read_status) {
		BT_LOGE("[APP] GATTC read failed, profile_id: %d, conn_handle: %d, type: %d, err: 0x%x\r\n",
				read_res->profile_id, read_res->conn_handle, read_res->type, read_res->err_code);
		BT_AT_PRINT("+BLEGATTC:read,%d,%d,%d,0x%x\r\n",
					read_status, read_res->conn_handle,
					read_res->type, read_res->err_code);
		return;
	} else if (RTK_BT_STATUS_CONTINUE == read_status) {
		if (!read_res->by_handle.len || !read_res->by_handle.value) {
			BT_LOGE("[APP] GATT client read value is empty!\r\n");
			return;
		}
		switch (read_res->type) {
		case RTK_BT_GATT_CHAR_READ_BY_HANDLE:
			handle = read_res->by_handle.handle;
			BT_LOGA("[APP] GATT client read result, profile_id: %d, conn_handle: %d, type: %d, status: %d, handle: 0x%04x\r\n",
					read_res->profile_id, read_res->conn_handle, read_res->type, read_status, handle);
			BT_DUMPA("[APP] GATTC read result:\r\n", read_res->by_handle.value, read_res->by_handle.len);
			BT_AT_PRINT("+BLEGATTC:read,%d,%d,%d,0x%04x",
						read_status, read_res->conn_handle,
						read_res->type, handle);
			BT_AT_DUMP("", read_res->by_handle.value, read_res->by_handle.len);
			break;

		case RTK_BT_GATT_CHAR_READ_BY_UUID:
			handle = read_res->by_uuid_per.handle;
			BT_LOGA("[APP] GATT client read result, profile_id: %d, conn_handle: %d, type: %d, status: %d, handle: 0x%04x\r\n",
					read_res->profile_id, read_res->conn_handle, read_res->type, read_status, handle);
			BT_DUMPA("[APP] GATTC read result:\r\n", read_res->by_uuid_per.value, read_res->by_uuid_per.len);
			BT_AT_PRINT("+BLEGATTC:read,%d,%d,%d,0x%04x",
						read_status, read_res->conn_handle,
						read_res->type, handle);
			BT_AT_DUMP("", read_res->by_uuid_per.value, read_res->by_uuid_per.len);
			break;

		case RTK_BT_GATT_CHAR_READ_MULTIPLE_VARIABLE:
			BT_LOGA("[APP] GATT client read result, profile_id: %d, conn_handle: %d, type: %d, status: %d\r\n",
					read_res->profile_id, read_res->conn_handle, read_res->type, read_status);
			BT_DUMPA("[APP] GATTC read result:\r\n", read_res->multiple_variable_per.value, read_res->multiple_variable_per.len);
			BT_AT_PRINT("+BLEGATTC:read,%d,%d,%d",
						read_status, read_res->conn_handle, read_res->type);
			BT_AT_DUMP("", read_res->multiple_variable_per.value, read_res->multiple_variable_per.len);
			break;

		default:
			break;
		}
	} else if (RTK_BT_STATUS_DONE == read_status) {
		BT_LOGA("[APP] GATTC read completed, profile_id: %d, conn_handle: %d, type: %d\r\n",
				read_res->profile_id, read_res->conn_handle, read_res->type);
		BT_AT_PRINT("+BLEGATTC:read,%d,%d,%d\r\n",
					read_status, read_res->conn_handle, read_res->type);
	}

}

void general_client_write_res_hdl(void *data)
{
	rtk_bt_gattc_write_ind_t *write_res = (rtk_bt_gattc_write_ind_t *)data;
	rtk_bt_status_t write_status =  write_res->status;

	if (RTK_BT_STATUS_FAIL == write_status) {
		BT_LOGE("[APP] GATTC write failed, profile_id: %d, conn_handle: %d, type: %d, err: 0x%x\r\n",
				write_res->profile_id, write_res->conn_handle, write_res->type, write_res->err_code);
		BT_AT_PRINT("+BLEGATTC:write,%d,%d,%d,0x%x\r\n",
					write_status, write_res->conn_handle,
					write_res->type, write_res->err_code);
		return;
	} else if (RTK_BT_STATUS_DONE == write_status) {
		BT_LOGA("[APP] GATTC write completed, profile_id: %d, conn_handle: %d, type: %d, handle: 0x%x\r\n",
				write_res->profile_id, write_res->conn_handle, write_res->type, write_res->handle);
		BT_AT_PRINT("+BLEGATTC:write,%d,%d,%d,0x%x\r\n",
					write_status, write_res->conn_handle,
					write_res->type, write_res->handle);
	} else {
		return;
	}
}
#include "gpio_api.h"
int16_t pwm_value = 0;
uint8_t reverse_mode = 0;
void general_client_notify_hdl(void *data)
{
	rtk_bt_gattc_cccd_value_ind_t *ntf_ind = (rtk_bt_gattc_cccd_value_ind_t *)data;

	if (!ntf_ind->len || !ntf_ind->value) {
		BT_LOGE("[APP] GATTC notify received value is empty!\r\n");
		return;
	}
	BT_LOGA("[APP] GATTC notify received, profile_id: %d, conn_handle: %d, handle: 0x%x\r\n",
			ntf_ind->profile_id, ntf_ind->conn_handle, ntf_ind->value_handle);
	BT_DUMPA("[APP] GATTC notify event:\r\n", ntf_ind->value, ntf_ind->len);
	uint16_t left_pwm = 0;
	uint16_t right_pwm = 0;

	if (ntf_ind->value[0] == 0x01) {
		left_pwm = (pwm_value * 2 > 20000) ? 20000 : (pwm_value * 2);
	} else if (ntf_ind->value[0] == 0x02) {
		right_pwm = (pwm_value * 2 > 20000) ? 20000 : (pwm_value * 2);
	} else if (ntf_ind->value[0] == 0x03) {
		left_pwm = pwm_value;
		right_pwm = pwm_value;
	}
	else if (ntf_ind->value[5] == 0x01) {
		if (pwm_value + 1000 > 0) {
			reverse_mode = 0;
		}
		if ( reverse_mode ){
			pwm_value -= pwm_value > 0 ? 1000 : 0;
		}
		else {
			pwm_value += pwm_value < 20000 ? 1000 : 0;
		}

	} else if (ntf_ind->value[5] == 0xff) {
		if (pwm_value - 1000 < 0) {
			reverse_mode = 1;
		}
		if ( reverse_mode ){
			pwm_value += pwm_value < 20000 ? 1000 : 0;
		}
		else {
			pwm_value -= pwm_value > 0 ? 1000 : 0;
		}
	}
	printf("reverse_mode: %d\r\n", reverse_mode);


	if (reverse_mode) {
		RTIM_CCRxSet(TIMx[8], left_pwm, 0);
		RTIM_CCRxSet(TIMx[8], 0, 1);
		RTIM_CCRxSet(TIMx[8], right_pwm, 2);
		RTIM_CCRxSet(TIMx[8], 0, 3);
	} else {
		RTIM_CCRxSet(TIMx[8], 0, 0);
		RTIM_CCRxSet(TIMx[8], left_pwm, 1);
		RTIM_CCRxSet(TIMx[8], 0, 2);
		RTIM_CCRxSet(TIMx[8], right_pwm, 3);
	}

	printf("pwm_value: %d\r\n", pwm_value);

	BT_AT_PRINT("+BLEGATTC:notify,%d,0x%x",
				ntf_ind->conn_handle, ntf_ind->value_handle);
	BT_AT_DUMP("", ntf_ind->value, ntf_ind->len);
}

void general_client_indicate_hdl(void *data)
{
	rtk_bt_gattc_cccd_value_ind_t *indicate_ind = (rtk_bt_gattc_cccd_value_ind_t *)data;

	if (!indicate_ind->len || !indicate_ind->value) {
		BT_LOGE("[APP] GATTC indicate received value is empty!\r\n");
		return;
	}
	BT_LOGA("[APP] GATTC indicate received, profile_id: %d, conn_handle: %d, handle: 0x%x\r\n",
			indicate_ind->profile_id, indicate_ind->conn_handle, indicate_ind->value_handle);
	BT_DUMPA("[APP] GATTC indicate event:\r\n", indicate_ind->value, indicate_ind->len);
	BT_AT_PRINT("+BLEGATTC:indicate,%d,0x%x",
				indicate_ind->conn_handle, indicate_ind->value_handle);
	BT_AT_DUMP("", indicate_ind->value, indicate_ind->len);
}

void general_client_cccd_enable_hdl(void *data)
{
	rtk_bt_gattc_cccd_update_ind_t *cccd_update =
		(rtk_bt_gattc_cccd_update_ind_t *)data;
	rtk_bt_status_t status = cccd_update->status;
	if (RTK_BT_STATUS_DONE == status) {
		BT_LOGA("[APP] GATTC enable cccd succeed, profile_id: %d, conn_handle: %d, handle: 0x%x\r\n",
				cccd_update->profile_id, cccd_update->conn_handle, cccd_update->cccd_handle);
		BT_AT_PRINT("+BLEGATTC:en_cccd,0,%d,0x%x\r\n",
					cccd_update->conn_handle,
					cccd_update->cccd_handle);
	} else {
		BT_LOGE("[APP] GATTC enable cccd failed, profile_id: %d, conn_handle: %d, handle: 0x%x, err: 0x%x\r\n",
				cccd_update->profile_id, cccd_update->conn_handle,
				cccd_update->cccd_handle, cccd_update->err_code);
		BT_AT_PRINT("+BLEGATTC:en_cccd,-1,%d,0x%x,0x%x\r\n",
					cccd_update->conn_handle, cccd_update->cccd_handle,
					cccd_update->err_code);
	}
}

void general_client_cccd_disable_hdl(void *data)
{
	rtk_bt_gattc_cccd_update_ind_t *cccd_update = (rtk_bt_gattc_cccd_update_ind_t *)data;
	rtk_bt_status_t status = cccd_update->status;
	if (RTK_BT_STATUS_DONE == status) {
		BT_LOGA("[APP] GATTC disable cccd succeed, profile_id: %d, conn_handle: %d, handle: 0x%x\r\n",
				cccd_update->profile_id, cccd_update->conn_handle, cccd_update->cccd_handle);
		BT_AT_PRINT("+BLEGATTC:dis_cccd,0,%d,0x%x\r\n",
					cccd_update->conn_handle,
					cccd_update->cccd_handle);
	} else {
		BT_LOGE("[APP] GATTC disable cccd failed, profile_id: %d, conn_handle: %d, handle: 0x%x, err: 0x%x\r\n",
				cccd_update->profile_id, cccd_update->conn_handle, cccd_update->cccd_handle, cccd_update->err_code);
		BT_AT_PRINT("+BLEGATTC:dis_cccd,-1,%d,0x%x,0x%x\r\n",
					cccd_update->conn_handle, cccd_update->cccd_handle,
					cccd_update->err_code);
	}
}

rtk_bt_evt_cb_ret_t general_client_app_callback(uint8_t event, void *data)
{
	switch (event) {
	case RTK_BT_GATTC_EVT_DISCOVER_RESULT_IND:
		general_client_discover_res_hdl(data);
		break;
	case RTK_BT_GATTC_EVT_READ_RESULT_IND:
		general_client_read_res_hdl(data);
		break;
	case RTK_BT_GATTC_EVT_WRITE_RESULT_IND:
		general_client_write_res_hdl(data);
		break;
	case RTK_BT_GATTC_EVT_NOTIFY_IND:
		general_client_notify_hdl(data);
		break;
	case RTK_BT_GATTC_EVT_INDICATE_IND:
		general_client_indicate_hdl(data);
		break;
	case RTK_BT_GATTC_EVT_CCCD_ENABLE_IND:
		general_client_cccd_enable_hdl(data);
		break;
	case RTK_BT_GATTC_EVT_CCCD_DISABLE_IND:
		general_client_cccd_disable_hdl(data);
		break;
	default:
		break;
	}

	return RTK_BT_EVT_CB_OK;
}

uint16_t general_client_attach_conn(uint16_t conn_handle)
{
	(void)conn_handle;
	return RTK_BT_OK;
}

uint16_t general_client_detach_conn(uint16_t conn_handle)
{
	(void)conn_handle;
	return RTK_BT_OK;
}

uint16_t general_client_add(void)
{
	return rtk_bt_gattc_register_profile(GCS_CLIENT_PROFILE_ID);
}

uint16_t general_client_delete(void)
{
	return RTK_BT_OK;
}

#else
/* NEW GATT CLIENT does NOT support general client */
rtk_bt_evt_cb_ret_t general_client_app_callback(uint8_t event, void *data)
{
	(void)event;
	(void)data;
	return RTK_BT_EVT_CB_OK;
}

uint16_t general_client_attach_conn(uint16_t conn_handle)
{
	(void)conn_handle;
	return 0;
}

uint16_t general_client_detach_conn(uint16_t conn_handle)
{
	(void)conn_handle;
	return 0;
}

uint16_t general_client_add(void)
{
	return 0;
}

uint16_t general_client_delete(void)
{
	return 0;
}

#endif