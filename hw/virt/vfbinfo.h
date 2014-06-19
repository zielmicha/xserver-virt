#ifndef _VFB_INFO_H
#define _VFB_INFO_H

typedef struct
{
    int width;
    int height;

    int depth;

    /* Computed when allocated */

    int paddedBytesWidth;
    int paddedWidth;

    int bitsPerPixel;

    /* Private */

    int sizeInBytes;

    void *pfbMemory;

    int shmid;
} vfbFramebufferInfo, *vfbFramebufferInfoPtr;

#endif
