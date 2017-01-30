/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
 * @defgroup nrf_qdec_example main.c
 * @{
 * @ingroup nrf_qdec_example
 * @brief QDEC example application main file.
 *
 * This is an example quadrature decoder application.
 * The example requires that the QDEC A,B inputs are connected with the QENC A,B outputs and
 * the QDEC LED output is connected with the QDEC LED input.
 *
 * The example uses the software quadrature encoder simulator QENC.
 * The quadrature encoder simulator uses one channel of the GPIOTE module.
 * The state of the encoder changes on the inactive edge of the sampling clock generated by the LED output.
 *
 * In an infinite loop, QENC produces a variable number of positive and negative pulses
 * synchronously with bursts of clock impulses generated by QDEC at the LED output.
 * The pulses are counted by QDEC operating in a REPORT mode.
 * The pulses counted by QDEC are compared with the pulses generated by QENC.
 * The test stops if there is a difference between the number of pulses counted and generated.
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "nrf.h"
#include "bsp.h"
#include "nrf_delay.h"
#include "nrf_drv_qdec.h"
#include "nrf_error.h"
#include "app_error.h"
#include "nordic_common.h"
#define NRF_LOG_MODULE_NAME "APP"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

static volatile bool m_report_ready_flag = false;
static volatile bool m_first_report_flag = true;
static volatile uint32_t m_accdblread;
static volatile int32_t m_accread;

#if (QDEC_CONFIG_LEDPRE >= 128)
#warning "This example assumes that the QDEC LED changes state. Make sure that 'Sample Period' in QDEC config is less than 'LED pre-time'."
#endif


// 30 = 120
// 60 = 240
// This value is 4x the advertised pulse rate.
#define QENC_WINDOWS 120.00
#define TIME_UNIT_CONVERSION 1000000.00


float M_PIx2 = 2*3.1415927410125;

float l2f(uint32_t a) {
    float b;
    char* aPointer = (char*)&a, *bPointer = (char*)&b;
    memcpy(bPointer, aPointer, sizeof(a));
    return b;
}

static void qdec_event_handler(nrf_drv_qdec_event_t event) {
	// SAMPLERDY will fire every time there is a new sample, even if there is no
	// change. Meaning, you will get this event every “value of NRF_QDEC->SAMPLEPER”
	// SAMPLERDY is the more frequent of the two events.
	if (event.type == NRF_QDEC_EVENT_SAMPLERDY) {
		//	m_sampleread = event.data.sample.value;
		//	m_sample_ready_flag = true;
	} else
	// REPORTRDY is the container or parent event of samples.
	// REPORTRDY is generated if accumulator value has changed since last REPORTPER*SAMPLEPER.
	if (event.type == NRF_QDEC_EVENT_REPORTRDY) {

		m_accdblread = event.data.report.accdbl;
		m_accread = event.data.report.acc;
		m_report_ready_flag = true;

		/* Pulse counting method
		   	  *
		   	  * 2PIn/NT
		   	  * Angular Velocity
		   	  * Sample Period
		   	  * Pulses During Sample Period
		   	  * Average Time For One Pulse
		   	  * Windows On The Disc
		   	  */

		uint32_t int_reportper = nrf_qdec_reportper_to_value(QDEC_CONFIG_REPORTPER);
		float f_reportper = (float)int_reportper;

	    uint32_t int_sampleper = nrf_qdec_sampleper_to_value(QDEC_CONFIG_SAMPLEPER);
	    float f_sampleper = (float)int_sampleper;

	    float M_PIx2_over_windows_x_sample_period_len = M_PIx2 / (QENC_WINDOWS * (f_reportper * f_sampleper/TIME_UNIT_CONVERSION));
	 	float angular_velocity = (float)m_accread * M_PIx2_over_windows_x_sample_period_len;

	 	NRF_LOG_INFO("m_accread: %d\n", m_accread);
	 	NRF_LOG_INFO("m_accread: " NRF_LOG_FLOAT_MARKER "\r\n", NRF_LOG_FLOAT(m_accread));
	    NRF_LOG_INFO("int_sampleper: %d\n", int_sampleper);
	    NRF_LOG_INFO("int_reportper: %d\n", int_reportper);
		NRF_LOG_INFO("Pi * 2 : " NRF_LOG_FLOAT_MARKER "\r\n", NRF_LOG_FLOAT(M_PIx2));
		NRF_LOG_INFO("QENC_WINDOWS : " NRF_LOG_FLOAT_MARKER "\r\n", NRF_LOG_FLOAT(QENC_WINDOWS));
		NRF_LOG_INFO("f_sampleper : " NRF_LOG_FLOAT_MARKER "\r\n", NRF_LOG_FLOAT(f_sampleper));
		NRF_LOG_INFO("f_reportper : " NRF_LOG_FLOAT_MARKER "\r\n", NRF_LOG_FLOAT(f_reportper));
		NRF_LOG_INFO("TIME_UNIT_CONVERSION : " NRF_LOG_FLOAT_MARKER "\r\n", NRF_LOG_FLOAT(TIME_UNIT_CONVERSION));
		NRF_LOG_INFO("M_PIx2_over_windows_x_sample_period_len : " NRF_LOG_FLOAT_MARKER "\r\n", NRF_LOG_FLOAT(M_PIx2_over_windows_x_sample_period_len));
		NRF_LOG_INFO("angular_velocity : " NRF_LOG_FLOAT_MARKER "\r\n", NRF_LOG_FLOAT(angular_velocity));

	}
	NRF_LOG_INFO("event\r\n");
}

int main(void) {
	uint32_t err_code;

	err_code = NRF_LOG_INIT(NULL);
	APP_ERROR_CHECK(err_code);

	// Initialize hardware
	err_code = nrf_drv_qdec_init(NULL, qdec_event_handler);
	APP_ERROR_CHECK(err_code);

	NRF_LOG_INFO("QDEC testing started\r\n");
	nrf_drv_qdec_enable();
	while (true) {
		__WFE();
	}
}

/** @} */
