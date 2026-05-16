#pragma once
// ============================================================
// VMProtect SDK 封装
// 在需要保护的代码段中使用 VMP_PROTECT_BEGIN("name") 和 VMP_PROTECT_END()
// ============================================================
#ifndef USE_VMPROTECT
    #ifdef NDEBUG
        #define USE_VMPROTECT
    #endif
#endif

// 包含 VMProtect SDK 头文件
// 确保在项目属性中配置了正确的包含路径：$(ProjectDir)..\..\Vendor\VMProtect\Include
#include "VMProtectSDK.h"
#ifdef USE_VMPROTECT
    // 根据平台自动链接正确的库
    #ifdef _WIN64
        #pragma comment(lib, "VMProtectSDK64.lib")
    #else
        #pragma comment(lib, "VMProtectSDK32.lib")
    #endif
//代码标记

    //受保护代码区域开头的标记
    #define VMP_PROTECT_BEGIN(name)                          VMProtectBegin(name)
    //预定义 "Ultra" 编译类型的开头标记
    #define VMP_PROTECT_BEGIN_ULTRA(name)                    VMProtectBeginUltra(name)
    //预定义 "虚拟化" 编译类型的开头标记
    #define VMP_PROTECT_BEGIN_VIRTUALIZATION(name)           VMProtectBeginVirtualization(name)
    //预定义 "变异" 编译类型的开头标记
    #define VMP_PROTECT_BEGIN_MUTATION(name)                 VMProtectBeginMutation(name)
    //预定义 "虚拟化" 并启用 "锁定到序列号" 的开头标记
    #define VMP_PROTECT_BEGIN_VIRTUALIZATION_KEY(name)       VMProtectBeginVirtualizationLockByKey(name)
    //预定义 "Ultra" 并启用 "锁定到序列号" 的开头标记
    #define VMP_PROTECT_BEGIN_ULTRA_KEY(name)                VMProtectBeginUltraLockByKey(name)
    //受保护代码区域结尾的标记
    #define VMP_PROTECT_END()                                VMProtectEnd()

//服务函数

    //解密 ANSI 字符串常量
    #define VMP_DECRYPT_STRINGA(str)                         VMProtectDecryptStringA(str)
    //解密 Unicode 字符串常量
    #define VMP_DECRYPT_STRINGW(str)                         VMProtectDecryptStringW(str)
    // 释放为解密字符串分配的动态内存
    #define VMP_FREE_STRING(ptr)                             VMProtectFreeString(ptr)
    //检查是否已加壳
    #define VMP_IS_PROTECTED()                               VMProtectIsProtected()
    //检测应用程序是否在调试器中运行
    #define VMP_IS_DEBUGGER_PRESENT(CheckKernelMode)         VMProtectIsDebuggerPresent(CheckKernelMode)
    //检测应用程序是否在虚拟机环境中运行
    #define VMP_IS_VIRTUALMACHINE_PRESENT()                  VMProtectIsVirtualMachinePresent()
    //检测可执行模块在进程内存中是否被修改
    #define VMP_IS_VALID_IMAGECRC()                          VMProtectIsValidImageCRC()

//授权函数

    //将序列号加载到授权系统
    #define VMP_SET_SERIAL_NUMBER(serial)                    VMProtectSetSerialNumber(serial)
    //返回序列号的状态标志
    #define VMP_GET_SERIAL_NUMBER_STATE()                    VMProtectGetSerialNumberState()
    // 获取序列号的详细信息
    #define VMP_GET_SERIAL_NUMBER_DATA(data, size)           VMProtectGetSerialNumberData(data, size)
    //获取当前计算机的硬件标识符
    #define VMP_GET_CURRENT_HWID(buffer, size)               VMProtectGetCurrentHWID(buffer, size)

