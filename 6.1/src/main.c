#include "esp_common.h"
#include "freertos/task.h"
#include "gpio.h"
#include "fsm.h"

#define ETS_GPIO_INTR_ENABLE() \
_xt_isr_unmask(1 << ETS_GPIO_INUM)
#define ETS_GPIO_INTR_DISABLE() \
_xt_isr_mask(1 << ETS_GPIO_INUM)
#define PERIOD_TICK 100/portTICK_RATE_MS
#define REBOUND_TICK 3

int done0 = 0;
int done1 = 0;


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
 DISABLE,
 ENABLE,
};

static fsm_trans_t alarma[] = {
 {DISABLE, f1, ENABLE, encender},
 {ENABLE, f1, DISABLE, apagar},
 {ENABLE, f2, ENABLE, activa},
 {-1, NULL, -1, NULL },
};

int f1 (fsm_t *this){
  return (done0);
}

int f2 (fsm_t *this) {
 return (done1);
}

void activa(fsm_t *this){
	done1=0;
  GPIO_OUTPUT_SET(2, 0);
}

void encender (fsm_t *this) {
  done0=0;
}

void apagar (fsm_t *this) {
  done0=0;
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
     done1 = 1;}
   }
   GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS,	status);
}

void task_alarma(void* ignore)
{
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