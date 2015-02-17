//===- Config.h.in --------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Hand-coded for Android build
//===----------------------------------------------------------------------===//

#ifndef MCLD_CONFIG_H
#define MCLD_CONFIG_H

#include <llvm/Config/config.h>

#ifdef LLVM_ON_UNIX
# define MCLD_ON_UNIX 1
#else
// Assume on Win32 otherwise.
# define MCLD_ON_WIN32 1
#endif

#define MCLD_VERSION "Cilai - 2.4.0"

#define MCLD_REGION_CHUNK_SIZE 32
#define MCLD_NUM_OF_INPUTS 32
#define MCLD_SECTIONS_PER_INPUT 16
#define MCLD_SYMBOLS_PER_INPUT 128
#define MCLD_RELOCATIONS_PER_INPUT 1024

#define MCLD_SEGMENTS_PER_OUTPUT 8

// Define MCLD_DEFAULT_TARGET_TRIPLE based on the target we're building. The
// triple value comes from libbcc/include/bcc/Config/Config.h
#if defined(__arm__)
  #define MCLD_DEFAULT_TARGET_TRIPLE "armv7-none-linux-gnueabi"
#elif defined (__thumb__)
  #define MCLD_DEFAULT_TARGET_TRIPLE "thumbv7-none-linux-gnueabi"
#elif defined(__mips__)
  #define MCLD_DEFAULT_TARGET_TRIPLE "mipsel-none-linux-gnueabi"
#elif defined(__i386__)
  #define MCLD_DEFAULT_TARGET_TRIPLE "i686-unknown-linux"
#elif defined(__x86_64__)
  #define DEFAULT_X86_64_TRIPLE_STRING "x86_64-unknown-linux"
#endif

#endif

