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
#include "randrstr.h"
#include "selection.h"

#include "xvirt.h"
}

#include "virtExtInit.h"
#include "xorg-version.h"

#include "query.h"

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
void* vncFbptr[MAXSCREENS] = { 0, };
int vncFbstride[MAXSCREENS];

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

static const int VirtExtNumberEvents = 3;
static const int VirtExtNumberErrors = 0;


void vncExtensionInit()
{
  if (vncExtGeneration == serverGeneration) {
    fprintf(stderr, "vncExtensionInit: called twice in same generation?\n");
    return;
  }
  vncExtGeneration = serverGeneration;

  ExtensionEntry* extEntry
    = AddExtension("Virt-Display", VirtExtNumberEvents, VirtExtNumberErrors,
                   ProcVncExtDispatch, SProcVncExtDispatch, vncResetProc,
                   StandardMinorOpcode);
  if (!extEntry) {
    ErrorF("vncExtInit: AddExtension failed\n");
    return;
  }

  vncErrorBase = extEntry->errorBase;
  vncEventBase = extEntry->eventBase;

  fprintf(stderr, "Virt-Display extension running\n");

  if (!AddCallback(&ClientStateCallback, vncClientStateChange, 0)) {
    FatalError("Add ClientStateCallback failed\n");
  }

  if (!AddCallback(&SelectionCallback, vncSelectionCallback, 0)) {
    FatalError("Add SelectionCallback failed\n");
  }
  //RegisterBlockAndWakeupHandlers(vncBlockHandler, vncWakeupHandler, 0);
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

static CARD32 queryConnectTimerCallback(OsTimerPtr timer,
                                        CARD32 now, pointer arg)
{
  return 0;
}


static int ProcVirtDisplayAdjust(ClientPtr client) {
    REQUEST(xVirtDisplayAdjustReq);
    stuff->name[sizeof(stuff->name) - 1] = 0;

    fprintf(stderr, "Create output: '%s'\n", stuff->name);

    RRCrtcPtr crtc;
    RRModePtr mode;
    Bool ret;
    ScreenPtr pScreen = screenInfo.screens[0];

    crtc = vncRandRCrtcCreate(pScreen, stuff->name);
    mode = vncRandRModeGet(pScreen->width, pScreen->height);
    if (mode == NULL) {
        fprintf(stderr, "Failed to get mode\n");
        return (BadRequest);
    }

    ret = vncRandRCrtcSet(pScreen, crtc, mode, 0, 0, RR_Rotate_0,
                          crtc->numOutputs, crtc->outputs);
    if(!ret) {
        fprintf(stderr, "crtc set failed\n");
    }
    RRModeDestroy(mode);

    return (client->noClientException);
}

static int ProcVncExtDispatch(ClientPtr client)
{
  REQUEST(xReq);
  switch (stuff->data) {
  case X_VirtDisplayAdjust:
      return ProcVirtDisplayAdjust(client);
  default:
      return BadRequest;
  }
}

static int SProcVncExtDispatch(ClientPtr client)
{
  fprintf(stderr, "SProcVncExtDispatch\n");
  REQUEST(xReq);
  switch (stuff->data) {
  default:
    return BadRequest;
  }
}
