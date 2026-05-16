#include "StdAfx.h"
#include "mimalloc-new-delete.h"
#include "serverform.h"
#include <iostream>
#include <conio.h>
#include <time.h>
#include "EventBus.h"

// 初始化 mimalloc, 替换系统内存分配器 - 基于 mimalloc v3.2.8 官方文档和最佳实践
struct MimallocInitializer {
	MimallocInitializer()
	{
		// 重置统计信息
		mi_stats_reset();

		// ============================================
		// 核心性能选项
		// ============================================

		// Arena 预提交内存：2=仅在 overcommit 系统上启用, 0=禁用, 1=总是启用
		// Windows 默认使用 2, 可以提升性能但会增加内存使用
		// MMORPG 服务端建议使用 2, 减少 VirtualAlloc 调用次数
		mi_option_set(mi_option_e::mi_option_arena_eager_commit, 2);

		// ============================================
		// 内存回收选项
		// ============================================

		// 内存回收延迟：延迟 N 毫秒后再回收内存
		// 0=立即回收, -1=不回收
		// MMORPG 服务端设置为 10000ms（10秒）, 玩家进出频繁, 避免频繁回收导致的性能抖动
		// 配合 purge_decommits=1, 10秒后真正释放内存, 平衡性能和内存使用
		mi_option_set(mi_option_e::mi_option_purge_delay, 10000);

		// 回收时是否解提交内存：1=解提交（释放给OS）, 0=重置内存但保留地址空间
		// 设置为 1 可以真正释放内存, 适合 7×24 小时长时间运行的服务器
		// 配合 purge_delay 延迟回收, 避免频繁系统调用
		mi_option_set(mi_option_e::mi_option_purge_decommits, 1);

		// 在 free 时回收 abandoned 页面：0=不允许, 1=允许（仅当页面来自当前 theap）, 其他=总是允许
		// MMORPG 多线程环境, 玩家对象可能在不同线程分配释放
		// 设置为 1 及时回收其他线程释放的内存, 减少跨线程内存占用
		mi_option_set(mi_option_e::mi_option_page_reclaim_on_free, 1);

		// ============================================
		// 内存保留选项
		// ============================================

		// Arena 初始保留大小（单位：KB）
		// 根据实际内存需求配置：
		// 每个进程独立 Arena, 避免相互影响
		mi_option_set(mi_option_e::mi_option_arena_reserve, 16 * 1024);

		// ============================================
		// NUMA 选项
		// ============================================

		// 使用 NUMA 节点：0=使用所有可用节点, N=最多使用 N 个节点
		// 分布式 MMORPG 服务端, 单机多进程架构, 使用所有 NUMA 节点最大化性能
		// 避免单个 NUMA 节点过载, 充分利用多核 CPU 资源
		mi_option_set(mi_option_e::mi_option_use_numa_nodes, 0);

		// ============================================
		// 大页选项
		// ============================================

		// 允许使用大页（2MB 或 4MB）：0=禁用, 1=启用
		// 大页可以提升 TLB 命中率, 但会增加内存粒度
		// MMORPG 服务端对象大小不固定, 建议禁用（0）, 避免内存浪费和碎片
		mi_option_set(mi_option_e::mi_option_allow_large_os_pages, 0);

		// 预留巨大页（1GB）：N=预留 N 个 1GB 页面
		// MMORPG 服务端一般不需要, 设置为 0
		// 巨大页适合大数据处理场景, 不适合游戏服务端的频繁小对象分配
		mi_option_set(mi_option_e::mi_option_reserve_huge_os_pages, 0);

		// 允许透明大页（THP）：0=禁用, 1=启用（Linux 特有功能）
		// Windows 不支持 THP, 此选项无效, 设置为 0 保持一致性
		// 避免让人误以为启用了某种大页功能
		mi_option_set(mi_option_e::mi_option_allow_thp, 0);

		// ============================================
		// 页面管理选项
		// ============================================

		// 保留的满页数：每个 size class 保留 N 个满页（默认=2）
		// MMORPG 服务端大量玩家对象、怪物、物品频繁创建销毁
		// 提高到 6, 减少页面分配/释放开销, 提升分配性能
		// 内存影响：约 50 size class × 8 × 4KB = 1.6MB, 可忽略不计
		mi_option_set(mi_option_e::mi_option_page_full_retain, 8);

		// 最大候选页面数：分配时考虑的最大候选页面数（默认=4）
		// MMORPG 服务端对象大小相对稳定（玩家、物品等）
		// 保持 4 即可, 在性能和搜索开销间取得最佳平衡
		mi_option_set(mi_option_e::mi_option_page_max_candidates, 4);

		// 最小回收大小：设置最小回收大小（单位：KB）
		// 0=无限制, 允许回收任意大小的页面
		// MMORPG 服务端有大量小对象（装备、道具、技能效果）, 设置为 0 及时释放内存
		// 配合 purge_delay=10000ms, 避免频繁系统调用, 防止碎片积累
		mi_option_set(mi_option_e::mi_option_minimal_purge_size, 0);

		// ============================================
		// 高级选项
		// ============================================

		// Arena 最大对象大小：Arena 中可分配的最大对象大小（单位：KB）
		// 64位系统默认 2GB, MMORPG 服务端保持默认即可
		// 足够处理大地图数据、玩家数据库等大对象
		mi_option_set(mi_option_e::mi_option_arena_max_object_size, 2 * 1024 * 1024);  // 2GB

		// 允许访问 abandoned 的 heap 块：0=不允许, 1=允许
		// 生产环境设置为 0, 提升安全性和性能
		// 用于调试时可以启用 1, 检测内存访问问题
		mi_option_set(mi_option_e::mi_option_visit_abandoned, 0);

		// 通用回收频率：每 N 次通用分配调用回收一次 heap（默认=10000）
		// MMORPG 服务端设置为 5000, 提高回收频率, 减少内存碎片
		// 高频分配场景下, 更频繁的回收可以保持内存健康状态
		mi_option_set(mi_option_e::mi_option_generic_collect, 5000);

		// ============================================
		// 调试选项（生产环境建议关闭）
		// ============================================

		// 显示错误信息：0=不显示
		mi_option_set(mi_option_e::mi_option_show_errors, 0);

		// 显示统计信息：0=不显示（生产环境）
		// 开发调试时可临时启用, 观察内存使用情况
		mi_option_set(mi_option_e::mi_option_show_stats, 0);

		// 详细输出：0=关闭
		mi_option_set(mi_option_e::mi_option_verbose, 0);

		// ============================================
		// 安全选项
		// ============================================

		// 内存重试时间（毫秒）：内存不足时重试的时间, 仅 Windows
		// MMORPG 服务端实时性要求高, 设置为 0 禁用重试
		// 避免内存分配失败时阻塞线程导致游戏卡顿, 快速失败并重启更安全
		// 配合外部监控系统（如进程守护）, 自动重启崩溃的服务器
		mi_option_set(mi_option_e::mi_option_retry_on_oom, 0);
	}
} g_mimalloc_init;

