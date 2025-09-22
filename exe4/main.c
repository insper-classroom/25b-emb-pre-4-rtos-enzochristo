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

QueueHandle_t xQueueButId, xQueueBtng;
SemaphoreHandle_t xSemaphore_r, xSemaphore_g;

void btn_callback(uint gpio, uint32_t events) {
    if (events == 0x4) { // fall edge
        if (gpio == BTN_PIN_R){
            xSemaphoreGiveFromISR(xSemaphore_r, 0);
        }
        else{
            xSemaphoreGiveFromISR(xSemaphore_g, 0);
        }
    }
}

void led_1_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int delay = 0;

    while (true) {
        if (xQueueReceive(xQueueButId, &delay, 0)) {
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

void btn_1_task(void *p) {
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);
    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true,
                                       &btn_callback);

    int delay = 0;
    while (true) {
        if (xSemaphoreTake(xSemaphore_r, pdMS_TO_TICKS(500)) == pdTRUE) {
            if (delay < 1000) {
                delay += 100;
            } else {
                delay = 100;
            }
            printf("delay btn %d \n", delay);
            xQueueSend(xQueueButId, &delay, 0);
        }
    }
}

void btn_g_task(void *p){
    gpio_init(BTN_PIN_G);
    gpio_set_dir(BTN_PIN_G, GPIO_IN);
    gpio_pull_up(BTN_PIN_G); // nivel high quando nao presisonado
    // o up e down sao referentes aos niveis de quando a tensao esta HIGH
    gpio_set_irq_enabled(BTN_PIN_G, GPIO_IRQ_EDGE_FALL, true); // referencia para chamar o callback e usarmos o sinal.
    gpio_set_irq_callback(btn_callback);

    int delay = 0;

    while(true){
        if(xSemaphoreTake(xSemaphore_g,0) == pdTRUE){ // o botao foi apertado, pois ele verifica quando nao eh o caso do pull up, pegando o 0x4
            // agora que meu botao foi apertado, preciso comunicar por meio da task para o LED.
            // vou enviar o valor do meu delay por meio da minha fila.
            // esse valor vai ser acessado pelo led task 2
            if (delay < 1000) { // limitacao dos 1s
                delay += 100; // intensificando a partir do click do botao
            }else{
                delay = 100;
            }
            xQueueSend(xQueueBtng, &delay, 0);
            // agora estou transmitindo isso para minha task 
            // ou seja, estou transmitindo para a task do led task o valor do meu delay
            // para que a task do led, a partir desse valor treansmitido,
            // consiga atualizar o quao rapido o led pisca.
        }
    }
}

void led_2_task( void *p){
    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);

    int delay = 0;

    while(true){
        if(xQueueReceive(xQueueBtng, &delay, 0)){
            printf("delay chegando no LED g : %d\n", delay);
        }
        if (delay > 0){
            gpio_put(LED_PIN_G,1);
            vTaskDelay(pdMS_TO_TICKS(delay)); // aqui eh a magica do delay, por isso usamos ele!
            // por meio desse taskdelay, vamos fazer a task dormir por delay ticks, que vai ser convertido em segundos!
            // a logica do "tamanho" do delay eh controlada pela outra task, a do botao. 
            // Essa task apenas recebe uma mensagem de delay e codifica isso em forma de luz, essa eh a unica preocupacao dela.
            gpio_put(LED_PIN_G,0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        }

        // como nao tenho nenhum comunicado/informacao para passar, 
        // nao tem pq eu enviar nada. Agora nosso codigo esta pronto. 
    }
}

int main() {
    stdio_init_all();
    printf("Start RTOS \n");

    xQueueButId = xQueueCreate(32, sizeof(int));
    xSemaphore_r = xSemaphoreCreateBinary();

    xQueueBtng = xQueueCreate(32, sizeof(int));
    xSemaphore_g = xSemaphoreCreateBinary();


    // criando tasks 1's
    xTaskCreate(led_1_task, "LED_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(btn_1_task, "BTN_Task 1", 256, NULL, 1, NULL);

    // criando tasks 2's
    xTaskCreate(led_2_task, "LED_Task 2", 256, NULL, 1, NULL);
    xTaskCreate(btn_g_task, "BTN_Task 2", 256, NULL, 1, NULL);


    vTaskStartScheduler();

    // em suma: 
    // nosso codigo esta fazendo uma verificacao em callback
    // quando esta pressionado o botao(fall edge), vai retornar ok para o semaforo!
    // Assim, o botao vai poder processar os delays, somando 100 ticks para cada click
    // Com isso, ele vai passar para a task do led o valor desse delay.
    // o led vai estar esperando esse delay e assim vai ser encarregado a apenas piscar o led.

    while (true)
        ;
}
