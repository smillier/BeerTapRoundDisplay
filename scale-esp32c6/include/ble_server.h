#ifndef __BLE_SERVER_H__
#define __BLE_SERVER_H__

extern void ble_server_init();
extern void ble_notify_weight(float grams);
extern bool ble_server_has_client();

#endif
