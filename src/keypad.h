#ifndef MATRIX_KEYPAD_H
#define MATRIX_KEYPAD_H

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <event_groups.h>
#include <queue.h>

// Bitmask for key events. Set to 0x00FFFFFFU to match FreeRTOS EventGroup's 24-bit limit.
// Do not modify unless you have a specific requirement for a different number of key codes.
#define KEY_EVENT_BITMASK 0x00FFFFFFU

// type define of structure holding the state of the keypad.
typedef struct {
    uint8_t col_count;            // Number of columns in the keypad matrix
    uint8_t row_count;            // Number of rows in the keypad matrix
    uint32_t keys_pressed;        // Bitmask representing the currently pressed keys
    uint32_t keys_down;           // Bitmask representing the keys that were just pressed
    TimerHandle_t timer_debounce; // Timer handle for key debounce functionality
} keypad_t;

// External declaration of the keypad event group handle.
// This handle can be used to broadcast the result of reading the keypad to other tasks.
// Other tasks can wait for specific key events by using this event group.
extern EventGroupHandle_t keypad_event_group;

// External declaration of the keypad queue handle.
// This handle can be used to broadcast the result of reading the keypad to other tasks.
// Other tasks can listen to the pressed keys by receiving messages from this queue.
extern QueueHandle_t keypad_queue;

// Function declaration for initializing the keypad.
// This function should be called to initialize the keypad before using it.
// It sets up the necessary configurations and prepares the keypad for key input.
void keypad_init(void);

#endif
