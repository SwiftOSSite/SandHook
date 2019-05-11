//
// Created by swift on 2019/5/11.
//

#include <sys/mman.h>
#include <platform.h>
#include "code_buffer.h"
#include "lock.h"

using namespace SandHook::Assembler;
using namespace SandHook::Utils;

void *AndroidCodeBuffer::getBuffer(U32 size) {
    AutoLock autoLock(allocSpaceLock);
    void* mmapRes;
    Addr exeSpace = 0;
    if (executeSpaceList.size() == 0) {
        goto label_alloc_new_space;
    } else if (executePageOffset + size > currentExecutePageSize) {
        goto label_alloc_new_space;
    } else {
        exeSpace = reinterpret_cast<Addr>(executeSpaceList.back());
        Addr retSpace = exeSpace + executePageOffset;
        executePageOffset += size;
        return reinterpret_cast<void *>(retSpace);
    }
label_alloc_new_space:
    currentExecutePageSize = static_cast<U32>(FIT(size, P_SIZE));
    mmapRes = mmap(NULL, currentExecutePageSize, PROT_READ | PROT_WRITE | PROT_EXEC,
                   MAP_ANON | MAP_PRIVATE, -1, 0);
    if (mmapRes == MAP_FAILED) {
        return 0;
    }
    memset(mmapRes, 0, currentExecutePageSize);
    executeSpaceList.push_back(mmapRes);
    executePageOffset = size;
    return mmapRes;
}




StaticCodeBuffer::StaticCodeBuffer(Addr pc) : pc(pc) {}

void *StaticCodeBuffer::getBuffer(U32 bufferSize) {
    memUnprotect(pc, bufferSize);
    return reinterpret_cast<void *>(pc);
}
