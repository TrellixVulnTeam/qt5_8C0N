// Copyright 2018 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_TRAP_HANDLER_HANDLER_INSIDE_POSIX_H_
#define V8_TRAP_HANDLER_HANDLER_INSIDE_POSIX_H_

#include <signal.h>
#include "include/v8config.h"

namespace v8 {
namespace internal {
namespace trap_handler {

#if defined(V8_OS_LINUX) || defined(V8_OS_FREEBSD)
constexpr int kOobSignal = SIGSEGV;
#elif V8_OS_MACOSX
constexpr int kOobSignal = SIGBUS;
#else
#error Posix trap handlers are only supported on Linux and MacOSX.
#endif

void HandleSignal(int signum, siginfo_t* info, void* context);

bool TryHandleSignal(int signum, siginfo_t* info, void* context);

}  // namespace trap_handler
}  // namespace internal
}  // namespace v8

#endif  // V8_TRAP_HANDLER_HANDLER_INSIDE_POSIX_H_
