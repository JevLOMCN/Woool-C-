# StartServer - 服务器管理器

## 功能说明
StartServer 是一个用于管理 MirWorld 服务器组件的控制台程序。

## 支持的服务器
- DBServer.exe - 数据库服务器
- GameServer.exe - 游戏服务器
- LoginServer.exe - 登录服务器
- SelectCharServer.exe - 角色选择服务器
- ServerCenter.exe - 服务器中心

## 特性
1. **隐藏窗口** - 所有服务器进程都在后台运行，不会显示单独的控制台窗口
2. **输出重定向** - 所有服务器的输出信息会统一显示在 StartServer 的控制台中
3. **统一管理** - 可以单独启动/停止每个服务器，也可以一键启动/停止所有服务器
4. **状态监控** - 实时查看每个服务器的运行状态

## 使用方法

### 命令列表
- `start` - 启动所有服务器
- `stop` - 停止所有服务器
- `start <name>` - 启动指定服务器 (如: start DBServer)
- `stop <name>` - 停止指定服务器 (如: stop DBServer)
- `status` - 查看所有服务器状态
- `help` - 显示帮助信息
- `exit/quit` - 退出程序

### 启动顺序建议
建议按以下顺序启动：
1. DBServer
2. ServerCenter
3. LoginServer
4. SelectCharServer
5. GameServer

`start all` 命令会自动按此顺序启动所有服务器。

## 编译说明
使用 Visual Studio 2022 或更高版本编译。
配置：Debug/Release
平台：Win32/x64

编译后的可执行文件会输出到 Work 目录下。
