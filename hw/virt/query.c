#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "query.h"

int main(void) {
   Display* dpy = XOpenDisplay(NULL);
   if (dpy == NULL) {
      fprintf(stderr, "Cannot open display\n");
      exit(1);
   }

   XExtCodes* codes = XInitExtension(dpy, "Virt-Display");
   if(codes == NULL) {
       fprintf(stderr, "Failed to find extension\n");
       exit(1);
   }

   xVirtDisplayAdjustReq* req;

   LockDisplay(dpy);
   GetReq(VirtDisplayAdjust, req);

   strcpy(req->name, "foobar");
   req->reqType = codes->major_opcode;
   req->virtExtReqType = 1;

   XFlush(dpy);
   UnlockDisplay(dpy);
   SyncHandle();

   XCloseDisplay(dpy);
   return 0;
}

//
