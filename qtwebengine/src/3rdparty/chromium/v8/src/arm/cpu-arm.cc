// Copyright 2006-2009 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// CPU specific code for arm independent of OS goes here.
#ifdef __arm__
#ifdef __QNXNTO__
#include <sys/mman.h>  // for cache flushing.
#undef MAP_TYPE
#elif defined(__FreeBSD__)
#include <sys/types.h>
#include <machine/sysarch.h> // for cache flushing.
#else
#include <sys/syscall.h>  // for cache flushing.
#endif
#endif

#if V8_TARGET_ARCH_ARM

#include "src/cpu-features.h"

namespace v8 {
namespace internal {

void CpuFeatures::FlushICache(void* start, size_t size) {
#if !defined(USE_SIMULATOR)
#if V8_OS_QNX
  msync(start, size, MS_SYNC | MS_INVALIDATE_ICACHE);
#elif defined(__FreeBSD__)
  struct arm_sync_icache_args args = { .addr = (uintptr_t)start, .len = size };
  sysarch(ARM_SYNC_ICACHE, (void *)&args);
#else
  register uint32_t beg asm("r0") = reinterpret_cast<uint32_t>(start);
  register uint32_t end asm("r1") = beg + size;
  register uint32_t flg asm("r2") = 0;

#ifdef __clang__
  // This variant of the asm avoids a constant pool entry, which can be
  // problematic when LTO'ing. It is also slightly shorter.
  register uint32_t scno asm("r7") = __ARM_NR_cacheflush;

  asm volatile("svc 0\n"
               :
               : "r"(beg), "r"(end), "r"(flg), "r"(scno)
               : "memory");
#else
  // Use a different variant of the asm with GCC because some versions doesn't
  // support r7 as an asm input.
  asm volatile(
    // This assembly works for both ARM and Thumb targets.

    // Preserve r7; it is callee-saved, and GCC uses it as a frame pointer for
    // Thumb targets.
    "  push {r7}\n"
                                  // r0 = beg
                                  // r1 = end
                                  // r2 = flags (0)
    "  ldr r7, =%c[scno]\n"       // r7 = syscall number
    "  svc 0\n"

    "  pop {r7}\n"
    :
    : "r" (beg), "r" (end), "r" (flg), [scno] "i" (__ARM_NR_cacheflush)
    : "memory");
#endif
#endif
#endif  // !USE_SIMULATOR
}

}  // namespace internal
}  // namespace v8

#endif  // V8_TARGET_ARCH_ARM
