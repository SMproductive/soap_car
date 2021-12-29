#include <math.h>
#include <stdio.h>
#include "driver/adc.h"
#include "driver/can.h"
#include "driver/gpio.h"

void can_init(gpio_num_t, gpio_num_t);
void adc_init(adc1_channel_t);

unsigned short int get_pedal_position(adc1_channel_t);
const adc1_channel_t pin_pedal = ADC1_CHANNEL_4; /* CHANNEL_0 = GPIO_32 */

void app_main(void)
{
	/* TODO
	 * Get power limit
	 * Get speed limit
	 * Calc Speed
	 * Send speed-power frame
	 */
	can_init(GPIO_NUM_21, GPIO_NUM_22);
	adc_init(pin_pedal);

	while(true) {
		get_pedal_position(pin_pedal);
	}
}

void can_init(gpio_num_t tx, gpio_num_t rx){
    can_filter_config_t filter_config = CAN_FILTER_CONFIG_ACCEPT_ALL();
    can_general_config_t general_config = CAN_GENERAL_CONFIG_DEFAULT(tx, rx, CAN_MODE_NORMAL);
	general_config.tx_queue_len = 0;
	general_config.alerts_enabled = 0;
	general_config.clkout_divider = 0;
    can_timing_config_t timing_config = CAN_TIMING_CONFIG_500KBITS();

    if (can_driver_install(&general_config, &timing_config, &filter_config) == ESP_OK) {
        printf("Driver installed\n");
    } else {
        printf("Failed to install driver\n");
        return;
    }

    if (can_start() == ESP_OK) {
        printf("Driver started\n");
    } else {
        printf("Failed to start driver\n");
        return;
    }
}

void adc_init(adc1_channel_t chan) {
	adc1_config_width(ADC_WIDTH_BIT_12);
	adc1_config_channel_atten(chan, ADC_ATTEN_DB_11);
}

unsigned short int get_pedal_position(adc1_channel_t chan){
	unsigned short int position;

	position = adc1_get_raw(chan)/pow(2,12);
	return position;
}
