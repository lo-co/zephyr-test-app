## Zephyr Test Application

This is a simple test of building the Zephyr RToS for a [STM32 Nucleo F767ZI]{https://www.st.com/en/evaluation-tools/nucleo-f767zi.html}.

### Hardware Setup

### LEDs 

#### DHT-11

This project uses a DHT-11 connected to the 5 V on the Nucleo and the pin associated with PC0 the location for 
the data line.

| DHT11 Pin | STM32 Pin | Dev Board Location | Description            |
| --------- | --------- | ------------------ | ---------------------- |
|    1      |  5V       |                    | Power for the DHT11    |
|    2      |  PC0      |                    | Data pin for hte DHT11 |         
|    3      |  NC       |                    |                        |
|    4      |  GND      |                    | Ground for the DHT11   |

Single data line with a simple protocol.  Returns 5 bytes:

| Byte | Description                      |
| ---- | -------------------------------- |
|  1   | RH high                          |
|  2   | RH Low; always 0                 |
|  3   | T high in degree Celsius         |
|  4   | T low; always 0                  |
|  5   | Parity - sum of previous 4 bytes |

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

