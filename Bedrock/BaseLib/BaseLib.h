#pragma once
#include "Types.h"

#include "BaseConfig.h"
#include "StackTracker.h"
#include "Memory.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <WinSock2.h>
#include <Windows.h>
#include <comdef.h>
#include <tchar.h>

#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "Array.h"
#include "Assert.h"
#include "Color.h"
#include "Constants.h"
#include "CrashTracker.h"
#include "CriticalSection.h"
#include "Dictionary.h"
#include "EnumHelpers.h"
#include "FileList.h"
#include "ImageDecompressor.h"
#include "Logger.h"
#include "Matrix.h"
#include "Plane.h"
#include "Quaternion.h"
#include "Random.h"
#include "Rectangle.h"
#include "RingBuffer.h"
#include "Socket.h"
#include "SpecMath.h"
#include "Sphere.h"
#include "StreamReader.h"
#include "StreamWriter.h"
#include "String.h"
#include "Timer.h"
#include "Vector.h"

#define NoEmptyFile()   namespace { char NoEmptyFileDummy##__LINE__; }
