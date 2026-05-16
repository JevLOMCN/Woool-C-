#include "stdafx.h"
#include "ServerProcess.h"

ServerProcess::ServerProcess(const std::string& name, const std::string& path)
    : m_name(name)
    , m_path(path)
    , m_hProcess(NULL)
    , m_hThread(NULL)
    , m_hOutput(NULL)
    , m_hInput(NULL)
    , m_dwProcessId(0)
    , m_bRunning(false)
    , m_bAutoRestart(true)
    , m_bManualStop(false)
    , m_restartCount(0)
{
}

ServerProcess::~ServerProcess()
{
    if (m_bRunning) Stop();
}

bool ServerProcess::Start()
{
    if (IsRunning())
    {
        printf("[%s] 已在运行中\n", m_name.c_str());
        return false;
    }

    m_bManualStop = false;

    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    HANDLE hReadPipe = NULL;
    HANDLE hWritePipe = NULL;
    HANDLE hReadInputPipe = NULL;
    HANDLE hWriteInputPipe = NULL;

    // 创建输出管道以读取子进程输出
    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0))
    {
        printf("[%s] 创建输出管道失败: %d\n", m_name.c_str(), GetLastError());
        return false;
    }

    // 创建输入管道以向子进程发送命令
    if (!CreatePipe(&hReadInputPipe, &hWriteInputPipe, &sa, 0))
    {
        printf("[%s] 创建输入管道失败: %d\n", m_name.c_str(), GetLastError());
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
        return false;
    }

    // 设置句柄继承属性
    SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hWritePipe, HANDLE_FLAG_INHERIT, 1);
    SetHandleInformation(hReadInputPipe, HANDLE_FLAG_INHERIT, 1);
    SetHandleInformation(hWriteInputPipe, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdError = hWritePipe;
    si.hStdOutput = hWritePipe;
    si.hStdInput = hReadInputPipe;
    si.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE; // 隐藏窗口

    ZeroMemory(&pi, sizeof(pi));

    // 添加 --noconsole 参数来禁用控制台窗口创建
    char cmdLine[MAX_PATH * 2 + 20];
    sprintf_s(cmdLine, sizeof(cmdLine), "\"%s\" --noconsole", m_path.c_str());

    printf("[%s] 正在启动...\n", m_name.c_str());

    // 获取当前工作目录
    char workDir[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, workDir);

    if (!CreateProcessA(NULL, cmdLine, NULL, NULL, TRUE, 
        0, NULL, workDir, &si, &pi))
    {
        DWORD error = GetLastError();
        printf("[%s] 启动失败: %d\n", m_name.c_str(), error);
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
        CloseHandle(hReadInputPipe);
        CloseHandle(hWriteInputPipe);
        return false;
    }

    m_hProcess = pi.hProcess;
    m_hThread = pi.hThread;
    m_dwProcessId = pi.dwProcessId;
    m_bRunning = true;
    m_lastRestartTime = std::chrono::steady_clock::now();

    // 保存管道句柄
    m_hOutput = hReadPipe;
    m_hInput = hWriteInputPipe;

    // 关闭子进程端的管道句柄
    CloseHandle(hWritePipe);
    CloseHandle(hReadInputPipe);

    // 创建读取输出线程
    unsigned int threadId = 0;
    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, ReadOutputThread, this, 0, &threadId);
    if (hThread)
        CloseHandle(hThread);
    else
        printf("[%s] 创建输出线程失败\n", m_name.c_str());

    // 创建监控线程
    HANDLE hMonitorThread = (HANDLE)_beginthreadex(NULL, 0, MonitorThread, this, 0, &threadId);
    if (hMonitorThread)
        CloseHandle(hMonitorThread);
    else
        printf("[%s] 创建监控线程失败\n", m_name.c_str());

    printf("[%s] 启动成功 (PID: %d)\n", m_name.c_str(), m_dwProcessId);
    return true;
}

bool ServerProcess::Stop()
{
    if (!m_bRunning)
    {
        printf("[%s] 未运行\n", m_name.c_str());
        return false;
    }

    m_bManualStop = true;
    printf("[%s] 正在停止...\n", m_name.c_str());

    // 强制终止进程...
    TerminateProcess(m_hProcess, 0);
    WaitForSingleObject(m_hProcess, 5000);

    CloseHandle(m_hProcess);
    CloseHandle(m_hThread);

    if (m_hOutput)
    {
        CloseHandle(m_hOutput);
        m_hOutput = NULL;
    }

    if (m_hInput)
    {
        CloseHandle(m_hInput);
        m_hInput = NULL;
    }

    m_hProcess = NULL;
    m_hThread = NULL;
    m_bRunning = false;

    printf("[%s] 已停止\n", m_name.c_str());
    return true;
}

