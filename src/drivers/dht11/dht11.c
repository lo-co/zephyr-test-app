/**
 * @file dht11.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2025-07-10
 *
 * @copyright Copyright (c) 2025
 *
 */
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include <dht11.h>

LOG_MODULE_REGISTER(dht11, 3);

/*******************************************************************************
 * Definitions
 ******************************************************************************/

 /** Time in ms to hold the line low by the MCU before communication starts */
#define START_SIGNAL_MS 18

/** Number of data bits to expect from the DHT11 after setup */
#define NUM_DATA_BITS 40

/** Threshold in microseconds for a high data bit.
 *
 * This is set to 50 microseconds to avoid any issues, but
 * a high bit should be >70 us while the low bit is < 29 us.
 */
#define HIGH_BIT_THRESHOLD_US 50

/** Device tree node based on label in DT */
#define DHT11_NODE DT_NODELABEL(dht11_sensor)

/** Setup time us to indicate transition to data transmission.
 *
 * The DHT11 will indicate that transmission is about to begin by
 * first toggling the line low for 80 us and then allowing it to remain
 * high for 80 us.
 */
#define DHT11_DATA_SETUP_US 80

/** Start index in bit array for the high RH byte */
#define DHT11_RH_BYTE_MAJOR     0

/** Start index in bit array for the low RH byte */
#define DHT11_RH_BYTE_MINOR     8

/** Start index in bit array for the high T byte */
#define DHT11_T_BYTE_MAJOR      16

/** Start index in bit array for the low T byte */
#define DHT11_T_BYTE_MINOR      24

/** Start index in bit array for the parity byte */
#define DHT11_PARITY_BYTE       32


/*******************************************************************************
 * Type Definitions
 ******************************************************************************/

typedef struct pulse_data_s
{
    uint8_t level;
    uint32_t start_time;
    bool new_bit;
} pulse_data_t;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

 /**
  * @brief Pack data bits retrieved from the DHT11.
  *
  * Data retrieved from the DHT11 contains 5 bytes of data.  Each byte represents
  * one of the data points in the dht11_data_t struct.  This packs the bits into
  * a byte in that struct.
  *
  * @param bit_array Pointer to array containing retrieved bits
  * @param start_index Index of start of data to retrieve
  * @return Packed byte containing the data of interest.
  */
static uint8_t pack_bits(uint8_t *bit_array, uint8_t start_index);

static void gpio_cb(const struct device *dev,
                           struct gpio_callback *cb,
                           uint32_t pins);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/** GPIO spec retrieved using the device tree description */
static const struct gpio_dt_spec dht11_gpio = GPIO_DT_SPEC_GET(DHT11_NODE, gpios);

static struct gpio_callback dht11_cb_data;

static pulse_data_t pulse_data[90];

static uint32_t pulse_width[40];

static uint8_t current_pulse = 0;

static bool conversion_started = false;

static uint32_t start_time = 0;
/*******************************************************************************
 * Function Definitions
 ******************************************************************************/

 // Described in .h
dht11_error_t dht11_init()
{
	if (!gpio_is_ready_dt(&dht11_gpio)) {
		return DHT11_ERROR_CONFIG_FAILURE;
	}
    gpio_init_callback(&dht11_cb_data, gpio_cb,BIT(dht11_gpio.pin));
    gpio_add_callback_dt(&dht11_gpio, &dht11_cb_data);

    return DHT11_ERROR_NONE;
}

dht11_error_t dht11_start_data_conversion()
{
    // MCU is master - toggle the line to indicate MCU is ready for transmission
	if (gpio_pin_configure_dt(&dht11_gpio, GPIO_OUTPUT) < 0) {
		return DHT11_ERROR_CONFIG_FAILURE;
	}

    gpio_pin_set_dt(&dht11_gpio, 0);

    // Hold low for > 18 ms
    k_msleep(START_SIGNAL_MS);

    // Set the line for input to rececive data from the DHT11.  Since there should
    // be a pullup on the line, this cause the line to go high.
    if (gpio_pin_configure_dt(&dht11_gpio, GPIO_INPUT) < 0) {
		return DHT11_ERROR_CONFIG_FAILURE;
	}

    gpio_pin_interrupt_configure_dt(&dht11_gpio, GPIO_INT_EDGE_BOTH);



    return DHT11_ERROR_NONE;
}

