#pragma once

// 处理MySQL服务的类
class MySQLProcess
{
public:
    MySQLProcess();
    ~MySQLProcess();

    bool Start();
    bool Stop();
    bool IsRunning() const;
    const std::string& GetName() const { return "MySQL"; }

private:
    bool IsMySQLRunning() const;
    bool StartMySQLService();
    bool StopMySQLService();
};