bool ServerProcess::IsRunning() const
{
    if (!m_bRunning || !m_hProcess)
        return false;
    DWORD exitCode;
    if (GetExitCodeProcess(m_hProcess, &exitCode))
        return (exitCode == STILL_ACTIVE);
    return false;
}

bool ServerProcess::SendCommand(const std::string& command)
{
    if (!m_bRunning || !m_hInput)
    {
        printf("[%s] 未运行或输入管道无效\n", m_name.c_str());
        return false;
    }

    std::string fullCommand = command + "\n";
    DWORD bytesWritten;
    
    if (!WriteFile(m_hInput, fullCommand.c_str(), (DWORD)fullCommand.length(), &bytesWritten, NULL))
    {
        DWORD error = GetLastError();
        printf("[%s] 发送命令失败: %d\n", m_name.c_str(), error);
        return false;
    }

    printf("[%s] 已发送命令: %s\n", m_name.c_str(), command.c_str());
    return true;
}

unsigned __stdcall ServerProcess::ReadOutputThread(void* param)
{
    ServerProcess* pThis = (ServerProcess*)param;
    pThis->ReadOutput();
    return 0;
}

unsigned __stdcall ServerProcess::MonitorThread(void* param)
{
    ServerProcess* pThis = (ServerProcess*)param;
    pThis->MonitorProcess();
    return 0;
}

void ServerProcess::MonitorProcess()
{
    // 等待进程结束
    if (m_hProcess)
    {
        WaitForSingleObject(m_hProcess, INFINITE);
    }

    // 进程已结束, 检查是否需要自动重启
    if (!m_bManualStop && m_bAutoRestart && m_bRunning)
    {
        printf("[%s] 进程意外终止, 准备自动重启...\n", m_name.c_str());

        // 清理旧资源
        if (m_hProcess)
        {
            CloseHandle(m_hProcess);
            m_hProcess = NULL;
        }
        if (m_hThread)
        {
            CloseHandle(m_hThread);
            m_hThread = NULL;
        }
        if (m_hOutput)
        {
            CloseHandle(m_hOutput);
            m_hOutput = NULL;
        }
        if (m_hInput)
        {
            CloseHandle(m_hInput);
            m_hInput = NULL;
        }
        m_bRunning = false;

        // 检查重启频率, 防止频繁重启
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastRestartTime).count();
        
        // 如果5秒内连续重启超过3次, 则停止自动重启
        if (elapsed < 5 && m_restartCount >= 3)
        {
            printf("[%s] 重启过于频繁, 已停止自动重启保护\n", m_name.c_str());
            if (m_restartCallback)
            {
                m_restartCallback(this);
            }
            return;
        }

        // 等待3秒后重启
        Sleep(3000);

        m_restartCount++;
        printf("[%s] 正在执行第 %d 次自动重启...\n", m_name.c_str(), m_restartCount);

        // 重新启动
        if (Start())
        {
            printf("[%s] 自动重启成功\n", m_name.c_str());
        }
        else
        {
            printf("[%s] 自动重启失败\n", m_name.c_str());
            if (m_restartCallback)
            {
                m_restartCallback(this);
            }
        }
    }
    else
    {
        // 正常停止, 清理状态
        m_bRunning = false;
    }
}

void ServerProcess::ReadOutput()
{
    char buffer[8192]{};
    DWORD bytesRead;
    while (m_bRunning && m_hOutput)
    {
        if (!ReadFile(m_hOutput, buffer, sizeof(buffer) - 1, &bytesRead, NULL))
        {
            DWORD error = GetLastError();
            if (error != ERROR_BROKEN_PIPE)
            {
                // 管道读取错误, 但进程可能还在运行
            }
            break;
        }

        if (bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            
            // 立即刷新输出, 确保实时显示
            fflush(stdout);
            
            // 为每行输出添加服务器名称前缀
            char* line = buffer;
            char* nextLine;
            
            while (line < buffer + bytesRead)
            {
                nextLine = strchr(line, '\n');
                if (nextLine)
                {
                    *nextLine = '\0';
                    // 移除行尾的 \r
                    size_t lineLen = strlen(line);
                    if (lineLen > 0 && line[lineLen - 1] == '\r')
                    {
                        line[lineLen - 1] = '\0';
                    }
                    if (strlen(line) > 0)
                    {
                        printf("[%s] %s\n", m_name.c_str(), line);
                        fflush(stdout);
                    }
                    line = nextLine + 1;
                }
                else
                {
                    // 处理最后一行（没有换行符）
                    size_t lineLen = strlen(line);
                    if (lineLen > 0 && line[lineLen - 1] == '\r')
                    {
                        line[lineLen - 1] = '\0';
                    }
                    if (strlen(line) > 0)
                    {
                        printf("[%s] %s", m_name.c_str(), line);
                        fflush(stdout);
                    }
                    break;
                }
            }
        }
        else
        {
            // 没有数据, 稍微休眠一下
            Sleep(10);
        }
    }
}