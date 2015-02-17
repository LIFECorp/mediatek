#include "asan_internal.h"

__attribute__((section(".preinit_array")))
  typeof(__asan_init) *__asan_preinit =__asan_init;
