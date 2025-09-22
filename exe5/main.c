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

QueueHandle_t xQueueBtn_r, xQueueBtn_y;
SemaphoreHandle_t xSemaphore_r, xSemaphore_y;

void btn_callback( uint gpio, uint32_t events){
    if(events == 0x4){ // fall edge
        if (gpio == BTN_PIN_R){
            xSemaphoreGiveFromISR(xSemaphore_r, 0); // nao tem como setar ele para false.
            // assim que ele for taken, ele ja vai ser setado como false.
        }
        else if (gpio == BTN_PIN_Y){
            printf("entrei amarelo");
            xSemaphoreGiveFromISR(xSemaphore_y, 0); // nao tem como setar ele para false.
        }
    }
}

void btn_1_task(void* p) {
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);
   
    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &btn_callback);

    int counter = 0;

    while (true) {
        if(xSemaphoreTake(xSemaphore_r, 100) == pdTRUE){
            counter ++;
        }
        vTaskDelay(pdMS_TO_TICKS(200));
        xQueueSend(xQueueBtn_r, &counter, 0);
    }
}

void btn_2_task(void *p){
    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true); // referencia para chamar o callback e usarmos o sinal.
    gpio_set_irq_callback(btn_callback);

    int counter = 0;

    while (true) {
        if(xSemaphoreTake(xSemaphore_y, 100) == pdTRUE){
            counter ++;
        }
        vTaskDelay(pdMS_TO_TICKS(200)); // tem que ter esse delay para que evite ma pressionamentos
        xQueueSend(xQueueBtn_y, &counter, 0);
    }
}


void led_1_task(void *p){
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int counter;
    int delay = 100;

    while(true){  
        if(xQueueReceive(xQueueBtn_r, &counter, 100)){
            // printf("counter led vermelho : %d\n", counter);
        }
        
        if (counter%2!=0){
            gpio_put(LED_PIN_R,1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_R,0);
        }
        else{
            gpio_put(LED_PIN_R,0);
        }
    }
}


void led_2_task(void *p){
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);

    int counter;
    int delay = 100;

    while(true){
        if(xQueueReceive(xQueueBtn_y, &counter, 100)){
            printf("counter led amarelo : %d\n", counter);
        }
        
        if (counter%2!=0){
            gpio_put(LED_PIN_Y,1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_Y,0);
        }
        else{
            gpio_put(LED_PIN_Y,0);
        }
    }
}


int main() {
    stdio_init_all();

    xQueueBtn_r = xQueueCreate(32, sizeof(int));
    xQueueBtn_y = xQueueCreate(32, sizeof(int));

    xSemaphore_r = xSemaphoreCreateBinary();
    xSemaphore_y = xSemaphoreCreateBinary();

    xTaskCreate(btn_1_task, "BTN_Task 1", 256, NULL, 1, NULL); // vai controlar o timer do botao 1
    xTaskCreate(led_1_task, "LED TASK 1", 256, NULL, 1, NULL); // vai aplicar o timer do led 1
    xTaskCreate(btn_2_task, "BTN_2_TASK", 256, NULL, 1, NULL); // vai controlar o timer do botao 2
    xTaskCreate(led_2_task, "LED_2_TASK", 256, NULL, 1, NULL); // vai aplicar o timer do led 2


    vTaskStartScheduler();

    while(1){}

    return 0;
}