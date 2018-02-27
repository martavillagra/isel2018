#include "esp_common.h"
#include "freertos/task.h"
#include "gpio.h"
#include "fsm.h"
#include <stdio.h>
#include <time.h>

#define ETS_GPIO_INTR_ENABLE() \
_xt_isr_unmask(1 << ETS_GPIO_INUM)
#define ETS_GPIO_INTR_DISABLE() \
_xt_isr_mask(1 << ETS_GPIO_INUM)
#define PERIOD_TICK 100/portTICK_RATE_MS
#define REBOUND_TICK 3
#define TIMEOUT_TICK 10000/portTICK_RATE_MS

int presencia (fsm_t *this);
int mirar_flag (fsm_t *this);
void update_code (fsm_t *this);
int timeout (fsm_t *this);
void next_index (fsm_t *this);
int codigo_incorrecto (fsm_t *this);
void limpiar_flag (fsm_t *this);
int codigo_correcto (fsm_t *this);
void apagar (fsm_t *this);
void led_on (fsm_t*this);

portTickType nexttimeout;
portTickType now;

volatile int done0=0;
volatile int done15=0;
int code_index;
int code_inserted[3];
int valido;

uint32 user_rf_cal_sector_set(void)
{
    flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;
    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}

enum fsm_state {
APAGADO,
ENCENDIDO,
};



int mirar_flag (fsm_t *this){
  if (code_index >2)
    return 0;
  if (code_inserted[code_index]>10)
    return 0;
  return done0;
}

void update_code (fsm_t *this) {
 code_inserted[code_index]++;
 done0=0;
 nexttimeout= xTaskGetTickCount() + 1000/portTICK_RATE_MS;
}

int timeout (fsm_t *this) {
now = xTaskGetTickCount();
if(nexttimeout < now)
  return 1;
return 0;
}

void next_index (fsm_t *this) {
  code_index++;
  nexttimeout= 0xFFFFFFFF;
  if (code_index>2){
    if (code_inserted[0]==0 && code_inserted[1] == 0 && code_inserted[2]==0)
      valido=1;
      valido=0;
}
}

int codigo_incorrecto (fsm_t *this) {
return (valido);
}

void limpiar_flag (fsm_t *this) {
code_index = 0;
int i;
for (i=0; i<3; i++){
  code_inserted[i]=0;
}
valido=0;
done0=0;
done15=0;
}

int codigo_correcto (fsm_t *this){
  return (valido);
}

void apagar (fsm_t *this) {
  code_index = 0;
  int i;
  for (i=0; i<3; i++){
    code_inserted[i]=0;
  }
  valido=0;
  done0=0;
  done15=0;
  GPIO_OUTPUT_SET(2, 1);
}

void led_on (fsm_t*this) {
  done15=0;
  GPIO_OUTPUT_SET(2,0);
}

int presencia (fsm_t *this){
  return (done15);
}

void isr_gpio(void* arg) {
 static portTickType xLastISRTick0 = 0;
  static portTickType xLastISRTick15 = 0;
 uint32 status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
 portTickType now = xTaskGetTickCount ();
  if (status & BIT(0)) {
    if (now > xLastISRTick0) {
     xLastISRTick0 = now + REBOUND_TICK;
     done0 = 1;
    }
  }
  if (status & BIT(15)) {
    if (now > xLastISRTick15) {
     xLastISRTick15 = now + REBOUND_TICK;
     done15 = 1;}
   }
   GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS,	status);
}

void task_alarma(void* ignore)
{

  static fsm_trans_t alarma[] = {
    { APAGADO, mirar_flag, APAGADO, update_code },
    { APAGADO, timeout, APAGADO, next_index},
    { APAGADO, codigo_incorrecto, APAGADO, limpiar_flag},


    { APAGADO, codigo_correcto, ENCENDIDO, limpiar_flag},

    { ENCENDIDO, codigo_correcto, APAGADO, apagar},

    { ENCENDIDO, mirar_flag, ENCENDIDO, update_code},
    { ENCENDIDO, timeout, ENCENDIDO, next_index },
    { ENCENDIDO, presencia, ENCENDIDO, led_on},
    { ENCENDIDO, codigo_incorrecto, ENCENDIDO, limpiar_flag},

    {-1, NULL, -1, NULL },
   };

  ETS_GPIO_INTR_ENABLE();
  PIN_FUNC_SELECT(GPIO_PIN_REG_15, FUNC_GPIO15);
  gpio_intr_handler_register((void*)isr_gpio, NULL);
  gpio_pin_intr_state_set(0, GPIO_PIN_INTR_NEGEDGE);
  gpio_pin_intr_state_set(15, GPIO_PIN_INTR_POSEDGE);
  fsm_t* fsm = fsm_new(alarma);
  apagar(fsm);
  portTickType xLastWakeTime;

    while(true) {
      xLastWakeTime = xTaskGetTickCount ();
       fsm_fire(fsm);
       vTaskDelayUntil(&xLastWakeTime, PERIOD_TICK);
       //gpio16_output_set(0);
        //vTaskDelay(1000/portTICK_RATE_MS);
    	//gpio16_output_set(1);
        //vTaskDelay(1000/portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void user_init(void)
{
    xTaskCreate(&task_alarma, "startup", 2048, NULL, 1, NULL);
}
