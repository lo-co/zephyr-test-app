/**
 * @file dht11.h
 * @brief API for use with the DHT11 atmospheric sensor
 *
 * The DHT11 is an atmospheric sensor that returns relative humidity in percent
 * and temperature in degrees Celsius.  The DHT11 has a simple 1-wire input. The
 * MCU communicates with the DHT11 on this wire as follows:
 *
 * 1. MCU pulls the data line low for more than 18 ms
 * 2. DHT11 responds by first pulling the line low for 80 us and 80 us high
 * 3. DHT11 then proceeds to send 40 bits with 0 and 1 distinguished by the high
 * time. The low proceeding the high portion of the pulse will be 50 us while
 * the pulse length for the high time will be between 26 and 70 us with the
 * longer pulse length representing a 1.
 *
 * @version 0.1
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

#include <stdint.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/

/**
 * @brief Data will be returned from the sensor as 4 bytes.
 *
 * Data is returned from the DHT11 sensor as 4 bytes.  The bytes and
 * order are:
 *
 * * High byte for relative humidity.  This will be the value to the left of the
 * decimal.
 * * Low byte for relative humidity.  This is the value to the right of the
 * decimal.
 * * High byte for temperature.  Defined as for the relative humidity.
 * * Low byte for temperature.  Defined as for the relative humidity.
 *
 */
typedef struct dht11_data_s {
  uint8_t rh_high; ///< High byte returned for the RH
  uint8_t rh_low;  ///< Low byte returned for the RH
  uint8_t t_high;  ///< High byte returned for temperature
  uint8_t t_low;   ///< Low byte returned for temperature
  uint8_t parity;  ///< Parity byte
} dht11_data_t;

typedef enum dht11_error_e {
  DHT11_ERROR_NONE = 0,
  DHT11_ERROR_CONFIG_FAILURE,
  DHT11_ERROR_SETUP_FAILED,
  DHT11_ERROR_PARITY_CHECK_FAILED,
  DHT11_ERROR_MAX
} dht11_error_t;

/*******************************************************************************
 * Variables Declarations
 ******************************************************************************/

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/**
 * @brief Initialize the DHT11 for operation
 *
 * @return DHT11_ERROR_NONE on success
 * @return DHT11_ERROR_CONFIG_FAILURE when fail to get ready value
 */
dht11_error_t dht11_init();

/**
 * @brief Retrieve the DHT11 serial data
 *
 * @param data Pointer to struct to store DHT11 data
 *
 * @return DHT11_ERROR_NONE on success
 * @return DHT11_ERROR_CONFIG_FAILURE failure to configure data
 * @return DHT11_ERROR_SETUP_FAILED Peripheral did not respond as expected
 * @return DHT11_ERROR_PARITY_CHECK_FAILED if parity byte indicates data
 * corruption.
 */
dht11_error_t dht11_get_data(dht11_data_t *data);
