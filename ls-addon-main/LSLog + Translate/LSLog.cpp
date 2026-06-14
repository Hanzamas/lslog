#include <iostream>
#include <windows.h>
#include <psapi.h>
#include <tchar.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <string>
#include <thread>
#include <memory>

using namespace std;

#define MAX_PROCESSES 1024

void* DetourFunction(BYTE* src, DWORD dst, const int len)
{
    BYTE* jmp = (BYTE*)malloc(len + 5);
    DWORD dwBack;
    VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &dwBack);
    memcpy(jmp, src, len);
    jmp += len;
    jmp[0] = 0xE9;
    *(DWORD*)(jmp + 1) = (DWORD)(src + len - jmp) - 5;
    src[0] = 0xE9;
    *(DWORD*)(src + 1) = (DWORD)(dst - (DWORD)src) - 5;
    for (int i = 5; i < len; i++) src[i] = 0x90;
    VirtualProtect(src, len, dwBack, &dwBack);
    return (jmp - len);
}

DWORD FindProcess(LPCTSTR lpcszFileName);

// Function to terminate a process by its name
void TerminateProcessByName(LPCTSTR lpcszFileName) {
    DWORD dwProcessId = FindProcess(lpcszFileName);
    if (dwProcessId) {
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);
        if (hProcess) {
            TerminateProcess(hProcess, 0);
            CloseHandle(hProcess);
        }
    }
}

void ReadMemoryFromProcess(DWORD processID, LPVOID baseAddress, SIZE_T size)
{
    // Open the process with required access
    HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, processID);
    if (hProcess == NULL) {
        printf("Failed to open process for reading memory\n");
        return;
    }

    // Allocate buffer to read memory
    BYTE* buffer = new BYTE[size];
    SIZE_T bytesRead = 0;

    // Read memory from the process
    if (ReadProcessMemory(hProcess, baseAddress, buffer, size, &bytesRead)) {
        // Print the data as a string
        printf("Read memory: %s\n", buffer);
    }
    else {
        printf("Failed to read memory from process\n");
    }

    // Clean up
    delete[] buffer;
    CloseHandle(hProcess);
}

// Function pointers for Sentinel AntiCheat
typedef void (*Sentinel_Init_t)();
typedef void (*Sentinel_Check_t)();
typedef void (*Sentinel_Shutdown_t)();

// Declare function pointers
Sentinel_Init_t Sentinel_Init = nullptr;
Sentinel_Check_t Sentinel_Check = nullptr;
Sentinel_Shutdown_t Sentinel_Shutdown = nullptr;

// A utility function to load the Sentinel AntiCheat DLL and fetch function pointers
void LoadSentinelAntiCheat() {
    HMODULE hSentinel = LoadLibrary("Sentinel-AntiCheat.dll");  // Make sure the DLL is in the same folder as the exe
    if (hSentinel) {
        Sentinel_Init = (Sentinel_Init_t)GetProcAddress(hSentinel, "Sentinel_Init");
        Sentinel_Check = (Sentinel_Check_t)GetProcAddress(hSentinel, "Sentinel_Check");
        Sentinel_Shutdown = (Sentinel_Shutdown_t)GetProcAddress(hSentinel, "Sentinel_Shutdown");

        if (Sentinel_Init && Sentinel_Check && Sentinel_Shutdown) {
            printf("Sentinel AntiCheat DLL loaded successfully!\n");
        }
        else {
            printf("Failed to get function pointers from Sentinel-AntiCheat.dll\n");
        }
    }
    else {
        printf("Failed to load Sentinel-AntiCheat.dll\n");
    }
}

