LAB 4
---

`Tasks`:
Sao funcoes que `nao` retornam nada e `possuem` laco infinito. 

```c

void led_1_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int delay = 250;
    while (true) {
        gpio_put(LED_PIN_R, 1);
        vTaskDelay(pdMS_TO_TICKS(delay));
        gpio_put(LED_PIN_R, 0);
        vTaskDelay(pdMS_TO_TICKS(delay));
    }
}

void led_2_task(void *p) {
    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);

    int delay = 500;
    while (true) {
        gpio_put(LED_PIN_G, 1);
        vTaskDelay(pdMS_TO_TICKS(delay));
        gpio_put(LED_PIN_G, 0);
        vTaskDelay(pdMS_TO_TICKS(delay));
    }
}


```
   
    vTaskDelay

    A função vTaskDelay() faz com que o RTOS libere processamento para outras tarefas durante o tempo especificado em sua chamada. Esse valor é determinado em ticks. Podemos traduzir ticks para ms, usando o define portTICK_PERIOD_MS como no exemplo anterior.


`Criando uma Task`:

Precisamos indicar para o OS quais tarefas vao se comportar como pequenos programas, ou seja, tarefas. 


    LEITURA QUE PODE SER UTIL: https://www.freertos.org/a00125.html
    


`xTaskCreate` pode ser utilizada para criar tarefas que têm acesso irrestrito a todo o mapa de memória do microcontrolador. Em sistemas que suportam MPU (Unidade de Proteção de Memória), tarefas com restrições de MPU podem ser criadas usando-se `xTaskCreateRestricted` como alternativa.


O processo de criacao de uma task eh na `main`, como evidenciado: 

```c 
int main() {
    stdio_init_all();
    printf("Start RTOS \n");
    xTaskCreate(led_1_task, "LED_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(led_2_task, "LED_Task 2", 256, NULL, 1, NULL);
    vTaskStartScheduler();
    
    // Programa nunca deve chegar aqui! 
    // vTaskStartScheduler() passa o comando do core para o FreeRTOS!
    while (true)
        ;
}
```
Parametros da funcao : 

    -TaskFunction_t pvTaskCode: Qual a funcao da task

    - const char const pcName: String que representa o nome da tarefa 

    - uint16_t usStackDepth: O tamanho da pilha (stack) atribuída à tarefa, especificado em palavras. Isso depende do microcontrolador e da complexidade da tarefa.

    - void pvParameters*: Um ponteiro para os parâmetros que a tarefa deve receber. Se a tarefa não necessitar de parâmetros, NULL pode ser passado.

    - UBaseType_t uxPriority: A prioridade da tarefa. Tarefas com prioridades mais altas são executadas preferencialmente em relação às tarefas com prioridades mais baixas.

    - TaskHandle_t pvCreatedTask*: Um ponteiro para uma variável do tipo TaskHandle_t que será preenchida com o identificador da tarefa criada. Esse identificador pode ser utilizado para se referenciar à tarefa após sua criação.


Um exemplo que consolida tudo isso de maneira bem simplificada, mas funcional : 

```c 
#include "pico/stdlib.h"
#include <FreeRTOS.h>
#include <stdio.h>
#include <task.h>

const int LED_PIN_R = 4;
const int LED_PIN_G = 5;

void led_1_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int delay = 250;
    while (true) {
        gpio_put(LED_PIN_R, 1);
        vTaskDelay(pdMS_TO_TICKS(delay));
        gpio_put(LED_PIN_R, 0);
        vTaskDelay(pdMS_TO_TICKS(delay));
    }
}

void led_2_task(void *p) {
    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);

    int delay = 250;
    while (true) {
        gpio_put(LED_PIN_G, 1);
        vTaskDelay(pdMS_TO_TICKS(delay));
        gpio_put(LED_PIN_G, 0);
        vTaskDelay(pdMS_TO_TICKS(delay));
    }
}

int main() {
    stdio_init_all();
    printf("Start RTOS \n");
    xTaskCreate(led_1_task, "LED_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(led_2_task, "LED_Task 2", 256, NULL, 1, NULL);
    vTaskStartScheduler();

    while (true)
        ;
}
```


