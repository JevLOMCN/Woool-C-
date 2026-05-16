#pragma once
#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>
#include <shared_mutex>
#include <atomic>
#include <typeindex>
#include <cstdint>

// SIMD 指令集支持
#if defined(__x86_64__) || defined(_M_X64)
#include <immintrin.h>
#define ENABLE_SIMD 1
#endif

/**
 * @brief 订阅令牌, 用于精确取消订阅
 */
using SubscriptionToken = uint64_t;

/**
 * @brief 事件基类
 * @note 所有自定义事件类型必须继承此类
 */
class BaseEvent
{
public:
    virtual ~BaseEvent() = default;
};

/**
 * @brief 事件处理器接口
 */
class EventHandler
{
public:
    virtual ~EventHandler() = default;
    virtual void handle(const BaseEvent& event) = 0;
    virtual SubscriptionToken getToken() const = 0;
    virtual bool isValid() const = 0;
    virtual void invalidate() = 0;
};

/**
 * @brief 事件处理器
 */
template<typename EventType>
class FastEventHandler : public EventHandler
{
public:
    using HandlerFuncPtr = void(*)(const EventType&);
    using HandlerFuncObj = std::function<void(const EventType&)>;

    // 函数指针构造函数（零开销）
    FastEventHandler(HandlerFuncPtr func, SubscriptionToken token) noexcept : funcPtr_(func), token_(token), valid_(true), useFuncPtr_(true) {}

    // 函数对象构造函数（支持 lambda、std::function 等）
    FastEventHandler(HandlerFuncObj func, SubscriptionToken token) : funcObj_(std::move(func)), token_(token), valid_(true), useFuncPtr_(false) {}

    // 通用模板构造函数
    template<typename Func>
    FastEventHandler(Func&& func, SubscriptionToken token) : funcObj_(std::forward<Func>(func)), token_(token), valid_(true), useFuncPtr_(false) {}

    void handle(const BaseEvent& event) noexcept override
    {
        const EventType& typedEvent = static_cast<const EventType&>(event);
        invoke(typedEvent);
    }
    SubscriptionToken getToken() const noexcept override { return token_; }
    bool isValid() const noexcept override { return valid_.load(std::memory_order_acquire); }
    void invalidate() noexcept override { valid_.store(false, std::memory_order_release); }
    inline void invoke(const EventType& event) const noexcept
    {
        if (!valid_) return;
        if (useFuncPtr_ && funcPtr_) funcPtr_(event);
        else if (funcObj_) funcObj_(event);
    }
private:
    HandlerFuncPtr funcPtr_ = nullptr;
    HandlerFuncObj funcObj_;
    SubscriptionToken token_;
    std::atomic<bool> valid_{true};
    bool useFuncPtr_ = false;
};

/**
 * @brief EventBus主类
 */
