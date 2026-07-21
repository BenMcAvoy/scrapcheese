#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>

#include <cstdint>
#include <string>
#include <unordered_map>

#define SM_UNPAREN(...) __VA_ARGS__

#define PROP(type, name, offset)                                                                                               \
    __declspec(property(get = get_##name, put = put_##name)) SM_UNPAREN type name;                                             \
    SM_UNPAREN type &get_##name() { return *reinterpret_cast<SM_UNPAREN type *>(reinterpret_cast<uintptr_t>(this) + offset); } \
    void put_##name(SM_UNPAREN type val) { *reinterpret_cast<SM_UNPAREN type *>(reinterpret_cast<uintptr_t>(this) + offset) = val; }

#define SINGLETON(klass, rva) \
    static klass *GetInstance() { return *reinterpret_cast<klass **>(reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr)) + rva); }

#define PUB public:
#define PRI private:

namespace SM
{
    namespace UTILS
    {
        class Console
        {
            PRI virtual ~Console() { /* implemented by game */ }

            PUB enum class LogAttributes : uint32_t {
                FgBlue = 0x0001,            // text color contains blue.
                FgGreen = 0x0002,           // text color contains green.
                FgRed = 0x0004,             // text color contains red.
                FgIntensity = 0x0008,       // text color is intensified.
                BgBlue = 0x0010,            // background color contains blue.
                BgGreen = 0x0020,           // background color contains green.
                BgRed = 0x0040,             // background color contains red.
                BgIntensity = 0x0080,       // background color is intensified.
                LvbLeadingByte = 0x0100,    // Leading Byte of DBCS
                LvbTrailingByte = 0x0200,   // Trailing Byte of DBCS
                LvbGridHorizontal = 0x0400, // DBCS: Grid attribute: top horizontal.
                LvbGridLVertical = 0x0800,  // DBCS: Grid attribute: left vertical.
                LvbGridRVertical = 0x1000,  // DBCS: Grid attribute: right vertical.
                LvbReverseVideo = 0x4000,   // DBCS: Reverse fore/back ground attribute.
                LvbUnderscore = 0x8000,     // DBCS: Underscore.
                LvbSbcsDbcs = 0x0300,       // SBCS or DBCS flag.
            };

            PUB enum class LogType : uint32_t {
                None = 0,

                Default = 1u << 0,  // 0x00000001
                Profile = 1u << 1,  // 0x00000002
                Resource = 1u << 2, // 0x00000004
                Shader = 1u << 3,   // 0x00000008
                Buffer = 1u << 4,   // 0x00000010
                Render = 1u << 5,   // 0x00000020
                Network = 1u << 6,  // 0x00000040

                Terrain = 1u << 8, // 0x00000100
                World = 1u << 9,   // 0x00000200
                Audio = 1u << 10,  // 0x00000400
                Lua = 1u << 11,    // 0x00000800
                Print = 1u << 12,  // 0x00001000
                UGC = 1u << 13,    // 0x00002000
                Steam = 1u << 14,  // 0x00004000

                System = 0x80000000u,
                Warning = 1u << 30
            };

            PUB virtual void Print(std::string msg, LogAttributes attribs, LogType type) { /* implemented by game */ }
            PUB virtual void PrintNoRepeat(std::string msg, LogAttributes attribs, LogType type) { /* implemented by game */ }
        };

        constexpr Console::LogAttributes operator|(Console::LogAttributes lhs, Console::LogAttributes rhs) noexcept
        {
            using T = std::underlying_type_t<Console::LogAttributes>;
            return static_cast<Console::LogAttributes>(
                static_cast<T>(lhs) | static_cast<T>(rhs));
        }

        constexpr Console::LogAttributes operator&(Console::LogAttributes lhs, Console::LogAttributes rhs) noexcept
        {
            using T = std::underlying_type_t<Console::LogAttributes>;
            return static_cast<Console::LogAttributes>(
                static_cast<T>(lhs) & static_cast<T>(rhs));
        }

        constexpr Console::LogAttributes operator~(Console::LogAttributes value) noexcept
        {
            using T = std::underlying_type_t<Console::LogAttributes>;
            return static_cast<Console::LogAttributes>(
                ~static_cast<T>(value));
        }

        constexpr Console::LogAttributes &operator|=(Console::LogAttributes &lhs, Console::LogAttributes rhs) noexcept
        {
            return lhs = lhs | rhs;
        }

        constexpr Console::LogAttributes &operator&=(Console::LogAttributes &lhs, Console::LogAttributes rhs) noexcept
        {
            return lhs = lhs & rhs;
        }
    }

