#include "UI/PrimaryUI.h"
#include "UI/Window.h"
#include "Assets.h"
#include "GameConnectionManager.h"
#include "Log.h"
#include "UserData.h"

#include <filesystem>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <thread>
#include <vector>

#include "WinCompat.h"

#pragma warning(disable: 4244)

#ifdef _WIN32
BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
#endif // _WIN32

bool RogueAssistant_MainLoop(Window* window, void* userData);
void RogueAssistant_StubFunc();
void RogueAssistant_ThreadFunc();

// The script must land beside the library, because that is where Lua's
// package.cpath looks for it. This previously used the working directory, which
// on Windows usually coincided but on Linux does not.
static std::filesystem::path GetExecutableDirectory()
{
#ifdef _WIN32
    wchar_t buffer[MAX_PATH];
    DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    if (length > 0 && length < MAX_PATH)
        return std::filesystem::path(buffer).parent_path();
#else
    std::error_code errorCode;
    std::filesystem::path exePath = std::filesystem::read_symlink("/proc/self/exe", errorCode);
    if (!errorCode)
        return exePath.parent_path();
#endif

    std::error_code fallbackError;
    return std::filesystem::current_path(fallbackError);
}

static void DumpScriptsNextToExe()
{
    {
        std::filesystem::path scriptPath = GetExecutableDirectory() / "RogueAssistant_mGBA.lua";

        std::ofstream fileStream;
        fileStream.open(scriptPath, std::ios::out);

        LOG_INFO("Dumping %s", scriptPath.string().c_str());

        auto const& data = bin2cpp::getRogueAssistant_mGBALuaFile();
        char const* ptr = data.getBuffer();

        for (; *ptr != 0; ++ptr)
        {
            // Ignore carriage return
            if (*ptr != '\r')
                fileStream << *ptr;
        }

        fileStream.close();
    }

    // TODO - Move assets around and only ship exe file
}

static std::unique_ptr<std::thread> s_BackgroundThread;
static bool s_CloseRequested = false;

ROGUE_EXPORT int RogueAssistant_Main(bool isStub, std::vector<std::string> const& args)
{
    if (isStub)
    {
        DumpScriptsNextToExe();
        UserData::Init();

        RogueAssistant_StubFunc();
        //RogueAssistant_ThreadFunc()
    }
    else
    {
        UserData::Init();

        s_BackgroundThread = std::make_unique<std::thread>(RogueAssistant_ThreadFunc);
    }

    return 0;
}

void RogueAssistant_Frame()
{
}

void RogueAssistant_Shutdown()
{
    if (s_BackgroundThread)
    {
        s_CloseRequested = true;
        s_BackgroundThread->join();
        s_BackgroundThread = nullptr;
    }
}

struct WindowData
{
    Window m_Window;
    PrimaryUI m_UI;
};

void RogueAssistant_StubFunc()
{
    WindowConfig config;
    config.title = "Rogue Assistant";
    config.canBeDestroyed = true;

    WindowData data =
    {
        Window(config),
        PrimaryUI()
    };
    data.m_UI.SetToStubTheme();

    if (data.m_Window.Create())
    {
        data.m_Window.EnterMainLoop(RogueAssistant_MainLoop, &data);
        data.m_Window.Destroy();
    }
}

void RogueAssistant_ThreadFunc()
{
#ifdef _WIN32
    SetThreadDescription(GetCurrentThread(), L"RogueAssistant");
#else
    pthread_setname_np(pthread_self(), "RogueAssistant");
#endif

    WindowConfig config;
    config.title = "Rogue Assistant";
    config.imGuiEnabled = false;
    config.canBeDestroyed = false;

    WindowData data =
    {
        Window(config),
        PrimaryUI()
    };

    if (data.m_Window.Create())
    {
        GameConnectionManager::Instance().OpenListener();

        data.m_Window.EnterMainLoop(RogueAssistant_MainLoop, &data);

        if (data.m_Window.Destroy())
        {
            GameConnectionManager::Instance().CloseListener();
        }
    }
}

bool RogueAssistant_MainLoop(Window* window, void* userData)
{
    WindowData* data = (WindowData*)userData;
    UserData::Update();
    GameConnectionManager::Instance().UpdateConnections();
    data->m_UI.Render(*window);

    if (s_CloseRequested)
        return false;

    return true;
}