## Zephyr Test Application

This is a simple test of building the Zephyr RToS for a [STM32 Nucleo F767ZI](https://www.st.com/en/evaluation-tools/nucleo-f767zi.html). It was initially based off of the blinky example provided by Zephyr, but has been expanded to 
flex functionality.  It is being used for demonstration and learning purposes at the moment.

### Hardware Setup

### LEDs 

The green LED on the dev board is toggled at a rate of 1 Hz. It is toggled in the main thread.

#### DHT-11

This project uses a DHT-11 for demonstrating peripheral communication.  The DHT-11 is a low cost (~$5) humidity and
temperature sensor. The accuracy is poor at about +/- 5% for the humidity and +/- 2 degrees Celsius for the temperature.

The wiring for the sensor is simple. The peripheral has three pins for powering and communicating as shown in the following table.

| DHT11 Pin | STM32 Pin | Dev Board Location | Description            |
| --------- | --------- | ------------------ | ---------------------- |
|    1      |  5V       |                    | Power for the DHT11    |
|    2      |  PC0      |                    | Data pin for hte DHT11 |         
|    3      |  NC       |                    |                        |
|    4      |  GND      |                    | Ground for the DHT11   |

The data is shipped using a simple protocol - the master (in this case the dev board) will toggle the line and low for > 17 ms 
and then release it.  This indicates to the sensor that the master is ready for data.  The sensor will respond, 
toggling the line twice to say it is preparing to send data.  After this, the senso will toggle the line for a fixed period -
30 us will indicate a 0 while 70 us will indicate a 1.  The sensor will ship 5 bytes as indicated in the following table:

| Byte | Description                      |
| ---- | -------------------------------- |
|  1   | RH high                          |
|  2   | RH Low; always 0                 |
|  3   | T high in degree Celsius         |
|  4   | T low; always 0                  |
|  5   | Parity - sum of previous 4 bytes |

Temperature and RH data are always integer values.  The low bytes as indicated in the table are always 0.

### Building the Application

Install in zephyr project directory.

To build: 

```
west build main_app -b nucleo_f767zi
```

To clean the build directory, add the `-p` switch to the build command.

To flash 

```
west flash
```

### Formatting

Uses `clang-format` with the zephyr format file.  Can be called with the command 

```
find . -name '*.[ch]' | xargs clang-format  -i --Werror
```

in the app directory.

![CI](https://github.com/lo-co/zephyr-test-app/actions/workflows/ci.yml/badge.svg)

