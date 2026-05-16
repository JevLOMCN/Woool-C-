#pragma once

#ifdef _WIN64
#define WIN64_LEAN_AND_MEAN
#else
#define WIN32_LEAN_AND_MEAN
#endif
// Windows 头文件:
#include <windows.h>
// C 运行时头文件
#include <stdio.h>
#include <string>
#include <vector>
#include <process.h>
#include <conio.h>
#include <iostream>
#include <chrono>
#include <functional>
// TODO: 在此处引用程序要求的附加头文件
#include "..\res\resource.h"
#include <wincrypt.h>
#include <mimalloc.h>

// 初始化 mimalloc, 替换系统内存分配器
struct MimallocInitializer {
    MimallocInitializer()
    {
        mi_stats_reset();
        // 基本配置 - 适用于管理器程序
        mi_option_set(mi_option_e::mi_option_purge_delay, 5000);
        mi_option_set(mi_option_e::mi_option_purge_decommits, 1);
        mi_option_set(mi_option_e::mi_option_page_reclaim_on_free, 1);
        mi_option_set(mi_option_e::mi_option_arena_reserve, 16 * 1024);
        mi_option_set(mi_option_e::mi_option_use_numa_nodes, 0);
        mi_option_set(mi_option_e::mi_option_allow_large_os_pages, 0);
        mi_option_set(mi_option_e::mi_option_show_errors, 0);
        mi_option_set(mi_option_e::mi_option_show_stats, 0);
        mi_option_set(mi_option_e::mi_option_verbose, 0);
    }
} g_mimalloc_init;

// 计算文件 MD5 哈希值
static std::string CalculateFileHash(const char* filePath)
{
    std::string hashResult;
    HANDLE hFile = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return hashResult;

    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    BYTE buffer[4096];
    DWORD bytesRead;
    BYTE hash[16];
    DWORD hashLen = 16;

    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
        goto cleanup;

    if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
        goto cleanup;

    while (ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0)
    {
        if (!CryptHashData(hHash, buffer, bytesRead, 0))
            goto cleanup;
    }

    if (!CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0))
        goto cleanup;

    // 转换为十六进制字符串
    char hex[33];
    for (int i = 0; i < 16; i++)
        sprintf(hex + i * 2, "%02x", hash[i]);
    hex[32] = '\0';
    hashResult = hex;

cleanup:
    if (hHash) CryptDestroyHash(hHash);
    if (hProv) CryptReleaseContext(hProv, 0);
    CloseHandle(hFile);
    return hashResult;
}

// 预定义的可执行文件哈希值（你需要用实际值替换这些）
struct FileHashInfo {
    const char* filePath;
    const char* expectedHash;  // 设为 "" 表示不验证哈希, 只检查存在性
};