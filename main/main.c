/* goals
send signal from cellphone to esp32 to lightup 7-segment display
    - use volume buttons (AVRCP) to increase and decrease number displayed
        - learn how display works   OK
        - manage to recieve volume change events from cellphone
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_log.h"

#include "esp_bt.h"
#include "bt_app_core.h"
#include "bt_app_av.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"

#include "7-segment-display.h"


#define D7S_PIN_A   16
#define D7S_PIN_B   17
#define D7S_PIN_C   18
#define D7S_PIN_D   19
#define D7S_PIN_E   21
#define D7S_PIN_F   22
#define D7S_PIN_G   23
#define D7S_PIN_DP  27

/* device name */
#define LOCAL_DEVICE_NAME    "ESP_BT_DEVICE"

/* event for stack up */
enum {
    BT_APP_EVT_STACK_UP = 0,
};

static const char *bt_gap_mode_str[] = { "Active", "Hold", "Sniff", "Park" };


#define POSSIBLE_VOLUMES_SIZE 16
static const uint8_t possible_volumes[POSSIBLE_VOLUMES_SIZE] = {0, 7, 15, 23, 31, 39, 47, 55, 63, 71, 79, 87, 95, 103, 111, 119};

static d7s_ios_num pin_numbers = {
    .sega_io_num = D7S_PIN_A,
    .segb_io_num = D7S_PIN_B,
    .segc_io_num = D7S_PIN_C,
    .segd_io_num = D7S_PIN_D,
    .sege_io_num = D7S_PIN_E,
    .segf_io_num = D7S_PIN_F,
    .segg_io_num = D7S_PIN_G,
    .segdp_io_num = D7S_PIN_DP,
};

/********************************
 * STATIC FUNCTION DECLARATIONS
 *******************************/

/* GAP callback function */
static void bt_app_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param);
/* handler for bluetooth stack enabled events */
static void bt_av_hdl_stack_evt(uint16_t event, void *p_param);

/*******************************
 * STATIC FUNCTION DEFINITIONS
 ******************************/

static void bt_av_hdl_stack_evt(uint16_t event, void *p_param)
{
    ESP_LOGD(BT_AV_TAG, "%s event: %d", __func__, event);

    switch (event) {
    /* when do the stack up, this event comes */
    case BT_APP_EVT_STACK_UP: {
        // set device bt name
        ESP_ERROR_CHECK(esp_bt_dev_set_device_name(LOCAL_DEVICE_NAME));

        // -- GAP setup
        // set the callback for Generic Acess Profile(GAP) events
        ESP_ERROR_CHECK(esp_bt_gap_register_callback(bt_app_gap_cb));
        // --

        // -- AVRCP setup
        assert(esp_avrc_tg_init() == ESP_OK);   // Initialize the AVRCP target module
        ESP_ERROR_CHECK(esp_avrc_tg_register_callback(bt_app_rc_tg_cb)); // set the callback for AVRCP target events
        // --

        // add notify event capability
        esp_avrc_rn_evt_cap_mask_t evt_set = {0};
        esp_avrc_rn_evt_bit_mask_operation(ESP_AVRC_BIT_MASK_OP_SET, &evt_set, ESP_AVRC_RN_VOLUME_CHANGE);
        assert(esp_avrc_tg_set_rn_evt_cap(&evt_set) == ESP_OK);

        assert(esp_a2d_sink_init() == ESP_OK);
        esp_a2d_register_callback(&bt_app_a2d_cb);

        /* set discoverable and connectable mode, wait to be connected */
        ESP_LOGI(BT_AV_TAG, "Start scan mode");
        ESP_ERROR_CHECK(esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE));
        break;
    }
    /* others */
    default:
        ESP_LOGE(BT_AV_TAG, "%s unhandled event: %d", __func__, event);
        break;
    }
}

