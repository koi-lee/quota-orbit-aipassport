#pragma once
#include <stdint.h>
typedef void *TaskHandle_t;
int xTaskCreate(void (*fn)(void *),const char *name,unsigned stack,void *arg,unsigned priority,TaskHandle_t *task);
void xTaskNotifyGive(TaskHandle_t task);
unsigned ulTaskNotifyTake(int clear,uint32_t timeout);

static inline unsigned uxTaskGetStackHighWaterMark(TaskHandle_t task) { (void)task; return 2048; }