//激活函数

    //将激活码发送到服务器并返回序列号
    #define VMP_ACTIVATE_LICENSE(code, serial, size)         VMProtectActivateLicense(code, serial, size)
    //将序列号发送到服务器进行停用
    #define VMP_DEACTIVATE_LICENSE(serial)                   VMProtectDeactivateLicense(serial)
    //获取离线激活字符串
    #define VMP_OFFLINE_ACTIVATE_STR(code, buffer, size)     VMProtectGetOfflineActivationString(code, buffer, size)
    //获取离线停用字符串
    #define VMP_OFFLINE_DEACTIVATE_STR(serial, buffer, size) VMProtectGetOfflineDeactivationString(serial, buffer, size)
#else
    // 代码标记 - 空宏，编译时完全无开销
    #define VMP_PROTECT_BEGIN(name)                          ((void)0)
    #define VMP_PROTECT_BEGIN_ULTRA(name)                    ((void)0)
    #define VMP_PROTECT_BEGIN_VIRTUALIZATION(name)           ((void)0)
    #define VMP_PROTECT_BEGIN_MUTATION(name)                 ((void)0)
    #define VMP_PROTECT_BEGIN_VIRTUALIZATION_KEY(name)       ((void)0)
    #define VMP_PROTECT_BEGIN_ULTRA_KEY(name)                ((void)0)
    #define VMP_PROTECT_END()                                ((void)0)

    //服务函数 - 未启用时直接返回原字符串
    #define VMP_DECRYPT_STRINGA(str)                         (str)
    #define VMP_DECRYPT_STRINGW(str)                         (str)
    #define VMP_FREE_STRING(ptr)                             ((void)0)
    //服务函数 - 未启用时直接返回FALSE
    #define VMP_IS_PROTECTED()                               (0)
    #define VMP_IS_DEBUGGER_PRESENT()                        (0)
    #define VMP_IS_VIRTUALMACHINE_PRESENT()                  (0)
    #define VMP_IS_VALID_IMAGECRC()                          (0)

    //授权函数 - 未启用时返回错误
    #define VMP_SET_SERIAL_NUMBER(serial)                    (-1)
    #define VMP_GET_SERIAL_NUMBER_STATE()                    (-1)
    #define VMP_GET_SERIAL_NUMBER_DATA(data, size)           (0)
    #define VMP_GET_CURRENT_HWID(buffer, size)               (0)

    //激活函数 - 未启用时返回错误
    #define VMP_ACTIVATE_LICENSE(code, serial, size)         (-1)
    #define VMP_DEACTIVATE_LICENSE(serial)                   (-1)
    #define VMP_OFFLINE_ACTIVATE_STR(code, buffer, size)     (0)
    #define VMP_OFFLINE_DEACTIVATE_STR(serial, buffer, size) (0)

#endif
// 作用域保护宏 - 自动在作用域结束时调用 VMP_PROTECT_END
// 用法：
//     VMP_PROTECT_SCOPE("my_marker") {
//         // 受保护的代码...
//     }
// 注意：此宏使用 do-while(0) 技巧确保在任何上下文都能正确使用
#define VMP_PROTECT_SCOPE(name) \
    for (struct { int i; } _vmp_scope = { 0 }; _vmp_scope.i < 1; _vmp_scope.i++) \
        for (VMP_PROTECT_BEGIN(name); _vmp_scope.i < 1; VMP_PROTECT_END(), _vmp_scope.i++)

// 带返回值的作用域保护（用于函数返回）
// 用法：
//     int result;
//     VMP_PROTECT_SCOPE_RESULT("calc", result) {
//         result = SomeCalculation();
//     }
#define VMP_PROTECT_SCOPE_RESULT(name, result_var) \
    for (struct { int i; } _vmp_scope = { 0 }; _vmp_scope.i < 1; _vmp_scope.i++) \
        for (VMP_PROTECT_BEGIN(name); _vmp_scope.i < 1; VMP_PROTECT_END(), _vmp_scope.i++)
