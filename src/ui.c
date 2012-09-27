#include "ui.h"

#define MAX_TIME_STR_LEN 10

#define UI_CMD_PLAY 1
#define UI_CMD_STOP 2
#define UI_CMD_PAUSE 3
#define UI_CMD_PREV 4
#define UI_CMD_NEXT 5 
#define UI_CMD_PLAY_ID 6
#define UI_CMD_PLAYLIST_ADD 7

/* welcome window 
static AG_Window *welcomewin; */

/* main window */
static AG_Window *mainwin;
static AG_Pane *plpane;
/*static AG_Box *playlistbox;
static AG_Box *librarybox;
static AG_Box *songbox;*/
static AG_Box *buttonbox;
static AG_Button *prevbutton;
static AG_Button *stopbutton;
static AG_Button *playbutton;
static AG_Button *pausebutton;
static AG_Button *nextbutton;
static AG_Table *playlisttable;
static AG_Table *librarytable;
static AG_Label *statuslbl;
static AG_Label *artistlbl;
static AG_Label *titlelbl;

/*
static void keyboard_handler(AG_Event *event)
{
	SDLKey sym = AG_SDLKEY(1);
	SDLMod mod = AG_SDLMOD(2);
	Uint32 unicode = (Uint32)AG_INT(3);

	printf("%s: sym=%u, modifier=0x%x, unicode=0x%x\n",
		event->name, (unsigned)sym, (unsigned)mod, unicode);
}
*/

/* format time to minutes:seconds string */
static char* format_time(int secs) {

	static char time_str[MAX_TIME_STR_LEN];

	if(secs==MPD_SONG_NO_TIME)
		return "unknown";

	snprintf(time_str, MAX_TIME_STR_LEN-1, "%i:%i", secs/60, secs%60);

	return time_str;

}

/* button mashing handler */
static void handle_ui_events(AG_Event *event) {

	/* AG_TextMsg(AG_MSG_INFO, "Hello, %i, %i!", AG_INT(1), AG_INT(2)); */

	switch(AG_INT(1)) {	
		case UI_CMD_PLAY:
			musicpd_play();
			break;
		case UI_CMD_STOP:
			musicpd_stop();
			break;
		case UI_CMD_PAUSE:
			musicpd_pause();
			break;
		case UI_CMD_NEXT:
			musicpd_next();
			break;
		case UI_CMD_PREV:
			musicpd_prev();
			break;
		case UI_CMD_PLAY_ID:
			musicpd_play_id(musicpd_get_id_playlist_entry(AG_INT(2)));
			break;
		case UI_CMD_PLAYLIST_ADD:
			musicpd_playlist_add_path(musicpd_get_path_library_entry(AG_INT(2)));
			break;
		default:
			break;
	}
}

/* todo: zrefaktoryzowac update_*table, bo moze wystarczy jedna funckcja? */
static void update_librarytable(AG_Event *event) {

        AG_Table *tbl = AG_SELF();

	if(libraryData) {
        	AG_TableBegin(tbl);

		do {
			if(libraryData->type == MPD_DATA_TYPE_SONG)  {

				if(libraryData->song->file && 
					libraryData->song->title && 
					libraryData->song->artist ) {  
				
					AG_TableAddRow(tbl, "%s:%s:%s:%s",
					libraryData->song->artist, 
					libraryData->song->title,
					format_time(libraryData->song->time),
					libraryData->song->file); 
				}
			}
			libraryData = mpd_data_get_next(libraryData);
		} while(libraryData);

		AG_TableEnd(tbl);
	}
}

static void update_playlisttable(AG_Event *event) {

        AG_Table *tbl = AG_SELF();

	if(playlistData) {
        	AG_TableBegin(tbl);

		do {
			if(playlistData->type == MPD_DATA_TYPE_SONG) 
				printf("%i, %s\n", 
				playlistData->song->id,
				playlistData->song->title);
				AG_TableAddRow(tbl, "%s:%s:%s:%i",
					playlistData->song->artist, 
					playlistData->song->title,
					format_time(playlistData->song->time),
					playlistData->song->id);

			playlistData = mpd_data_get_next(playlistData);

			/*if(mpd_data_is_last(playlistData)) {
				printf("last\n");
			}*/

		} while(playlistData);

		AG_TableEnd(tbl);
	}
}

/*
 * Set up main window, draw it.
 */
