/* ANSI includes */
#include <stdio.h>
#include <unistd.h>
/* OS-dependent includes */
#include <getopt.h>
#include <sys/utsname.h>
/* external libs */
#include <agar/core.h>
#include <agar/gui.h>
#include <SDL.h>
/* local includes */
#include "version.h"
#include "ui.h"
#include "mpd.h"
#include "conf.h"

#ifdef GUILE_SCRIPTING 
#include "scripting.h"
#endif

static void usage(void) {

	printf("-h                 Print this help.\n"
		   "-H hostname        Connect to MPD at specified hostname.\n"
		   "-o                 Force OpenGL for GUI rendering.\n"
		   "-p port            Connect to MPD using specified port.\n"
		   "-P password        Connect to MPD using specified password.\n"
		   "-r XRESxYRESxBPP   Start GUI in specified resolution.\n"
		   "-R refreshrate     Refresh GUI specified times per second.\n"
		   );	
}

/*
 * Print txt banner.
 */
static void banner(void) {

	struct utsname uts;
	AG_CPUInfo agCPU;
	AG_AgarVersion agVersion;

	uname(&uts);
	AG_GetCPUInfo(&agCPU);
	AG_GetVersion(&agVersion);

	printf("Geirvarta v%i.%i on %s %s (Agar %i.%i.%i, libmpd %s)\n", 
		GEIRVARTA_V_MAJOR, GEIRVARTA_V_MINOR, uts.sysname, agCPU.arch,
		agVersion.major, agVersion.minor, agVersion.patch, musicpd_version() );
	printf(GEIRVARTA_COPYRIGHT);
	printf("\n");	

}

/*
 * UI / mpd event loop.
 */
static void event_loop(void) {
	SDL_Event ev;
	AG_Window *win;
	Uint32 Tr1 = SDL_GetTicks(), Tr2 = 0;
 
	for (;;) {
		Tr2 = SDL_GetTicks();
		if (Tr2-Tr1 >= agView->rNom) { /* Time to redraw? */
			AG_LockVFS(agView);
 
			/* Render GUI elements */
			AG_BeginRendering();
			AG_TAILQ_FOREACH(win, &agView->windows, windows) {
				AG_ObjectLock(win);
				AG_WindowDraw(win);
				AG_ObjectUnlock(win);
			}
			AG_EndRendering();
			AG_UnlockVFS(agView);
			
			SDL_Flip(agView->v); /* TODO: only if doublebuffered */
 
			/* Recalibrate the effective refresh rate. */
			Tr1 = SDL_GetTicks();
			agView->rCur = agView->rNom - (Tr1-Tr2);
			if (agView->rCur < 1) {
				agView->rCur = 1;
			}
			
		} else if (SDL_PollEvent(&ev) != 0) {
			/* Send all SDL events to Agar-GUI. */
			AG_ProcessEvent(&ev);
		} else if (AG_TAILQ_FIRST(&agTimeoutObjQ) != NULL) {
			/* Advance the timing wheels. */
			AG_ProcessTimeout(Tr2);
		} else if (agView->rCur > agIdleThresh) {
			/* Idle the rest of the time. */
			SDL_Delay(agView->rCur - agIdleThresh);
		}
		musicpd_update(); /* Check for mpd events */
	}
}

/* 
 * Main program is main program.
 */
int main(int argc, char *argv[]) {

	int c, opterr, index, opengl;
	int agvideo_flags;

	char* gui_xres_str, * gui_yres_str, * gui_bpp_str;

	int gui_bpp = CONFIG_DFLT_GUI_BPP;
	int gui_xres = CONFIG_DLFT_GUI_XRES;
	int gui_yres = CONFIG_DFLT_GUI_YRES;
	int gui_rr = CONFIG_DFLT_GUI_REFRATE;
	char* mpd_host = CONFIG_DFLT_MPD_HOST;
	int mpd_port = CONFIG_DFLT_MPD_PORT;
	char* mpd_pass = CONFIG_DFLT_MPD_PASSWORD;
	
	opengl = 0;

	banner();
	
	opterr = 0;
	
	while ((c = getopt (argc, argv, "ohiH:p:P:r:R:")) != -1)
	switch (c) {
		case 'h':
			usage();
			exit(0);
		case 'o':
			opengl = true;
			break;
		case 'H':
			mpd_host = optarg;
			break;
		case 'p':
			mpd_port = atoi(optarg);
			break;
		case 'P':
			mpd_pass = optarg;
			break;
		case 'r':
			/* format XRESxYRESxBPP */
			gui_xres_str = strsep(&optarg, "x"); 
			gui_yres_str = strsep(&optarg, "x");
			gui_bpp_str = strsep(&optarg, "x");
	
			if( gui_xres_str == NULL || gui_yres_str == NULL || gui_bpp_str == NULL ) {
				usage();
				exit(1);
			}
			gui_xres = atoi(gui_xres_str); gui_yres = atoi(gui_yres_str); gui_bpp = atoi(gui_bpp_str);
			/*printf("trying gui_bpp: %i\n", gui_bpp);*/		 
			break;
		case 'R':
			gui_rr = atoi(optarg);
			break;
		case '?':
			return(1);
		default:
			abort(); /* sth went terribly wrong */
	}
	
	for (index = optind; index < argc; index++)
		printf ("Non-option argument %s\n", argv[index]);
	

	if (AG_InitCore("geirvarta", AG_VERBOSE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return(1);
	}	

	if(opengl) 
		agvideo_flags = AG_VIDEO_OPENGL;
	else 
		agvideo_flags = AG_VIDEO_HWSURFACE | AG_VIDEO_DOUBLEBUF;
	
	/* tu inituje sie de facto SDL - jesli nie ma myszy to trzeba ustawic SDL_NOMOUSE
	 * poza tym nie wiadomo czemu wisi na srodowisku emb do nacisniecia CTRL+C...
	 */ 
	if(AG_InitVideo(gui_xres, gui_yres, gui_bpp, agvideo_flags) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return(2);		
	}	

	/*ui_welcome();*/
	ui_bindkeys();
	ui_hwvideoinfo();

	if(!musicpd_init(mpd_host, mpd_port, mpd_pass)) {
		/*fprintf(stderr, "MPD init error\n");*/
		/*return(3);*/
	}	

	ui_main();

	AG_SetRefreshRate(gui_rr);
#ifdef GUILE_SCRIPTING 
	scripting_prepare(argc, argv);
#endif
	event_loop(); /* run GUI loop */

	/* exiting, clean up - musicpd_disconnect? */
	AG_Destroy();

	return(EXIT_SUCCESS);
}

