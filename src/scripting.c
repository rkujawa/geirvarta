/*
 * Embedded guile interpreter. Runs in a different thread than UI.
 */

#include <pthread.h>
#include <stdbool.h>
#include <signal.h>

#include "mpd.h"
#include "scripting.h"
#include "version.h"

static int g_argc;
static char** g_argv;

static SCM wrap_musicpd_connected(void) {
	bool b = musicpd_connected();
	return scm_from_bool(b);
}

static SCM wrap_musicpd_lib_version(void) {
	char *s  = musicpd_version();
	if(s)
		return scm_from_locale_string(s);
	
	return SCM_BOOL_F;
}

static SCM wrap_musicpd_playlist_add_path(SCM path) {
	bool b;
	
	b = musicpd_playlist_add_path(scm_to_locale_string(path));
	return scm_from_bool(b);
}

static SCM wrap_player_version(void) {
	char s[8];
	snprintf(s, 8, "%i.%i", GEIRVARTA_V_MAJOR, GEIRVARTA_V_MINOR);

	if(*s)
		return scm_from_locale_string(s);

	return SCM_BOOL_F;
}

static SCM wrap_musicpd_pause(void) {
	bool b = musicpd_pause();
	return scm_from_bool(b);
}

static SCM wrap_musicpd_play(SCM id) {
	bool b;
	int mpdid;

	b = SCM_BOOL_F;
	if(scm_is_integer(id)) {
		mpdid = scm_to_int(id);
		b = musicpd_play_id(mpdid);
	} else {
		b = musicpd_play();
	}
	
	return scm_from_bool(b);
}

static SCM wrap_musicpd_stop(void) {
	bool b;
	
	b = musicpd_stop();
	return scm_from_bool(b);
}

static SCM wrap_musicpd_prev(void) {
	bool b;
	
	b = musicpd_prev();
	return scm_from_bool(b);
}

static SCM wrap_musicpd_next(void) {
	bool b;
	
	b = musicpd_next();
	return scm_from_bool(b);
}

static SCM wrap_musicpd_server_version(void) {
	char *s = musicpd_server_version();
	if(s)
		return scm_from_locale_string(s);

	return SCM_BOOL_F;
}

static void guile_main(void* data, int argc, char** argv) {
							/* req, opt, rest */
	scm_c_define_gsubr("musicpd-server-version", 0, 0, 0, wrap_musicpd_server_version);
	scm_c_define_gsubr("musicpd-playlist-add-path", 1, 0, 0, wrap_musicpd_playlist_add_path);
	scm_c_define_gsubr("musicpd-play", 0, 1, 0, wrap_musicpd_play);
	scm_c_define_gsubr("musicpd-prev", 0, 0, 0, wrap_musicpd_prev);
	scm_c_define_gsubr("musicpd-next", 0, 0, 0, wrap_musicpd_next);
	scm_c_define_gsubr("musicpd-pause", 0, 0, 0, wrap_musicpd_pause);
	scm_c_define_gsubr("musicpd-stop", 0, 0, 0, wrap_musicpd_stop);
	scm_c_define_gsubr("musicpd-lib-version", 0, 0, 0, wrap_musicpd_lib_version);
	scm_c_define_gsubr("musicpd-connected", 0, 0, 0, wrap_musicpd_connected);
	scm_c_define_gsubr("player-version", 0, 0, 0, wrap_player_version);
	/*scm_shell(argc,argv);*/
	scm_shell(0,NULL); 
}

void *scripting_thread(void *data) {
	printf("Booting guile v%i.%i.%i...\n", SCM_MAJOR_VERSION, SCM_MINOR_VERSION, SCM_MICRO_VERSION);
	scm_boot_guile(g_argc, g_argv, guile_main, 0);
	pthread_exit(NULL);
}

void scripting_start() {
	int rc;
	static pthread_t scr;


	rc = pthread_create(&scr, NULL, scripting_thread, NULL);

	if(rc != 0) /* scripting thread failed? */
		fprintf(stderr, "Booting guile failed!\n");
}

bool scripting_prepare(int argc, char** argv) {
	g_argc = argc;
	g_argv = argv;

	/* TODO: support guile cli arguments */

	if(signal(SIGUSR1, scripting_start) == SIG_ERR)
		return false;
	
	return true;
}

