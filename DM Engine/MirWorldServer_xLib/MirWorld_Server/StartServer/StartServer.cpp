#include "stdafx.h"
#include "ServerManager.h"

static void PrintHelp()
{
    printf("\n================================ 服务器管理器 ================================\n");
    printf("命令列表:\n");
    printf("  start [name]      - 启动所有服务器或指定服务器\n");
    printf("  stop [name]       - 停止所有服务器或指定服务器\n");
    printf("  restart [name]    - 重启所有服务器或指定服务器\n");
    printf("  test [name]       - 启动除了指定服务外的其他服务\n");
    printf("  send <name> <cmd> - 向指定服务器发送命令\n");
    printf("  status            - 查看服务器状态\n");
    printf("  help              - 显示帮助信息\n");
    printf("  exit/quit         - 退出程序\n");
    printf("==============================================================================\n");
    printf("可用服务:\n");
    printf("  - MySQL\n");
    printf("  - DBServer\n");
    printf("  - GameServer\n");
    printf("  - LoginServer\n");
    printf("  - SelectCharServer\n");
    printf("  - ServerCenter\n");
    printf("==============================================================================\n\n");
}

int main()
{
    setlocale(LC_ALL, ".936");
    // 重定向标准输入输出
    FILE* fp;
    freopen_s(&fp, "CONIN$", "r", stdin);
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    // 获取控制台句柄
    HANDLE m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (m_hConsole == INVALID_HANDLE_VALUE)return FALSE;
    // 设置默认颜色为白字黑底, 确保输入文字可见
    SetConsoleTextAttribute(m_hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    // 设置控制台标题
    SetConsoleTitleA("龙陆引擎 - 管理器");
    // 设置窗口大小（单位：字符）
    // 先设置窗口大小, 再设置缓冲区大小（缓冲区必须 >= 窗口）
    SMALL_RECT windowSize{};
    windowSize.Left = 0;
    windowSize.Top = 0;
    windowSize.Right = 79;   // 宽度-1（80列）
    windowSize.Bottom = 29;  // 高度-1（30行）
    SetConsoleWindowInfo(m_hConsole, TRUE, &windowSize);
    // 设置缓冲区大小（必须 >= 窗口大小）
    COORD coord{};
    coord.X = 80;    // 缓冲区宽度（与窗口一致或更大）
    coord.Y = 500;  // 缓冲区高度（用于滚动历史）
    SetConsoleScreenBufferSize(m_hConsole, coord);
    // 设置控制台字体大小（需要 Windows Vista 或更高版本）
    CONSOLE_FONT_INFOEX cfi{};
    cfi.cbSize = sizeof(cfi);
    cfi.nFont = 0;
    cfi.dwFontSize.X = 0;      // 宽度自动
    cfi.dwFontSize.Y = 12;     // 高度（字号）
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_NORMAL;
    wcscpy_s(cfi.FaceName, L"新宋体");  // 字体名称
    SetCurrentConsoleFontEx(m_hConsole, FALSE, &cfi);
    // 使用系统默认代码页（GBK）以支持中文显示
    SetConsoleOutputCP(GetACP());
    SetConsoleCP(GetACP());
    // 禁用标准输出缓冲, 确保实时显示
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    printf("\n");
    printf("******************************************************************************\n");
    printf("*                              龙陆引擎 管理器                               *\n");
    printf("*                                版本 Ver1.00                                *\n");
    printf("******************************************************************************\n\n");

    ServerManager manager;
    char command[512];
    bool bRunning = true;
    if (!manager.Initialize())
    {
        bRunning = false;
        printf("按任意键退出...");
        _getch();
        return 0;
    }

    PrintHelp();

    while (bRunning)
    {
        printf("> ");
        if (fgets(command, sizeof(command), stdin) == NULL)
            break;
        // 移除换行符
        size_t len = strlen(command);
        if (len > 0 && command[len - 1] == '\n')
            command[len - 1] = '\0';
        // 跳过空命令
        if (strlen(command) == 0)
            continue;
        // 解析命令
        char cmd[64] = { 0 };
        char param1[128] = { 0 };
        char param2[256] = { 0 };
        int count = sscanf(command, "%63s %127s %255[^\n]", cmd, param1, param2);

        if (_stricmp(cmd, "start") == 0)
        {
            if (count >= 2 && strlen(param1) > 0) // 启动指定服务器
                manager.StartServer(param1);
            else
                manager.StartAll(); // 启动所有服务器
        }
        else if (_stricmp(cmd, "stop") == 0)
        {
            if (count >= 2 && strlen(param1) > 0)
                manager.StopServer(param1); // 停止指定服务器
            else
                manager.StopAll(); // 停止所有服务器
        }
        else if (_stricmp(cmd, "restart") == 0)
        {
            if (count >= 2 && strlen(param1) > 0)
            {
                if (manager.StopServer(param1))
                    manager.StartServer(param1);
            }
            else
            {
                if (manager.StopAll())
                    manager.StartAll();
            }
        }
        else if (_stricmp(cmd, "test") == 0)
        {
            if (count >= 2 && strlen(param1) > 0)
                manager.StartTest(param1); // 启动除了param1服务不启动的其他服务器
            else
                manager.StartAll(); // 启动所有服务器
        }
        else if (_stricmp(cmd, "send") == 0)
        {
            if (count >= 3 && strlen(param1) > 0 && strlen(param2) > 0)
                manager.SendCommand(param1, param2); // 向指定服务器发送命令
            else
            {
                printf("用法: send <server> <command>\n");
                printf("示例: send DBServer ReloadBanList\n");
                printf("示例: send GameServer ReloadTrustList\n");
            }
        }
        else if (_stricmp(cmd, "status") == 0)
            manager.PrintStatus();
        else if (_stricmp(cmd, "help") == 0 || strcmp(cmd, "?") == 0)
            PrintHelp();
        else if (_stricmp(cmd, "exit") == 0 || _stricmp(cmd, "quit") == 0)
        {
            printf("\n正在退出...\n");
            bRunning = false;
        }
        else
            printf("未知命令: %s (输入 'help' 查看帮助)\n", cmd);
    }
    // 确保所有服务器都已停止
    manager.StopAll();
    printf("按任意键退出...");
    _getch();
    return 0;
}