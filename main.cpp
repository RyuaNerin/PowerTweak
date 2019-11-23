#include <mutex>

#include <Windows.h>

#include <shlobj.h>
#pragma comment(lib, "shell32.lib")

#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#include <powrprof.h>
#pragma comment(lib, "PowrProf.lib")

constexpr auto LINK_DESCRIPTION = L"Simple tweak for power key in keyboard";

constexpr auto MUTEX_NAME = L"B8A238FC-927E-4E71-B652-BA4AFAFCBAC2";

constexpr auto ARG_INSTALL = L"/install";
constexpr auto ARG_UNINSTALL = L"/uninstall";

constexpr auto POWER_KEY_CODE = 0xFF;
constexpr auto POWER_KEY_DELTA = 1000;

int execMain();
int execInstall();
int execUninstall();

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    int argc;
    LPWSTR* argv = CommandLineToArgvW(pCmdLine, &argc);

    for (int i = 0; i < argc; i++)
    {
        if (lstrcmpiW(argv[i], ARG_INSTALL) == 0)
            return execInstall();

        else if (lstrcmpiW(argv[i], ARG_UNINSTALL) == 0)
            return execUninstall();
    }

    return execMain();
}

DWORD WINAPI KeyInputTicker(PVOID param);
std::mutex  m_lock;
bool        m_tickerRunning = false;
int         m_count = 0;
int execMain()
{
    auto hMutexInstance = CreateMutexW(NULL, FALSE, MUTEX_NAME);
    if (GetLastError() == ERROR_ALREADY_EXISTS)
        return 0;

    RegisterHotKey(NULL, 0, MOD_NOREPEAT, POWER_KEY_CODE);

    MSG msg;
    ULONGLONG now;
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        switch (msg.message)
        {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_HOTKEY:
            now = GetTickCount64();

            //if (LOWORD(msg.lParam) == KeyCode && (HIWORD(msg.lParam) & (MOD_ALT | MOD_CONTROL | MOD_SHIFT | MOD_WIN)) != 0)
            if (HIWORD(msg.lParam) == POWER_KEY_CODE)
            {
                m_lock.lock();
                if (!m_tickerRunning)
                {
                    m_tickerRunning = true;

                    DWORD threadId;
                    CreateThread(NULL, 0, KeyInputTicker, NULL, 0, &threadId);
                    m_count = 0;
                }
                m_count++;
                m_lock.unlock();
            }

            break;

        default:
            break;
        }
    }
    
    return 0;
}

DWORD WINAPI KeyInputTicker(PVOID param)
{
    Sleep(POWER_KEY_DELTA);

    m_lock.lock();
    m_tickerRunning = false;
    auto c = m_count;
    m_lock.unlock();

    switch (m_count)
    {
    case 1:
        SendMessageW(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, 2);
        break;

    case 2:
        LockWorkStation();
        break;

    case 3:
        SetSuspendState(FALSE, FALSE, FALSE);
        break;
    }

    return 0;
}

bool getDirectory(LPWSTR pathExe, DWORD pathExeLength, LPWSTR pathLink, DWORD  pathLinkLength)
{
    if (GetModuleFileNameW(NULL, pathExe, pathExeLength) == 0)
        return false;

    if (FAILED(SHGetFolderPathW(NULL, CSIDL_STARTUP, NULL, pathLinkLength, pathLink)))
        return false;

    if (!PathAppendW(pathLink, PathFindFileNameW(pathExe)))
        return false;

    if (!PathRenameExtensionW(pathLink, L".lnk"))
        return false;

    return true;
}

int execInstall()
{
    WCHAR pathExe[MAX_PATH];
    WCHAR pathLink[MAX_PATH];

    if (!getDirectory(pathExe, MAX_PATH, pathLink, MAX_PATH))
        return 0;

    if (FAILED(CoInitialize(NULL)))
        return 0;

    IShellLinkW* psl;
    if (FAILED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (LPVOID*)& psl)))
        return 0;

    psl->SetPath(pathExe);
    psl->SetDescription(LINK_DESCRIPTION);

    IPersistFile* ppf;
    if (SUCCEEDED(psl->QueryInterface(IID_IPersistFile, (LPVOID*)& ppf)))
    {
        ppf->Save(pathLink, TRUE);
        ppf->Release();
    }
    psl->Release();
    
    return 0;
}

int execUninstall()
{
    WCHAR pathExe[MAX_PATH];
    WCHAR pathLink[MAX_PATH];

    if (!getDirectory(pathExe, MAX_PATH, pathLink, MAX_PATH))
        return 0;

    DeleteFileW(pathLink);

    return 0;
}
