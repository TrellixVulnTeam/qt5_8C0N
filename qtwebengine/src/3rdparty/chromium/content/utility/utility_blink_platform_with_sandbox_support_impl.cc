// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/utility/utility_blink_platform_with_sandbox_support_impl.h"

#include "build/build_config.h"

#if defined(OS_MACOSX)
#include "content/child/child_process_sandbox_support_impl_mac.h"
#elif defined(OS_LINUX) || defined(OS_BSD)
#include "content/child/child_process_sandbox_support_impl_linux.h"
#endif

namespace content {

UtilityBlinkPlatformWithSandboxSupportImpl::
    UtilityBlinkPlatformWithSandboxSupportImpl(
        service_manager::Connector* connector) {
#if defined(OS_LINUX) || defined(OS_BSD)
  font_loader_ = sk_make_sp<font_service::FontLoader>(connector);
  SkFontConfigInterface::SetGlobal(font_loader_);
  sandbox_support_ = std::make_unique<WebSandboxSupportLinux>(font_loader_);
#elif defined(OS_MACOSX)
  sandbox_support_ = std::make_unique<WebSandboxSupportMac>(connector);
#endif
}

UtilityBlinkPlatformWithSandboxSupportImpl::
    ~UtilityBlinkPlatformWithSandboxSupportImpl() {}

blink::WebSandboxSupport*
UtilityBlinkPlatformWithSandboxSupportImpl::GetSandboxSupport() {
#if defined(OS_LINUX) || defined(OS_MACOSX) || defined(OS_BSD)
  return sandbox_support_.get();
#else
  return nullptr;
#endif
}

}  // namespace content