// Described in .h
dht11_error_t dht11_get_data(dht11_data_t *data)
{
    // Make sure this value is 0'ed
    data->rh_high = 0;
    data->rh_low = 0;
    data->t_high = 0;
    data->t_low = 0;

    // MCU is master - toggle the line to indicate MCU is ready for transmission
	if (gpio_pin_configure_dt(&dht11_gpio, GPIO_OUTPUT) < 0) {
		return DHT11_ERROR_CONFIG_FAILURE;
	}

    // Retrieve an IRQ lock so that we can decode without any interrupts thus
    // creating an issue for determining the bit value
    unsigned int key = irq_lock();

    gpio_pin_set_dt(&dht11_gpio, 0);

    // Hold low for > 18 ms
    k_msleep(START_SIGNAL_MS);

    // Set the line for input to rececive data from the DHT11.  Since there should
    // be a pullup on the line, this cause the line to go high.
    if (gpio_pin_configure_dt(&dht11_gpio, GPIO_INPUT) < 0) {
		return DHT11_ERROR_CONFIG_FAILURE;
	}

    /* After we have pulled the pin low for 18 ms, the data line will be
     * setup for output from the DHT11.  Before data is transmitted, the
     * line will be 1) set low for 80 us and then 2) high for 80 us.  If
     * we do not see the line go high for 80 us, indicate that the setup
     * has failed.
     */

     /* Use this to indicate the start time for our measurement */
    uint32_t start = k_cycle_get_32();

    /* Last good start time */
    uint32_t pstart = start;

    /* Pin will likely initially be high because it is pulled high when
     * the MCU releases control due to pull up resistor.
     */
    while(gpio_pin_get_dt(&dht11_gpio));

    /* Pin will go low for 80 us but we don't care */
    while(!gpio_pin_get_dt(&dht11_gpio))
    {
        // Capture the last valid time for measuring duration
        pstart = start;
        start = k_cycle_get_32();
    }

    // Wait until the line goes low again
    while(gpio_pin_get_dt(&dht11_gpio));

    // Duration is the difference between the last low and the end of the high.
    // This will add an extra 5 us or so, but we don't care as we are only interested
    // in the duration greater than 80 us.
    uint32_t duration = k_cyc_to_us_ceil32(k_cycle_get_32() - pstart);

    if (duration < DHT11_DATA_SETUP_US)
    {
        LOG_ERR("Duration check failed: %d", duration);
        return DHT11_ERROR_SETUP_FAILED;
    }

    /* Storage for the bit data from the DHT11 */
    uint8_t bit_array[40];

    // Start retrieving data
    for (uint8_t bit = 0; bit < NUM_DATA_BITS; bit++)
    {
        start = k_cycle_get_32();
        pstart = start;

        // Low doesn't matter
        while(!gpio_pin_get_dt(&dht11_gpio))
        {
            pstart = start;
            start = k_cycle_get_32();
        }

        while(gpio_pin_get_dt(&dht11_gpio));

        // The value of the bit is determined by the high time
        duration = k_cyc_to_us_ceil32(k_cycle_get_32() - pstart);
        bit_array[bit] = duration > HIGH_BIT_THRESHOLD_US;
    }

    // Release the lock, we have finished probing the data line
    irq_unlock(key);

    data->rh_high = pack_bits(bit_array, DHT11_RH_BYTE_MAJOR);
    data->rh_low = pack_bits(bit_array, DHT11_RH_BYTE_MINOR);
    data->t_high = pack_bits(bit_array, DHT11_T_BYTE_MAJOR);
    data->t_low = pack_bits(bit_array, DHT11_T_BYTE_MINOR);
    data->parity = pack_bits(bit_array, DHT11_PARITY_BYTE);

    uint8_t parity_byte = data->rh_high + data->rh_low + data->t_high + data->t_low;

    // Check the parity byte
    if (data->parity != parity_byte)
    {
        return DHT11_ERROR_PARITY_CHECK_FAILED;
    }

    return DHT11_ERROR_NONE;
}

// Described above
static uint8_t pack_bits(uint8_t *bit_array, uint8_t start_index)
{
    uint8_t packed_data = 0;
    uint8_t upper_bound = start_index + 8;

    for (uint8_t idx = start_index; idx < upper_bound; idx++)
    {
        packed_data |= (bit_array[idx] << ((upper_bound - 1)-idx));
    }
    return packed_data;
}

static void gpio_cb(const struct device *dev,
                           struct gpio_callback *cb,
                           uint32_t pins)
{
    conversion_started = true;
    pulse_data[current_pulse].level = gpio_pin_get_dt(&dht11_gpio);
    pulse_data[current_pulse].start_time = k_cycle_get_32();
    pulse_data[current_pulse++].new_bit = true;
}

