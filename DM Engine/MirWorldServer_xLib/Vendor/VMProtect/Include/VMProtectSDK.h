#pragma once

#if defined(__APPLE__) || defined(__unix__)
#define VMP_IMPORT 
#define VMP_API
#define VMP_WCHAR unsigned short
#else
#define VMP_IMPORT __declspec(dllimport)
#define VMP_API __stdcall
#define VMP_WCHAR wchar_t
#ifdef _M_IX86
#pragma comment(lib, "VMProtectSDK32.lib")
#elif _M_X64 
#pragma comment(lib, "VMProtectSDK64.lib")
#elif _M_ARM64 
#pragma comment(lib, "VMProtectARM64.lib")
#else
#error "不支持的目标架构"
#endif
#endif // __APPLE__ || __unix__

#ifdef __cplusplus
extern "C" {
#endif
//代码标记

//受保护代码区域开头的标记
VMP_IMPORT void VMP_API VMProtectBegin(const char *);
//预定义 "虚拟化" 编译类型的开头标记
VMP_IMPORT void VMP_API VMProtectBeginVirtualization(const char *);
//预定义 "变异" 编译类型的开头标记
VMP_IMPORT void VMP_API VMProtectBeginMutation(const char *);
//预定义 "Ultra" 编译类型的开头标记
VMP_IMPORT void VMP_API VMProtectBeginUltra(const char *);
//预定义 "虚拟化" 并启用 "锁定到序列号" 的开头标记
VMP_IMPORT void VMP_API VMProtectBeginVirtualizationLockByKey(const char *);
//预定义 "Ultra" 并启用 "锁定到序列号" 的开头标记
VMP_IMPORT void VMP_API VMProtectBeginUltraLockByKey(const char *);
//受保护代码区域结尾的标记
VMP_IMPORT void VMP_API VMProtectEnd(void);

//服务函数

//如果文件已被 VMProtect 处理，则返回 True
VMP_IMPORT bool VMP_API VMProtectIsProtected();
//检测应用程序是否在调试器中运行
VMP_IMPORT bool VMP_API VMProtectIsDebuggerPresent(bool);
//检测应用程序是否在虚拟机环境中运行
VMP_IMPORT bool VMP_API VMProtectIsVirtualMachinePresent(void);
//检测可执行模块在进程内存中是否被修改
VMP_IMPORT bool VMP_API VMProtectIsValidImageCRC(void);
//解密 ANSI 字符串常量
VMP_IMPORT const char * VMP_API VMProtectDecryptStringA(const char *value);
//解密 Unicode 字符串常量
VMP_IMPORT const VMP_WCHAR * VMP_API VMProtectDecryptStringW(const VMP_WCHAR *value);
//释放为解密字符串分配的动态内存
VMP_IMPORT bool VMP_API VMProtectFreeString(const void *value);

// 密钥状态的位标志掩码
enum VMProtectSerialStateFlags
{
	SERIAL_STATE_SUCCESS				= 0, //成功
	SERIAL_STATE_FLAG_CORRUPTED			= 0x00000001, //授权系统已损坏。可能原因：保护项目设置不正确、破解尝试。
	SERIAL_STATE_FLAG_INVALID			= 0x00000002, //序列号不正确。授权系统无法解密序列号。
	SERIAL_STATE_FLAG_BLACKLISTED		= 0x00000004, //序列号与产品匹配，但在VMProtect 中被列入黑名单。
	SERIAL_STATE_FLAG_DATE_EXPIRED		= 0x00000008, //序列号已过期。
	SERIAL_STATE_FLAG_RUNNING_TIME_OVER	= 0x00000010, //程序运行时间已耗尽。
	SERIAL_STATE_FLAG_BAD_HWID			= 0x00000020, //硬件标识符与序列号中的不匹配。
	SERIAL_STATE_FLAG_MAX_BUILD_EXPIRED	= 0x00000040, //序列号与受保护程序的当前版本不匹配。
};

#pragma pack(push, 1)
//保护日期
typedef struct
{
	unsigned short	wYear; //年份
	unsigned char	bMonth; //月份
	unsigned char	bDay; //日期
} VMProtectDate;

//序列号详细数据
typedef struct
{
	int				nState;				// 密钥状态的位标志掩码
	VMP_WCHAR		wUserName[256];		// 用户名
	VMP_WCHAR		wEMail[256];		// 邮箱
	VMProtectDate	dtExpire;			// 密钥过期日期。
	VMProtectDate	dtMaxBuild;			// 密钥可用的最大产品构建日期。
	int				bRunningTime;		// 程序运行的最大分钟数。
	unsigned char	nUserDataLength;	// 用户数据的长度。
	unsigned char	bUserData[255];		// 放入密钥的用户数据。
} VMProtectSerialNumberData;
#pragma pack(pop)

//授权函数

//将序列号加载到授权系统
VMP_IMPORT int VMP_API VMProtectSetSerialNumber(const char *serial);
//返回序列号的状态标志
VMP_IMPORT int VMP_API VMProtectGetSerialNumberState();
// 获取序列号的详细信息
VMP_IMPORT bool VMP_API VMProtectGetSerialNumberData(VMProtectSerialNumberData *data, int size);
//获取当前计算机的硬件标识符
VMP_IMPORT int VMP_API VMProtectGetCurrentHWID(char *hwid, int size);

// 激活状态的位标志掩码
enum VMProtectActivationFlags
{
	ACTIVATION_OK = 0, //激活成功。序列号已放入 serial变量。
	ACTIVATION_SMALL_BUFFER, //缓冲区太小。
	ACTIVATION_NO_CONNECTION, //无法连接到 Web License Manager。
	ACTIVATION_BAD_REPLY, //激活服务器返回了意外结果。
	ACTIVATION_BANNED, //此激活码已被软件供应商在服务器上封禁。
	ACTIVATION_CORRUPTED, //激活模块自检系统出错，通常表示破解尝试。
	ACTIVATION_BAD_CODE, //指定的代码在激活服务器数据库中未找到。
	ACTIVATION_ALREADY_USED, //此代码的激活计数器已耗尽。
	ACTIVATION_SERIAL_UNKNOWN, //给定的序列号在服务器数据库中未找到。
	ACTIVATION_EXPIRED, //代码的激活期已过期。
	ACTIVATION_NOT_AVAILABLE //激活/停用不可用。
};

//激活函数

//将激活码发送到服务器并返回序列号
VMP_IMPORT int VMP_API VMProtectActivateLicense(const char *code, char *serial, int size);
//将序列号发送到服务器进行停用
VMP_IMPORT int VMP_API VMProtectDeactivateLicense(const char *serial);
//获取离线激活字符串
VMP_IMPORT int VMP_API VMProtectGetOfflineActivationString(const char *code, char *buf, int size);
//获取离线停用字符串
VMP_IMPORT int VMP_API VMProtectGetOfflineDeactivationString(const char *serial, char *buf, int size);

#ifdef __cplusplus
}
#endif
