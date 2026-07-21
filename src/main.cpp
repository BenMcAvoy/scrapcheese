#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "sdk.hpp"

DWORD
MainThread(LPVOID lpParam)
{
    SM::ConsoleSink<std::mutex>::init();

    auto cmc = SM::CharacterManagerClient::GetInstance();
    if (cmc)
    {
        auto count = cmc->characters.size();
        SM_LogInfo("CharacterManagerClient character count: {}", count);

        for (const auto &[id, character] : cmc->characters)
        {
            auto &pos = character->controller->positionHolder->position;
            const auto &addr = character.get();

            SM_LogInfo("Character ID: {} @ ({}, {}, {}) @ 0x{:X}", id, pos.x, pos.y, pos.z, reinterpret_cast<uintptr_t>(addr));
        }
    }
    else
    {
        SM_LogWarn("CharacterManagerClient instance is null");
    }

    FreeLibraryAndExitThread(static_cast<HMODULE>(lpParam), 0);
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    if (fdwReason != DLL_PROCESS_ATTACH)
        return TRUE;

    DisableThreadLibraryCalls(hinstDLL);
    CreateThread(nullptr, 0, MainThread, hinstDLL, 0, nullptr);

    return TRUE;
}
