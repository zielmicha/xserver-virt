#ifndef _VIRT_QUERY_H
#define _VIRT_QUERY_H

#define X_VirtDisplayAdjust 1

typedef struct {
    CARD8 reqType;       /* always VncExtReqCode */
    CARD8 virtExtReqType;
    CARD16 length B16;
    CARD32 pad B32;
    char name[32];
} xVirtDisplayAdjustReq;

#define sz_xVirtDisplayAdjustReq 40

int assertSize[sz_xVirtDisplayAdjustReq == sizeof(xVirtDisplayAdjustReq) ? 1 : -1];

typedef struct VirtDisplayAdjustRep {
    BYTE type; /* X_Reply */
    BYTE success;
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD32 pad0 B32;
    CARD32 pad1 B32;
    CARD32 pad2 B32;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
} xVirtDisplayAdjustRep;

#endif
