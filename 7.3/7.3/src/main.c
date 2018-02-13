#include "esp_common.h"
#include "freertos/task.h"
#include "gpio.h"

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

void holamundo(void* ignore)
{
  const char* str = "hola mundo"
  const char* buf;
  int n=0;
  str2morse(buf, n, str);
}

/* Devuelve la correspondencia en Morse del carácter c */
const char* morse (char c){
  static const char * morse_ch[] = {
    "._  ", /* A */
    "_...  ", /* B */
    "_._.  ", /* C*/
    "_..  ", /* D */
    ".  ", /* E */
    ".._.  ", /* F */
    "__.  ", /* G */
    "....  ", /* H */
    "..  ", /* I */
    ".___  ", /* J */
    "_._  ", /* K */
    "._..  ", /* L */
    "__  ", /* M */
    "_.  ", /* N */
    "___  ", /* O */
    ".__.  ", /* P */
    "__._  ", /* Q */
    "._.  ", /* R */
    "...  ", /* S */
    "_  ", /* T */
    ".._  ", /* U */
    "..._  ", /* V */
    ".__  ", /* W */
    "_.._  ", /* X */
    "_.__  ", /* Y */
    "__..  ", /* Z */
    "_____  ", /* 0 */
    ".____  ", /* 1 */
    "..___  ", /* 2 */
    "...__  ", /* 3 */
    "...._  ", /* 4 */
    ".....  ", /* 5 */
    "_....  ", /* 6 */
    "__...  ", /* 7 */
    "___..  ", /* 8 */
    "____.  ", /* 9 */
    "......  ", /* . */
    "._._._  ", /* , */
    "..__..  ", /* ? */
    "__..__  " /* ! */
  };
  if (c=' '){
    return "  ";
  }
  else{
  return morse_ch [c - 'a'];
  }
}

/* Copia en buf la versión en Morse del mensaje str, con un límite de n caracteres */
int str2morse (char *buf, int n, const char* str){
  int i;

  while (*(str + n) != '\0'){
    n++;
  }

  for (i=0; i<n;i++){
    buf= morse(*(str+i));
    buf++;
  }

  buf = buf -n;

  for (i=0; i<n; i++){
    morse_send(buf + i);
  }
}

/*Envía el mensaje msg, ya codificado en Morse, encenciendo y apagando el LED */
void morse_send (const char* msg){
  switch (*msg) {
    case '.':
    GPIO_OUTPUT_SET(2,1);
    vTaskDelay(250/portTICK_RATE_MS);
    GPIO_OUTPUT_SET(2,0);
    vTaskDelay(250/portTICK_RATE_MS);
    break;

    case '_':
    GPIO_OUTPUT_SET(2,1);
    vTaskDelay(750/portTICK_RATE_MS);
    GPIO_OUTPUT_SET(2,0);
    vTaskDelay(250/portTICK_RATE_MS);
    break;

    case ' ':
    GPIO_OUTPUT_SET(2,0);
    vTaskDelay(250/portTICK_RATE_MS);
    break;

    case '\0':
    return;
  }
  morse_send(++msg);
}

void user_init(void)
{
    xTaskCreate(&holamundo, "startup", 2048, NULL, 1, NULL);
}
