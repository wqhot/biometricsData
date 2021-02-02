// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <conio.h>
#include <windows.h>
#include <chrono>

#import "C:\\Program Files (x86)\\Biometrics Ltd\\DataLINK\\OnLineInterface.dll" no_namespace
IOnLine* pOnline;	// Declare a global pointer to the interface class instance

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        OleInitialize(NULL);
        CoCreateInstance(__uuidof(OnLine), NULL, CLSCTX_INPROC_SERVER, __uuidof(IOnLine), (void**)&pOnline);
        if (pOnline == NULL)
        {
            printf("Cannot find OnLineInterface.dll.\n\n"
                "Please ensure that DataLINK was correctly installed from\n"
                "the original CD as this also registers the DLL with Windows.");
        }
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


long get_data(int ch, int num, int rate, short *data, double &stamp)
{
    long sampleNum;
    int sizems = int(num * 1000.0 / (float)rate);
    SAFEARRAY* pSafeArrayZeroDim = SafeArrayCreate(VT_I2, 0, 0);
    SAFEARRAY* pSafeArray = pSafeArrayZeroDim;
    if ((pOnline->OnLineGetData(ch, sizems, &pSafeArray, &sampleNum) == ONLINE_OK) &&
        (sampleNum > 0))
    {
        std::chrono::system_clock::duration d = std::chrono::system_clock::now().time_since_epoch();
        std::chrono::nanoseconds nan = std::chrono::duration_cast<std::chrono::nanoseconds>(d);
        stamp = nan.count() / 1000000000.0;
        short* pValue;
        if (SafeArrayAccessData(pSafeArray, (LPVOID*)&pValue) == S_OK)
        {
            for (int sample = 0; sample < sampleNum; sample++, pValue++)
            {
                *(data + sample) = *pValue;
            }
        }

    }
    return sampleNum;
}

void bio_usleep(int us)
{
    HANDLE timer;
    LARGE_INTEGER ft;

    ft.QuadPart = -(10 * us); // Convert to 100 nanosecond interval, negative value indicates relative time

    timer = CreateWaitableTimer(NULL, TRUE, NULL);
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);
}
