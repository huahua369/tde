// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "td.h"

//extern "C"
BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		td_on();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		// 不需要卸载
		//td_off();
		break;
	}
	return TRUE;
}
