
#include "PCH.h"
#include "Core/Engine.h"
#include "Platform/Windows/WindowsOS.h"
#include "Platform/Windows/WindowsWindow.h"

#include <GLFW/glfw3.h>
#include <dwmapi.h>


#if SL_RELEASE
#define ASSIMP_DLL_NAME  L"Resources/assimp-vc143-mt.dll"
#define SHADERC_DLL_NAME L"Resources/shaderc_shared.dll"
#else
#define ASSIMP_DLL_NAME  L"Resources/assimp-vc143-mtd.dll"
#define SHADERC_DLL_NAME L"Resources/shaderc_sharedd.dll"
#endif


namespace Silex
{
    // DLL ハンドル
    static HMODULE assimpDLL = nullptr;
    static HMODULE shadercDLL = nullptr;


    std::wstring ToUTF16(const std::string& utf8)
    {
        if (utf8.empty())
            return std::wstring();

        int size = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), NULL, 0);
        std::wstring utf16(size, 0);

        MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), &utf16[0], size);

        return utf16;
    }

    std::string ToUTF8(const std::wstring& utf16)
    {
        if (utf16.empty())
            return std::string();

        int size_needed = WideCharToMultiByte(CP_UTF8, 0, utf16.c_str(), (int)utf16.size(), NULL, 0, NULL, NULL);
        std::string utf8(size_needed, 0);

        WideCharToMultiByte(CP_UTF8, 0, utf16.c_str(), (int)utf16.size(), &utf8[0], size_needed, NULL, NULL);

        return utf8;
    }


    // Windows Window
    static Window* CreateWindowsWindow(const char* title, uint32 width, uint32 height)
    {
        return Memory::Allocate<WindowsWindow>(title, width, height);
    }



    WindowsOS::WindowsOS()
    {
        CHECK_HRESULT(::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED));
    }

    WindowsOS::~WindowsOS()
    {
        ::CoUninitialize();
    }

    void WindowsOS::Run()
    {
        while (true)
        {
            Window::Get()->PumpMessage();

            if (!Engine::Get()->MainLoop())
            {
                break;
            }
        }
    }

    void WindowsOS::Initialize()
    {
#if SL_DEBUG

        // メモリリークトラッカ有効化
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

        // コンソールのエンコードを UTF8 にする
        defaultConsoleCP = GetConsoleOutputCP();
        SetConsoleOutputCP(65001);

        // コンソールの標準入出力ハンドル取得
        outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);