CServerForm* CServerForm::s_pInstance = nullptr;
CServerForm::CServerForm(void)
{
	m_pServer = nullptr;
	m_pCmdLine = nullptr;
	m_pszServerName = nullptr;
	m_hConsole = nullptr;
	m_wOriginalAttributes = 0;
	m_bRunning = FALSE;
	m_bStatus = FALSE;
	s_pInstance = this;
}

// 设置 Arena 预留内存大小（单位：KB）
// 需要在 Create() 之后、StartServer() 之前调用
void CServerForm::SetArenaReserve(size_t kbSize)
{
	mi_option_set(mi_option_e::mi_option_arena_reserve, kbSize);
}

CServerForm::~CServerForm(void)
{
	if (m_hConsole)
		SetConsoleTextAttribute(m_hConsole, m_wOriginalAttributes);
	s_pInstance = nullptr;
}

BOOL CServerForm::Create(const char* pszTitle, const char* pszCmdLine)
{
	TRY_BEGIN
	m_pszServerName = (char*)pszTitle;
	m_pCmdLine = (char*)pszCmdLine;
	bool bNoConsole = (pszCmdLine && strstr(pszCmdLine, "--noconsole") != nullptr);
	if (!bNoConsole)
	{
		if (!AllocConsole())
			AttachConsole(ATTACH_PARENT_PROCESS);
		FILE* fp;
		freopen_s(&fp, "CONIN$", "r", stdin);
		freopen_s(&fp, "CONOUT$", "w", stdout);
		freopen_s(&fp, "CONOUT$", "w", stderr);
	}
	else
	{
		FILE* fp;
		freopen_s(&fp, "CONOUT$", "w", stdout);
		freopen_s(&fp, "CONOUT$", "w", stderr);
	}
	// 获取控制台句柄
	m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (m_hConsole == INVALID_HANDLE_VALUE)return FALSE;
	// 设置为白字黑底作为默认颜色
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (GetConsoleScreenBufferInfo(m_hConsole, &csbi))
		m_wOriginalAttributes = csbi.wAttributes;
	// 设置默认颜色为白字黑底, 确保输入文字可见
	SetConsoleTextAttribute(m_hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	// 设置控制台标题（仅在非静默模式下）
	if (!bNoConsole)
	{
		SetConsoleTitleA(pszTitle);
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
		coord.Y = 800;  // 缓冲区高度（用于滚动历史）
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
	}
	// 使用系统默认代码页（GBK）以支持中文显示
	// 注意：确保源代码文件保存为 GBK 编码
	SetConsoleOutputCP(GetACP());
	SetConsoleCP(GetACP());
	// 注册控制台控制处理程序（仅在非静默模式下）
	if (!bNoConsole)
	{
		SetConsoleCtrlHandler(ConsoleHandler, TRUE);
	}

	// 切换到父目录作为工作目录
	SetCurrentDirectoryA("..\\");
	
	return TRUE;
	TRY_END
}

VOID CServerForm::OnCommand(UINT id)
{
	switch (id)
	{
	case 10000: // IDM_STARTSERVER
	{
		if (!StartServer())
			OutPutStatic(GetColor(ERROR_RED), "启动服务器失败!\n");
		else
			OutPutStatic(GetColor(SUCCESS_GREEN), "服务器启动成功!\n");
	}
	break;
	case 10001: // IDM_STOPSERVER
	{
		if (!StopServer())
			OutPutStatic(GetColor(ERROR_RED), "停止服务器失败!\n");
		else
			OutPutStatic(GetColor(SUCCESS_GREEN), "服务器已停止!\n");
	}
	break;
	}
}

BOOL CServerForm::StartServer()
{
	if (m_pServer != nullptr)
	{
		CSettingFile sf;
		if (!sf.Open("Config.ini"))
		{
			OutPutStatic(GetColor(ERROR_RED), "启动服务器失败：找不到配置文件!\n");
			return FALSE;
		}
		return m_pServer->Start(sf);
	}
	OutPutStatic(GetColor(ERROR_RED), "启动服务器失败：服务器对象为空!\n");
	return FALSE;
}

BOOL CServerForm::StopServer()
{
	if (m_pServer != nullptr)
		return m_pServer->Stop();
	return TRUE;
}

BOOL CServerForm::OnClose()
{
	if (m_pServer)
	{
		m_pServer->Stop();
	}
	m_bRunning = FALSE;
	return TRUE;
}

BOOL WINAPI CServerForm::ConsoleHandler(DWORD dwCtrlType)
{
	if (s_pInstance)
	{
		switch (dwCtrlType)
		{
		case CTRL_C_EVENT:
		case CTRL_BREAK_EVENT:
		case CTRL_CLOSE_EVENT:
		case CTRL_LOGOFF_EVENT:
		case CTRL_SHUTDOWN_EVENT:
			return s_pInstance->OnClose() ? TRUE : FALSE;
		}
	}
	return FALSE;
}

VOID CServerForm::OnTimer()
{
	SERVERSTATE serverstate;
	if (m_pServer != nullptr)
	{
		m_pServer->GetServerState(serverstate);
	}
	
	// 设置控制台标题显示状态
	char title[256];
	sprintf(title, "%s - %s | 连接:%d 发送:%u 接收:%u", 
		m_pszServerName, serverstate.pServerDescript,
		serverstate.numConnection, serverstate.dwSendBytes, serverstate.dwRecvBytes);
	SetConsoleTitleA(title);
}

WORD CServerForm::GetConsoleColor(DWORD dwColor)
{
	// 将RGB颜色映射到控制台颜色
	// CIOConsole::GetColor() 返回的是RGB值, 所以这里直接比较RGB值
	switch (dwColor)
	{
	case 0xF8F8F2:        // TEXT_WHITE - 柔白色
		return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
	case 0x88C0D0:        // COOL_BLUE - 静谧蓝
		return FOREGROUND_BLUE | FOREGROUND_INTENSITY;
	case 0x98C379:        // STRING_GREEN - 活力绿
		return FOREGROUND_GREEN | FOREGROUND_INTENSITY;
	case 0xE5C07B:        // WARN_YELLOW - 明艳黄
		return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
	case 0xFF6188:        // KEYWORD_PINK - 珊瑚橙/粉红
		return FOREGROUND_RED | FOREGROUND_INTENSITY;
	case 0xBD93F9:        // FUNC_PURPLE - 淡雅紫
		return FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
	case 0x05D9E8:        // CYAN - 荧光青
		return FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
	case 0xD19A66:        // ORANGE - 低饱和橙
		return FOREGROUND_RED | FOREGROUND_GREEN;
	case 0xFF00:          // SUCCESS_GREEN - 鲜绿色
		return FOREGROUND_GREEN | FOREGROUND_INTENSITY;
	case 0xFF:            // ERROR_RED - 纯红色
		return FOREGROUND_RED | FOREGROUND_INTENSITY;
	default:
		// 处理直接的RGB值（如 0xFF0000 格式）
		if (dwColor == 0xFF)           // 纯红色
			return FOREGROUND_RED | FOREGROUND_INTENSITY;
		if (dwColor == 0xFF00)         // 纯绿色
			return FOREGROUND_GREEN | FOREGROUND_INTENSITY;
		if (dwColor == 0xFF0000)       // 纯蓝色
			return FOREGROUND_BLUE | FOREGROUND_INTENSITY;
		// 默认白色
		return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
	}
}

void CServerForm::SetConsoleColor(WORD color) const
{
	if (m_hConsole)
		SetConsoleTextAttribute(m_hConsole, color);
}

VOID CServerForm::OutPutStatic(DWORD dwColor, const char* pszString)
{
	WORD color = GetConsoleColor(dwColor);
	SetConsoleColor(color);
	printf("%s", pszString);
	SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

int CServerForm::EnterMessageLoop()
{
	// 启动服务器
	OnCommand(10000); // IDM_STARTSERVER
	m_bRunning = TRUE;
	HANDLE hTimerThread = nullptr;
	if (m_bStatus) // 是否开启 启动定时器线程来更新状态
	{
		DWORD dwTimerThreadId = 0;
		hTimerThread = CreateThread(nullptr, 0, [](LPVOID lpParam) -> DWORD {
			CServerForm* pForm = (CServerForm*)lpParam;
			while (pForm->m_bRunning)
			{
				pForm->OnTimer();
				Sleep(1000);
			}
			return 0;
			}, this, 0, &dwTimerThreadId);
	}
	// 主循环：读取用户输入（支持键盘和管道输入）
	char szInput[1024];
	HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
	// 检查是否是管道输入（StartServer 启动时）
	bool bIsPipeInput = false;
	if (hStdIn != INVALID_HANDLE_VALUE)
	{
		DWORD dwMode = 0;
		// 如果 GetConsoleMode 失败, 说明不是控制台而是管道
		bIsPipeInput = !GetConsoleMode(hStdIn, &dwMode);
	}
	m_tmrCleanup.Savetime();
	while (m_bRunning)
	{
		// 每 10 分钟执行一次
		if (m_tmrCleanup.IsTimeOut(10 * 60 * 1000))
		{
			EventBus::getInstance().cleanupInvalidHandlers();
			m_tmrCleanup.Savetime();
		}

		bool bHasInput = false;
		// 检查键盘输入（本地运行时使用）
		if (!bIsPipeInput && _kbhit())
		{
			bHasInput = true;
		}
		// 检查管道输入（StartServer 发送命令时使用）
		else if (bIsPipeInput && hStdIn != INVALID_HANDLE_VALUE)
		{
			DWORD dwAvail = 0;
			if (PeekNamedPipe(hStdIn, NULL, 0, NULL, &dwAvail, NULL) && dwAvail > 0)
			{
				bHasInput = true;
			}
		}
		if (bHasInput)
		{
			if (fgets(szInput, sizeof(szInput), stdin) != nullptr)
			{
				// 去掉换行符
				size_t len = strlen(szInput);
				if (len > 0 && (szInput[len - 1] == '\n' || szInput[len - 1] == '\r'))
				{
					szInput[len - 1] = '\0';
					if (len > 1 && szInput[len - 2] == '\r')
						szInput[len - 2] = '\0';
				}
				// 处理命令
				if (_stricmp(szInput, "start") == 0)
					OnCommand(10000);
				else if (_stricmp(szInput, "stop") == 0)
					OnCommand(10001);
				else if (_stricmp(szInput, "quit") == 0 || _stricmp(szInput, "exit") == 0)
				{
					if (OnClose())break;
				}
				else if (strlen(szInput) > 0)
				{
					// 将输入字符串传递给输入监听器
					if (m_pInputListener)
						m_pInputListener->OnInput(szInput);
				}
			}
		}
		Sleep(10);
	}
	// 等待定时器线程结束
	if (hTimerThread)
	{
		WaitForSingleObject(hTimerThread, 1000);
		CloseHandle(hTimerThread);
	}
	FreeConsole();
	return 0;
}