#include <std_include.hpp>
#include "common.hpp"

namespace utils::common
{
    void WriteString(const char* file, const char* szString)
    {
        FILE* stream;
        if(fopen_s(&stream, file, "a")) return;

        if (stream)
        {
            fprintf_s(stream, "%s\n", szString);
            fclose(stream);
        }
    }

    DWORD64 getModuleBaseAddr(LPCSTR Module) // https://stackoverflow.com/questions/4298331/exe-or-dll-image-base-address
    {
        const auto module = GetModuleHandle(Module);
        return uint64_t(module);
    }

    DWORD64 getModuleImgSize(LPCSTR Module) // https://stackoverflow.com/questions/4298331/exe-or-dll-image-base-address
    {
        MODULEINFO module_info; memset(&module_info, 0, sizeof(module_info));
        if (GetModuleInformation(GetCurrentProcess()/*(HANDLE) - 1*/, GetModuleHandle(Module), &module_info, sizeof(module_info))) {
            DWORD module_size = module_info.SizeOfImage;
            BYTE* module_ptr = (BYTE*)module_info.lpBaseOfDll;

            return module_size;
        }
        return 0;
    }

    std::string getModuleFileName(LPCSTR Module) // https://stackoverflow.com/questions/6006319/converting-tchar-to-string-in-c, https://stackoverflow.com/questions/10204145/convert-wchar260-to-stdstring
    {
        TCHAR szModuleFileName[MAX_PATH];
        GetModuleFileName(GetModuleHandle(Module), szModuleFileName, MAX_PATH);

        std::string szModuleFileNameSTR(&szModuleFileName[0]);

        std::vector <std::string> ModuleFileNamePatch;
        ModuleFileNamePatch = utils::string::split(szModuleFileNameSTR, '\\');
        std::string ModuleFileName = ModuleFileNamePatch[ModuleFileNamePatch.size() - 1];

        return ModuleFileName;
    }

    std::string GetWindowTitleString(HWND hwnd) // https://stackoverflow.com/questions/45874097/winapi-getwindowtext-as-string
    {
        int len = GetWindowTextLength(hwnd) + 1;
        std::vector<wchar_t> buf(len);
        GetWindowTextW(hwnd, &buf[0], len);
        std::wstring wstr = &buf[0];
        std::string str(wstr.begin(), wstr.end());
        return str;
    }

    HWND getCurrentProcessWindowHandle()
    {
        HWND activeHWND = NULL; DWORD activePID = 0;
        DWORD PID = GetCurrentProcessId();
        while (true)
        {
            activeHWND = GetForegroundWindow();
            GetWindowThreadProcessId(activeHWND, &activePID);

            if (PID == activePID) return activeHWND; else Sleep(100);
        }
    }
}