#endif
        // DLL ロード
        assimpDLL  = ::LoadLibraryW(ASSIMP_DLL_NAME);
        shadercDLL = ::LoadLibraryW(SHADERC_DLL_NAME);

        // Windows OS バージョンを取得
        CheckOSVersion();

        // クロックカウンター初期化
        ::timeBeginPeriod(1);
        ::QueryPerformanceFrequency((LARGE_INTEGER*)&tickPerSecond);
        ::QueryPerformanceCounter((LARGE_INTEGER*)&startTickCount);

        //==============================================
        // glfw初期化 ※WindowsWindow の WinAPI移行後に削除
        //==============================================
        ::glfwInit();

        // 各プラットフォーム生成関数を登録
        Window::RegisterCreateFunction(&CreateWindowsWindow);
    }

    void WindowsOS::Finalize()
    {
#if SL_DEBUG
        SetConsoleOutputCP(defaultConsoleCP);
#endif
        ::glfwTerminate();
        ::timeEndPeriod(1);

        // DLL 解放
        ::FreeLibrary(assimpDLL);
        ::FreeLibrary(shadercDLL);
    }

    uint64 WindowsOS::GetTickSeconds()
    {
        //==============================================================
        // OS::Initialize からの経過時間をマイクロ秒(μs)で返す
        //--------------------------------------------------------------
        // 実際にはOS起動時からのクロックカウントを、秒間クロック数で除算して
        // 経過時間を 秒(sec) でもとめる。また、uint64では小数点以下繰り上げ
        // なので、1,000,000 倍してマイクロ秒にしている。
        // 
        // UINT64_MAX の範囲が 0 ～ 18,446,744,073,709,551,615 なので 
        // tick * 1,000,000 の計算がオーバーフローするのは tick の値が
        // 18,446,744,073,709の時になります。
        // 
        // 私の環境では、tickPerSecond(秒間クロック数) == 10,000,000 なので
        // 18,446,744,073,709 / 10,000,000 = 1,844,674(sec)
        // これは 1,844,674 / (60 * 60 * 24) = 21.35(日)になります。
        // 
        // オーバーフロー軽減のため、剰余算で整数部と小数点部に分けることで
        // 本来のuint64 の精度で計算できる
        //
        // UINT64_MAX / 10,000,000 / (60 * 60 * 24) * 365.25 = 58,454年
        //==============================================================

        uint64 tick;
        ::QueryPerformanceCounter((LARGE_INTEGER*)&tick);

        tick -= startTickCount;

        uint64 μsec     = 1'000'000;
        uint64 seconds  = (tick / tickPerSecond) * μsec;                 // 整数部
        uint64 decimal  = (tick % tickPerSecond) * μsec / tickPerSecond; // 小数点部

        return seconds + decimal;
    }

    void WindowsOS::Sleep(uint32 millisec)
    {
        ::Sleep(millisec);
    }

    std::string WindowsOS::OpenFile(const char* filter)
    {
        OPENFILENAMEA ofn;
        CHAR szFile[260] = { 0 };

        std::memset(&ofn, 0, sizeof(OPENFILENAME));
        ofn.lStructSize  = sizeof(OPENFILENAME);
        ofn.hwndOwner    = (HWND)Window::Get()->GetWindowHandle();
        ofn.lpstrFile    = szFile;
        ofn.nMaxFile     = sizeof(szFile);
        ofn.lpstrFilter  = filter;
        ofn.nFilterIndex = 1;
        ofn.Flags        = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (::GetOpenFileNameA(&ofn) == TRUE)
            return ofn.lpstrFile;

        return {};
    }

    std::string WindowsOS::SaveFile(const char* filter, const char* extention)
    {
        OPENFILENAMEA ofn;
        CHAR szFile[260] = { 0 };

        std::memset(&ofn, 0, sizeof(OPENFILENAME));
        ofn.lStructSize  = sizeof(OPENFILENAME);
        ofn.hwndOwner    = (HWND)Window::Get()->GetWindowHandle();
        ofn.lpstrFile    = szFile;
        ofn.nMaxFile     = sizeof(szFile);
        ofn.lpstrFilter  = filter;
        ofn.lpstrDefExt  = extention;
        ofn.nFilterIndex = 1;
        ofn.Flags        = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (::GetSaveFileNameA(&ofn) == TRUE)
            return ofn.lpstrFile;

        return {};
    }

    void WindowsOS::SetConsoleAttribute(uint16 color)
    {
#if SL_DEBUG
        ::SetConsoleTextAttribute(outputHandle, color);
#endif
    }

    void WindowsOS::OutputConsole(uint8 color, const std::string& message)
    {
#if SL_DEBUG
        // コンソールカラー
        // https://blog.csdn.net/Fdog_/article/details/103764196
        static uint8 levels[6] = { 0xcf, 0x0c, 0x06, 0x02, 0x08, 0x03 };
        ::SetConsoleTextAttribute(outputHandle, levels[color]);

        // コンソール出力
        ::WriteConsoleW(outputHandle, message.c_str(), (uint32)message.size(), nullptr, nullptr);
#endif
    }

    void WindowsOS::OutputDebugConsole(const std::string& message)
    {
        ::OutputDebugStringW(ToUTF16(message).c_str());
    }

    int32 WindowsOS::Message(OSMessageType type, const std::wstring& message)
    {
        uint32 messageType = type == OS_MESSEGA_TYPE_ALERT ? MB_OK | MB_ICONERROR : MB_OK | MB_ICONINFORMATION;
        return ::MessageBoxW(NULL, message.c_str(), L"Error", messageType);
    }


    // SDKで定義されているかどうかを確認（ver 10.0.22000.0 ~)
    SL_DECLARE_ENUMERATOR_TRAITS(DWMWINDOWATTRIBUTE,           DWMWA_WINDOW_CORNER_PREFERENCE);
    SL_DECLARE_ENUMERATOR_TRAITS(DWM_WINDOW_CORNER_PREFERENCE, DWMWCP_ROUND);
    SL_DECLARE_ENUMERATOR_TRAITS(DWM_WINDOW_CORNER_PREFERENCE, DWMWCP_DONOTROUND);

    HRESULT WindowsOS::TrySetWindowCornerStyle(HWND hWnd, bool tryRound)
    {
        HRESULT hr = NOERROR;

        //----------------------------------------------------------
        // NOTE:
        // Windows 11 (OS build: 22000) 以降をサポート
        // SDK は 10.0.22000.0 以降 (DWM_WINDOW_CORNER_PREFERENCE)
        //----------------------------------------------------------
        if (osBuildNumber >= 22000)
        {
            if constexpr (DWMWA_WINDOW_CORNER_PREFERENCE_t::defined && DWMWCP_ROUND_t::defined && DWMWCP_DONOTROUND_t::defined)
            {
                const DWM_WINDOW_CORNER_PREFERENCE corner = tryRound ? DWMWCP_ROUND : DWMWCP_DONOTROUND;
                hr = ::DwmSetWindowAttribute(hWnd, DWMWA_WINDOW_CORNER_PREFERENCE, &corner, sizeof(DWM_WINDOW_CORNER_PREFERENCE));
            }
            else
            {
                // エラーではないが、SDKバージョンで定義される値が一致していると確証できないため、現状は実行しない
                // const int32 corner = tryRound ? 2 : 1;
                // hr = DwmSetWindowAttribute(hWnd, 33, &corner, sizeof(int32));
            }
        }

        return hr;
    }

    void WindowsOS::CheckOSVersion()
    {
        const auto hModule = ::LoadLibraryW(TEXT("ntdll.dll"));
        if (hModule)
        {
            const auto address = ::GetProcAddress(hModule, "RtlGetVersion");
            if (address)
            {
                using RtlGetVersionType = NTSTATUS(WINAPI*)(OSVERSIONINFOEXW*);
                const auto RtlGetVersion = reinterpret_cast<RtlGetVersionType>(address);

                OSVERSIONINFOEXW os = { sizeof(os) };
                if (SUCCEEDED(RtlGetVersion(&os)))
                {
                    osBuildNumber  = os.dwBuildNumber;
                    osVersionMinor = os.dwMinorVersion;
                    osVersionMajor = os.dwMajorVersion;
                }
            }

            ::FreeLibrary(hModule);
        }
    }
}