static void bt_app_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    switch (event) {
    /* when authentication completed, this event comes */
    case ESP_BT_GAP_AUTH_CMPL_EVT: {
        if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
            ESP_LOGI(BT_AV_TAG, "authentication success: %s", param->auth_cmpl.device_name);
            esp_log_buffer_hex(BT_AV_TAG, param->auth_cmpl.bda, ESP_BD_ADDR_LEN);
        } else {
            ESP_LOGE(BT_AV_TAG, "authentication failed, status: %d", param->auth_cmpl.stat);
        }
        break;
    }

#if (CONFIG_BT_SSP_ENABLED == true)
    /* when Security Simple Pairing user confirmation requested, this event comes */
    case ESP_BT_GAP_CFM_REQ_EVT:
        ESP_LOGI(BT_AV_TAG, "ESP_BT_GAP_CFM_REQ_EVT Please compare the numeric value: %d", param->cfm_req.num_val);
        esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
        break;
    /* when Security Simple Pairing passkey notified, this event comes */
    case ESP_BT_GAP_KEY_NOTIF_EVT:
        ESP_LOGI(BT_AV_TAG, "ESP_BT_GAP_KEY_NOTIF_EVT passkey: %d", param->key_notif.passkey);
        break;
    /* when Security Simple Pairing passkey requested, this event comes */
    case ESP_BT_GAP_KEY_REQ_EVT:
        ESP_LOGI(BT_AV_TAG, "ESP_BT_GAP_KEY_REQ_EVT Please enter passkey!");
        break;
#endif

    /* when GAP mode changed, this event comes */
    case ESP_BT_GAP_MODE_CHG_EVT:
        ESP_LOGI(BT_AV_TAG, "ESP_BT_GAP_MODE_CHG_EVT mode: %s", bt_gap_mode_str[param->mode_chg.mode]);
        break;
    /* others */
    default: {
        ESP_LOGI(BT_AV_TAG, "event: %d", event);
        break;
    }
    }
}

void update_display(void *param)
{
    uint8_t converted_volume = 0;
    while (true)
    {
        if (s_volume > possible_volumes[POSSIBLE_VOLUMES_SIZE-1])
        {
            converted_volume = 0x10;
        }
        else
        {
            for (uint8_t i = 0; i < POSSIBLE_VOLUMES_SIZE; i++)
            {
                if (s_volume == possible_volumes[i])
                {
                    converted_volume = i;
                }
            }
        }
        
        set_display_number(pin_numbers, converted_volume);

        vTaskDelay(300 / portTICK_PERIOD_MS);
    }
    
}

/*******************************
 * INIT FUNCTION
 ******************************/

void init_function(void)
{
    // init 7-segment display pins
    init_display_pins(pin_numbers);

    // == BT initializations

    // Using Classical Bluetooth, so release the controller memory for Bluetooth Low Energy.
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_err_t err;
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((err = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }
    if ((err = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }
    if ((err = esp_bluedroid_init()) != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s initialize bluedroid failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }
    if ((err = esp_bluedroid_enable()) != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s enable bluedroid failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }

#if (CONFIG_BT_SSP_ENABLED == true)
    /* set default parameters for Secure Simple Pairing */
    esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
    esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_IO;
    ESP_ERROR_CHECK(esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t)));
#endif

    /* set default parameters for Legacy Pairing (use fixed pin code 1234) */
    esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_FIXED;
    esp_bt_pin_code_t pin_code;
    pin_code[0] = '1';
    pin_code[1] = '2';
    pin_code[2] = '3';
    pin_code[3] = '4';
    ESP_ERROR_CHECK(esp_bt_gap_set_pin(pin_type, 4, pin_code));

}

/*******************************
 * MAIN ENTRY POINT
 ******************************/

void app_main(void)
{
    /* initialize NVS — it is used to store PHY calibration data */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // init 7-segment and BT services
    init_function();

    // starts-up the bt_app queue
    bt_app_task_start_up();

    // dispach BT_APP_EVT_STACK_UP event
    bt_app_work_dispatch(bt_av_hdl_stack_evt, BT_APP_EVT_STACK_UP, NULL, 0, NULL);

    xTaskCreate(&update_display, "update_display_task", 1024, NULL, 5, NULL);
}