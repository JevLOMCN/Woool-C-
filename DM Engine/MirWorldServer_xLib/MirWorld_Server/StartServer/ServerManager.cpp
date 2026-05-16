#include "stdafx.h"
#include "ServerManager.h"
#include "MySQLProcess.h"
#include "ServerProcess.h"
#include <wincrypt.h>
#pragma comment(lib, "advapi32.lib")

static const FileHashInfo FILE_HASH_TABLE[] = {
    {"mysqld.exe", ""},
    {"DBServer.exe", ""},
    {"GameServer.exe", ""},
    {"LoginServer.exe", ""},
    {"SelectCharServer.exe", ""},
    {"ServerCenter.exe", ""}
};

ServerManager::ServerManager() : m_pMySQL(NULL)
{
    // 工作目录
    m_workDir = ".\\Bin\\";
}

ServerManager::~ServerManager()
{
    StopAll();
    for (size_t i = 0; i < m_servers.size(); i++)
    {
        delete m_servers[i];
    }
    m_servers.clear();
    if (m_pMySQL)
    {
        delete m_pMySQL;
        m_pMySQL = NULL;
    }
}

bool ServerManager::Initialize()
{
    // 设置工作目录为 Work 目录
    SetCurrentDirectoryA(m_workDir.c_str());
    // 显示工作目录
    printf("工作目录: %s\n", m_workDir.c_str());
    // 检查可执行文件是否存在并验证哈希值
    printf("检查可执行文件...\n");
    int fileCount = sizeof(FILE_HASH_TABLE) / sizeof(FILE_HASH_TABLE[0]);
    bool allValid = true;
    for (int i = 0; i < fileCount; i++)
    {
        const char* filePath = FILE_HASH_TABLE[i].filePath;
        const char* expectedHash = FILE_HASH_TABLE[i].expectedHash;
        // 检查文件是否存在
        if (GetFileAttributesA(filePath) == INVALID_FILE_ATTRIBUTES)
        {
            printf("  [缺失] %s\n", filePath);
            allValid = false;
            continue;
        }
        // 计算并显示文件哈希值
        std::string fileHash = CalculateFileHash(filePath);
        if (fileHash.empty())
        {
            printf("  [错误] %s - 无法计算哈希值\n", filePath);
            allValid = false;
            continue;
        }
        printf("  [存在] %s\n", filePath);
        // 验证哈希值（如果配置了期望值）
        if (expectedHash && expectedHash[0] != '\0')
        {
            if (fileHash != expectedHash)
            {
                printf("         [警告] 哈希值不匹配!期望: %s\n", expectedHash);
                allValid = false;
            }
            else
                printf("         [验证通过]\n");
        }
        else
            printf("         MD5: %s\n", fileHash.c_str());
    }
    if (!allValid)
    {
        printf("可执行文件检查未通过, 不能正常使用\n");
        return false;
    }
    // 创建 MySQL 管理器
    m_pMySQL = new MySQLProcess();
    // 添加所有服务器
    m_servers.push_back(new ServerProcess("ServerCenter", ".\\ServerCenter.exe"));
    m_servers.push_back(new ServerProcess("DBServer", ".\\DBServer.exe"));
    m_servers.push_back(new ServerProcess("LoginServer", ".\\LoginServer.exe"));
    m_servers.push_back(new ServerProcess("SelectCharServer", ".\\SelectCharServer.exe"));
    m_servers.push_back(new ServerProcess("GameServer", ".\\GameServer.exe"));
    return true;
}

bool ServerManager::StartAll()
{
    printf("========== 启动所有服务器 ==========\n");
    if (m_pMySQL)
    {
        m_pMySQL->Start();
        Sleep(2000); // 等待 MySQL 完全启动
    }
    for (size_t i = 0; i < m_servers.size(); i++)
    {
        m_servers[i]->Start();
        Sleep(500); // 间隔500ms启动下一个
    }
    printf("========== 所有服务器启动完成 ==========\n\n");
    return true;
}

bool ServerManager::StartTest(const std::string& name)
{
    printf("========== 启动测试服务器 ==========\n");
    if (m_pMySQL)
    {
        m_pMySQL->Start();
        Sleep(2000); // 等待 MySQL 完全启动
    }
    for (size_t i = 0; i < m_servers.size(); i++)
    {
        if (_stricmp(m_servers[i]->GetName().c_str(), name.c_str()) == 0) // 排除指定服务不启动
            continue;
        m_servers[i]->Start();
        Sleep(500); // 间隔500ms启动下一个
    }
    printf("========== 测试服务器启动完成 ==========\n\n");
    return true;
}

bool ServerManager::StopAll()
{
    printf("========== 停止所有服务器 ==========\n");
    for (int i = (int)m_servers.size() - 1; i >= 0; i--)
    {
        m_servers[i]->Stop();
        Sleep(300);
    }
    if (m_pMySQL)
        m_pMySQL->Stop();
    printf("========== 所有服务器已停止 ==========\n\n");
    return true;
}

void ServerManager::PrintStatus()
{
    printf("\n========== 服务器状态 ==========\n");
    if (m_pMySQL)
    {
        const char* status = m_pMySQL->IsRunning() ? "[运行中]" : "[已停止]";
        printf("%-20s %s\n", "MySQL", status);
    }
    // 显示其他服务器状态
    for (size_t i = 0; i < m_servers.size(); i++)
    {
        const char* status = m_servers[i]->IsRunning() ? "[运行中]" : "[已停止]";
        printf("%-20s %s\n", m_servers[i]->GetName().c_str(), status);
    }
    printf("==============================\n\n");
}

bool ServerManager::StartServer(const std::string& name)
{
    if (_stricmp(name.c_str(), "mysql") == 0)
    {
        if (m_pMySQL)
            return m_pMySQL->Start();
        printf("MySQL 管理器未初始化\n");
        return false;
    }
    for (size_t i = 0; i < m_servers.size(); i++)
    {
        if (_stricmp(m_servers[i]->GetName().c_str(), name.c_str()) == 0)
            return m_servers[i]->Start();
    }
    printf("未找到服务器: %s\n", name.c_str());
    return false;
}

bool ServerManager::StopServer(const std::string& name)
{
    if (_stricmp(name.c_str(), "mysql") == 0)
    {
        if (m_pMySQL)
            return m_pMySQL->Stop();
        printf("MySQL 管理器未初始化\n");
        return false;
    }
    for (size_t i = 0; i < m_servers.size(); i++)
    {
        if (_stricmp(m_servers[i]->GetName().c_str(), name.c_str()) == 0)
            return m_servers[i]->Stop();
    }
    printf("未找到服务器: %s\n", name.c_str());
    return false;
}

bool ServerManager::SendCommand(const std::string& serverName, const std::string& command)
{
    if (_stricmp(serverName.c_str(), "mysql") == 0)
    {
        printf("MySQL 不支持通过终端发送命令\n");
        return false;
    }
    for (size_t i = 0; i < m_servers.size(); i++)
    {
        if (_stricmp(m_servers[i]->GetName().c_str(), serverName.c_str()) == 0)
            return m_servers[i]->SendCommand(command);
    }
    printf("未找到服务器: %s\n", serverName.c_str());
    return false;
}