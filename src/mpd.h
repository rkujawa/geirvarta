#ifndef _MPD_H_
#define _MPD_H_

#include <stdbool.h>
#include <libmpd/libmpd.h>

bool musicpd_init(char* mpd_host, int mpd_port, char* mpd_pass);
bool musicpd_disconnect(void);
bool musicpd_play(void);
bool musicpd_play_id(int id);
bool musicpd_stop(void);
bool musicpd_pause(void);
bool musicpd_next(void);
bool musicpd_prev(void);
void musicpd_update(void);
bool musicpd_connected(void);
char* musicpd_version(void);
int musicpd_get_id_playlist_entry(int n);
char* musicpd_get_path_library_entry(int n);
char* musicpd_server_version(void);
bool musicpd_playlist_add_path(char *path);
bool musicpd_playlist_clear(void);
/*void status_changed(MpdObj *mi, ChangedStatusType what);*/

MpdData *playlistData; 
MpdData *libraryData; 

#endif /* _MPD_H_ */
