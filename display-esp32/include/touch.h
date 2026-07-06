#ifndef __TOUCH_H__
#define __TOUCH_H__

// CST816 capacitive touch controller on the round display (JCZN
// ESP32-2424S012 board). A single tap anywhere on the screen sends a tare
// command to the remote scale, same action as the "Tare" button in the web UI.
extern void touch_init();
extern void touch_loop();

#endif