    class Contraption;
    template <typename Mutex>
    class ConsoleSink : public spdlog::sinks::base_sink<Mutex>
    {
    protected:
        void sink_it_(const spdlog::details::log_msg &msg) override
        {
            using namespace SM::UTILS;

            Console::LogAttributes attribs = Console::LogAttributes::FgGreen;
            Console::LogType log_type = Console::LogType::Default;

            switch (msg.level)
            {
            case spdlog::level::trace:
                attribs = Console::LogAttributes::FgBlue | Console::LogAttributes::FgRed;
                log_type = Console::LogType::Print;
                break;
            case spdlog::level::debug:
                attribs = Console::LogAttributes::FgBlue;
                log_type = Console::LogType::Print;
                break;
            case spdlog::level::info:
                attribs = Console::LogAttributes::FgRed | Console::LogAttributes::FgGreen | Console::LogAttributes::FgBlue;
                log_type = Console::LogType::Print;
                break;
            case spdlog::level::warn:
                attribs = Console::LogAttributes::FgRed | Console::LogAttributes::FgGreen;
                log_type = Console::LogType::Warning;
                break;
            case spdlog::level::err:
                attribs = Console::LogAttributes::FgRed;
                log_type = Console::LogType::System;
                break;
            case spdlog::level::critical:
                attribs = Console::LogAttributes::FgRed | Console::LogAttributes::FgIntensity;
                log_type = Console::LogType::System;
                break;
            default:
                attribs = Console::LogAttributes::FgRed | Console::LogAttributes::FgGreen | Console::LogAttributes::FgBlue;
                log_type = Console::LogType::Print;
                break;
            }

            std::string_view source_file = msg.source.filename;
            auto last_slash_pos = source_file.find_last_of("\\");
            std::string_view source_file_name = (last_slash_pos != std::string_view::npos) ? source_file.substr(last_slash_pos + 1) : source_file;

            spdlog::memory_buf_t formatted;
            spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
            std::string fmt_msg = fmt::to_string(formatted);

            SM::Contraption::GetInstance()->console->Print(fmt_msg, attribs, log_type);
        }

        void flush_() override {}

        PUB static void init()
        {
            auto consoleSink = std::make_shared<ConsoleSink<Mutex>>();

            spdlog::set_default_logger(std::make_shared<spdlog::logger>("scrapcheese", consoleSink));
            // spdlog::set_formatter("%v %s:%#");
            spdlog::set_formatter(std::make_unique<spdlog::pattern_formatter>("%v [%s:%#]", spdlog::pattern_time_type::local, ""));
            spdlog::set_level(spdlog::level::trace);
        }
    };

#define SM_LogInfo(...) SPDLOG_INFO(__VA_ARGS__)
#define SM_LogWarn(...) SPDLOG_WARN(__VA_ARGS__)
#define SM_LogError(...) SPDLOG_ERROR(__VA_ARGS__)
#define SM_LogDebug(...) SPDLOG_DEBUG(__VA_ARGS__)
#define SM_LogTrace(...) SPDLOG_TRACE(__VA_ARGS__)
#define SM_LogCritical(...) SPDLOG_CRITICAL(__VA_ARGS__)

    class Contraption
    {
        PUB SINGLETON(Contraption, 0x1267498);

        PUB PROP((UTILS::Console *), console, 0x58);
    };

    class Vector3
    {
        PUB float x;
        PUB float y;
        PUB float z;
    };

    class Aligned16Base
    {
    };
    class NetObj : public Aligned16Base
    {
    };

    class PhysicsProxy : public Aligned16Base
    {
    };

    class CharacterPhysicsProxy : public PhysicsProxy
    {
    };

    class PositionHolder : public Aligned16Base
    {
        PUB PROP((Vector3), position, 0x40);
    };

    class CharacterController
    {
        PUB PROP((PositionHolder *), positionHolder, 0x38);
    };

    class CharacterData
    {
        PUB PROP((CharacterController *), controller, 0x110);
    };

    class Character : public NetObj
    {
        PUB PROP((CharacterData *), data, 0xB38);
        PUB PROP((std::shared_ptr<CharacterController>), controller, 0xB48);
    };

    class CharacterManager
    {
        PUB SINGLETON(CharacterManager, 0x1267698);

        PUB PROP((std::unordered_map<int, std::shared_ptr<Character>>), characters, 0x8);
    };
    class CharacterManagerClient : public CharacterManager
    {
    };
}

#undef PROP
#undef SINGLETON
#undef PUB
#undef PRI