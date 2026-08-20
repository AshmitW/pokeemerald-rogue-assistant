
#include <string>
#include <vector>

#ifdef _WIN32
#include <Windows.h>

#pragma warning(disable: 4244)

__declspec(dllimport) int RogueAssistant_Main(bool isStub, std::vector<std::string> const& args);
#else
int RogueAssistant_Main(bool isStub, std::vector<std::string> const& args);
#endif

// Linux always uses main(); Windows keeps WinMain for Release so no console appears.
#if !defined(_WIN32) || defined(_DEBUG)
int main(int argc, const char** argv)
{
    std::vector<std::string> args;

    for (int i = 0; i < argc; ++i)
    {
        args.push_back(std::string(argv[i]));
    }

    return RogueAssistant_Main(true, args);
}

#else

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow)
{
    std::vector<std::string> args;

    for (int i = 0; i < __argc; ++i)
    {
        std::string str(__argv[i]);
        args.push_back(str);
    }

    return RogueAssistant_Main(true, args);
}

#endif