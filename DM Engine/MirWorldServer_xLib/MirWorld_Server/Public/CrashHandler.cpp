#include "StdAfx.h"
#include "CrashHandler.h"
#include <psapi.h>

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "psapi.lib")

std::string CrashHandler::m_dumpPath = "..\\日志\\";
const char* (*CrashHandler::m_additionalInfoCallback)() = nullptr;
bool CrashHandler::m_initialized = false;

// 获取异常代码的描述
const char* CrashHandler::GetExceptionCodeString(DWORD code)
{
    switch (code)
    {
        case EXCEPTION_ACCESS_VIOLATION: return "访问违规 (EXCEPTION_ACCESS_VIOLATION)";
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "数组越界 (EXCEPTION_ARRAY_BOUNDS_EXCEEDED)";
        case EXCEPTION_BREAKPOINT: return "断点异常 (EXCEPTION_BREAKPOINT)";
        case EXCEPTION_DATATYPE_MISALIGNMENT: return "数据类型未对齐 (EXCEPTION_DATATYPE_MISALIGNMENT)";
        case EXCEPTION_FLT_DENORMAL_OPERAND: return "浮点数非正常操作数 (EXCEPTION_FLT_DENORMAL_OPERAND)";
        case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "浮点数除零 (EXCEPTION_FLT_DIVIDE_BY_ZERO)";
        case EXCEPTION_FLT_INEXACT_RESULT: return "浮点数结果不精确 (EXCEPTION_FLT_INEXACT_RESULT)";
        case EXCEPTION_FLT_INVALID_OPERATION: return "浮点数无效操作 (EXCEPTION_FLT_INVALID_OPERATION)";
        case EXCEPTION_FLT_OVERFLOW: return "浮点数溢出 (EXCEPTION_FLT_OVERFLOW)";
        case EXCEPTION_FLT_STACK_CHECK: return "浮点数栈检查失败 (EXCEPTION_FLT_STACK_CHECK)";
        case EXCEPTION_FLT_UNDERFLOW: return "浮点数下溢 (EXCEPTION_FLT_UNDERFLOW)";
        case EXCEPTION_ILLEGAL_INSTRUCTION: return "非法指令 (EXCEPTION_ILLEGAL_INSTRUCTION)";
        case EXCEPTION_IN_PAGE_ERROR: return "页面错误 (EXCEPTION_IN_PAGE_ERROR)";
        case EXCEPTION_INT_DIVIDE_BY_ZERO: return "整数除零 (EXCEPTION_INT_DIVIDE_BY_ZERO)";
        case EXCEPTION_INT_OVERFLOW: return "整数溢出 (EXCEPTION_INT_OVERFLOW)";
        case EXCEPTION_INVALID_DISPOSITION: return "无效的处置 (EXCEPTION_INVALID_DISPOSITION)";
        case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "不可继续的异常 (EXCEPTION_NONCONTINUABLE_EXCEPTION)";
        case EXCEPTION_PRIV_INSTRUCTION: return "特权指令 (EXCEPTION_PRIV_INSTRUCTION)";
        case EXCEPTION_SINGLE_STEP: return "单步执行 (EXCEPTION_SINGLE_STEP)";
        case EXCEPTION_STACK_OVERFLOW: return "栈溢出 (EXCEPTION_STACK_OVERFLOW)";
        default: return "未知异常";
    }
}