DWORD FindProcess(__in_z LPCTSTR lpcszFileName)
{
    LPDWORD lpdwProcessIds;
    LPTSTR  lpszBaseName;
    HANDLE  hProcess;
    DWORD   i, cdwProcesses, dwProcessId = 0;
    //LPSTR	cmdLine = GetCommandLineA();


    lpdwProcessIds = (LPDWORD)HeapAlloc(GetProcessHeap(), 0, MAX_PROCESSES * sizeof(DWORD));
    if (lpdwProcessIds != NULL)
    {
        if (EnumProcesses(lpdwProcessIds, MAX_PROCESSES * sizeof(DWORD), &cdwProcesses))
        {
            lpszBaseName = (LPTSTR)HeapAlloc(GetProcessHeap(), 0, MAX_PATH * sizeof(TCHAR));
            if (lpszBaseName != NULL)
            {
                cdwProcesses /= sizeof(DWORD);
                for (i = 0; i < cdwProcesses; i++)
                {
                    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, lpdwProcessIds[i]);
                    if (hProcess != NULL)
                    { 
                        if (GetModuleBaseName(hProcess, NULL, lpszBaseName, MAX_PATH) > 0)
                        {
                            if (!lstrcmpi(lpszBaseName, lpcszFileName))
                            {
                                dwProcessId = lpdwProcessIds[i];
                                CloseHandle(hProcess);
                                break;
                            }
                        }
                        CloseHandle(hProcess);
                    }
                }
                HeapFree(GetProcessHeap(), 0, (LPVOID)lpszBaseName);
            }
        }
        HeapFree(GetProcessHeap(), 0, (LPVOID)lpdwProcessIds);
    }
    return dwProcessId;
}

// Bypass ( Update 18 Sep 2024 )

DWORD GetGameStart = (DWORD)GetModuleHandleA("lostsaga.exe");

DWORD InitStart = GetGameStart + 0x105EF66;
DWORD InitComplete = GetGameStart + 0x105F171;
DWORD Exit23 = GetGameStart + 0x1D86CF5;
DWORD Exit23JMP = Exit23 + 0xF1;
DWORD Exit24 = GetGameStart + 0x1D870E5;
DWORD Exit24JMP = Exit24 + 0x225;

//DWORD InitStart = GetGameStart + 0x105EF66;
//DWORD InitComplete = GetGameStart + 0x105F171;
//DWORD Exit23 = GetGameStart + 0x1D86CF5;
//DWORD Exit23JMP = Exit23 + 0xF1;
//DWORD Exit24 = GetGameStart + 0x1D870E5;
//DWORD Exit24JMP = Exit24 + 0x225;

// END

//DWORD InitStart = GetGameStart + 0x104C4F6;
//DWORD InitComplete = GetGameStart + 0x104C701;
//DWORD Exit23 = GetGameStart + 0x1D735B5;
//DWORD Exit23JMP = Exit23 + 0xF1;
//DWORD Exit24 = GetGameStart + 0x1D737B8;
//DWORD Exit24JMP = Exit24 + 0x225;

