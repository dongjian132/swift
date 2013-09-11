//===--- Stubs.cpp - Swift Language ABI Runtime Stubs ---------------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2015 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://swift.org/LICENSE.txt for license information
// See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
//
// Misc stubs for functions which should be in swift.swift, but are difficult
// or impossible to write in swift at the moment.
//
//===----------------------------------------------------------------------===//

#include <mach/mach_time.h>
#include <sys/resource.h>
#include <sys/errno.h>
#include <pthread.h>
#include <unistd.h>
#include <cstring>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <sys/stat.h> // stat
#include <fcntl.h>    // open
#include <unistd.h>  // read, close
#include <dirent.h>
#include <limits.h>
#include "llvm/ADT/StringExtras.h"

// static func String(v : Int128, radix : Int) -> String
extern "C"
unsigned long long
print_int(char* TmpBuffer, __int64_t buf_len, __int128_t X, uint64_t Radix,
          bool uppercase) {
  assert(Radix != 0 && Radix <= 36 && "Invalid radix for string conversion");
  char *P = TmpBuffer;

  bool WasNeg = X < 0;
  __uint128_t Y = WasNeg ? -X : X;

  if (Y == 0) {
    *P++ = '0';
  } else if (Radix == 10) {
    while (Y) {
      *P++ = '0' + char(Y % 10);
      Y /= 10;
    }
  } else {
    unsigned Radix32 = Radix;
    while (Y) {
      *P++ = llvm::hexdigit(Y % Radix32, !uppercase);
      Y /= Radix32;
    }
  }

  if (WasNeg) *P++ = '-';
  std::reverse(TmpBuffer, P);
  return size_t(P - TmpBuffer);
}

// static func String(v : UInt128, radix : Int) -> String
extern "C"
unsigned long long
print_uint(char* TmpBuffer, __int64_t buf_len, __uint128_t Y, uint64_t Radix,
           bool uppercase) {
  assert(Radix != 0 && Radix <= 36 && "Invalid radix for string conversion");
  char *P = TmpBuffer;

  if (Y == 0) {
    *P++ = '0';
  } else if (Radix == 10) {
    while (Y) {
      *P++ = '0' + char(Y % 10);
      Y /= 10;
    }
  } else {
    unsigned Radix32 = Radix;
    while (Y) {
      *P++ = llvm::hexdigit(Y % Radix32, !uppercase);
      Y /= Radix32;
    }
  }

  std::reverse(TmpBuffer, P);
  return size_t(P - TmpBuffer);
}

// static func String(v : Double) -> String
extern "C"
unsigned long long
print_double(char* Buffer, double X) {
  long long i = sprintf(Buffer, "%g", X);
  // Add ".0" to a float that (a) is not in scientific notation, (b) does not
  // already have a fractional part, (c) is not infinite, and (d) is not a NaN
  // value.
  if (strchr(Buffer, 'e') == nullptr && strchr(Buffer, '.') == nullptr &&
      strchr(Buffer, 'n') == nullptr) {
    Buffer[i++] = '.';
    Buffer[i++] = '0';
  }
  if (i < 0) {
    __builtin_trap();
  }
  return i;
}

// FIXME: We shouldn't be writing implemenetations for functions in the swift
// module in C, and this isn't really an ideal place to put those
// implementations.
extern "C" void _TSs5printFT3valSi_T_(int64_t l) {
  printf("%lld", l);
}

extern "C" void _TSs5printFT3valSu_T_(uint64_t l) {
  printf("%llu", l);
}

extern "C" void _TSs5printFT3valSd_T_(double l) {
  char Buffer[256];
  unsigned long long i = print_double(Buffer, l);
  Buffer[i] = '\0';
  printf("%s", Buffer);
}

static bool
_swift_replOutputIsUTF8(void) {
  const char *lang = getenv("LANG");
  return lang && strstr(lang, "UTF-8");
}

extern "C"
uint32_t
swift_replOutputIsUTF8(void) {
  static auto rval = _swift_replOutputIsUTF8();
  return rval;
}

extern "C"
int
swift_file_open(const char* filename)
{
  return open(filename, O_RDONLY);
}

extern "C"
int
swift_file_close(int fd)
{
  return close(fd);
}

extern "C"
size_t
swift_file_read(int fd, char* buf, size_t nb)
{
  return read(fd, buf, nb);
}

extern "C"
size_t
swift_fd_size(int fd)
{
  struct stat buf;
  int err = fstat(fd, &buf);
  assert(err == 0);
  (void) err;
  return buf.st_size;
}

struct readdir_tuple_s {
  char *str;
  int64_t len;
};

extern "C"
struct readdir_tuple_s
posix_readdir_hack(DIR *d)
{
  struct readdir_tuple_s rval = { NULL, 0 };
  struct dirent *dp;
  if ((dp = readdir(d))) {
    rval.str = dp->d_name;
    rval.len = dp->d_namlen;
  }
  return rval;
}

extern "C"
int64_t
posix_isDirectory_hack(const char *path)
{
  struct stat sb;
  int r = stat(path, &sb);
  (void)r;
  assert(r != -1);
  return S_ISDIR(sb.st_mode);
}

extern "C"
int
posix_get_errno(void)
{
  return errno; // errno is not a global, but a macro
}

extern "C"
void
posix_set_errno(int value)
{
  errno = value; // errno is not a global, but a macro
}
