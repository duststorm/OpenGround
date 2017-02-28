/*
 Copyright 2016 rav <AT> raav.info

 This program is free software: you can redistribute it and/ or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http:// www.gnu.org/ licenses/>.

 author: rav <AT> raav.info
 */

#include "main.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "multi4in1.h"
#include "debug.h"
#include "config.h"
#include "adc.h"
#include "storage.h"

#include "stm32f0xx_rcc.h"
#include "stm32f0xx_usart.h"

#define MULTI_CHANS 16
#define MULTI_CHAN_BITS 11

static inline int constrain(int amt, int low, int high) {
    if (amt < low)
        return low;
    else if (amt > high)
        return high;
    else
        return amt;
}

static uint8_t multi4in1_bind = 0;
static uint8_t multi4in1_range_check = 0;

void multi4in1_init(void) {
    debug("multi4in1: init usart\n");
    debug_flush();
    multi4in1_init_usart();

    debug("multi4in1: init timer\n");
    debug_flush();
    multi4in1_init_timer();

    debug("multi4in1: init done\n");
    debug_flush();
}

void multi4in1_init_usart(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    // Turn on USART1
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    // Turn on IO Port C
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOD, ENABLE);

    // Configure USART3 rx tx as push-pull
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // enable alternate function 1: usart
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_0);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_0);

    // set baudrate
    USART_StructInit(&USART_InitStructure);
    USART_InitStructure.USART_BaudRate = 100000;
    USART_InitStructure.USART_WordLength = USART_WordLength_9b; // required because of parity bit
    USART_InitStructure.USART_Parity = USART_Parity_Even;
    USART_InitStructure.USART_StopBits = USART_StopBits_2;
    USART_Init(USART3, &USART_InitStructure);

    // enable usart
    USART_Cmd(USART3, ENABLE);
}

void multi4in1_init_timer(void) {
    // init timer3 for 9ms
    TIM_TimeBaseInitTypeDef timebase_init;
    TIM_OCInitTypeDef oc_init;
    NVIC_InitTypeDef nvic_init;

    // pre-initialise structs
    TIM_TimeBaseStructInit(&timebase_init);
    TIM_OCStructInit(&oc_init);

    // TIM3 clock enable
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    // Enable the TIM3 gloabal Interrupt
    nvic_init.NVIC_IRQChannel = TIM3_IRQn;
    nvic_init.NVIC_IRQChannelPriority = NVIC_PRIO_FRSKY;
    nvic_init.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_init);

    // compute prescaler value
    // we want one ISR every 9ms
    // setting TIM_Period to 9000 will reuqire
    // a prescaler so that one timer tick is 1us (1MHz)
    uint16_t prescaler = (uint16_t)(SystemCoreClock / 1000000) - 1;

    // time base configuration as calculated above
    // timer counts with 1MHz thus 9000 ticks = 9ms
    timebase_init.TIM_Period = 9000 - 1;
    timebase_init.TIM_Prescaler = prescaler;
    timebase_init.TIM_ClockDivision = 0;
    timebase_init.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &timebase_init);

    // should be done by timebaseinit...
    // TIM_PrescalerConfig(TIM3, prescaler, TIM_PSCReloadMode_Immediate);

    // TIM Interrupts enable
    // DO NOT ENABLE IT YET
    // TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    // TIM3 enable counter
    TIM_Cmd(TIM3, ENABLE);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
}

void TIM3_IRQHandler(void) {
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

        multi4in1_send_packet();

        TIM_SetAutoreload(TIM3, 9000 - 1);
    }
}

void USART_SendDataBlocking(USART_TypeDef* USARTx, uint16_t data) {
    while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET) {  // tx buffer empty
    }
    USART_SendData(USARTx, data);
    // USART_FLAG_TC transmission complete
}

// based on https://github.com/opentx/opentx/blob/next/radio/src/pulses/multi_arm.cpp
// based on https://github.com/pascallanger/DIY-Multiprotocol-TX-Module/blob/master/Multiprotocol/Multiprotocol.h

void multi4in1_enable_bind() {
    multi4in1_bind = 1;
}

void multi4in1_send_packet() {
    MULTI_4IN1_DESC settings = storage.model[storage.current_model].multi_4in1_settings;
    uint8_t data;

    // header
    if (settings.protocol < 32) {
        USART_SendDataBlocking(USART3, 0x55);
    } else {
        USART_SendDataBlocking(USART3, 0x54);
    }

    // Stream[1] = sub_protocol|RangeCheckBit|AutoBindBit|BindBit;
    data = settings.protocol & 0x1f;
    data |= multi4in1_range_check << 5;
    data |= settings.auto_bind << 6;
    data |= multi4in1_bind << 7;
    USART_SendDataBlocking(USART3, data);

    // Stream[2] = RxNum | Type | Power;
    data = settings.rx_num & 0x0f;
    data |= (settings.sub_protocol & 0x07) << 4;
    data |= settings.low_power << 7;
    USART_SendDataBlocking(USART3, data);

    // Stream[3] = option_protocol;
    USART_SendDataBlocking(USART3, settings.option);

    // Stream[4] to [25] = Channels
    uint32_t bits = 0;
    uint8_t bitsavailable = 0;

    // fetch adc channel data
    uint16_t adc_data[8];
    uint32_t i;
    for (i = 0; i < 8; i++) {
        adc_data[i] = adc_get_channel_rescaled(i);
    }
    adc_process();

    for (int i = 0; i < MULTI_CHANS; i++) {
        int value = i < 8 ? adc_data[i] : 42;
        // Scale to 80%
        value = value * 800 / 1000 + 1024;
        bits |= constrain(value, 0, 2047) << bitsavailable;
        bitsavailable += MULTI_CHAN_BITS;
        while (bitsavailable >= 8) {
            USART_SendDataBlocking(USART3, (uint8_t)(bits & 0xff));
            bits >>= 8;
            bitsavailable -= 8;
        }
    }

    multi4in1_bind = 0;
}