// 打印异常信息
void CrashHandler::PrintExceptionInfo(EXCEPTION_POINTERS* pExceptionInfo, FILE* fp)
{
    fprintf(fp, "========== 异常信息 ==========\n");
    fprintf(fp, "异常代码: 0x%08X\n", pExceptionInfo->ExceptionRecord->ExceptionCode);
    fprintf(fp, "异常描述: %s\n", GetExceptionCodeString(pExceptionInfo->ExceptionRecord->ExceptionCode));
    fprintf(fp, "异常地址: 0x%p\n", pExceptionInfo->ExceptionRecord->ExceptionAddress);
    fprintf(fp, "异常标志: 0x%08X\n", pExceptionInfo->ExceptionRecord->ExceptionFlags);
    fprintf(fp, "参数数量: %u\n", pExceptionInfo->ExceptionRecord->NumberParameters);
    
    // 打印异常参数
    for (DWORD i = 0; i < pExceptionInfo->ExceptionRecord->NumberParameters && i < EXCEPTION_MAXIMUM_PARAMETERS; i++)
    {
        fprintf(fp, "参数[%u]: 0x%p\n", i, (void*)pExceptionInfo->ExceptionRecord->ExceptionInformation[i]);
    }
    fprintf(fp, "\n");

    // 访问违规的特殊处理
    if (pExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
    {
        fprintf(fp, "访问违规详情:\n");
        fprintf(fp, "  操作类型: %s\n", 
                pExceptionInfo->ExceptionRecord->ExceptionInformation[0] == 0 ? "读取" :
                pExceptionInfo->ExceptionRecord->ExceptionInformation[0] == 1 ? "写入" : "执行");
        fprintf(fp, "  访问地址: 0x%p\n", (void*)pExceptionInfo->ExceptionRecord->ExceptionInformation[1]);
        fprintf(fp, "\n");
    }
}

// 打印调用栈
void CrashHandler::PrintStackTrace(EXCEPTION_POINTERS* pExceptionInfo, FILE* fp)
{
    HANDLE hProcess = GetCurrentProcess();
    HANDLE hThread = GetCurrentThread();
    
    // 初始化符号处理器
    SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME | SYMOPT_LOAD_LINES);
    SymInitialize(hProcess, NULL, TRUE);
    
    fprintf(fp, "========== 调用栈 ==========\n");
    
    CONTEXT context = *pExceptionInfo->ContextRecord;
    
    // 根据架构设置栈帧
    STACKFRAME64 stackFrame = {0};
#ifdef _WIN64
    stackFrame.AddrPC.Offset = context.Rip;
    stackFrame.AddrPC.Mode = AddrModeFlat;
    stackFrame.AddrFrame.Offset = context.Rbp;
    stackFrame.AddrFrame.Mode = AddrModeFlat;
    stackFrame.AddrStack.Offset = context.Rsp;
    stackFrame.AddrStack.Mode = AddrModeFlat;
#else
    stackFrame.AddrPC.Offset = context.Eip;
    stackFrame.AddrPC.Mode = AddrModeFlat;
    stackFrame.AddrFrame.Offset = context.Ebp;
    stackFrame.AddrFrame.Mode = AddrModeFlat;
    stackFrame.AddrStack.Offset = context.Esp;
    stackFrame.AddrStack.Mode = AddrModeFlat;
#endif
    
    // 获取模块信息
    IMAGEHLP_MODULE64 moduleInfo = {0};
    moduleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
    
    char symbolBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
    SYMBOL_INFO* pSymbol = (SYMBOL_INFO*)symbolBuffer;
    pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    pSymbol->MaxNameLen = MAX_SYM_NAME;
    
    IMAGEHLP_LINE64 lineInfo = {0};
    lineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
    
    DWORD displacement = 0;
    
    int frameCount = 0;
    while (StackWalk64(IMAGE_FILE_MACHINE_AMD64, hProcess, hThread, &stackFrame, 
                      &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL) && 
           frameCount < 100)
    {
        fprintf(fp, "[%2d] 0x%p", frameCount, (void*)stackFrame.AddrPC.Offset);
        
        // 获取符号信息
        if (SymFromAddr(hProcess, stackFrame.AddrPC.Offset, NULL, pSymbol))
        {
            fprintf(fp, " %s", pSymbol->Name);
        }
        
        // 获取行号信息
        if (SymGetLineFromAddr64(hProcess, stackFrame.AddrPC.Offset, &displacement, &lineInfo))
        {
            fprintf(fp, " (%s:%d)", lineInfo.FileName, lineInfo.LineNumber);
        }
        
        // 获取模块信息
        if (SymGetModuleInfo64(hProcess, stackFrame.AddrPC.Offset, &moduleInfo))
        {
            fprintf(fp, " [%s]", moduleInfo.ModuleName);
        }
        
        fprintf(fp, "\n");
        frameCount++;
    }
    
    if (frameCount == 0)
    {
        fprintf(fp, "无法获取调用栈信息\n");
    }
    
    fprintf(fp, "\n");
    
    // 清理符号处理器
    SymCleanup(hProcess);
}