`vTaskDelay`
---
A função `vTaskDelay()` faz com que a tarefa fique em estado de blocked (permitindo que outras tarefas utilizem a CPU) por um determinado número de ticks.

Quando estamos tratando de `ticks`, queremos dizer em relacao a isso : 

O Tick de um RTOS define quantas `vezes` por segundo o escalonador irá `executar` o algoritmo de `mudança` de tarefas, no ARM o tick é implementado utilizando um timer do próprio CORE da ARM chamado de system clock ou systick, criado para essa função.

Uma otima explicacao advinda do corsi, eh a seguinte : 


"Por exemplo, um RTOS que opera com um tick de 10ms irá decidir pelo chaveamento de suas tarefas `100` vezes por `segundo`, já um tick configurado para 1ms irá executar o escalonador a uma taxa de 1000 vezes por segundo. Trechos de código que necessitam executar a uma taxa `maior` que `1000` vezes por segundo (tick = 1ms) `não` devem ser implementados em `tasks` do RTOS, mas sim via `interrupção` de timer."

`OBS`: 
- O impacto do tick na função vTaskDelay é que a mesma só pode ser chamada com `múltiplos` inteiros `referente` ao `tick`.

- Frequência máxima recomendada para o FreeRTOS em uma ARM é de `1000 Hz`, ou seja, quando dentro dele estiver 1000, uma vez que equivaleria a um segundo.


`Pode ser util:` Maquina de estados de tasks - `https://www.freertos.org/RTOS-task-states.html`

Exemplo de como fazer o delay: 
```c
int ticks = 100; // equivalente a 0,1 segundo. 
vTaskDelay(pdMS_TO_TICKS(ticks));
```

Semaforos 
---
Fundamentais para a sincronizacao em programcao concorrente e OS. Sao responsaveis por gerenciar o acesso a recursos compartilhados a multitarefas.


` Eles funcionam como um contador que regula quantas tarefas podem acessar um determinado recurso simultaneamente.`

Quando eh requisitado por uma tarefa de acessar um recurso que eh gerenciado por um semaforo, ela realiza a operacao de espera, `wait`.

- Essa operacao vai verificar se o contador do semaforo eh `maior que zero`, indicando se o recurso esta disponivel. Se for, a tarefa decrementa o contador e ganha acesso ao recurso.

- Se o contador for `zero`, isso indica que nao ha recursos disponiveis no momento, provavelmente pq outra tarefa esta sendo efetuada, entao entra no estado de espera ate que a outra tarefa acabe e ela ganhe espaco para rodar, que eh justamente a situacao do boolet anterior.


Por que semaforos ? 

```c

void btn_callback(uint gpio, uint32_t events) {
    if (events == 0x4) {         // fall edge
        flag_f_r = 1;
    }
} 

void main(void) {
 // ....
 
    while(1) { 
        if(flag_f_r) {
            // faz alguma coisa
            flag_f_r = 0;
        }
    }
}
```

Antes, faziamos isso, que eh chamado de bare-metal. Agora, com o uso de semaforos:


```c 
void btn_callback(uint gpio, uint32_t events) {
    if (events == 0x4) {         // fall edge
        xSemaphoreGiveFromISR(xSemaphore, 0);
    }
} 

void task_main(void) {
 // ....

    while(1) { 

        if(xSemaphoreTake(xSemaphore, pdMS_TO_TICKS(100))) {
            // faz alguma coisa
        } else { 
            // cai aqui se o semáforo não for liberado em 100 ms!
        }
        
    }
}
```

ISR
---
Eh usado para que eu interaja com o hardware e software. Assim,fazemos essa conexao pela interrupcao que acontece. Ou seja, ela e sempre prioritaria. Quando lidamos com tasks etc, estamos apenas usando interacao entre software, nao ligando com o que acontece com o hardware. Para isso, ultilizamos as interrupcoes que vao nos ajudar a controlar as interrupcoes.

Para sabermos que estamos lidando com a interrupcao e mudar alguma coisa dentro do nosso codigo, precisamos adicionar no final da linha de codigo o trecho `FromISR`

Um caso comum eh o seguinte : 

```c
bool timer_0_callback(repeating_timer_t *rt) {
    xSemaphoreGiveFromISR(xSemaphore, 0);
    return true; // keep repeating
}
```

