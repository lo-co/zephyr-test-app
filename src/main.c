/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/kernel/thread_stack.h>

#include <stdbool.h>

#include <common.h>
#include <dht11.h>
#include <event_module.h>

#include <button_module.h>

LOG_MODULE_REGISTER(main_app, 3);

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/** DHT11 stack size (1 kB) */
#define STACK_SIZE 1024

/* Time to sleep in ms before toggling the green LED */
#define SLEEP_TIME_MS 500

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

/* The devicetree node identifier for the "led0" alias. */
#define LED2_NODE DT_ALIAS(led2)

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/**
 * @brief DHT11 polling thread
 *
 * @param arg1 UNUSED
 * @param arg2 UNUSED
 * @param arg3 UNUSED
 */
static void dht11_thread_start(void *arg1, void *arg2, void *arg3);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/** LED spec based on the DT description */
static const struct gpio_dt_spec green_led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

/**  Red LED spec based on the DT description */
static const struct gpio_dt_spec red_led = GPIO_DT_SPEC_GET(LED2_NODE, gpios);

/** Define a memory region for the DHT11 thread stack */
K_THREAD_STACK_DEFINE(thread_stack, STACK_SIZE);

/**  DHT11 thread structure */
static struct k_thread dht11_thread;

/*******************************************************************************
 * Function Definitions
 ******************************************************************************/

/**
 * @brief Main application
 *
 * @return int
 */
int main(void) {
  bool led_state = true;

  event_module_init();
  button_module_init();

  if (!gpio_is_ready_dt(&red_led)) {
    COMMON_LOG_ERR("LED GPIO is not ready");
    return -1;
  }

  if (!gpio_is_ready_dt(&green_led)) {
    COMMON_LOG_ERR("LED GPIO is not ready");
    return -1;
  }

  if (gpio_pin_configure_dt(&red_led, GPIO_OUTPUT_ACTIVE)) {
    COMMON_LOG_ERR("Unable to configure pin");
    return -1;
  }

  if (gpio_pin_configure_dt(&green_led, GPIO_OUTPUT_ACTIVE)) {
    COMMON_LOG_ERR("Unable to configure pin");
    return -1;
  }

  gpio_pin_set_dt(&red_led, 0);

  // Create and start the DHT11 acquisition thread
  k_tid_t dht11_tid = k_thread_create(
      &dht11_thread, thread_stack, K_THREAD_STACK_SIZEOF(thread_stack),
      dht11_thread_start, NULL, NULL, NULL, 7, 0, K_NO_WAIT);

  while (1) {
    if (gpio_pin_toggle_dt(&green_led)) {
      return -1;
    }

    led_state = !led_state;
    uint32_t events = event_module_wait_on_event(SLEEP_TIME_MS);

    if (events & EVENT_BUTTON_1S) {
    }
  }
  return 0;
}

// Described above
static void dht11_thread_start(void *arg1, void *arg2, void *arg3) {
  COMMON_LOG_INF("Starting DHT11 thread.");

  if (dht11_init(false)) {
    COMMON_LOG_ERR("DHT11 initialization failed.");
  }
  k_msleep(1000);

  while (1) {
    dht11_data_t data;
    dht11_error_t err = dht11_get_data(NULL, &data);
    if (err) {
      COMMON_LOG_ERR("Error retrieving DHT11 data. Err=%d", err);
    }
    COMMON_LOG_INF("RH=%d, T=%d, parity=%d", data.rh_high, data.t_high, data.parity);
    k_msleep(3000);
  }
}
