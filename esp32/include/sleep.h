#pragma once
// sleep.h

#include <stdbool.h>
#include <stdint.h>

#define IDLE_TIMEOUT_MS 30000 // Time of inactivity before entering modem sleep (30 seconds)

void sleep_manager_init(void);

void sleep_notify_activity(void);

void sleep_exit_modem_sleep(void);