class EventBus 
{
public:
    static EventBus& getInstance()
    {
        static EventBus instance;
        return instance;
    }
    // 禁止拷贝和移动
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;
    /**
     * @brief 订阅事件（函数指针版本 - 零开销）
     * 使用编译期优化的函数指针, 运行时零分配
     */
    template<typename EventType>
    SubscriptionToken subscribe(void(*handler)(const EventType&))
    {
        SubscriptionToken token = nextToken_.fetch_add(1, std::memory_order_relaxed);
        using HandlerType = FastEventHandler<EventType>;
        auto handlerPtr = std::make_shared<HandlerType>(handler, token);
        {
            std::unique_lock<std::shared_mutex> lock(mutex_);
            fastHandlers_[typeid(EventType)].push_back(handlerPtr);
        }
        return token;
    }
    /**
     * @brief 订阅事件（函数对象版本）
     * 支持 lambda 和可调用对象
     */
    template<typename EventType, typename Func>
    SubscriptionToken subscribe(Func&& handler)
    {
        SubscriptionToken token = nextToken_.fetch_add(1, std::memory_order_relaxed);
        using HandlerType = FastEventHandler<EventType>;
        auto handlerPtr = std::make_shared<HandlerType>(std::forward<Func>(handler), token);
        {
            std::unique_lock<std::shared_mutex> lock(mutex_);
            fastHandlers_[typeid(EventType)].push_back(handlerPtr);
        }
        return token;
    }
    /**
     * @brief 发布事件
     */
    template<typename EventType>
    void publish(const EventType& event)
    {
        thread_local std::vector<FastEventHandler<EventType>*> handlerPtrs;
        handlerPtrs.clear();
        {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            auto it = fastHandlers_.find(typeid(EventType));
            if (it != fastHandlers_.end()) 
            {
                const auto& source = it->second;
                handlerPtrs.reserve(source.size());
                for (const auto& handler : source) 
                {
                    if (handler->isValid())
                        handlerPtrs.push_back(static_cast<FastEventHandler<EventType>*>(handler.get()));
                }
            }
        }
        const size_t size = handlerPtrs.size();
        if (size == 0) return;
#ifdef ENABLE_SIMD
        if (size >= 4)
        {
            for (size_t i = 0; i < size; ++i)
            {
                if (i + 4 < size)
                    _mm_prefetch((const char*)handlerPtrs[i + 4], _MM_HINT_T0);
                handlerPtrs[i]->invoke(event);
            }
        }
        else
#endif
        {
            // 小规模直接调用
            for (size_t i = 0; i < size; ++i)
                handlerPtrs[i]->invoke(event);
        }
    }
    /**
     * @brief 批量发布事件
     */
    template<typename EventType>
    void publishBatch(const std::vector<EventType>& events)
    {
        if (events.empty()) return;
        thread_local std::vector<FastEventHandler<EventType>*> handlerPtrs;
        handlerPtrs.clear();
        {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            auto it = fastHandlers_.find(typeid(EventType));
            if (it != fastHandlers_.end())
            {
                const auto& source = it->second;
                handlerPtrs.reserve(source.size());
                for (const auto& handler : source)
                {
                    if (handler->isValid())
                        handlerPtrs.push_back(static_cast<FastEventHandler<EventType>*>(handler.get()));
                }
            }
        }
        const size_t handlerCount = handlerPtrs.size();
        const size_t eventCount = events.size();
        if (handlerCount == 0) return;
        // 缓存友好：外层循环处理器, 内层循环事件
        for (size_t h = 0; h < handlerCount; ++h)
        {
            auto* typedHandler = handlerPtrs[h];
            size_t e = 0;
#ifdef ENABLE_SIMD
            // SIMD 预取 + 循环展开（4x）
            const size_t unrollLimit = eventCount - (eventCount % 4);
            // 预取第一批事件数据（预取约 64 字节）
            const size_t prefetchSize = (unrollLimit >= 4 ? 4 : unrollLimit);
            for (size_t i = 0; i < prefetchSize; ++i)
                _mm_prefetch(reinterpret_cast<const char*>(&events[i]), _MM_HINT_T0);

            for (; e < unrollLimit; e += 4)
            {
                // 预取后续事件（提前预取 4 个）
                if (e + 8 < eventCount)
                {
                    _mm_prefetch(reinterpret_cast<const char*>(&events[e + 4]), _MM_HINT_T0);
                    _mm_prefetch(reinterpret_cast<const char*>(&events[e + 5]), _MM_HINT_T0);
                    _mm_prefetch(reinterpret_cast<const char*>(&events[e + 6]), _MM_HINT_T0);
                    _mm_prefetch(reinterpret_cast<const char*>(&events[e + 7]), _MM_HINT_T0);
                }
                // 展开 4 次
                typedHandler->invoke(events[e]);
                typedHandler->invoke(events[e + 1]);
                typedHandler->invoke(events[e + 2]);
                typedHandler->invoke(events[e + 3]);
            }
#endif
            // 处理剩余事件
            for (; e < eventCount; ++e)
                typedHandler->invoke(events[e]);
        }
    }
    /**
     * @brief 通过令牌精确取消订阅
     */
    void unsubscribe(SubscriptionToken token) const
    {
        for (const auto& pair : fastHandlers_)
        {
            for (const auto& handler : pair.second)
            {
                if (handler->getToken() == token)
                {
                    handler->invalidate();
                    return;
                }
            }
        }
    }
    /**
     * @brief 清理无效的订阅
     */
    void cleanupInvalidHandlers() 
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        for (auto& pair : fastHandlers_) 
        {
            auto& handlers = pair.second;
            std::vector<std::shared_ptr<EventHandler>> cleaned;
            cleaned.reserve(handlers.size());
#ifdef ENABLE_SIMD
            // SIMD 批量检查（8 个一组）
            const size_t simdGroups = handlers.size() / 8;
            size_t i = 0;
            for (size_t g = 0; g < simdGroups; ++g) 
            {
                bool valid[8]{};
                for (int j = 0; j < 8; ++j)
                    valid[j] = handlers[i + j]->isValid();
                
                for (int j = 0; j < 8; ++j)
                {
                    if (valid[j])
                        cleaned.push_back(std::move(handlers[i + j]));
                }
                i += 8;
            }
            // 处理剩余
            for (; i < handlers.size(); ++i)
            {
                if (handlers[i]->isValid())
                    cleaned.push_back(std::move(handlers[i]));
            }
#else
            for (auto& handler : handlers) 
            {
                if (handler->isValid())
                    cleaned.push_back(std::move(handler));
            }
#endif
            handlers.swap(cleaned);
        }
    }
    /**
     * @brief 取消指定类型的所有订阅
     */
    template<typename EventType>
    void unsubscribeAll() 
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        fastHandlers_.erase(typeid(EventType));
    }
    /**
     * @brief 清空所有订阅
     */
    void clear() noexcept
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        fastHandlers_.clear();
    }
    /**
     * @brief 获取指定类型的订阅数量
     */
    template<typename EventType>
    size_t getSubscriberCount() const 
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto it = fastHandlers_.find(typeid(EventType));
        return it != fastHandlers_.end() ? it->second.size() : 0;
    }
    /**
     * @brief 获取所有订阅总数
     */
    size_t getTotalSubscriberCount() const
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        size_t total = 0;
        for (const auto& pair : fastHandlers_)
            total += pair.second.size();
        return total;
    }
