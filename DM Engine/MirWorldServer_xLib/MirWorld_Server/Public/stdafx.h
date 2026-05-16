// stdafx.h : 标准系统包含文件的包含文件, 
// 或是常用但不常更改的项目特定的包含文件

#pragma once

#ifdef _WIN64
#define WIN64_LEAN_AND_MEAN
#else
#define WIN32_LEAN_AND_MEAN
#endif

#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib" )
#include <Windows.h>
#include "stdio.h"
#include "stdlib.h"