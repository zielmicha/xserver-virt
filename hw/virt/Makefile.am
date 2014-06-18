bin_PROGRAMS = Xvirt

XVNC_MORE_CPPFLAGS=-I../../Xext -I../../fb -I../../miext/damage/ \
	-I../../mi -I../../render -I../../randr -DDPMSExtension \
	-Wno-declaration-after-statement -Wno-old-style-definition \
	-Wno-missing-prototypes -Wno-shadow

Xvirt_SOURCES = xvirt.c virtExtInit.cc \
	$(top_srcdir)/Xi/stubs.c $(top_srcdir)/mi/miinitext.c \
	$(top_srcdir)/fb/fbcmap_mi.c buildtime.c

nodist_Xvirt_SOURCES = fbrop.h fb.h pixman.h

Xvirt_CPPFLAGS = $(XVNC_CPPFLAGS) -DNO_MODULE_EXTS \
	-UHAVE_CONFIG_H \
	-DXFree86Server -DVENDOR_RELEASE="$(VENDOR_RELEASE)" \
	-DVENDOR_STRING="\"$(VENDOR_STRING)\"" -I$(TIGERVNC_SRCDIR)/common \
	-I$(top_srcdir)/include ${XSERVERLIBS_CFLAGS} -I$(includedir) $(XVNC_MORE_CPPFLAGS)

Xvirt_CFLAGS = $(XVNC_CPPFLAGS) -DNO_MODULE_EXTS \
	-UHAVE_CONFIG_H \
	-DXFree86Server -DVENDOR_RELEASE="$(VENDOR_RELEASE)" \
	-DVENDOR_STRING="\"$(VENDOR_STRING)\"" -I$(TIGERVNC_SRCDIR)/common \
	-I$(top_srcdir)/include ${XSERVERLIBS_CFLAGS} -I$(includedir) $(XVNC_MORE_CPPFLAGS)

Xvirt_LDADD = $(XVNC_LIBS)  \
	$(XSERVER_LIBS) $(XSERVER_SYS_LIBS) $(XVNC_SYS_LIBS) -lX11

Xvirt_LDFLAGS = $(LD_EXPORT_SYMBOLS_FLAG)
