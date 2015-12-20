// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef _MSC_VER

// http://stackoverflow.com/questions/1191248/handling-stdafx-h-in-cross-platform-code

#include "targetver.h"

// TODO: reference additional headers your program requires here

#else
#include <stdint.h>

typedef int8_t __int8;
typedef int16_t __int16;
typedef int32_t __int32;
typedef int64_t __int64;
#endif
