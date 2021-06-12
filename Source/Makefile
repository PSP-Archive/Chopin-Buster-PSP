# ソフトのタイトル ---------------------------------
PSP_EBOOT_TITLE	= ChopinBuster


# ソースファイル(中間ファイル) ---------------------
OBJS = Entry.o CharImg.o shoot_wav.o bomb_wav.o


# その他 -------------------------------------------
TARGET			= ModelApp
CFLAGS = -O2 -G0 -Wall -g
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
LIBS = -lpspgum -lpspgu -lpspaudiolib -lpspaudio -lm

EXTRA_TARGETS = EBOOT.PBP

PSPSDK=$(shell psp-config --pspsdk-path)
PSPDIR=$(shell psp-config --psp-prefix)
include $(PSPSDK)/lib/build.mak
