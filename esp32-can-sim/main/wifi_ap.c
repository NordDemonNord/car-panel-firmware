/*

    Name:       wifi_ap.c
    Purpose:    init soft AP for remote control by Wi-Fi though HTTP server
    Author:     Nord

    Current state:  set up soft AP "esp32-can-sim"

    TODO:           make an HTTP server with interactive elements

*/

#include "wifi_ap.h" /* header */

#include <stdint.h>

#include "esp_wifi.h"
#include "nvs_flash.h"


/* --- AP configuration --- */

#define AP_SSID     "esp32-can-sim" /* AP name */
#define AP_PASSWORD "01234567"      /* AP password */
#define AP_CHANNEL  1               /* 2.4 GHz frequence */
#define AP_MAX_CONN 2               /* maximum of connections */


void wifi_init(void)

{

    /* --- Variables --- */

    const wifi_init_config_t wifi_conf = WIFI_INIT_CONFIG_DEFAULT(); /* defoult wifi driver parametrs */

    wifi_mode_t mode = WIFI_MODE_AP; /* wifi mode as soft AP */

    wifi_interface_t interface_conf = WIFI_IF_AP; /* soft-AP for conf function */

    /* AP configuration */
    wifi_config_t ap_config = 

    {

        .ap = 

        {

            .ssid               = AP_SSID,
            .password           = AP_PASSWORD,
            .channel            = AP_CHANNEL,
            .max_connection     = AP_MAX_CONN,
            .authmode           = WIFI_AUTH_WPA2_PSK,
        
        },

    };

    /* --- Main part --- */

    /* NVS storage check for errors, only one time */

    esp_err_t nvs_check = nvs_flash_init();

    /* check for two errors */

    if ( nvs_check == ESP_ERR_NVS_NO_FREE_PAGES || nvs_check == ESP_ERR_NVS_NEW_VERSION_FOUND )

    {

        /* erase, repeat if there's one of two main errors and remember the status */

        ESP_ERROR_CHECK( nvs_flash_erase() );

        nvs_check = nvs_flash_init();

    }

    ESP_ERROR_CHECK( nvs_check ); /* if there's another error */    

    ESP_ERROR_CHECK( esp_netif_init() ); /* tcp/ip stack for DHCP and HTTPS */

    ESP_ERROR_CHECK( esp_event_loop_create_default() ); /* for getting info of connection status */
    
    esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();
    assert(ap_netif != NULL); 

    /* --- Init --- */

    ESP_ERROR_CHECK (esp_wifi_init( &wifi_conf ) ); /* init wifi driver with defolt parametrs */

    /* --- Set AP mode --- */

    ESP_ERROR_CHECK( esp_wifi_set_mode( mode ) );  /* set soft AP mode */

    /* --- Set the configuration before start the AP --- */

    ESP_ERROR_CHECK( esp_wifi_set_config( interface_conf, &ap_config ) );

    /* --- Start Wi-Fi with current config --- */

    ESP_ERROR_CHECK( esp_wifi_start() ); /* start soft AP */

}