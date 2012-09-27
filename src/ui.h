#ifndef _UI_H_
#define _UI_H_

#include <stdio.h>

#include <agar/core.h>
#include <agar/gui.h>

#include "mpd.h"

#define STATUS_BUF_LEN 200
#define ARTIST_BUF_LEN 200
#define TITLE_BUF_LEN 200

void ui_main();
void ui_welcome();
void ui_bindkeys();
void ui_hwvideoinfo();

/* HURR DURR GLOBALS R EVIL */

char statusBuf[STATUS_BUF_LEN+1];
char artistBuf[ARTIST_BUF_LEN+1];
char titleBuf[TITLE_BUF_LEN+1];

#endif /* _UI_H_ */
