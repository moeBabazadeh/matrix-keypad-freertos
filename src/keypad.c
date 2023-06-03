#include <keypad.h>
#include <keypad_config.h>

// Global variables
keypad_t keypad;                              // Structure holding the state of the keypad.
EventGroupHandle_t keypad_event_group = NULL; //Event group handle for keypad events.
QueueHandle_t keypad_queue = NULL;            //Queue handle for keypad input.

/**
 * @brief Initializes the GPIO pins for the rows and columns of the keypad.
 *
 * This function configures the GPIO pins for the keypad rows as output pins with open-drain,
 * and the GPIO pins for the keypad columns as input pins with internal pull-up resistors enabled.
 * The row pins are set to a floating state by default to avoid short circuits when multiple keys
 * are pressed simultaneously. The column pins can be read to detect when a key connected to them is pressed.
 *
 * @note This function is called internally by the `keypad_init` function and should not be called directly.
 */
static void keypad_gpio_init(void) {
    uint8_t i;

    // Initialize GPIO for rows
    // We are iterating over each row pin defined in KEYPAD_ROW_GPIO and initializing it.
    for (i = 0; i < keypad.row_count; i++) {
        // Enable the peripheral clock for the GPIO port associated with the current row pin
        KEYPAD_GPIO_ENABLE_CLK(KEYPAD_ROW_GPIO[i].periph);

        // Configure the current row pin as output with open-drain.
        // This is to ensure that even if multiple keys are pressed at the same time,
        // no short circuit will occur between two output pins with different voltage levels.
        // The open-drain configuration allows the pin to be driven low (0) or to be left floating (Z),
        // which effectively disconnects the pin from the circuit.
        KEYPAD_GPIO_INIT(KEYPAD_ROW_GPIO[i].port, KEYPAD_ROW_GPIO[i].pin, KEYPAD_GPIO_MODE_OUT_OD);

        // Set the row pin to a floating state (no voltage applied).
        // This is the default state of the row pins when no keypress is being scanned.
        // The floating state also helps to avoid potential short circuits
        // when multiple keys are pressed simultaneously.
        KEYPAD_GPIO_SET(KEYPAD_ROW_GPIO[i].port, KEYPAD_ROW_GPIO[i].pin);
    }

    // Initialize GPIO for columns
    // We are iterating over each column pin defined in KEYPAD_COL_GPIO and initializing it.
    for (i = 0; i < keypad.col_count; i++) {
        // Enable the peripheral clock for the GPIO port associated with the current column pin
        KEYPAD_GPIO_ENABLE_CLK(KEYPAD_COL_GPIO[i].periph);

        // Configure the current column pin as input with internal pull-up resistor enabled.
        // This configuration allows us to read the state of the column pin
        // and detect when a key connected to it is pressed.
        KEYPAD_GPIO_INIT(KEYPAD_COL_GPIO[i].port, KEYPAD_COL_GPIO[i].pin, KEYPAD_GPIO_MODE_IPU);
    }
}

/**
 * @brief Dummy timer callback function.
 *
 * This function is a placeholder timer callback that is required as an argument to create FreeRTOS timers.
 * However, it does nothing and serves as a dummy function.
 *
 * @param x_timer The timer handle associated with the timer callback.
 *
 * @note This function does not perform any actions and can be used as a placeholder for timers that do not require a specific callback function.
 */
static void dummy_timer_callback(TimerHandle_t x_timer) {
    // This is a dummy function and does not perform any actions.
    // It serves as a placeholder for timers that do not require a specific callback function.
}

/**
 * @brief Scans the keypad for any pressed keys.
 *
 * This function scans the keypad to detect any keypresses. It iteratively 
 * drives each row pin low while leaving others floating, and checks the state
 * of the column pins. If a key is pressed, the corresponding column pin will be
 * pulled low, and the function detects this change.
 *
 * Each bit in the returned 32-bit value corresponds to a key, with a '1' indicating
 * a pressed key and '0' indicating an unpressed key. The bit position (0 to 31) corresponds 
 * to the key label defined in keypad_config.h.
 *
 * Note: This function is blocking and will not return until all row and column pins 
 * have been scanned. Therefore, it should be called from a context that is not time-sensitive.
 *
 * @return uint32_t: A 32-bit value representing the state of all keys on the keypad.
 */
uint32_t keypad_scan(void) {
    // Initialize key variable to 0. This variable will store the states of each key press.
    uint32_t key = 0;

    // Variables for the row and column iteration.
    uint8_t row, col;

    // Iterate over each row in the keypad.
    for (row = 0; row < keypad.row_count; row++) {
        // Drive the current row's GPIO pin to low. This is done to prepare for reading the column inputs.
        KEYPAD_GPIO_RESET(KEYPAD_ROW_GPIO[row].port, KEYPAD_ROW_GPIO[row].pin);

        // Insert a short delay to ensure the GPIO pin voltage level has stabilized.
        vTaskDelay(KEYPAD_GPIO_STABILIZATION_TIME);

        // Now, iterate over each column for the current row.
        for (col = 0; col < keypad.col_count; col++) {
            // Check if the current key (at the intersection of the current row and column) is pressed.
            // The key is considered pressed if the column GPIO pin is reading low.
            if (KEYPAD_GPIO_GET(KEYPAD_COL_GPIO[col].port, KEYPAD_COL_GPIO[col].pin) == 0) {
                // If the key is pressed, mark its corresponding bit in the 'key' variable.
                // We use bitwise OR operation to set the bit without affecting other bits.
                key |= 1 << ((row * keypad.col_count) + col);
            }
        }

        // Set the current row's GPIO pin back to high, effectively returning it to the floating state.
        // This is done after we finish scanning each column for the current row.
        KEYPAD_GPIO_SET(KEYPAD_ROW_GPIO[row].port, KEYPAD_ROW_GPIO[row].pin);
    }

    // Return the resultant 'key' variable, where each bit represents the state of a key on the keypad.
    // '1' signifies that the corresponding key is pressed, while '0' means it's not pressed.
    return key;
}