Isso no caso do `hardware`. No caso do `software`, usamos a seguinte forma: 


```c
void task_1(void) {
    // ..
    xSemaphoreGive(xSemaphore);

}
```
Veja que nao ha o final `FromISR`. 

Devemos usar o sufixo `FromISR` quando `chamadas` de uma ISR, nao quando estamos recebendo um dado. Ou seja, por ser uma interrupcao, `sempre` vamos usar em um callback.


`Instrucoes uteis:`

- Criar a variável global que representará o semáforo
    SemaphoreHandle_t xSemaphore;

- Criar o semáforo (na função main)
    xSemaphoreCreateBinary();

- Liberar o semáforo
    xSemaphoreGiveFromISR(xSemaphore, 0); (se for liberado de uma ISR)
    xSemaphoreGive(xSemaphore); (se for liberado de outra task)


- Esperar pelo semaforo 
    xSemaphoreTake(xSemaphore, 500) - > Espera 500 ticks checando se o retorno do semaforo foi "true" ou "false", esta entre aspas, pois nao eh necessariamente isso.


if (xSemaphoreTake(xSemaphore_r, pdMS_TO_TICKS(500)) == pdTRUE), faz o que o codigo acima explica, mas agora em milissegundos ao inves de ticks, ou seja, eh uma conversao rapida de tick para segundo.


Aqui esta um exemplo de task - task, ou seja como elas se comunicam, `independentemente` do hardware: 



```c
/* ... Código omitido */

SemaphoreHandle_t xSemaphore_r;

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

void led_1_task(void *p) {
  gpio_init(LED_PIN_R);
  gpio_set_dir(LED_PIN_R, GPIO_OUT);

  int delay = 250;
  int status = 0;

  while (true) {

    if (xSemaphoreTake(xSemaphore_r, pdMS_TO_TICKS(500)) == pdTRUE) {
      gpio_put(LED_PIN_R, 1);
      vTaskDelay(pdMS_TO_TICKS(delay));
      gpio_put(LED_PIN_R, 0);
      vTaskDelay(pdMS_TO_TICKS(delay));
    }
  }
}

int main() {
  /* ... Código omitido */
  
  xSemaphore_r = xSemaphoreCreateBinary();

  xTaskCreate(led_1_task, "LED_Task 1", 256, NULL, 1, NULL);
  xTaskCreate(btn_1_task, "BTN_Task 1", 256, NULL, 1, NULL);

  /* ... Código omitido */
  ```


Agora com task-IRS, ou seja, hardware-software:

```c
SemaphoreHandle_t xSemaphore;

void but_callback(void){
  // libera semáforo 
  xSemaphoreGiveFromISR(xSemaphore, 0);
}

static void task_led(void *pvParameters){
  init_led1();   // inicializa LED1
  init_but1();   // inicializa botao 1, com callback
  
  for (;;) {
      // aguarda por até 500 ms pelo se for liberado entra no if
      if( xSemaphoreTake(xSemaphore, 500 / portTICK_PERIOD_MS) == pdTRUE ){
        LED_Toggle(LED0);
      }
    }
}

void main(void) {
  // .... //

  // cria semáforo binário
  xSemaphore = xSemaphoreCreateBinary();

  // verifica se semáforo foi criado corretamente
  if (xSemaphore == NULL)
      printf("falha em criar o semaforo \n");
```

`OBS:`
- O semáforo deve ser sempre alocado antes do seu uso, caso alguma parte do firmware tente liberar o semáforo antes dele ser criado `(xSemaphoreCreateBinary())` o código irá travar.

- Você deve usar fromISR SEMPRE que liberar um semáforo de uma interrupção, caso contrário usar a função `xSemaphoreGive()`


Queue/Fila
---
Eh uma maneira de enviarmos dados entre tarefas de um sistema operacional. Podemos comunicar interrupcao-tarefa e tarefa-tarefa. No entanto, a `principal` diferenca entre isso e um semaforo, eh que com a `fila`, podemos mandar coisas alem de booleanas, que funcionam como liberar e travar, podemos efetivamente mandar informacoes. 

