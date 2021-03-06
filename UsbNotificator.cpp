// UsbNotificator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <devguid.h>
#include <Dbt.h>
#include <iostream>
#include <vector>
#include <SetupAPI.h>
#include <codecvt>

#pragma comment(lib,"Setupapi.lib")
using namespace std;

WNDPROC wndProc = NULL;
HDEVNOTIFY devNotify = NULL;
WNDCLASSEX  WindowClassEx;
vector <wstring> currentList;
HANDLE hConsole = NULL;
WORD attributes = 0;

wstring stringToWstring(const char* utf8Bytes)
{
    using convert_type = std::codecvt_utf8<typename std::wstring::value_type>;
    std::wstring_convert<convert_type, typename std::wstring::value_type> converter;
    return converter.from_bytes(utf8Bytes);
}

wstring getData(HDEVINFO hDevInfo, SP_DEVINFO_DATA deviceInfoData, DWORD SPDRP) {

    DWORD dataT;
    char value[2048] = { 0 };
    DWORD buffersize = 2048;
    DWORD req_bufsize = 0;

    if (!SetupDiGetDeviceRegistryPropertyA(hDevInfo, &deviceInfoData, SPDRP, &dataT, (PBYTE)value, buffersize, &req_bufsize))
    {
        return NULL;
    }

    return stringToWstring(value);
}

vector<wstring> loadDevices() {

    vector<wstring> vector;

    DEVPROPTYPE devPropType;
    SP_DEVINFO_DATA DeviceInfoData = { sizeof(DeviceInfoData) };

    HDEVINFO hDevInfo = SetupDiGetClassDevs(
        &GUID_DEVCLASS_USB,
        NULL,
        NULL,
        DIGCF_PRESENT);

    if (hDevInfo == INVALID_HANDLE_VALUE)
    {
        printf("Invalid handle value", -2);
        exit(-2);
    }

    for (int i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); i++)
    {

        wstring description = getData(hDevInfo, DeviceInfoData, SPDRP_DEVICEDESC);
        wstring hardwareid = getData(hDevInfo, DeviceInfoData, SPDRP_HARDWAREID);

        hardwareid = hardwareid + L"::" + description;
        vector.push_back(hardwareid);
    }

    return vector;
}


