#pragma once

#include <windows.h>
#include <dbghelp.h>
#include <stdio.h>
#include <time.h>
#include <string>

#pragma comment(lib, "dbghelp.lib")

// 崩溃处理器类
class CrashHandler
{
public:
    // 初始化崩溃处理器
    static void Initialize();
    // 设置崩溃转储文件保存路径
    static void SetDumpPath(const char* path);
    // 设置额外的崩溃信息回调
    static void SetAdditionalInfoCallback(const char* (*callback)());
private:
    // 未处理异常的回调函数
    static LONG WINAPI UnhandledExceptionFilter(EXCEPTION_POINTERS* pExceptionInfo);
    // 生成MiniDump
    static void CreateMiniDump(EXCEPTION_POINTERS* pExceptionInfo);
    // 打印调用栈
    static void PrintStackTrace(EXCEPTION_POINTERS* pExceptionInfo, FILE* fp);
    // 打印异常信息
    static void PrintExceptionInfo(EXCEPTION_POINTERS* pExceptionInfo, FILE* fp);
    // 获取异常代码描述
    static const char* GetExceptionCodeString(DWORD code);
private:
    static std::string m_dumpPath;
    static const char* (*m_additionalInfoCallback)();
    static bool m_initialized;
};
// 便利宏定义
#define CRASH_HANDLER_INIT() CrashHandler::Initialize()
#define CRASH_HANDLER_SETPATH(path) CrashHandler::SetDumpPath(path)
