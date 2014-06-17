/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright 2011 Pierre Ossman for Cendio AB
 * Copyright 2014 Michał Zieliński <michal@zielinscy.org.pl>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdio.h>
#include <errno.h>

extern "C" {
#define class c_class
#define public c_public
#define NEED_EVENTS
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/Xpoll.h>
#include "misc.h"
#include "os.h"
#include "dixstruct.h"
#include "extnsionst.h"
#include "scrnintstr.h"
#include "selection.h"
#define _VNCEXT_SERVER_
#define _VNCEXT_PROTO_
#include "virtExt.h"
#undef class
#undef xalloc
#undef public
}
#undef max
#undef min

#include "virtExtInit.h"
#include "xorg-version.h"

extern "C" {

  extern void vncExtensionInit();
  static void vncResetProc(ExtensionEntry* extEntry);
  static void vncBlockHandler(pointer data, OSTimePtr t, pointer readmask);
  static void vncWakeupHandler(pointer data, int nfds, pointer readmask);
  void vncWriteBlockHandler(fd_set *fds);
  void vncWriteWakeupHandler(int nfds, fd_set *fds);
  static void vncClientStateChange(CallbackListPtr*, pointer, pointer);
  static void SendSelectionChangeEvent(Atom selection);
  static int ProcVncExtDispatch(ClientPtr client);
  static int SProcVncExtDispatch(ClientPtr client);
  static void vncSelectionCallback(CallbackListPtr *callbacks, pointer data,
				   pointer args);

  extern char *display;
  extern char *listenaddr;
}
static unsigned long vncExtGeneration = 0;
static bool initialised = false;
void* vncFbptr[MAXSCREENS] = { 0, };
int vncFbstride[MAXSCREENS];

static char* clientCutText = 0;
static int clientCutTextLen = 0;
bool noclipboard = false;

static void* queryConnectId = 0;
static int queryConnectTimeout = 0;
static OsTimerPtr queryConnectTimer = 0;

static struct VncInputSelect* vncInputSelectHead = 0;
struct VncInputSelect {
  VncInputSelect(ClientPtr c, Window w, int m) : client(c), window(w), mask(m)
  {
    next = vncInputSelectHead;
    vncInputSelectHead = this;
  }
  ClientPtr client;
  Window window;
  int mask;
  VncInputSelect* next;
};

static int vncErrorBase = 0;
static int vncEventBase = 0;
int vncInetdSock = -1;


void vncExtensionInit()
{
  fprintf(stderr, "vncExtensionInit\n");
  //assert(0);
  if (vncExtGeneration == serverGeneration) {
      fprintf(stderr, "vncExtensionInit: called twice in same generation?\n");
      return;
  }
  vncExtGeneration = serverGeneration;

  ExtensionEntry* extEntry
    = AddExtension(VNCEXTNAME, VncExtNumberEvents, VncExtNumberErrors,
                   ProcVncExtDispatch, SProcVncExtDispatch, vncResetProc,
                   StandardMinorOpcode);
  if (!extEntry) {
    ErrorF("vncExtInit: AddExtension failed\n");
    return;
  }

  vncErrorBase = extEntry->errorBase;
  vncEventBase = extEntry->eventBase;

  printf("VNC extension running!\n");

  if (!AddCallback(&ClientStateCallback, vncClientStateChange, 0)) {
    FatalError("Add ClientStateCallback failed\n");
  }

  if (!AddCallback(&SelectionCallback, vncSelectionCallback, 0)) {
    FatalError("Add SelectionCallback failed\n");
  }
  RegisterBlockAndWakeupHandlers(vncBlockHandler, vncWakeupHandler, 0);
}

static void vncResetProc(ExtensionEntry* extEntry)
{
}

static void vncSelectionCallback(CallbackListPtr *callbacks, pointer data, pointer args)
{
  SelectionInfoRec *info = (SelectionInfoRec *) args;
  Selection *selection = info->selection;

  //SendSelectionChangeEvent(selection->selection);
}

static void vncWriteBlockHandlerFallback(OSTimePtr timeout);
static void vncWriteWakeupHandlerFallback();

//
// vncBlockHandler - called just before the X server goes into select().  Call
// on to the block handler for each desktop.  Then check whether any of the
// selections have changed, and if so, notify any interested X clients.
//

