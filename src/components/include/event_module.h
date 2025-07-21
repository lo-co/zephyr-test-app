/**
 * @file event_module.h
 * @brief
 *
 * @copyright Copyright (c) 2025
 *
 */

 #include <zephyr/kernel.h>

 #pragma once

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define NO_EVENT        0x0
#define EVENT_BUTTON_1S 0x1
/*******************************************************************************
 * Type Definitions
 ******************************************************************************/

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

void event_module_init();

struct k_event * const event_module_get_event_object();

uint32_t event_module_wait_on_event(uint32_t wait_ms);