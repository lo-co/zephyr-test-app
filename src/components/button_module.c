/**
 * @file button_module.c
 * @brief
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "include/button_module.h"
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/kernel/thread_stack.h>
#include <zephyr/logging/log.h>

#include <stdbool.h>

LOG_MODULE_REGISTER(btn_mod, 3);

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* size of stack area used by each thread */
#define STACKSIZE 256

/* scheduling priority used by each thread */
#define PRIORITY 7

/** User button node ID */
#define USER_BTN DT_ALIAS(sw0)

#define DEBOUNCE_TIME_MS 20

#define BTN_HOLD_TIME_MS 1000

/* Check to see if the button is available */
#if !DT_NODE_HAS_STATUS_OKAY(USER_BTN)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif

/** Define a memory region for the DHT11 thread stack */
K_THREAD_STACK_DEFINE(btn_thread_stack, STACKSIZE);

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/** Function called when the task is launched */
static void button_task();

/*******************************************************************************
 * Variables
 ******************************************************************************/

/** User button spec based on the DT description */
static const struct gpio_dt_spec button =
    GPIO_DT_SPEC_GET_OR(USER_BTN, gpios, {0});

/**  DHT11 thread structure */
static struct k_thread button_thread;

/*******************************************************************************
 * Function Definitions
 ******************************************************************************/

// Described in .h
int8_t button_module_init() {
  if (!gpio_is_ready_dt(&button)) {
    LOG_ERR("Button is not ready");
    return -1;
  }

  if (gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_BOTH)) {
    return -1;
  }

  // Create and start the DHT11 acquisition thread
  k_tid_t button_thread_id = k_thread_create(
      &button_thread, btn_thread_stack, K_THREAD_STACK_SIZEOF(btn_thread_stack),
      button_task, NULL, NULL, NULL, 7, 0, K_NO_WAIT);

  return 0;
}

static void button_task() {
  uint8_t current_button_state = 0;
  uint8_t previous_button_state = 0;

  enum BUTTON_STATE {
    BUTTON_STATE_INACTIVE,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_RELEASED,
    BUTTON_STATE_MAX,
  };

  enum BUTTON_STATE btn_state = BUTTON_STATE_INACTIVE;

  uint32_t cnt_start = 0;

  uint32_t btn_timer_start = 0;

  while (1) {
    current_button_state = gpio_pin_get_dt(&button);

    if (current_button_state != previous_button_state) {
      if (!cnt_start) {
        cnt_start = k_cycle_get_32();
      } else if (k_cyc_to_ms_ceil32(k_cycle_get_32() - cnt_start) >=
                 DEBOUNCE_TIME_MS) {
        previous_button_state = current_button_state;
        cnt_start = 0;

        LOG_INF("New button state is %d", current_button_state);

        if (current_button_state) // BUTTON PRESSED
        {
          // Start the timer on press
          btn_timer_start = k_cycle_get_32();
        } else // BUTTON RELEASED
        {
          uint32_t stop_time = k_cycle_get_32();
          if (k_cyc_to_ms_ceil32(stop_time - btn_timer_start) >=
              BTN_HOLD_TIME_MS) {
            LOG_INF("Button hold time was exceeded.  Hold time is %d",
                    k_cyc_to_ms_ceil32(stop_time - btn_timer_start));
          }
          btn_timer_start = 0;
        }

        // Make sure to reset the debounce timer once the event has been
        // processed
        cnt_start = 0;
      }
    } else {
      cnt_start = 0;
    }

    k_msleep(1);
  }
}