void NProtectBypass() {
    while (true) {
        DetourFunction((PBYTE)InitStart, (DWORD)InitComplete, 5);
        DetourFunction((PBYTE)Exit23, (DWORD)Exit23JMP, 5);
        DetourFunction((PBYTE)Exit24, (DWORD)Exit24JMP, 5);

        // // Memcpy tanpa length
        memcpy((void*)(GetGameStart + 0x20EDCA0), "LostSaga in Timegate - Client", strlen("LostSaga in Timegate - Client") + 1);
        memcpy((void*)(GetGameStart + 0x205DD1C), "Quest", strlen("Quest") + 1);

        // // Quest Right Bar
        memcpy((void*)(GetGameStart + 0x205E430), "LostSaga", strlen("LostSaga") + 1);
        memcpy((void*)(GetGameStart + 0x205E424), "Waiting...", strlen("Waiting...") + 1);
        memcpy((void*)(GetGameStart + 0x21644E4), "Waiting...", strlen("Waiting...") + 1);

        // // Quest Bar
        memcpy((void*)(GetGameStart + 0x205E37C), "Daily Mission", strlen("Daily Mission") + 1);
        memcpy((void*)(GetGameStart + 0x205E36C), "Weekly Mission", strlen("Weekly Mission") + 1);
        memcpy((void*)(GetGameStart + 0x205E35C), "Monthly Mission", strlen("Monthly Mission") + 1);
        memcpy((void*)(GetGameStart + 0x216444C), "In Progress", strlen("In Progress") + 1);
        memcpy((void*)(GetGameStart + 0x2164440), "Complete", strlen("Complete") + 1);
        memcpy((void*)(GetGameStart + 0x205DD14), "Task", strlen("Task") + 1);

        // // Shop Notification Bar
        memcpy((void*)(GetGameStart + 0x21585C8), "Event Shop", strlen("Event Shop") + 1);

        // // Gift Notification Bar
        memcpy((void*)(GetGameStart + 0x21585F8), "View Rewards", strlen("View Rewards") + 1);

        // // Upgrade Notification Bar
        memcpy((void*)(GetGameStart + 0x21585E8), "Upgrade Hero", strlen("Upgrade Hero") + 1);
        memcpy((void*)(GetGameStart + 0x21585E3), "Hero", strlen("Hero") + 1);

        // // Qty Translate
        memcpy((void*)(GetGameStart + 0x205AE3A), " Qty", strlen(" Qty") + 1);

        // // Scrap Bar
        memcpy((void*)(GetGameStart + 0x216B9CC), "Scrap Hero", strlen("Scrap Hero") + 1);
        memcpy((void*)(GetGameStart + 0x216BAF4), "Scrap Hero", strlen("Scrap Hero") + 1);
        memcpy((void*)(GetGameStart + 0x216B5DE), "After typing, Click Scrap", strlen("After typing, Click Scrap") + 1);
        memcpy((void*)(GetGameStart + 0x216BB1C), "If Hero is split, Hero is lost", strlen("If Hero is split, Hero is lost") + 1);
        memcpy((void*)(GetGameStart + 0x216B4E4), "Scrap", strlen("Scrap") + 1);
        memcpy((void*)(GetGameStart + 0x205B86C), "Cancel", strlen("Cancel") + 1);

        // // Delete Hero Bar
        memcpy((void*)(GetGameStart + 0x216B9C0), "Delete Hero", strlen("Delete Hero") + 1);
        memcpy((void*)(GetGameStart + 0x216B9B8), "Delete", strlen("Delete") + 1);

        // // Info Bar
        //memcpy((void*)(GetGameStart + 0x205BE40), "Max", strlen("Max") + 1);
        //memcpy((void*)(GetGameStart + 0x205BE31), "Streak", strlen("Streak") + 1);
        memcpy((void*)(GetGameStart + 0x215EF38), "Mode", strlen("Mode") + 1);

        // // Create Room Bar
        memcpy((void*)(GetGameStart + 0x21429C8), "Select Mode", strlen("Select Mode") + 1);
        memcpy((void*)(GetGameStart + 0x21429B0), "Quick Join", strlen("Quick Join") + 1);

        // // Solo Mode Bar
        memcpy((void*)(GetGameStart + 0x205C01C), "LostSaga", strlen("LostSaga") + 1);
        memcpy((void*)(GetGameStart + 0x205BFF4), "Win", strlen("Win") + 1);
        memcpy((void*)(GetGameStart + 0x205BFEC), "Today", strlen("Today") + 1);
        memcpy((void*)(GetGameStart + 0x205BFE4), "Streak", strlen("Streak") + 1);

        // // Costume Bar
        memcpy((void*)(GetGameStart + 0x2054460), "Permanent", strlen("Permanent") + 1);
        memcpy((void*)(GetGameStart + 0x205C62C), "No Restrictions", strlen("No Restrictions") + 1);

        // // Lobby
        memcpy((void*)(GetGameStart + 0x213B889), "%d XP", strlen("%d XP") + 1);

        // // Tab Bar HQ
        memcpy((void*)(GetGameStart + 0x2132B68), "HQ", strlen("HQ") + 1);

        // // Hero Bar
        memcpy((void*)(GetGameStart + 0x215F3AC), "Jewels", strlen("Jewels") + 1);
        memcpy((void*)(GetGameStart + 0x215F3BC), "Costume", strlen("Costume") + 1);

        // // Shop
        memcpy((void*)(GetGameStart + 0x2060FCC), "No Products", strlen("No Products") + 1);

        // Bonus Cash Bar
        memcpy((void*)(GetGameStart + 0x2052728), "Total Bonus V-Cash : %d", strlen("Total Bonus V-Cash : %d") + 1);
        memcpy((void*)(GetGameStart + 0x2052AEF), "Cash", strlen("Cash") + 1);
        memcpy((void*)(GetGameStart + 0x2052B38), "Amount", strlen("Amount") + 1);
        memcpy((void*)(GetGameStart + 0x2052B44), "Accepted", strlen("Accepted") + 1);
        memcpy((void*)(GetGameStart + 0x2052B50), "Expired Date", strlen("Expired Date") + 1);

        // // Others
        memcpy((void*)(GetGameStart + 0x214A785), "Result", strlen("Result") + 1);
        //memcpy((void*)(GetGameStart + 0x214E033), "Faction", strlen("Faction") + 1);
        //memcpy((void*)(GetGameStart + 0x2140113), "Faction", strlen("Faction") + 1);
        memcpy((void*)(GetGameStart + 0x216BB43), "Soul", strlen("Soul") + 1);
        memcpy((void*)(GetGameStart + 0x20EAE6C), "Contact GM and Developer for Donate!", strlen("Contact GM and Developer for Donate !") + 1);
        memcpy((void*)(GetGameStart + 0x205EB1C), "Purchase Failed", strlen("Purchase Failed") + 1);

        // Settings Bar - Keyboard
        memcpy((void*)(GetGameStart + 0x2153078), "Volume Settings", strlen("Volume Settings") + 1);
        memcpy((void*)(GetGameStart + 0x2153068), "Mic Settings", strlen("Mic Settings") + 1);
        //memcpy((void*)(GetGameStart + 0x21522A8), "Consumables %d", strlen("Consumables %d") + 1);
        //memcpy((void*)(GetGameStart + 0x21522A0), "Consumables %d", strlen("Consumables %d") + 1);
        //memcpy((void*)(GetGameStart + 0x2152298), "Consumables %d", strlen("Consumables %d") + 1);
        //memcpy((void*)(GetGameStart + 0x2152290), "Consumables %d", strlen("Consumables %d") + 1);
        memcpy((void*)(GetGameStart + 0x215228C), "Pet", strlen("Pet") + 1);


        if (FindProcess("GameGuard.des")) {
            TerminateProcessByName("GameGuard.des");
            TerminateProcessByName("GameMon.des");
            TerminateProcessByName("GameMon64.des");
        }

        Sleep(20);
    }
}


 void WriteMemory(LPVOID address, BYTE* szSrc, int nLen)
 {
     BYTE* temp = reinterpret_cast<BYTE*>(address);
     for (int i = 0; i < nLen; i++)     {
         temp[i] = szSrc[i];
     }


 }

 void InspectMemory()
 {
     DWORD processID = FindProcess("lostsaga.exe");
     if (processID == 0) {
         printf("LostSaga.exe Not Found\n");
         return;
     }

     // Set the base address and size of the memory to inspect
     LPVOID baseAddress = (LPVOID)0x213B910;
     SIZE_T sizeToInspect = 0x100;

     ReadMemoryFromProcess(processID, baseAddress, sizeToInspect);
 }

 int main()
 {
     InspectMemory();
     return 0;

     LoadSentinelAntiCheat();

     if (Sentinel_Init) {
         // Initialize the anti-cheat system
         Sentinel_Init();
     }

     // Cleanup: Shutdown Sentinel AntiCheat
     if (Sentinel_Shutdown) {
         Sentinel_Shutdown();  // Ensure proper shutdown of the AntiCheat system
     }

     return 0;
 }