static void vncBlockHandler(pointer data, OSTimePtr timeout, pointer readmask)
{
  vncWriteBlockHandlerFallback(timeout);
}

static void vncWakeupHandler(pointer data, int nfds, pointer readmask)
{
  vncWriteWakeupHandlerFallback();
}

//
// vncWriteBlockHandler - extra hack to be able to get the main select loop
// to monitor writeable fds and not just readable. This requirers a modified
// Xorg and might therefore not be called. When it is called though, it will
// do so before vncBlockHandler (and vncWriteWakeupHandler called after
// vncWakeupHandler).
//

static bool needFallback = true;
static fd_set fallbackFds;
static struct timeval tw;

void vncWriteBlockHandler(fd_set *fds)
{
  needFallback = false;
}

void vncWriteWakeupHandler(int nfds, fd_set *fds)
{
}

static void vncWriteBlockHandlerFallback(OSTimePtr timeout)
{
  if (!needFallback)
    return;

  FD_ZERO(&fallbackFds);
  vncWriteBlockHandler(&fallbackFds);
  needFallback = true;

  if (!XFD_ANYSET(&fallbackFds))
    return;

  if ((*timeout == NULL) ||
      ((*timeout)->tv_sec > 0) || ((*timeout)->tv_usec > 10000)) {
    tw.tv_sec = 0;
    tw.tv_usec = 10000;
    *timeout = &tw;
  }
}

static void vncWriteWakeupHandlerFallback()
{
  int ret;
  struct timeval timeout;

  if (!needFallback)
    return;

  if (!XFD_ANYSET(&fallbackFds))
    return;

  timeout.tv_sec = 0;
  timeout.tv_usec = 0;

  ret = select(XFD_SETSIZE, NULL, &fallbackFds, NULL, &timeout);
  if (ret < 0) {
    ErrorF("vncWriteWakeupHandlerFallback(): select: %s\n",
           strerror(errno));
    return;
  }

  if (ret == 0)
    return;

  vncWriteWakeupHandler(ret, &fallbackFds);
}

static void vncClientStateChange(CallbackListPtr*, pointer, pointer p)
{
  ClientPtr client = ((NewClientInfoRec*)p)->client;
  if (client->clientState == ClientStateGone) {
    VncInputSelect** nextPtr = &vncInputSelectHead;
    for (VncInputSelect* cur = vncInputSelectHead; cur; cur = *nextPtr) {
      if (cur->client == client) {
        *nextPtr = cur->next;
        delete cur;
        continue;
      }
      nextPtr = &cur->next;
    }
  }
}

void vncBell()
{
}

void vncClientGone(int fd)
{
  if (fd == vncInetdSock) {
    fprintf(stderr,"inetdSock client gone\n");
    GiveUp(0);
  }
}

void vncClientCutText(const char* str, int len)
{
  delete [] clientCutText;
  clientCutText = new char[len];
  memcpy(clientCutText, str, len);
  clientCutTextLen = len;
  xVncExtClientCutTextNotifyEvent ev;
  ev.type = vncEventBase + VncExtClientCutTextNotify;
  for (VncInputSelect* cur = vncInputSelectHead; cur; cur = cur->next) {
    if (cur->mask & VncExtClientCutTextMask) {
      ev.sequenceNumber = cur->client->sequence;
      ev.window = cur->window;
      ev.time = GetTimeInMillis();
      if (cur->client->swapped) {
#if XORG < 112
        int n;
        swaps(&ev.sequenceNumber, n);
        swapl(&ev.window, n);
        swapl(&ev.time, n);
#else
        swaps(&ev.sequenceNumber);
        swapl(&ev.window);
        swapl(&ev.time);
#endif
      }
      WriteToClient(cur->client, sizeof(xVncExtClientCutTextNotifyEvent),
                    (char *)&ev);
    }
  }
}


static CARD32 queryConnectTimerCallback(OsTimerPtr timer,
                                        CARD32 now, pointer arg)
{
  return 0;
}

static int ProcVncExtDispatch(ClientPtr client)
{
  REQUEST(xReq);
  switch (stuff->data) {
  default:
    return BadRequest;
  }
}

static int SProcVncExtDispatch(ClientPtr client)
{
  REQUEST(xReq);
  switch (stuff->data) {
  default:
    return BadRequest;
  }
}
