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

// cria a fila
QueueHandle_t xQueueButId, xQueueBnt2;

void led_1_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int delay = 0;
    while (true) {
        if (xQueueReceive(xQueueButId, &delay, 0)) { // se nao fosse 0, ele ficaria travado ate passar o tempo ou se chegasse o item da fila
            // nesse caso, ele so vai verificar se a fila esta com o retorno esperado. Se nao, ele simplesmente pula, eh basicamente um if ocm uma cond verdadeira
            // se fosse com tempo, seria ao mesmo tempo um if com condicao verdade e um timer, se a condicao nao obter ele vai esperar o tempo e se passar o tempo "alarme estourar", ele nao entra no if e pula o bloco.
            printf("%d\n", delay);
            
        }

        if (delay > 0) {
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        }
    }
}

void led_2_task(void *p){
    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);
    int delay = 0; 

    while(true){
        if(xQueueReceive(xQueueBnt2, &delay, 0)){
            // a primeira condicao do loop eh para verificar se a lista atualziou 
            // preciso usar isso aqui, pois se nao ele nao vai rodar inifnitamente, pois
            // se nao ele vai atualizar todas as vezes que a lista mudar, e se ela esta sendo atualizada constantemente,
            // ele so roda uma vez.
            printf("delay btn2: %d",delay);
        }

        if (delay > 0){
            // se eu colocasse o delay e nao a questao da lista, pois se a lista nao for atualizada, nao vai rodar
            // nesse caso, eh exatamente isso, ela so vai atualizar quando eu clicar, mas o que quero eh que
            // como meu delay eh > 0 ele vai continuar rodando.
            gpio_put(LED_PIN_G,1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_G, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));

        }
    }
}

void btn_2_task(void *p){
    gpio_init(BTN_PIN_G);
    gpio_set_dir(BTN_PIN_G,GPIO_IN);
    gpio_pull_up(BTN_PIN_G);

    int delay = 0;

    while (true)
    {
        if(!gpio_get(BTN_PIN_G)){
            while(!gpio_get(BTN_PIN_G)){
                vTaskDelay(pdMS_TO_TICKS(1));
            }
            // isso faz com que seja possivel controlar a frequencia do piscar do led a cada apertada,
            // mas quando o delay chega num nivel ele reinicia.
            if (delay < 1000){
                delay+=100;
            }
            else {
                delay = 100;
            }
            printf("delay btn2 : %d", delay);
            xQueueSend(xQueueBnt2, &delay, 0);
        }
    }
    
}

void btn_1_task(void *p) {
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    int delay = 0;
    while (true) {
        if (!gpio_get(BTN_PIN_R)) {

            while (!gpio_get(BTN_PIN_R)) {
                vTaskDelay(pdMS_TO_TICKS(1));
            }

            if (delay < 1000) {
                delay += 100;
            } else {
                delay = 100;
            }
            printf("delay btn %d \n", delay);
            xQueueSend(xQueueButId, &delay, 0); // 0, pois ele nao tem que esperar nenhum tick.
        }
    }
}


int main() {
    stdio_init_all();
    printf("Start RTOS \n");

    xQueueButId = xQueueCreate(32, sizeof(int));
    xQueueBnt2 = xQueueCreate(32, sizeof(int)); // colocando o numero de itens dessa fila e o tipo dos elementos, nesse caso int.

    xTaskCreate(led_1_task, "LED_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(btn_1_task, "BTN_Task 1", 256, NULL, 1, NULL);

    xTaskCreate(led_2_task, "LED_Task 2", 256, NULL, 1, NULL);
    xTaskCreate(btn_2_task, "BTN_Task 2", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
