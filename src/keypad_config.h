#ifndef MATRIX_KEYPAD_CONFIG_H
#define MATRIX_KEYPAD_CONFIG_H

#include "gd32f10x.h"

// Configuration for the matrix keypad
#define KEYPAD_GPIO_STABILIZATION_TIME    (1UL / portTICK_PERIOD_MS)  // Delay for GPIO pin stabilization
#define KEYPAD_TASK_DELAY_TIME            (5UL / portTICK_PERIOD_MS)  // Delay for keypad tasks
#define KEYPAD_DEBOUNCE_TIME              (50UL / portTICK_PERIOD_MS) // Time to stabilize key after being pressed
#define KEYPAD_QUEUE_SIZE                 10                          // Size of keypad queue

// Keypad Key Definitions
#define KEY_NONE                          0
#define KEY_LONG                          0x10000
#define KEY_1                             0x0001
#define KEY_2                             0x0002
#define KEY_3                             0x0004
#define KEY_MEM                           0x0008
#define KEY_4                             0x0010
#define KEY_5                             0x0020
#define KEY_6                             0x0040
#define KEY_CHECK                         0x0080
#define KEY_7                             0x0100
#define KEY_8                             0x0200
#define KEY_9                             0x0400
#define KEY_MESSAGE                       0x0800
#define KEY_STAR                          0x1000
#define KEY_0                             0x2000
#define KEY_POUND                         0x4000
#define KEY_ENTER                         0x8000

/**
 * GPIO function mappings for keypad library.
 *
 * These defines map the generic GPIO functions used in the keypad library
 * to device-specific functions or macros. By editing these lines, the library
 * can be easily used with different devices by providing the appropriate mappings.
 *
 * The provided values below are for the GD32f103 device. Users need to adjust them
 * based on the target device they desire to use.
 *
 * - KEYPAD_GPIO_MODE_OUT_OD: Defines the mode for setting GPIO pins as output with open-drain configuration.
 * - KEYPAD_GPIO_MODE_IPU: Defines the mode for setting GPIO pins as input with pull-up configuration.
 * - KEYPAD_GPIO_ENABLE_CLK(PORT): Enables the peripheral clock for the GPIO port associated with the given PORT.
 * - KEYPAD_GPIO_INIT(PORT, PIN, MODE): Initializes a GPIO pin with the specified PORT, PIN, and MODE.
 * - KEYPAD_GPIO_SET(PORT, PIN): Sets a GPIO pin to a logic high state.
 * - KEYPAD_GPIO_RESET(PORT, PIN): Resets a GPIO pin to a logic low state.
 * - KEYPAD_GPIO_GET(PORT, PIN): Reads the current logic level of a GPIO pin.
 *
 * Users should modify these mappings to match the GPIO functions of their target device.
 */
#define KEYPAD_GPIO_MODE_OUT_OD           GPIO_MODE_OUT_OD
#define KEYPAD_GPIO_MODE_IPU              GPIO_MODE_IPU
#define KEYPAD_GPIO_ENABLE_CLK(PERIPH)    rcu_periph_clock_enable(PERIPH);
#define KEYPAD_GPIO_INIT(PORT, PIN, MODE) gpio_init(PORT, MODE, GPIO_OSPEED_2MHZ, PIN)
#define KEYPAD_GPIO_SET(PORT, PIN)        gpio_bit_set(PORT, PIN)
#define KEYPAD_GPIO_RESET(PORT, PIN)      gpio_bit_reset(PORT, PIN)
#define KEYPAD_GPIO_GET(PORT, PIN)        gpio_input_bit_get(PORT, PIN)

/**
 * Structure defining the GPIO configuration for the keypad.
 * Users need to adjust the types and values of the members based on their target device's GPIO port and peripherals.
 * This example is tailored for the GD32f103 device.
 *
 * - uint32_t port: GPIO port for the keypad.
 * - uint32_t pin: GPIO pin for the keypad.
 * - rcu_periph_enum periph: Peripheral enumeration for the GPIO port.
 *   Users should adjust the type and name of this member based on their target device's peripheral enumeration.
 */
typedef struct {
    uint32_t port;          // GPIO port for the keypad
    uint32_t pin;           // GPIO pin for the keypad
    rcu_periph_enum periph; // Peripheral enumeration for the GPIO port
} keypad_gpio_t;

/**
 * This section defines the GPIO configuration for the keypad, indicating how it is connected to the microcontroller.
 * Users should edit these configurations based on their specific board's GPIO mapping and naming conventions.
 *
 * Note: The provided GPIO configurations below are specific to the GD32f101 device and may need to be adjusted
 * for different microcontrollers with different GPIO mappings.
 */
const keypad_gpio_t KEYPAD_ROW_GPIO[] = {
    {GPIOC, GPIO_PIN_7, RCU_GPIOC}, // Edit these values to match the keypad row GPIO connections on your board
    {GPIOC, GPIO_PIN_8, RCU_GPIOC}, // Edit these values to match the keypad row GPIO connections on your board
    {GPIOC, GPIO_PIN_9, RCU_GPIOC}, // Edit these values to match the keypad row GPIO connections on your board
    {GPIOA, GPIO_PIN_8, RCU_GPIOA}  // Edit these values to match the keypad row GPIO connections on your board
};

const keypad_gpio_t KEYPAD_COL_GPIO[] = {
    {GPIOA, GPIO_PIN_9, RCU_GPIOA},  // Edit these values to match the keypad column GPIO connections on your board
    {GPIOA, GPIO_PIN_10, RCU_GPIOA}, // Edit these values to match the keypad column GPIO connections on your board
    {GPIOA, GPIO_PIN_11, RCU_GPIOA}, // Edit these values to match the keypad column GPIO connections on your board
    {GPIOA, GPIO_PIN_12, RCU_GPIOA}  // Edit these values to match the keypad column GPIO connections on your board
};

#endif
