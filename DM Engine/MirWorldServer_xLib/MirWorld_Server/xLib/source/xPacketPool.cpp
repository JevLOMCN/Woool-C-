#include "..\header\xPacket.h"

// 静态成员初始化
bool xPacketPool::s_pool_enabled = false; // 默认禁用内存池

// xPacketPool 方法实现 - 简化版本，直接使用标准分配
void* xPacketPool::alloc(size_t size) {
    // 直接使用标准分配
    return static_cast<void*>(new char[size]);
}

void xPacketPool::dealloc(void* ptr) {
    if (ptr) {
        delete[] static_cast<char*>(ptr);
    }
}

void xPacketPool::setPoolEnabled(bool enabled) {
    s_pool_enabled = enabled; // 设置但不启用，因为简化版本不支持
}

bool xPacketPool::isPoolEnabled() {
    return s_pool_enabled;
}