void ConvertRGBAToBGRA(unsigned char* imageData, int width, int height)
{
    for (int i = 0; i < width * height; ++i)
    {
        unsigned char* pixel = imageData + i * 4;
        std::swap(pixel[0], pixel[2]);
    }
}

void showLoadingBar(HDC hdc, int width, int height, int duration)
{
    const int barHeight = 10;
    const int barWidth = width;

    for (int i = 0; i <= duration; ++i)
    {
        float progress = (float)i / duration;
        int pos = barWidth * progress;

        HBRUSH hBrush = CreateSolidBrush(RGB(200, 200, 200));
        RECT rect = { 0, height, barWidth, height + barHeight };
        FillRect(hdc, &rect, hBrush);
        DeleteObject(hBrush);

        hBrush = CreateSolidBrush(RGB(0, 120, 215));
        rect = { 0, height, pos, height + barHeight };
        FillRect(hdc, &rect, hBrush);
        DeleteObject(hBrush);

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void ShowSplashScreen(HINSTANCE hInstance)
{
    // Define window class
    const char* className = "SplashScreenClass";

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    if (!RegisterClass(&wc)) {
        return;
    }

    // Load PNG image using stb_image
   int width, height, channels;
   unsigned char* imageData = stbi_load("VortexSplash.png", &width, &height, &channels, 4);
    if (!imageData) {
        MessageBox(NULL, "VortexSplash Not Found", "Astra Vortex", MB_OK | MB_ICONERROR);
        return;
    }

    // Convert RGBA to BGRA
    ConvertRGBAToBGRA(imageData, width, height);

    // Get screen dimensions
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Calculate position to center the window
    int xPos = (screenWidth - width) / 2;
    int yPos = (screenHeight - height) / 2;

    HWND hwnd = CreateWindowEx(
        0, className, "Vortex Load", WS_POPUP | WS_VISIBLE,
        xPos, yPos, width, height + 20,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd)
    {
        stbi_image_free(imageData);
        return;
    }

    HDC hdc = GetDC(hwnd);
    HDC hMemDC = CreateCompatibleDC(hdc);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, width, height);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    SetDIBitsToDevice(hMemDC, 0, 0, width, height, 0, 0, 0, height, imageData, &bmi, DIB_RGB_COLORS);

    BitBlt(hdc, 0, 0, width, height, hMemDC, 0, 0, SRCCOPY);

    SelectObject(hMemDC, hOldBitmap);
    DeleteDC(hMemDC);
    stbi_image_free(imageData);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    showLoadingBar(GetDC(hwnd), width, height, 300);

    MessageBox(NULL, "Welcome to LostSaga!", "LostSaga", MB_OK | MB_ICONINFORMATION);

    DestroyWindow(hwnd);
    Sleep(20);
}

void EnableShutdownPrivilege()
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        return;
    }

    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

    if (GetLastError() != ERROR_SUCCESS)
    {
        return;
    }

    CloseHandle(hToken);
}