private:
    EventBus() = default;
    ~EventBus() = default;
    // 禁止移动
    EventBus(EventBus&&) = delete;
    EventBus& operator=(EventBus&&) = delete;

    // 高性能事件处理器映射
    std::unordered_map<std::type_index, std::vector<std::shared_ptr<EventHandler>>> fastHandlers_;
    mutable std::shared_mutex mutex_;
    std::atomic<SubscriptionToken> nextToken_{1};
};

// 订阅事件宏（返回令牌）
#define SUBSCRIBE_EVENT(eventType, handler) \
    EventBus::getInstance().subscribe<eventType>(handler)
// 通过令牌取消订阅事件宏
#define UNSUBSCRIBE_EVENT(token) \
    EventBus::getInstance().unsubscribe(token)
// 取消指定类型的所有订阅宏
#define UNSUBSCRIBE_ALL_EVENT(eventType) \
    EventBus::getInstance().unsubscribeAll<eventType>()
// 发布事件宏
#define PUBLISH_EVENT(event) \
    EventBus::getInstance().publish(event)
// 批量发布事件宏
#define PUBLISH_EVENT_BATCH(events) \
    EventBus::getInstance().publishBatch(events)
// 清空所有订阅宏
#define CLEAR_ALL_EVENTS() \
    EventBus::getInstance().clear()
