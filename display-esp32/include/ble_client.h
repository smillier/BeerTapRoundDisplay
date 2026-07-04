#ifndef __BLE_CLIENT_H__
#define __BLE_CLIENT_H__

// Starts scanning for the scale board and manages the connection lifecycle.
extern void ble_client_init();

// Call this from loop(): drives (re)connection/(re)scan state machine.
extern void ble_client_loop();

// Sends a tare command to the scale, if currently connected.
extern void ble_client_send_tare();

extern bool ble_client_is_connected();
extern float ble_client_get_weight();
extern unsigned long ble_client_last_update();

#endif