/**
 * @brief Task function dedicated to handling the keypad.
 *
 * This function `keypad_read` runs indefinitely as a task in the FreeRTOS environment,
 * dedicated to monitoring and handling user interactions with the keypad.
 *
 * Its primary responsibilities involve:
 * - Scanning the keypad for keypresses
 * - Applying a software debouncing mechanism to filter unintended quick, repeated actuations
 *
 * In each iteration of the main task loop, the function first scans the entire keypad for
 * any pressed keys. To address the problem of key 'bouncing', it makes use of a FreeRTOS
 * timer which adds a delay before a keypress is registered. This debounce timer is reset
 * whenever a keypress is detected.
 *
 * When no keys are pressed, the function the timer and resets the necessary tracking
 * variables.
 *
 * The state of the keypad is communicated to other parts of the application through two means
 * an event group and a queue. Both of these provide similar functionality and can be disabled
 * based on the user's requirements and application scenario. They facilitate inter-task communication
 * and synchronization.
 *
 * @note Even though the `param` parameter is included in the function signature, it is not
 * currently utilized in this implementation. Its presence is required to comply with
 * the FreeRTOS task function signature.
 *
 * @param param Unused parameter.
 */
void keypad_read(void* param) {
    // Create a timer to handle debounce. The dummy_timer_callback function is used as the callback.
    keypad.timer_debounce = xTimerCreate("Debounce", KEYPAD_DEBOUNCE_TIME, pdFALSE, (void*)0, dummy_timer_callback);

    // Create an Event Group to handle synchronization across tasks that depend on keypress events.
    keypad_event_group = xEventGroupCreate();

    // All bits in the event group are cleared at the start to avoid carrying over any previous state.
    xEventGroupClearBits(keypad_event_group, KEY_EVENT_BITMASK);

    // Create a queue to handle inter-task communication. The queue size is defined by KEYPAD_QUEUE_SIZE.
    keypad_queue = xQueueCreate(KEYPAD_QUEUE_SIZE, sizeof(uint32_t));

    // The main loop of the task.
    for (;;) {
        // Scan the keypad and store the result in keys_pressed.
        keypad.keys_pressed = keypad_scan();

        // If any key is currently pressed:
        if (keypad.keys_pressed) {
            // If the pressed keys differ from the keys in the previous iteration:
            if (keypad.keys_pressed != keypad.keys_down) {
                // Reset the debounce timer
                xTimerReset(keypad.timer_debounce, portMAX_DELAY);
            }

            // Update keys_down to remember the current state of keys.
            keypad.keys_down |= keypad.keys_pressed;
        } else {
            // Key released
            // Check if the debounce timer is not active (debounce time has passed) and a key was previously pressed
            if (keypad.keys_down != KEY_NONE && !xTimerIsTimerActive(keypad.timer_debounce)) {
                // Broadcast the released key through the event group
                xEventGroupSetBits(keypad_event_group, (keypad.keys_down & KEY_EVENT_BITMASK));
                // Add the released key to the keypad queue for further processing
                xQueueSend(keypad_queue, &keypad.keys_down, 0);
            }

            // Reset keys_down state
            keypad.keys_down = KEY_NONE;
        }

        // Yield the CPU to other tasks
        vTaskDelay(KEYPAD_TASK_DELAY_TIME);
    }
}

/**
 * @brief Initializes the keypad.
 *
 * This function initializes the keypad by performing the following steps:
 * - Calculates the number of rows and columns based on the size of the GPIO arrays.
 * - Sets the initial state of key-related variables.
 * - Calls the keypad_gpio_init function to initialize GPIO.
 * - Creates a FreeRTOS task for reading keypad input.
 *
 * @note This function should be called before using the keypad.
 *       It sets up the necessary components and starts the keypad task for input scanning.
 */
void keypad_init(void) {
    // Calculate number of rows and columns based on the size of the GPIO array
    // We get the total size of each array and divide it by the size of an individual element to get the count.
    keypad.row_count = sizeof(KEYPAD_ROW_GPIO) / sizeof(KEYPAD_ROW_GPIO[0]);
    keypad.col_count = sizeof(KEYPAD_COL_GPIO) / sizeof(KEYPAD_COL_GPIO[0]);

    // Initialize key state
    keypad.keys_pressed = 0; // Set all keys to not pressed (0) as the initial state.
    keypad.keys_down = 0;    // Set no keys as being pressed down (0) as the initial state.

    // Calls the function to initialize the GPIO pins for the keypad.
    keypad_gpio_init();

    // Create a FreeRTOS task for reading keypad
    // The task is named "Keypad", and uses the minimal stack size for it.
    // The priority of the task is set to 3 units higher than the idle priority.
    // We are not passing any parameters to the task function, hence NULL.
    // Also, we are not using the task handle, so we pass NULL for it too.
    xTaskCreate(keypad_read, "Keypad", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 3), NULL);
}