void messageReceiver(HWND hWnd)
{
    MSG msg;
    int retVal;

    while ((retVal = GetMessage(&msg, NULL, 0, 0)) != 0)
    {
        if (retVal == -1)
        {
            //ErrorHandler(L"GetMessage");
            break;
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}


wstring findAddedDevice(vector<wstring> newDevices) {

    for (int i = 0; i < newDevices.size(); i++)
    {
        bool isFound = false;
        for (int z = 0; z < currentList.size(); z++)
        {
            if (currentList[z] == newDevices[i]) {
                isFound = true;
                break;
            }
        }

        if (!isFound) {
            currentList = newDevices;
            return newDevices[i];
        }
    }

    return NULL;
}

wstring findRemovedDevice(vector<wstring> newDevices) {

    for (int i = currentList.size() -1; i >= 0; i--)
    {
        bool isFound = false;
        for (int z = 0; z < newDevices.size(); z++)
        {
            if (newDevices[z] == currentList[i]) {
                isFound = true;
                break;
            }
        }

        if (!isFound) {

            wstring d(currentList[i]);
            currentList = newDevices;
            return d;
        }
    }

    return NULL;
}



BOOL DoRegisterDeviceInterfaceToHwnd(
    IN GUID InterfaceClassGuid,
    IN HWND hWnd,
    OUT HDEVNOTIFY *hDeviceNotify
)
{
    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

    ZeroMemory(&NotificationFilter, sizeof(NotificationFilter));
    NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    NotificationFilter.dbcc_classguid = InterfaceClassGuid;

    *hDeviceNotify = RegisterDeviceNotification(
        hWnd,                       // events recipient
        &NotificationFilter,        // type of device
        DEVICE_NOTIFY_WINDOW_HANDLE // type of recipient handle
    );

    if (NULL == *hDeviceNotify)
    {
        return FALSE;
    }

    return TRUE;
}

INT_PTR WINAPI WinProcCallback(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    LRESULT lRet = 1;
    static HDEVNOTIFY hDeviceNotify;
    static HWND hEditWnd;
    static ULONGLONG msgCount = 0;

    switch (message)
    {
    case WM_CREATE:
        if (!DoRegisterDeviceInterfaceToHwnd(
            GUID_DEVCLASS_USB,
            hWnd,
            &hDeviceNotify))
        {
            // Terminate on failure.
            //ErrorHandler(TEXT("DoRegisterDeviceInterfaceToHwnd"));
            ExitProcess(1);
        }
        break;

    case WM_DEVICECHANGE:
        switch (wParam)
        {
        case DBT_DEVICEARRIVAL:
        {
            printf("Device Connected\n");
            wstring added = findAddedDevice(loadDevices());
            wprintf(L"%s\n", added.c_str());
            printf("Detect: %d Devices\n", currentList.size());
        }
        break;
        case DBT_DEVICEREMOVECOMPLETE: {
            printf("Device Remove\n");
            wstring added = findRemovedDevice(loadDevices());
            wprintf(L"%s\n", added.c_str());
            printf("Detect: %d Devices\n", currentList.size());
        }
        break;
        case DBT_DEVNODES_CHANGED:
            break;
        default:
            break;
        }
        break;

    case WM_CLOSE:
        UnregisterDeviceNotification(hDeviceNotify);
        DestroyWindow(hWnd);
        break;
    default:
        lRet = DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return lRet;
}


BOOL createHiddenWindow(HINSTANCE hInstanceExe) {


    HINSTANCE histance = hInstanceExe;
    ZeroMemory(&WindowClassEx, sizeof(WNDCLASSEX));
    /*********/
    WindowClassEx.cbSize = sizeof(WNDCLASSEX);
    WindowClassEx.hInstance = reinterpret_cast<HINSTANCE>(GetModuleHandle(0));
    WindowClassEx.lpfnWndProc = reinterpret_cast<WNDPROC>(WinProcCallback);
    WindowClassEx.hInstance = histance;
    WindowClassEx.lpszClassName = L"dummy";
    /*********/
    if (RegisterClassEx(&WindowClassEx) != 0)
    {
        return TRUE;
    }
    return FALSE;

}


void showHeader() {
    SetConsoleTextAttribute(hConsole,
        FOREGROUND_BLUE | FOREGROUND_BLUE | FOREGROUND_INTENSITY);

    printf("   __  _______ ____  _   __      __  _ _____          \n");
    printf("  / / / / ___// __ )/ | / /___  / /_(_) __(_)__  _____\n");
    printf(" / / / /\\__ \\/ __  /  |/ / __ \\/ __/ / /_/ / _ \\/ ___/\n");
    printf("/ /_/ /___/ / /_/ / /|  / /_/ / /_/ / __/ /  __/ /    \n");
    printf("\\____//____/_____/_/ |_/\\____/\\__/_/_/ /_/\\___/_/     \n");
    printf("----------------------------------------------------------\n");
    printf("[USBNotifier] by: RudeNetworks.com | version: 0.1 beta\n");
    printf(" - administrator privileges required.\n\n");
}

BOOL WINAPI consoleHandler(DWORD signal) {

    if (signal == CTRL_C_EVENT)
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), attributes);

    return FALSE;
}

int main(_In_ HINSTANCE hInstanceExe,
    _In_opt_ HINSTANCE, // should not reference this parameter
    _In_ PTSTR lpstrCmdLine,
    _In_ int nCmdShow)
{

    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO Info;
    GetConsoleScreenBufferInfo(hConsole, &Info);
    attributes = Info.wAttributes;

    SetConsoleCtrlHandler(consoleHandler, TRUE);

    showHeader();

    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);

    currentList = loadDevices();

    printf("Detect: %d Devices\n", currentList.size());

    printf("Register notification filter, OK!\n");

    DEV_BROADCAST_DEVICEINTERFACE notificationFilter;

    memset(&notificationFilter, 0, sizeof(DEV_BROADCAST_DEVICEINTERFACE));

    notificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    notificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    notificationFilter.dbcc_classguid = GUID_DEVCLASS_USB;

    devNotify =
        RegisterDeviceNotification(GetConsoleWindow(),
            &notificationFilter,
            DEVICE_NOTIFY_ALL_INTERFACE_CLASSES);


    printf("Create hidden window and connect to procedure, OK!\n");
    printf("\n");
    printf("\n");
    if (createHiddenWindow(hInstanceExe)) {
        HWND hWnd = CreateWindowEx(
            WS_EX_CLIENTEDGE | WS_EX_APPWINDOW,
            WindowClassEx.lpszClassName,
            NULL,
            WS_OVERLAPPEDWINDOW, // style
            CW_USEDEFAULT, 0,
            0, 0,
            NULL, NULL,
            hInstanceExe,
            NULL);

        ShowWindow(hWnd, SW_HIDE);

        messageReceiver(hWnd);
    }

   

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), attributes);
    return 1;
}

