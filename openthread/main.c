/*
 * Copyright (C) 2017 Baptiste CLENET
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @file
 * @brief       OpenThread test application
 *
 * @author      Baptiste Clenet <bapclenet@gmail.com>
 */

#include <stdio.h>
#include <rtt_stdio.h>
#include "ot.h"
//#include "periph/dmac.h"
#include "periph/adc.h"
#include "periph/i2c.h"
#include "periph/spi.h"

#ifndef SAMPLE_INTERVAL
#define SAMPLE_INTERVAL (2000000000UL)
#endif

int main(void)
{
    puts("This a test for OpenThread");
    /* Example of how to call OpenThread stack functions */
    puts("Get PANID ");
#if DMAC_ENABLE
    dmac_init();
    adc_set_dma_channel(DMAC_CHANNEL_ADC);
    i2c_set_dma_channel(I2C_0,DMAC_CHANNEL_I2C);
    spi_set_dma_channel(0,DMAC_CHANNEL_SPI_TX,DMAC_CHANNEL_SPI_RX);
#endif 
   //uint16_t panid = 0;
    //uint8_t res = ot_call_command("panid", NULL, (void*)&panid);
    //printf("Current panid: 0x%x (res:%x)\n", panid, res);
	while (1) {
		//Sample
	    //sample(&frontbuf);
		//aes_populate();
		//Send
		//send_udp("ff02::1",4747,obuffer,sizeof(obuffer));
		//Sleep
		//xtimer_periodic_wakeup(&last_wakeup, interval_with_jitter());
		xtimer_usleep(SAMPLE_INTERVAL);
    }
    return 0;
}
