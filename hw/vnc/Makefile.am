bin_PROGRAMS = Xvnc

XVNC_MORE_CPPFLAGS=-I../../Xext -I../../fb -I../../miext/damage/ \
	-I../../mi -I../../render -I../../randr -DDPMSExtension

Xvnc_SOURCES = xvnc.c vncExtInit.cc \
	$(top_srcdir)/Xi/stubs.c $(top_srcdir)/mi/miinitext.c \
	$(top_srcdir)/fb/fbcmap_mi.c buildtime.c

nodist_Xvnc_SOURCES = fbrop.h fb.h pixman.h

Xvnc_CPPFLAGS = $(XVNC_CPPFLAGS) -DTIGERVNC -DNO_MODULE_EXTS \
	-UHAVE_CONFIG_H \
	-DXFree86Server -DVENDOR_RELEASE="$(VENDOR_RELEASE)" \
	-DVENDOR_STRING="\"$(VENDOR_STRING)\"" -I$(TIGERVNC_SRCDIR)/common \
	-I$(top_srcdir)/include ${XSERVERLIBS_CFLAGS} -I$(includedir) $(XVNC_MORE_CPPFLAGS)

Xvnc_CFLAGS = $(XVNC_CPPFLAGS) -DTIGERVNC -DNO_MODULE_EXTS \
	-UHAVE_CONFIG_H \
	-DXFree86Server -DVENDOR_RELEASE="$(VENDOR_RELEASE)" \
	-DVENDOR_STRING="\"$(VENDOR_STRING)\"" -I$(TIGERVNC_SRCDIR)/common \
	-I$(top_srcdir)/include ${XSERVERLIBS_CFLAGS} -I$(includedir) $(XVNC_MORE_CPPFLAGS)

Xvnc_LDADD = $(XVNC_LIBS)  \
	$(XSERVER_LIBS) $(XSERVER_SYS_LIBS) $(XVNC_SYS_LIBS) -lX11

Xvnc_LDFLAGS = $(LD_EXPORT_SYMBOLS_FLAG)
