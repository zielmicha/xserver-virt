#ifndef _MMAPFB_H
#define _MMAPFB_H

#include "vfbinfo.h"

void vfbInitMmapMemoryFramebuffer(const char* name);
void vfbAllocateMmapMemoryFramebuffer(vfbFramebufferInfoPtr pfb);
void vfbFreeMmapMemoryFramebuffer(vfbFramebufferInfoPtr pfb);

#endif
