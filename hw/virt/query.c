#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "query.h"

Display* getDisplay() {
    return XOpenDisplay(NULL);
}

XExtCodes* getExtCodes(Display* dpy) {
    return XInitExtension(dpy, "Virt-Display");
}

bool createOutput(Display* dpy, XExtCodes* codes, const char* name) {
    xVirtDisplayAdjustReq* req;

    LockDisplay(dpy);
    GetReq(VirtDisplayAdjust, req);

    strcpy(req->name, name);
    req->reqType = codes->major_opcode;
    req->virtExtReqType = 1;

    XFlush(dpy);
    UnlockDisplay(dpy);
    SyncHandle();
    return true;
}

#ifdef VIRTDISPLAY_WITH_MAIN
int main(int argc, char** argv) {
   Display* dpy = XOpenDisplay(NULL);
   if (dpy == NULL) {
       fprintf(stderr, "Cannot open display\n");
       exit(1);
   }

   XExtCodes* codes = getExtCodes(dpy);

   if(codes == NULL) {
       fprintf(stderr, "Failed to initialize extension\n");
       exit(1);
   }

   if(argc != 2) {
       fprintf(stderr, "Usage: %s output-name\n", argv[0]);
       exit(1);
   }

   char* name = argv[1];

   if(!createOutput(dpy, codes, name)) {
       fprintf(stderr, "Virt-Display extension returned error\n");
       exit(1);
   }

   XCloseDisplay(dpy);
   return 0;
}
#endif

//
