// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_OS_CRYPT_OS_CRYPT_H_
#define COMPONENTS_OS_CRYPT_OS_CRYPT_H_

#include <memory>
#include <string>

#include "base/component_export.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/single_thread_task_runner.h"
#include "base/strings/string16.h"
#include "build/build_config.h"

#if (defined(OS_LINUX) && !defined(OS_CHROMEOS)) || defined(OS_BSD)
class KeyStorageLinux;
#endif  // (defined(OS_LINUX) && !defined(OS_CHROMEOS)) || defined(OS_BSD)

#if defined(OS_MACOSX) && !defined(OS_IOS)
class PrefRegistrySimple;
class PrefService;
#endif

namespace os_crypt {
struct Config;
}

// The OSCrypt class gives access to simple encryption and decryption of
// strings. Note that on Mac, access to the system Keychain is required and
// these calls can block the current thread to collect user input. The same is
// true for Linux, if a password management tool is available.
class OSCrypt {
 public:
#if (defined(OS_LINUX) && !defined(OS_CHROMEOS)) || defined(OS_BSD)
  // Set the configuration of OSCrypt.
  static COMPONENT_EXPORT(OS_CRYPT) void SetConfig(
      std::unique_ptr<os_crypt::Config> config);
#endif  // (defined(OS_LINUX) && !defined(OS_CHROMEOS)) || defined(OS_BSD)

#if defined(OS_MACOSX) || (defined(OS_LINUX) && !defined(OS_CHROMEOS)) || defined(OS_BSD)
  // On Linux returns true iff the real secret key (not hardcoded one) is
  // available. On MacOS returns true if Keychain is available (for mock
  // Keychain it returns true if not using locked Keychain, false if using
  // locked mock Keychain).
  static COMPONENT_EXPORT(OS_CRYPT) bool IsEncryptionAvailable();
#endif

  // Encrypt a string16. The output (second argument) is really an array of
  // bytes, but we're passing it back as a std::string.
  static COMPONENT_EXPORT(OS_CRYPT) bool EncryptString16(
      const base::string16& plaintext,
      std::string* ciphertext);

  // Decrypt an array of bytes obtained with EncryptString16 back into a
  // string16. Note that the input (first argument) is a std::string, so you
  // need to first get your (binary) data into a string.
  static COMPONENT_EXPORT(OS_CRYPT) bool DecryptString16(
      const std::string& ciphertext,
      base::string16* plaintext);

  // Encrypt a string.
  static COMPONENT_EXPORT(OS_CRYPT) bool EncryptString(
      const std::string& plaintext,
      std::string* ciphertext);

  // Decrypt an array of bytes obtained with EnctryptString back into a string.
  // Note that the input (first argument) is a std::string, so you need to first
  // get your (binary) data into a string.
  static COMPONENT_EXPORT(OS_CRYPT) bool DecryptString(
      const std::string& ciphertext,
      std::string* plaintext);

#if defined(OS_MACOSX) && !defined(OS_IOS)
  // Registers preferences used by OSCrypt.
  static COMPONENT_EXPORT(OS_CRYPT) void RegisterLocalPrefs(
      PrefRegistrySimple* registry);

  // Initialises OSCrypt.
  // This method should be called on the main UI thread before any calls to
  // encryption or decryption.
  static COMPONENT_EXPORT(OS_CRYPT) void Init(PrefService* local_state);
#endif

#if defined(OS_MACOSX)
  // For unit testing purposes we instruct the Encryptor to use a mock Keychain
  // on the Mac. The default is to use the real Keychain. Use OSCryptMocker,
  // instead of calling this method directly.
  static COMPONENT_EXPORT(OS_CRYPT) void UseMockKeychainForTesting(
      bool use_mock);

  // When Keychain is locked, it's not possible to get the encryption key. This
  // is used only for testing purposes. Enabling locked Keychain also enables
  // mock Keychain. Use OSCryptMocker, instead of calling this method directly.
  static COMPONENT_EXPORT(OS_CRYPT) void UseLockedMockKeychainForTesting(
      bool use_locked);

  // Get the raw encryption key to be used for all AES encryption. Returns an
  // empty string in the case password access is denied or key generation error
  // occurs. This method is thread-safe.
  static COMPONENT_EXPORT(OS_CRYPT) std::string GetRawEncryptionKey();

  // Set the raw encryption key to be used for all AES encryption.
  // This method is thread-safe.
  static COMPONENT_EXPORT(OS_CRYPT) void SetRawEncryptionKey(
      const std::string& key);
#endif

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(OSCrypt);
};

#if (defined(OS_LINUX) && !defined(OS_CHROMEOS)) || defined(OS_BSD)
// For unit testing purposes, inject methods to be used.
// |get_key_storage_mock| provides the desired |KeyStorage| implementation.
// If the provider returns |nullptr|, a hardcoded password will be used.
// |get_password_v11_mock| provides a password to derive the encryption key from
// If one parameter is |nullptr|, the function will be not be replaced.
// If all parameters are |nullptr|, the real implementation is restored.
COMPONENT_EXPORT(OS_CRYPT)
void UseMockKeyStorageForTesting(
    std::unique_ptr<KeyStorageLinux> (*get_key_storage_mock)(),
    std::string* (*get_password_v11_mock)());

// Clears any caching and most lazy initialisations performed by the production
// code. Should be used after any test which required a password.
COMPONENT_EXPORT(OS_CRYPT) void ClearCacheForTesting();
#endif  // (defined(OS_LINUX) && !defined(OS_CHROMEOS)) || defined(OS_BSD)

#endif  // COMPONENTS_OS_CRYPT_OS_CRYPT_H_
