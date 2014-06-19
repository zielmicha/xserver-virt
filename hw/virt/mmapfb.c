#include "mmapfb.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

static int vfb_fd;

void vfbInitMmapMemoryFramebuffer(const char* name) {
    fprintf(stderr, "using %s as a framebuffer\n", name);
    vfb_fd = open(name, O_CREAT | O_TRUNC | O_RDWR, 0700);
    if(vfb_fd < 0) {
        perror("open");
    }
}

void vfbAllocateMmapMemoryFramebuffer(vfbFramebufferInfoPtr pfb) {
    int size = pfb->sizeInBytes;
    if(ftruncate(vfb_fd, size) < 0) {
        perror("ftruncate");
        return;
    }
    void* addr = mmap(NULL, size, PROT_READ | PROT_WRITE,
                      MAP_SHARED, vfb_fd, 0);
    if(addr == NULL) {
        perror("mmap");
        return;
    }
    pfb->pfbMemory = addr;
}

void vfbFreeMmapMemoryFramebuffer(vfbFramebufferInfoPtr pfb) {
    if(munmap(pfb->pfbMemory, pfb->sizeInBytes) < 0) {
        perror("munmap");
    }
}
