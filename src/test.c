/*
 * Unit tests using ATF.
 */

#include <stdio.h>
#include <unistd.h>
#include <atf-c.h>

#include "mpd.h"

/* define test environment */

#define MPD_TEST_HOST "10.113.0.14"
#define MPD_TEST_PORT 6600
#define MPD_TEST_PASS 0 /* no password */
#define MPD_TEST_SONG1_PATH "Electro/Air/10000Hz_Legend/08-People_In_The_City.mp3"
#define MPD_TEST_SONG2_PATH "Electro/Air/10000Hz_Legend/01-Eletronic_Performers.mp3"

/* typical test case template:
 
ATF_TC(musicpd_TEMPLATE_test);
ATF_TC_HEAD(musicpd_TEMPLATE_test, tc) {
	atf_tc_set_md_var(tc, "descr", "musicpd_TEMPLATE", "function");
}
ATF_TC_BODY(musicpd_TEMPLATE_test, tc) {
	ATF_REQUIRE(musicpd_init(MPD_TEST_HOST, MPD_TEST_PORT, MPD_TEST_PASS));
	ATF_REQUIRE(musicpd_connected());
	
	//
 
	ATF_REQUIRE(musicpd_disconnect());
}
 
*/
 
ATF_TC(musicpd_playlist_add_test);
ATF_TC_HEAD(musicpd_playlist_add_test, tc) {
	atf_tc_set_md_var(tc, "descr", "musicpd_playlist_add", "function");
}
ATF_TC_BODY(musicpd_playlist_add_test, tc) {
	ATF_REQUIRE(musicpd_init(MPD_TEST_HOST, MPD_TEST_PORT, MPD_TEST_PASS));
	ATF_REQUIRE(musicpd_connected());

	ATF_REQUIRE(musicpd_playlist_add_path(MPD_TEST_SONG1_PATH));
	ATF_REQUIRE(musicpd_playlist_add_path(MPD_TEST_SONG2_PATH));
 
	ATF_REQUIRE(musicpd_disconnect());
}
 
ATF_TC(musicpd_playlist_clear_test);
ATF_TC_HEAD(musicpd_playlist_clear_test, tc) {
	atf_tc_set_md_var(tc, "descr", "musicpd_playlist_clear", "function");
}
ATF_TC_BODY(musicpd_playlist_clear_test, tc) {
	ATF_REQUIRE(musicpd_init(MPD_TEST_HOST, MPD_TEST_PORT, MPD_TEST_PASS));
	ATF_REQUIRE(musicpd_connected());
	
	ATF_REQUIRE(musicpd_playlist_clear());

	/* if playlist size 0 then ok */	
	
	ATF_REQUIRE(musicpd_disconnect());
}


ATF_TC(musicpd_play_test);
ATF_TC_HEAD(musicpd_play_test, tc) {
	atf_tc_set_md_var(tc, "descr", "musicpd_play", "function");
}
ATF_TC_BODY(musicpd_play_test, tc) {
	ATF_REQUIRE(musicpd_init(MPD_TEST_HOST, MPD_TEST_PORT, MPD_TEST_PASS));
	ATF_REQUIRE(musicpd_connected());

	ATF_REQUIRE(musicpd_play());
	/* TODO: if(status == playin) then ok, should fail if not */
	ATF_REQUIRE(musicpd_stop());
	
	ATF_REQUIRE(musicpd_disconnect());
}

ATF_TC(musicpd_play_id_test);
ATF_TC_HEAD(musicpd_play_id_test, tc) {
	atf_tc_set_md_var(tc, "descr", "musicpd_play_id", "function");
}
ATF_TC_BODY(musicpd_play_id_test, tc) {
	ATF_REQUIRE(musicpd_init(MPD_TEST_HOST, MPD_TEST_PORT, MPD_TEST_PASS));
	ATF_REQUIRE(musicpd_connected());

	ATF_REQUIRE(musicpd_play_id(musicpd_get_id_playlist_entry(0)));
	ATF_REQUIRE(musicpd_play_id(musicpd_get_id_playlist_entry(1)));
	/* TODO: if(status == playin) and right song
 	 * then ok, should fail if not */
	ATF_REQUIRE(musicpd_stop());
	
	ATF_REQUIRE(musicpd_disconnect());
}

ATF_TC(musicpd_prevnext_test);
ATF_TC_HEAD(musicpd_prevnext_test, tc) {
	atf_tc_set_md_var(tc, "descr", "musicpd_prev", "function");
}
ATF_TC_BODY(musicpd_prevnext_test, tc) {
	ATF_REQUIRE(musicpd_init(MPD_TEST_HOST, MPD_TEST_PORT, MPD_TEST_PASS));
	ATF_REQUIRE(musicpd_connected());

	ATF_REQUIRE(musicpd_play_id(musicpd_get_id_playlist_entry(0)));
	ATF_REQUIRE(musicpd_next());
	ATF_REQUIRE(musicpd_prev());
	/* TODO: if(status == playin) and right song
 	 * then ok, should fail if not */
	ATF_REQUIRE(musicpd_stop());
	
	ATF_REQUIRE(musicpd_disconnect());
}

ATF_TC(musicpd_init_test);
ATF_TC_HEAD(musicpd_init_test, tc) {
	atf_tc_set_md_var(tc, "descr", "musicpd_init, musicpd_connected, musicpd_disconnected", "function");
}
ATF_TC_BODY(musicpd_init_test, tc) {
	ATF_REQUIRE(musicpd_init(MPD_TEST_HOST, MPD_TEST_PORT, MPD_TEST_PASS));
	ATF_REQUIRE(musicpd_connected());
	ATF_REQUIRE(musicpd_disconnect());
}

ATF_TC(musicpd_version_test);
ATF_TC_HEAD(musicpd_version_test, tc) {
	atf_tc_set_md_var(tc, "descr", "musicpd_version", "function");
}
ATF_TC_BODY(musicpd_version_test, tc) {
	char *libmpdver = musicpd_version();
	printf("libmpd %s \n", libmpdver);
	ATF_REQUIRE(libmpdver);
}

ATF_TC(musicpd_server_version_test);
ATF_TC_HEAD(musicpd_server_version_test, tc) {
	atf_tc_set_md_var(tc, "descr", "musicpd_server_version", "function");
}
ATF_TC_BODY(musicpd_server_version_test, tc) {
	char *mpdver;

	ATF_REQUIRE(musicpd_init(MPD_TEST_HOST, MPD_TEST_PORT, MPD_TEST_PASS));
	ATF_REQUIRE(musicpd_connected());
	
	mpdver = musicpd_server_version();
	printf("MPD %s \n", mpdver);
	ATF_REQUIRE(mpdver);

	ATF_REQUIRE(musicpd_disconnect());
}

ATF_TP_ADD_TCS(tp) {

	ATF_TP_ADD_TC(tp, musicpd_init_test);
	ATF_TP_ADD_TC(tp, musicpd_version_test);
	ATF_TP_ADD_TC(tp, musicpd_server_version_test);
	ATF_TP_ADD_TC(tp, musicpd_playlist_clear_test);
	ATF_TP_ADD_TC(tp, musicpd_playlist_add_test);
	ATF_TP_ADD_TC(tp, musicpd_play_test);
	ATF_TP_ADD_TC(tp, musicpd_play_id_test);
	ATF_TP_ADD_TC(tp, musicpd_prevnext_test);

	return atf_no_error();

}

