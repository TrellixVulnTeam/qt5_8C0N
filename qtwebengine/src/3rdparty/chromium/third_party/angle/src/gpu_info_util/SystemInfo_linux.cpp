//
// Copyright (c) 2013-2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SystemInfo_linux.cpp: implementation of the Linux-specific parts of SystemInfo.h

#include "gpu_info_util/SystemInfo_internal.h"

#include <cstring>
#include <fstream>

#include "common/angleutils.h"
#include "common/debug.h"

namespace angle
{

namespace
{

bool ReadWholeFile(const char *filename, std::string *content)
{
    std::ifstream file(filename);

    if (!file)
    {
        return false;
    }

    *content = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    return true;
}

// Scan /sys/module/amdgpu/version.
bool GetAMDBrahmaDriverVersion(std::string *version)
{
    *version = "";
    std::string content;

    return ReadWholeFile("/sys/module/amdgpu/version", &content) &&
           ParseAMDBrahmaDriverVersion(content, version);
}

// Scan /etc/ati/amdpcsdb.default for "ReleaseVersion".
bool GetAMDCatalystDriverVersion(std::string *version)
{
    *version = "";
    std::string content;

    return ReadWholeFile("/etc/ati/amdpcsdb.default", &content) &&
           ParseAMDCatalystDriverVersion(content, version);
}

}  // anonymous namespace

#if !defined(GPU_INFO_USE_X11)
bool GetNvidiaDriverVersionWithXNVCtrl(std::string *version)
{
    return false;
}
#endif

#if !defined(GPU_INFO_USE_LIBPCI)
bool GetPCIDevicesWithLibPCI(std::vector<GPUDeviceInfo> *devices)
{
    return false;
}
#endif

bool GetSystemInfo(SystemInfo *info)
{
    return false;
#if 0
#if defined(__FreeBSD__)
    if (!CollectMesaCardInfo(&(info->gpus)))
    {
        if (!GetPCIDevicesFreeBSD(&(info->gpus)))
            return false;
    }
#else
    if (!GetPCIDevicesWithLibPCI(&(info->gpus)))
    {
        return false;
    }
#endif

    if (info->gpus.size() == 0)
    {
        return false;
    }

    FindPrimaryGPU(info);

    for (size_t i = 0; i < info->gpus.size(); ++i)
    {
        GPUDeviceInfo *gpu = &info->gpus[i];

        // New GPUs might be added inside this loop, don't query for their driver version again
        if (!gpu->driverVendor.empty())
        {
            continue;
        }

        if (IsAMD(gpu->vendorId))
        {
            std::string version;
            if (GetAMDBrahmaDriverVersion(&version))
            {
                gpu->driverVendor  = "AMD (Brahma)";
                gpu->driverVersion = std::move(version);
            }
            else if (GetAMDCatalystDriverVersion(&version))
            {
                gpu->driverVendor  = "AMD (Catalyst)";
                gpu->driverVersion = std::move(version);
            }
        }

        if (IsNvidia(gpu->vendorId))
        {
            std::string version;
            if (GetNvidiaDriverVersionWithXNVCtrl(&version))
            {
                gpu->driverVendor  = "Nvidia";
                gpu->driverVersion = std::move(version);
            }
        }

        // In dual-GPU cases the PCI scan sometimes only gives us the Intel GPU.
        // If we are able to query for the Nvidia driver version, it means there
        // was hidden Nvidia GPU, so we add it to the list and make it primary.
        if (IsIntel(gpu->vendorId) && info->gpus.size() == 1)
        {
            std::string version;
            if (GetNvidiaDriverVersionWithXNVCtrl(&version))
            {
                GPUDeviceInfo nvidiaInfo;
                nvidiaInfo.vendorId = kVendorID_Nvidia;
                nvidiaInfo.deviceId = 0;
                gpu->driverVendor   = "Nvidia";
                gpu->driverVersion  = std::move(version);

                info->gpus.emplace_back(std::move(nvidiaInfo));
                info->isOptimus = true;
            }
        }
    }

    return true;
#endif
}

}  // namespace angle
