#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>

const int BTN_PIN_R = 28;
const int BTN_PIN_G = 26;

const int LED_PIN_R = 4;
const int LED_PIN_G = 6;

SemaphoreHandle_t xSemaphore_r, xSemaphore_g; // sempre fazer isso, para ele identificar o tipo.

// A partir de quando tem o botao ja notificado, ele executa.
void led_1_task(void *p) {
  gpio_init(LED_PIN_R);
  gpio_set_dir(LED_PIN_R, GPIO_OUT);

  int delay = 250;

  while (true) {

    if (xSemaphoreTake(xSemaphore_r, pdMS_TO_TICKS(500)) == pdTRUE) {
      gpio_put(LED_PIN_R, 1);
      vTaskDelay(pdMS_TO_TICKS(delay));
      gpio_put(LED_PIN_R, 0);
      vTaskDelay(pdMS_TO_TICKS(delay));
    }
  }
}

// prineiro executa essa, que pega o estado do botao
void btn_1_task(void *p) {
  gpio_init(BTN_PIN_R);
  gpio_set_dir(BTN_PIN_R, GPIO_IN);
  gpio_pull_up(BTN_PIN_R);

  while (true) {
    if (!gpio_get(BTN_PIN_R)) {
      while (!gpio_get(BTN_PIN_R)) {
        vTaskDelay(pdMS_TO_TICKS(1));
      }
      xSemaphoreGive(xSemaphore_r);
    }
  }
}


void btn_2_task(void *p){
  //iniciando o botao
  gpio_init(BTN_PIN_G);
  gpio_set_dir(BTN_PIN_G, GPIO_IN);
  gpio_pull_up(BTN_PIN_G);

  while(true){
    // nesse caso nao estamos usando callback, por isso nao tem a segunda parte do give,
    // mas isso aqui eh o caso de estar usando o IRS.
    if(!gpio_get(BTN_PIN_G)){ // se estiver pressionando
      while(!gpio_get(BTN_PIN_G)){ // enquanto estiver pressionando
        vTaskDelay(pdMS_TO_TICKS(1)); // equivalente a 1 ms
      }
      // Vai passar o semaforo para a variavel, como nao tem o ISR, nao foi advinda de interrupcao, 
      // mas tem o "mesmo" efeito de interrupcao.
      // SOmente qdo ele libera o botao, que ele da o sinal como true, para a funcao.
      xSemaphoreGive(xSemaphore_g); 
    }
  }
}

void led_2_task(void*p){
  // iniciando o LED:
  gpio_init(LED_PIN_G);
  gpio_set_dir(LED_PIN_G, GPIO_OUT);
  int delay = 250;

  while(true){
    // Tenho que esperar o semaforo com a "flag" verdadeira. Assim, ele vai entrar.
    if(xSemaphoreTake(xSemaphore_g, pdMS_TO_TICKS(500)) == pdTRUE){
      gpio_put(LED_PIN_G,1);
      vTaskDelay(pdMS_TO_TICKS(delay));
      gpio_put(LED_PIN_G,0);
      vTaskDelay(pdMS_TO_TICKS(delay));
      
    }
  }
}

int main() {
  stdio_init_all();
  printf("Start RTOS \n");

  // cria as instancias de semaforos(que vao controlar as flags)
  xSemaphore_r = xSemaphoreCreateBinary();
  xSemaphore_g = xSemaphoreCreateBinary();

  // cria as tasks
  xTaskCreate(led_1_task, "LED_Task 1", 256, NULL, 1, NULL);
  xTaskCreate(btn_1_task, "BTN_Task 1", 256, NULL, 1, NULL);
  xTaskCreate(btn_2_task, "BTN_Task 2", 256, NULL, 1, NULL);
  xTaskCreate(led_2_task, "LED_Task 2", 256, NULL, 1, NULL);

  vTaskStartScheduler();

  while (true)
    ;
}