// 生成MiniDump
void CrashHandler::CreateMiniDump(EXCEPTION_POINTERS* pExceptionInfo)
{
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "%Y%m%d_%H%M%S", t);
    
    char dumpPath[MAX_PATH];
    sprintf_s(dumpPath, "%scrash_%s.dmp", m_dumpPath.c_str(), timeStr);
    
    char logPath[MAX_PATH];
    sprintf_s(logPath, "%scrash_%s.log", m_dumpPath.c_str(), timeStr);
    
    // 创建MiniDump文件
    HANDLE hDumpFile = CreateFileA(dumpPath, GENERIC_WRITE, 0, NULL, 
                                   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hDumpFile != INVALID_HANDLE_VALUE)
    {
        MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
        dumpInfo.ExceptionPointers = pExceptionInfo;
        dumpInfo.ThreadId = GetCurrentThreadId();
        dumpInfo.ClientPointers = FALSE;
        
        MINIDUMP_TYPE dumpType = (MINIDUMP_TYPE)(
            MiniDumpWithFullMemory | 
            MiniDumpWithHandleData | 
            MiniDumpWithThreadInfo | 
            MiniDumpWithProcessThreadData |
            MiniDumpWithFullMemoryInfo |
            MiniDumpWithUnloadedModules);
        
        if (MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), 
                             hDumpFile, dumpType, &dumpInfo, NULL, NULL))
        {
            printf("崩溃转储已生成: %s\n", dumpPath);
        }
        else
        {
            printf("生成崩溃转储失败: %d\n", GetLastError());
        }
        
        CloseHandle(hDumpFile);
    }
    
    // 创建详细的崩溃日志文件
    FILE* fp = fopen(logPath, "w");
    if (fp)
    {
        fprintf(fp, "========================================\n");
        fprintf(fp, "      程序崩溃报告\n");
        fprintf(fp, "========================================\n\n");
        
        fprintf(fp, "崩溃时间: %04d-%02d-%02d %02d:%02d:%02d\n", 
                t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                t->tm_hour, t->tm_min, t->tm_sec);
        fprintf(fp, "进程ID: %u\n", GetCurrentProcessId());
        fprintf(fp, "线程ID: %u\n", GetCurrentThreadId());
        fprintf(fp, "转储文件: %s\n", dumpPath);
        fprintf(fp, "\n");
        
        // 打印异常信息
        PrintExceptionInfo(pExceptionInfo, fp);
        
        // 打印调用栈
        PrintStackTrace(pExceptionInfo, fp);
        
        // 打印额外的信息
        if (m_additionalInfoCallback)
        {
            const char* additionalInfo = m_additionalInfoCallback();
            if (additionalInfo)
            {
                fprintf(fp, "========== 额外信息 ==========\n");
                fprintf(fp, "%s\n", additionalInfo);
                fprintf(fp, "\n");
            }
        }
        
        // 打印内存信息
        fprintf(fp, "========== 内存信息 ==========\n");
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
        {
            fprintf(fp, "页面错误数: %lu\n", pmc.PageFaultCount);
            fprintf(fp, "工作集大小: %Iu KB\n", pmc.WorkingSetSize / 1024);
            fprintf(fp, "峰值工作集: %Iu KB\n", pmc.PeakWorkingSetSize / 1024);
            fprintf(fp, "分页池大小: %Iu KB\n", pmc.QuotaPagedPoolUsage / 1024);
            fprintf(fp, "非分页池: %Iu KB\n", pmc.QuotaNonPagedPoolUsage / 1024);
            fprintf(fp, "页面文件使用: %Iu KB\n", pmc.PagefileUsage / 1024);
            fprintf(fp, "峰值页面文件: %Iu KB\n", pmc.PeakPagefileUsage / 1024);
        }
        fprintf(fp, "\n");
        
        fclose(fp);
        printf("崩溃日志已生成: %s\n", logPath);
    }
}

// 未处理异常的回调函数
LONG WINAPI CrashHandler::UnhandledExceptionFilter(EXCEPTION_POINTERS* pExceptionInfo)
{
    printf("\n========== 程序崩溃 ==========\n");
    printf("捕获到未处理的异常, 正在生成崩溃报告...\n");
    
    // 生成MiniDump和详细日志
    CreateMiniDump(pExceptionInfo);
    
    printf("崩溃报告已生成, 程序即将退出.\n");
    printf("=============================\n\n");
    
    return EXCEPTION_EXECUTE_HANDLER;
}

// 初始化崩溃处理器
void CrashHandler::Initialize()
{
    if (m_initialized) return;
    SetUnhandledExceptionFilter(UnhandledExceptionFilter);
    m_initialized = true;
}

// 设置崩溃转储文件保存路径
void CrashHandler::SetDumpPath(const char* path)
{
    m_dumpPath = path;
    // 确保路径以反斜杠结尾
    if (!m_dumpPath.empty() && m_dumpPath.back() != '\\')
    {
        m_dumpPath += '\\';
    }
}

// 设置额外的崩溃信息回调
void CrashHandler::SetAdditionalInfoCallback(const char* (*callback)())
{
    m_additionalInfoCallback = callback;
}
