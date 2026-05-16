#pragma once
#include "MySQLProcess.h"
#include "ServerProcess.h"

// 服务管理类
class ServerManager
{
public:
    ServerManager();
    ~ServerManager();

    bool Initialize();
    bool StartAll();
    bool StartTest(const std::string& name);
    bool StopAll();
    void PrintStatus();
    bool StartServer(const std::string& name);
    bool StopServer(const std::string& name);
    bool SendCommand(const std::string& serverName, const std::string& command);

private:
    std::vector<ServerProcess*> m_servers;
    MySQLProcess* m_pMySQL;
    std::string m_workDir;
};
