#pragma once

#ifdef _WIN64
#define WIN64_LEAN_AND_MEAN
#else
#define WIN32_LEAN_AND_MEAN
#endif
//#define ASSERT()

#include <winsock2.h>
#pragma comment( lib, "ws2_32.lib" )
#include <windows.h>
#include <process.h>
#include <mmsystem.h>
#pragma comment( lib, "winmm.lib")
#include <Mswsock.h>
#pragma comment( lib, "mswsock.lib" )

#include <stdio.h>
#include <stdlib.h>

#include ".\xsupport.h"
#include ".\xError.h"
//#include "..\header\xpacket.h"
