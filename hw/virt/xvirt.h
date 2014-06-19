#ifndef _XVIRT_H_
#define _XVIRT_H_

Bool vncRandRCrtcSet(ScreenPtr pScreen, RRCrtcPtr crtc, RRModePtr mode,
                     int x, int y, Rotation rotation, int num_outputs,
                     RROutputPtr *outputs);
RROutputPtr vncRandROutputCreate(ScreenPtr pScreen, char* name);
RRModePtr vncRandRModeGet(int width, int height);
RRCrtcPtr vncRandRCrtcCreate(ScreenPtr pScreen, char* name);

#endif
