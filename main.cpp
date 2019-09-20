#include <Windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "Shlwapi.lib")

#define LINK_DESCRIPTION    L"Lock the screen by pressing the power button twice."

#define MUTEX_NAME      L"B8A238FC-927E-4E71-B652-BA4AFAFCBAC2"

#define ARG_INSTALL     L"/install"
#define ARG_UNINSTALL   L"/uninstall"

#define KeyDelta    1000
#define KeyCount    2

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

ULONGLONG   lTime = 0;
int         count = 0;
int execMain()
{
    auto hMutexInstance = CreateMutexW(NULL, FALSE, MUTEX_NAME);
    if (GetLastError() == ERROR_ALREADY_EXISTS)
        return 0;

    RegisterHotKey(NULL, 0, MOD_NOREPEAT, 0xFF);

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

            if (now < lTime)
            {
                if (++count >= KeyCount)
                    LockWorkStation();
            }
            else
            {
                lTime = now + KeyDelta;

                count = 1;
            }

            break;

        default:
            break;
        }
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