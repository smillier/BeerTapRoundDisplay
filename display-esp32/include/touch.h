#ifndef __TOUCH_H__
#define __TOUCH_H__

// CST816 capacitive touch controller on the round display (JCZN
// ESP32-2424S012 board). A long-press anywhere on the screen sends a tare
// command to the remote scale, same action as the "Tare" button in the web UI.
// Requires a long-press rather than a plain tap so it isn't triggered by
// accidentally brushing the screen.
extern void touch_init();
extern void touch_loop();

#endif
