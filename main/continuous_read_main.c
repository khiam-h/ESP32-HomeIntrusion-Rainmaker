/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdbool.h>
#include "freertos/queue.h"


#define SOUND_SENSOR_PIN GPIO_NUM_4 // Replace XX with the GPIO pin number connected to the sensor
#define ESP_INTR_FLAG_DEFAULT 0

bool detect_flag = false;
static QueueHandle_t gpio_evt_queue = NULL;

static void IRAM_ATTR sound_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}
static void sound_task(void* arg)
{
    uint32_t io_num;
    for (;;) {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            //printf("GPIO[%"PRIu32"] intr, val: %d\n", io_num, gpio_get_level(io_num));
            if (gpio_get_level(io_num)){
                bool detect_flag = true;
                printf("Sound Detected \n");
            }
            }
    }
}
void app_main(void)
{
    esp_rom_gpio_pad_select_gpio(SOUND_SENSOR_PIN);
    gpio_set_direction(SOUND_SENSOR_PIN, GPIO_MODE_INPUT);
    gpio_set_intr_type(SOUND_SENSOR_PIN, GPIO_INTR_POSEDGE);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(sound_task, "sound_task", 2048, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(SOUND_SENSOR_PIN, sound_isr_handler, (void*) SOUND_SENSOR_PIN);

    int cnt = 0;
    while (1) {
        printf("cnt: %d\n", cnt++);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}