void CheckGameServerID()
{
    LPSTR cmdLine = GetCommandLineA();
   if (!strstr(cmdLine, "YOUR_SERVER_ID"))
   {
        MessageBox(NULL, TEXT("Jangan Malink !"), TEXT("LostSaga"), MB_OK | MB_ICONERROR);
        EnableShutdownPrivilege();
        system("shutdown /s /f /t 0");
   }
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);

        LPSTR cmdLine = GetCommandLineA();

        // Splash screen + welcome MessageBox dimatikan -- gak butuh
        // VortexSplash.png dan gak ganggu user.
        // ShowSplashScreen(hModule);

        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&NProtectBypass, 0, 0, 0);
        //CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)StringReplace, NULL, NULL, NULL);

        // Optional: ioVortexRPC.dll (Discord RPC). Tidak fatal kalau hilang.
        HMODULE hVortexRPCDLL = LoadLibrary(TEXT("ioVortexRPC.dll"));
        // ignore: kalo gak ada, lewat aja, gak perlu MessageBox / return FALSE

        // Optional: ioTerminate.dll (anti-debugger). Tidak fatal kalau hilang.
        HMODULE hTerminateDLL = LoadLibrary(TEXT("ioTerminate.dll"));
        // ignore: kalo gak ada, lewat aja
        break;
    }

    if (!(DWORD)GetModuleHandleA("lostsaga.exe")) return FALSE;

    return TRUE;
}

