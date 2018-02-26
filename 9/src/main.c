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

portTickType start_timeout;

int done0=0;
int done15=0;

int f1 (fsm_t *this);
int f2 (fsm_t *this);
int timeout (fsm_t *this);
void activa (fsm_t *this);
void pulsado (fsm_t *this);
void timeEND (fsm_t *this);
void desactivar (fsm_t *this);

int f_or (fsm_t *this);
void encender(fsm_t *this);
void apagar(fsm_t *this);

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
 APAGADO0,
 APAGADO1,
 APAGADO2,
 APAGADO3,
 ENCENDIDO0,
 ENCENDIDO1,
 ENCENDIDO2,
 ENCENDIDO3,
 LED_ON,
 LED_OFF,
};

int f1 (fsm_t *this){
  return (done0);
}

int f2 (fsm_t *this) {
 return (done15);
}

int timeout (fsm_t *this) {
  long diff_t = xTaskGetTickCount () - start_timeout;
  return (TIMEOUT_TICK < diff_t);
}

void activa (fsm_t *this) {
  done15=0;
  GPIO_OUTPUT_SET(2, 0);
}

void pulsado (fsm_t *this) {
  done0=0;
  start_timeout = xTaskGetTickCount ();
}

void timeEND (fsm_t *this) {
  printf("tiempo agotado");
}

void desactivar (fsm_t *this) {
	done0=0;
  GPIO_OUTPUT_SET(2, 1);
}

int f_or (fsm_t *this){
  return (done0 || done15);
}

void encender(fsm_t *this){
  GPIO_OUTPUT_SET(2, 0);
}

void apagar(fsm_t *this){
  GPIO_OUTPUT_SET(2, 1);
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
  { APAGADO0, f1, APAGADO1, pulsado },
  { APAGADO1, f1, APAGADO2, pulsado},
  { APAGADO2, f1, APAGADO3, pulsado},
  { APAGADO3, timeout, ENCENDIDO0, timeEND },

  { APAGADO1, timeout, APAGADO0, timeEND},
  { APAGADO2, timeout, APAGADO0, timeEND},
  { APAGADO3, f1, APAGADO0, pulsado },

  { ENCENDIDO0, f2, ENCENDIDO0, activa},

  { ENCENDIDO0, f1, ENCENDIDO1, pulsado},
  { ENCENDIDO1, f1, ENCENDIDO2, pulsado },
  { ENCENDIDO2, f1, ENCENDIDO3, pulsado},
  { ENCENDIDO3, timeout, APAGADO0, desactivar},

  { ENCENDIDO1, timeout, ENCENDIDO0, timeEND },
  { ENCENDIDO2, timeout, ENCENDIDO0, timeEND},
  { ENCENDIDO3, f1, ENCENDIDO0, pulsado},
  {-1, NULL, -1, NULL },
 };

 static fsm_trans_t lamp[] = {
 {LED_ON, f_or, LED_OFF, apagar},
 {LED_OFF, f_or, LED_ON, encender},
 {-1, NULL, -1, NULL },
};

  ETS_GPIO_INTR_ENABLE();
  PIN_FUNC_SELECT(GPIO_PIN_REG_15, FUNC_GPIO15);
  gpio_intr_handler_register((void*)isr_gpio, NULL);
  gpio_pin_intr_state_set(0, GPIO_PIN_INTR_NEGEDGE);
  gpio_pin_intr_state_set(15, GPIO_PIN_INTR_POSEDGE);
  fsm_t* fsm_alarma = fsm_new(alarma);
  fsm_t* fsm_lamp = fsm_new(lamp);
  desactivar(fsm_alarma);
  desactivar(fsm_lamp);
  portTickType xLastWakeTime;

    while(true) {
      xLastWakeTime = xTaskGetTickCount ();
       fsm_fire(fsm_alarma);
       fsm_fire(fsm_lamp);
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
