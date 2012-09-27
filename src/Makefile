# make all buduje projekt, make unittests buduje testy jednostkowe
#
# Stala BUILD_TYPE okresla system docelowy, dla ktorego budujemy, przykladowe konfiguracje dla:
#	osx - MacOS X 10.5
#	std - typowy UNIX
#	emblinux - wbudowany Linux z uClibc

PROJECT_NAME=geirvarta
BUILD_TYPE=std

#COMMON_CFLAGS=-Wall -pg -g -O0 -DGUILE_SCRIPTING
COMMON_CFLAGS=-show -O3 -DGUILE_SCRIPTING

OBJECTS=main.o ui.o mpd.o scripting.o
UNITTESTS_OBJECTS=test.o mpd.o

AGAR_DIR=/usr/local/
AGAR_LIBDIR=$(AGAR_DIR)/lib
AGAR_INCLUDE=$(AGAR_DIR)/include/agar/

ATF_DIR=/opt/atf
ATF_INCLUDE=$(ATF_DIR)/include
ATF_LIBDIR=$(ATF_DIR)/lib

# MacOS X 10.5 build (libSDL, FreeType from MacPorts, and static Agar)
CFLAGS_osx=$(COMMON_CFLAGS) -arch i386 -I$(AGAR_INCLUDE) -I/opt/local/include/SDL -I/opt/local/include/libmpd-1.0 -I/opt/local/include
LDFLAGS_osx=-arch i386 -L/opt/local/lib -L/usr/X11/lib -lpthread -ldl -lSDL -lSDLmain -lmpd -lGL -lfreetype -lguile -framework Cocoa
STATICLIBS_osx=$(AGAR_LIBDIR)/libag_core.a $(AGAR_LIBDIR)/libag_gui.a

# standard UNIX build (dynamic + static Agar)
CFLAGS_std=$(COMMON_CFLAGS) -I$(AGAR_INCLUDE) -I/usr/include/SDL -I/usr/include/libmpd-1.0
LDFLAGS_std=-pg -lpthread -ldl -lSDL -lmpd -lguile #-lGL -lfreetype
STATICLIBS_std=$(AGAR_LIBDIR)/libag_core.a $(AGAR_LIBDIR)/libag_gui.a

# linux/uclibc - embedded
CFLAGS_emblinux=$(COMMON_CFLAGS) -I$(AGAR_INCLUDE) -I/usr/local/include/SDL -I/root/libmpd-includes/
LDFLAGS_emblinux= -lpthread -ldl -lSDL -liconv -lintl
STATICLIBS_emblinux=$(AGAR_LIBDIR)/libag_core.a $(AGAR_LIBDIR)/libag_gui.a /root/libmpd.a /root/libglib-2.0.a

##########################################

LDFLAGS=$(LDFLAGS_$(BUILD_TYPE))
STATICLIBS=$(STATICLIBS_$(BUILD_TYPE))
CFLAGS=$(CFLAGS_$(BUILD_TYPE))

all: $(PROJECT_NAME)-$(BUILD_TYPE)

$(PROJECT_NAME)-$(BUILD_TYPE) : $(OBJECTS)
	$(CC) -o $(PROJECT_NAME) $(LDFLAGS) $(OBJECTS) $(STATICLIBS)

unittests : $(UNITTESTS_OBJECTS)
	$(CC) -o unittests $(LDFLAGS) $(UNITTESTS_OBJECTS) $(STATICLIBS) $(ATF_LIBDIR)/libatf-c.a

test.o : test.c
	$(CC) $(CFLAGS) -I$(ATF_INCLUDE) -c $<

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean : 
	rm -f $(OBJECTS) $(UNITTESTS_OBJECTS)
	rm -f $(PROJECT_NAME) 
	rm -f gmon.out
	rm -f unittests
