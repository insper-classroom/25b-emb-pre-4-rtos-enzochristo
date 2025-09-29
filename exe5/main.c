/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>


#include <stdio.h>
#include <string.h> 
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

const int BTN_PIN_R = 28;
const int BTN_PIN_Y = 21;

const int LED_PIN_R = 5;
const int LED_PIN_Y = 10;

QueueHandle_t xQueueBtn;
SemaphoreHandle_t xSemaphoreLedR, xSemaphoreLedY;


void btn_callback(uint gpio, uint32_t events) {
    int static flag_y = 0, flag_r = 0;
    if (events == 0x4) { // fall edge
        if (gpio == BTN_PIN_R){
            // printf("passei");
            flag_r = 1;
            xQueueSendFromISR(xQueueBtn, &flag_r, 0);

        }
        else{
            flag_y = 2;
            xQueueSendFromISR(xQueueBtn, &flag_y, 0);
        }
    }
}



void btn_task(void* p) {


    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);

    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    // Define o callback E habilita a IRQ para o primeiro pino
    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &btn_callback);

    // Apenas habilita a IRQ para os pinos seguintes
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true);

    int flag = 0;
    int ligou_r = 0;
    int ligou_y = 0;

    while (true) {
        if(xQueueReceiveFromISR(xQueueBtn, &flag,0  )){
            printf("flag:  %d", flag);
        }
        if(flag == 1){
            ligou_r = !ligou_r;
            flag = 0;
        }

        if (flag == 2){
            ligou_y = !ligou_y;
            flag = 0;
        }

        if(ligou_r){
            xSemaphoreGive(xSemaphoreLedR);
        }
          
        if(ligou_y){
            xSemaphoreGive(xSemaphoreLedY);
        }

        if (ligou_r && ligou_y){
            xSemaphoreGive(xSemaphoreLedY);
            xSemaphoreGive(xSemaphoreLedR);
        }
    
    }
}


void led_r_task(void *p){

    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    while(true){

        if(xSemaphoreTake(xSemaphoreLedR, 100) == pdTRUE){ // vai receber 0 ou 1.
          
            gpio_put(LED_PIN_R,1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(LED_PIN_R,0);
            vTaskDelay(pdMS_TO_TICKS(100));
            
        }
        
    }

}

void led_y_task(void * p ){
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);

    while(true){

        if(xSemaphoreTake(xSemaphoreLedY, 100) == pdTRUE){

            gpio_put(LED_PIN_Y,1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(LED_PIN_Y,0);
            vTaskDelay(pdMS_TO_TICKS(100));
        
        }
    }
    
}


// em resumo: A queue vai mandar a diversificacao para sabermos se estamos tratando do botao Y ou R
// A partir disso, vamos fazer a intermediacao para mandar ou para o botao Y ou R
// Com isso definido, o controle efetivo vai ocorrer na ley tyask.


int main() {
    stdio_init_all();

    xSemaphoreLedR = xSemaphoreCreateBinary();
    xSemaphoreLedY = xSemaphoreCreateBinary();

    xQueueBtn = xQueueCreate(32, sizeof(int));

    xTaskCreate(btn_task, "BTN_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(led_y_task, "LED_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(led_r_task, "LED_Task 2", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while(1){}

    return 0;
}