void ui_main() {

	mainwin = AG_WindowNew(AG_WINDOW_PLAIN);

	plpane = AG_PaneNew(mainwin, AG_PANE_VERT, 0);
	AG_Expand(plpane);

	{
		AG_LabelNew(plpane->div[0], 0, "Playlist");
		playlisttable = AG_TableNewPolled(plpane->div[0], AG_TABLE_EXPAND, update_playlisttable, NULL);
		AG_TableSizeHint(playlisttable, 200, 15);
		AG_SetInt(playlisttable->hbar, "max", 0); /* HURR jak wylaczyc scrollbar?
		//AG_TableAddCol(playlisttable, "id", "<foo>", NULL); */
		AG_TableAddCol(playlisttable, "Artist", "<some lengthy artist name>", NULL);
		AG_TableAddCol(playlisttable, "Title", NULL, NULL);
		AG_TableAddCol(playlisttable, "Length", "<9999:99>", NULL);

		/*AG_TableSetRowDblClickFn(playlisttable, handle_ui_events, "%i,%p", 
			UI_CMD_PLAY_ID, playlisttable ); */
 
		AG_TableSetRowDblClickFn(playlisttable, handle_ui_events, "%i", 
			UI_CMD_PLAY_ID );
		//jak to samo klawiatura osiagnac?*/
		//AG_SetEvent(playlisttable, "window-keydown", keyboard_handler, NULL);

	}	

	{
		AG_LabelNew(plpane->div[1], 0, "Library");
		librarytable = AG_TableNewPolled(plpane->div[1], AG_TABLE_EXPAND, update_librarytable, NULL);		
		AG_SetInt(librarytable->hbar, "max", 0); /* HURR jak wylaczyc scrollbar? */
		AG_TableAddCol(librarytable, "Artist", "<some lengthy artist name>", NULL);
		AG_TableAddCol(librarytable, "Title", NULL, NULL);
		AG_TableAddCol(librarytable, "Length", "<9999:99>", NULL);

		AG_TableSetRowDblClickFn(librarytable, handle_ui_events, "%i", 
			UI_CMD_PLAYLIST_ADD );
	}	

	artistlbl = AG_LabelNewPolled(mainwin, 0, "Artist: %s", &artistBuf);
	titlelbl = AG_LabelNewPolled(mainwin, 0, "Title: %s", &titleBuf);

	buttonbox = AG_BoxNew(mainwin, AG_BOX_HORIZ, 0);
	AG_ExpandHoriz(buttonbox);
	{
		prevbutton = AG_ButtonNewFn(buttonbox, 0, "|<", handle_ui_events, "%i", UI_CMD_PREV);
		stopbutton = AG_ButtonNewFn(buttonbox, 0, "[]", handle_ui_events, "%i", UI_CMD_STOP);
		pausebutton = AG_ButtonNewFn(buttonbox, 0, "||", handle_ui_events, "%i", UI_CMD_PAUSE);
		playbutton = AG_ButtonNewFn(buttonbox, 0, ">", handle_ui_events, "%i", UI_CMD_PLAY);
		nextbutton = AG_ButtonNewFn(buttonbox, 0, ">|", handle_ui_events, "%i", UI_CMD_NEXT);
	}

	statuslbl = AG_LabelNewPolled(mainwin, 0, "Status: %s", &statusBuf);

	AG_WindowMaximize(mainwin);
	AG_WindowShow(mainwin);

}

/* gdyby byl na to czas... 
void ui_welcome(void) {

	welcomewin = AG_WindowNew(AG_WINDOW_PLAIN);

	AG_WindowMaximize(welcomewin);
	AG_WindowShow(welcomewin);
	sleep(4);
	printf("baka\n");
	AG_WindowHide(welcomewin);


}
*/ 

void ui_bindkeys(void) {

	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	/*
	AG_SetEvent(win, "window-keydown", keyboard_handler, NULL);
	AG_SetEvent(win, "window-keyup", keyboard_handler, NULL);
	*/
}

void ui_hwvideoinfo(void) {

        const SDL_VideoInfo* vi;

        vi = SDL_GetVideoInfo();
	if(!vi)
		printf("Error getting hardware acceleration info: %s\n", SDL_GetError());
	else if(!(vi->hw_available))
		printf("NO hardware 2D acceleration for UI, expect high CPU usage.\n");
	else {
                printf("Hardware acceleration for UI available: ");
		printf( (vi->blit_hw) ? "blit_hw " : "");
		printf( (vi->blit_hw_CC) ? "blit_hw_CC " : "");
		printf( (vi->blit_hw_A) ? "blit_hw_A " : "");
		printf( (vi->blit_sw) ? "blit_sw " : "");
		printf( (vi->blit_sw_CC) ? "blit_sw_CC " : "");
		printf( (vi->blit_sw_A) ? "blit_sw_A " : "");
		printf( (vi->blit_fill) ? "blit_fill" : "");
		printf("\n");
		printf("SDL thinks we've got %i KB of video memory\n", vi->video_mem);
	}

}