A fila eh a primeira forma de comunicacao entre tasks e entre interrupcoes e tasks. Na maior parte dos casos, sao usados com FIFO, com os dados mais novos ficando no final da fila. 

    As filas vão ser utilizadas para transmitir dado entre partes de um programa, e isso pode ser de um callback para um task, de uma task para um callback ou entre tasks


Um efeito de comparacao entre o que fizemos no `PRA3`, esta o seguinte exemplo: 

```c
// apenas xQueueTime é global (fila do RTOS)

void gpio_callback(uint gpio, uint32_t events) {
    int time = 0;
    if (events == 0x4) { // fall edge
        time = to_us_since_boot(get_absolute_time());
    } else if (events == 0x8) { // rise edge
        time = to_us_since_boot(get_absolute_time());
    }
    xQueueSendFromISR(xQueueTime, &time, 0);
}
```

Note que ha a adicao do `xQueueSendFromISR`, que faz toda a diferenca. 


Podemos ler o codigo da seguinte maneira: 


```c
void task_1(void){
  
  int gpio;
  while(1) {
  
    if (xQueueReceive(xQueueBtn, &gpio,  pdMS_TO_TICKS(100))) {
      printf("Botão pressionado pino %d", gpio);
    } else {
      // cai aqui se não chegar um dado em 100 ms!
    }
  }
  
}
```

os parametros : 
- `xQueueBtn`: Fila que desejamos ler
- `&gpio`: Local que vamos armazenar o valor lido
- `(pdMS_TO_TICKS(100))`: Tempo que iremos esperar pelo dado na fila.


No caso de uma fila, vamos colocar os dados da fila em uma isr e ler isso na task:

```c
// neste exemplo colocamos o valor do botão que gerou a ISR na fila!
void btn_callback(uint gpio, uint32_t events) {
    xQueueSendFromISR(xQueueBtn, &gpio, 0);
}
```
No entanto, se for o caso de entre tasks, usamos : 

```c
xQueueSend(xQueueBtn, &dado, 0);
```
Para usar:

- Criar a variável global que representará a fila: `QueueHandle_t xQueueButId`

- Cria a fila na `main`:  `xQueueButId = xQueueCreate(32, sizeof(char) );`
- - Ao criar a fila você deve informar a quantidade de itens (32) nessa fila e o tipo dos itens `(sizeof(char))`.

- Para colocar dados na fila : 

- `xQueueSendFromISR(xQueueButId, &id, 0);` - caso de uma interrupcao `(ISR)`

- `xQueueSend(xQueueButId, &id);`

- Receber dados da fila: `xQueueReceive( xQueueButId, &id, ( TickType_t ) 500 )`


Como o codigo fica:

- #### Task-Task:

```c
// fila
QueueHandle_t xQueueData;
 
static void task_1(void *pvParameters){

  int var = 3;
  while(1) {
    xQueueSend(xQueueData, &var, 0);
    vTaskDelay(100);
  }
}

static void task_2(void *pvParameters){
  int var = 0;
  while(1) {
    if( xQueueReceive( xQueueData, &var, pdMS_TO_TICKS(100))){
      printf("var: %d\n", var);
    }
  }
}
```

- ### IRQ-Task:

```c
// fila
QueueHandle_t xQueueButId;
 
void btn_callback(uint gpio, uint32_t events) {
  char id;
  if (events == 23) {         
      id = 23;
  } else if (gpio == 22) {  
      id = 22;
  }
  
  xQueueSendFromISR(xQueueButId, &id, 0);
}


static void task_led(void *pvParameters){
  init_led1();   // inicializa LED1
  init_but1();   // inicializa botao 1, com callback
  init_but2();   // inicializa botao 2, com callback
  
  // variável local para leitura do dado da fila
  char id;

  for (;;) {
      // aguarda por até 500 ms pelo se for liberado entra no if
      if( xQueueReceive( xQueueButId, &id, pdMS_TO_TICKS(100))){
        for (i =0; i < 10; i++) {
          gpio_put(id, i/2);
          vTaskDelay(pdMS_TO_TICKS(100));
        }
      }
    }
}

void main(void) {
  // .... //

  // cria fila de 32 slots de char
  xQueueButId = xQueueCreate(32, sizeof(char) );
  
  // verifica se fila foi criada corretamente
  if (xQueueButId == NULL)
      printf("falha em criar a fila \n");
}
```



