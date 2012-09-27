/*
 * Interface to libmpd backend.
 */

#include <unistd.h>
#include <stdio.h>

#include "mpd.h"
#include "conf.h"
#include "ui.h"

static MpdObj *obj = NULL;

void musicpd_update(void) {
	if(obj)
		mpd_status_update(obj);
}

static void error_callback(MpdObj *mi,int errorid, char *msg, void *userdata)
{
        printf("Error %i: '%s'\n", errorid, msg);
	snprintf(statusBuf, STATUS_BUF_LEN, "%s", msg);
} 

static void update_playlistdata() {
	printf("Function update_playlistdata() called\n");// DEBUG

	playlistData = mpd_playlist_get_changes(obj,-1);
	if(!playlistData) 
		fprintf(stderr, "update_playlistdata() called but MPD returned no data?\n");
}

static void update_librarydata() {
	printf("Function update_librarydata() called\n");// DEBUG

	libraryData = mpd_database_get_complete(obj);
	if(!libraryData) 
		fprintf(stderr, "update_librarydata() called but MPD returned no data?\n");
}

static void status_changed(MpdObj *mi, ChangedStatusType eventType) {
	
	mpd_Song *currSong;

	if(eventType&MPD_CST_SONGID) {
		currSong = mpd_playlist_get_current_song(mi);
		if(currSong) { 
			snprintf(artistBuf, ARTIST_BUF_LEN, "%s", currSong->artist);
			snprintf(titleBuf, TITLE_BUF_LEN, "%s", currSong->title);

			/* update playlist position? */
		}
	}

	if(eventType&MPD_CST_PLAYLIST) {
		printf("Playlist changed\n");
		update_playlistdata();	
	}

	if(eventType&MPD_CST_DATABASE) {
		printf("Database changed\n");
		update_librarydata();	
	}

	if(eventType&MPD_CST_STATE) {

		switch(mpd_player_get_state(mi)) {
			case MPD_PLAYER_PAUSE:
				snprintf(statusBuf, STATUS_BUF_LEN, "Paused.");
				break;
			case MPD_PLAYER_PLAY:
				currSong = mpd_playlist_get_current_song(mi);
				if(currSong) { 
					snprintf(artistBuf, ARTIST_BUF_LEN, "%s", currSong->artist);
					snprintf(titleBuf, TITLE_BUF_LEN, "%s", currSong->title);
				}
				snprintf(statusBuf, STATUS_BUF_LEN, "Playing...");
				break;
			case MPD_PLAYER_STOP:
				snprintf(statusBuf, STATUS_BUF_LEN, "Stopped.");
				snprintf(artistBuf, ARTIST_BUF_LEN, " ");
				snprintf(titleBuf, TITLE_BUF_LEN, " ");
				break;
			default:
				printf("%i\n", mpd_player_get_state(mi));
				break;
		}
	}

}

bool musicpd_playlist_add_path(char *path) {
	printf("add: %s\n", path);
	
	if( mpd_playlist_add(obj,path) == MPD_OK )
		return true;
	else
		return false;
}

bool musicpd_play_id(int id) {
	if( mpd_player_play_id(obj, id) == MPD_OK )
		return true;
	else
		return false;
}

bool musicpd_play(void) {
	if( mpd_player_play(obj) == MPD_OK )
		return true;
	else
		return false;
}

bool musicpd_stop(void) {
	if( mpd_player_stop(obj) == MPD_OK ) 
		return true;
	else
		return false;
}

bool musicpd_prev(void) {
	if( mpd_player_prev(obj) == MPD_OK ) 
		return true;
	else
		return false;
}

bool musicpd_next(void) {
	if( mpd_player_next(obj) == MPD_OK ) 
		return true;
	else
		return false;
}

bool musicpd_pause(void) {
	if( mpd_player_pause(obj) == MPD_OK ) 
		return true;
	else
		return false;
}

/*
 * Check if we're connected to server.
 */
bool musicpd_connected(void) {
	
	if(obj != NULL)
		return mpd_check_connected(obj);

	return false;
}

bool musicpd_init(char* mpd_host, int mpd_port, char* mpd_pass) {
	obj = mpd_new(mpd_host, mpd_port, mpd_pass); 
        /* Connect signals */
        mpd_signal_connect_error(obj,(ErrorCallback)error_callback, NULL);
        mpd_signal_connect_status_changed(obj,(StatusChangedCallback)status_changed, NULL);
        /* Set timeout */
        mpd_set_connection_timeout(obj, 10);

	playlistData = NULL;
	libraryData = NULL;

	if(mpd_connect(obj) == MPD_OK) {
		update_librarydata();
		/*snprintf(statusBuf, STATUS_BUF_LEN, "Connected to MPD.");*/
		return(true);
	} else {
		obj = NULL;
		return(false);
	}
}

/*
 * Return MPD server version.
 */
char *musicpd_server_version(void) {
	
	return mpd_server_get_version(obj);
}

/*
 * Return libmpd version.
 *
 * TODO: rename to musicpd_lib_version?, fix string len
 */
char *musicpd_version(void) {

	static char libmpdver[25];

#ifdef LIBMPD_VERSION /* for old libmpd */
	snprintf(libmpdver, 24, "%s", LIBMPD_VERSION);
#else
	snprintf(libmpdver, 24, "%i.%i.%i",
		LIBMPD_MAJOR_VERSION, LIBMPD_MINOR_VERSION, LIBMPD_MICRO_VERSION);
#endif

	return libmpdver;
}

/*
 * Get id for playlist entry n, as Agar GUI is stupid.
 */
int musicpd_get_id_playlist_entry(int n) {
	MpdData *pl = mpd_playlist_get_changes(obj,-1);
	int i = 0;

	printf("n is %i\n", n);
	while(pl) {
		/* if(pl->song->file && pl->song->title && pl->song->artist) ? */
		if(pl->type == MPD_DATA_TYPE_SONG) {
		
			if(n==i) 
				return(pl->song->id);
			i++;
		}
		pl = mpd_data_get_next(pl);	
	} 
	return(-1);
}

char *musicpd_get_path_library_entry(int n) {

	MpdData *lib = mpd_database_get_complete(obj);
	int i = 0;

	while(lib) {
		if(lib->type == MPD_DATA_TYPE_SONG
		/* to nie bedzie potrz jak sie poprawi dod do tabeli w ui.c */
			&& lib->song->artist && lib->song->title
		/* XXX */
		) {
			if(n==i) {
				return(lib->song->file);
			}
			i++;
		}
		lib = mpd_data_get_next(lib);
	}
	return NULL;
}

bool musicpd_disconnect(void) {
	if(mpd_disconnect(obj) == MPD_OK)
		return true;
	
	return false;
}

bool musicpd_playlist_clear(void) {
	if(!mpd_playlist_clear(obj))
		return true;

	return false;
}