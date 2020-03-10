
#include "pch.h"
#include <string>
#include <map>
#include <set>
#include <detours/detours.h>
#include "rdll.h"
#include <Shlwapi.h>
#include <Tlhelp32.h>
//#pragma comment(lib,"detours.lib")
#pragma comment(lib,"Shlwapi.lib")
void HookOn();
void HookOff();

static int(WINAPI* OldMesssageBoxA)(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) = MessageBoxA;
static int (WINAPI* OldMesssageBoxW)(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType) = MessageBoxW;
int WINAPI MyFunction0(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
int WINAPI MyFunction1(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);

BOOL mPathFileExistsW(LPCWSTR str)
{
	return PathFileExistsW(str);
}

// 调用被 HOOK 的函数可以用被 HOOK 函数的指针，不能用原函数。
int WINAPI MyFunction0(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{

	std::string str = "Hooking your MessageBoxA! ", t = "Warming: ";
	if (lpText)
	{
		str += lpText;
	}
	if (lpCaption)
	{
		t += lpCaption;
	}
	return OldMesssageBoxA(NULL, str.c_str(), t.c_str(), MB_OKCANCEL);
}
int WINAPI MyFunction1(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
	std::wstring str = L"Hooking your MessageBoxA! ", t = L"Warming: ";
	if (lpText)
	{
		str += lpText;
	}
	if (lpCaption)
	{
		t += lpCaption;
	}
	return OldMesssageBoxW(NULL, str.c_str(), t.c_str(), MB_OKCANCEL);
}

void HookOn(std::map<PVOID*, PVOID>& funs)
{
	//开始事务
	DetourTransactionBegin();
	//更新线程信息  
	auto tid = GetCurrentThread();
	DetourUpdateThread(tid);
	//将拦截的函数附加到原函数的地址上,这里可以拦截多个函数。
	for (auto& [k, v] : funs)
	{
		DetourAttach(k, v);
	}
	//DetourAttach(&(PVOID&)OldMesssageBoxW, MyFunction1);
	//结束事务
	DetourTransactionCommit();
}

void HookOff(std::map<PVOID*, PVOID>& funs)
{
	//开始事务
	DetourTransactionBegin();
	//更新线程信息 
	DetourUpdateThread(GetCurrentThread());
	//将拦截的函数从原函数的地址上解除，这里可以解除多个函数。
	//DetourDetach(&(PVOID&)OldMesssageBoxA, MyFunction0);
	for (auto& [k, v] : funs)
	{
		DetourDetach(k, v);
	}
	//结束事务
	DetourTransactionCommit();
}


DWORD rd_t::GetProcessIdByName(const std::string& szProcessName)
{
	STARTUPINFO st;
	PROCESS_INFORMATION pi;
	PROCESSENTRY32 ps;
	HANDLE hSnapshot;
	DWORD dwPID = 0;
	ZeroMemory(&st, sizeof(STARTUPINFO));
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
	st.cb = sizeof(STARTUPINFO);
	ZeroMemory(&ps, sizeof(PROCESSENTRY32));
	ps.dwSize = sizeof(PROCESSENTRY32);
	// 遍历进程  
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		return 0;
	}
	if (!Process32First(hSnapshot, &ps))
	{
		return 0;
	}
	std::set<std::string> lst;
	do
	{
		lst.insert(ps.szExeFile);
		// 比较进程名  
		if (lstrcmpiA(ps.szExeFile, szProcessName.c_str()) == 0)
		{
			// 找到了  
			dwPID = ps.th32ProcessID;
			printf("%s:%d\n", ps.szExeFile, dwPID);
			break;
		}
	} while (Process32Next(hSnapshot, &ps));
	// 没有找到  
	//for (auto& it : lst)
	//{
	//	printf("%s\n", it.c_str());
	//}
	if (lst.empty())
	{
		printf("空\n");
	}
	CloseHandle(hSnapshot);
	return dwPID;
}

int rd_t::init(std::string pen, std::string dllpath)
{
	//打开目标进程句柄
	HANDLE hProcess = NULL;
	DWORD dwProcessId = 0;
	dwProcessId = GetProcessIdByName(pen);
	hProcess = OpenProcess(PROCESS_CREATE_THREAD |   //创建线程所必需的
		PROCESS_QUERY_INFORMATION |                //检索有关进程的某些信息
		PROCESS_VM_OPERATION |                     //需要对进程的地址空间执行操作
		PROCESS_VM_WRITE |                         //需要对进程的地址空间执行写操作
		PROCESS_VM_READ,                          //需要对进程的地址空间执行读操作
		FALSE,
		dwProcessId);

	//向目标进程中写入句柄
	//writedll(hProcess, dllpath.c_str());
	LPTSTR psLibFileRemote = NULL;
	int dwSize = dllpath.size() + 1;
	psLibFileRemote = (LPTSTR)VirtualAllocEx(hProcess,
		NULL,
		dwSize,
		MEM_COMMIT,
		PAGE_READWRITE);
	if (psLibFileRemote == NULL)
	{
		printf("VirtualAllocEx:%d\n", GetLastError());
		return FALSE;
	}
	BOOL bRet = WriteProcessMemory(hProcess,
		psLibFileRemote,
		(LPCVOID)dllpath.data(),
		dwSize,
		NULL);
	//获取LoadLibrary的地址
	PTHREAD_START_ROUTINE pfnStartAddr = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("Kernel32.dll"), "LoadLibraryA");
	if (pfnStartAddr == NULL)
	{
		printf("GetProcAddress %d\n", GetLastError());
		return FALSE;
	}
	//利用CreateRemoteThread函数调用LoadLibrary加载dll
	//CreateThreadThread
	HANDLE hThread = CreateRemoteThread(hProcess,
		NULL,
		0,
		pfnStartAddr,
		psLibFileRemote,
		0,
		NULL);
	printf("%p\n", hThread);
	// 等待LoadLibrary加载完毕  
	if (hThread)
		WaitForSingleObject(hThread, INFINITE);
	// 释放目标进程中申请的空间  
	VirtualFreeEx(hProcess, psLibFileRemote, dwSize, MEM_DECOMMIT);
	CloseHandle(hThread);
	CloseHandle(hProcess);
	//MessageBoxA(0, "123", "标题", MB_OKCANCEL);
	//hz::Shared::loadShared("");
	return 0;
}

void rd_t::writedll(HANDLE hProcess, const wchar_t* lpszDll)
{
	// 向目标进程地址空间写入DLL名称  
	size_t dwSize;
	size_t dwWritten;
	dwSize = lstrlenW(lpszDll) + 2;
	LPVOID lpBuf = VirtualAllocEx(hProcess, NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
	if (NULL == lpBuf)
	{
		CloseHandle(hProcess);
		// 失败处理  
	}
	if (WriteProcessMemory(hProcess, lpBuf, (LPVOID)lpszDll, dwSize, &dwWritten))
	{
		// 要写入字节数与实际写入字节数不相等，仍属失败  
		if (dwWritten != dwSize)
		{
			VirtualFreeEx(hProcess, lpBuf, dwSize, MEM_DECOMMIT);
			CloseHandle(hProcess);
			// 失败处理  
		}
	}
	else
	{
		CloseHandle(hProcess);
		// 失败处理  
	}
	// 使目标进程调用LoadLibrary，加载DLL  
	DWORD dwID;
	LPVOID pFunc = LoadLibraryW;
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0,
		(LPTHREAD_START_ROUTINE)pFunc, lpBuf, 0, &dwID);
	// 等待LoadLibrary加载完毕  
	WaitForSingleObject(hThread, INFINITE);
	// 释放目标进程中申请的空间  
	VirtualFreeEx(hProcess, lpBuf, dwSize, MEM_DECOMMIT);
	CloseHandle(hThread);
	CloseHandle(hProcess);
}
#if 0
typedef void(*dcom_init_func)();
int test_run()
{
	//MessageBoxA(0, "a", "tsxlsx", MB_OKCANCEL);
	//auto notepadhandle = FindWindow("tsxlsx", NULL);

	auto tdstr = R"(E:\code\libxlsx\libxlsx\bin\TD.dll)";
	auto dslib = LoadLibraryA("dsound.dll");
	auto td = LoadLibraryA(tdstr);
	//dcom_init_func dcom_init = (dcom_init_func)GetProcAddress(td, "dcom_init");
	//dcom_init();
	//rd_t::init("kugou.exe", tdstr);
	rd_t::init("qq.exe", tdstr);
	//auto kg=read_json( "akg.json");
	//printf();
	while (0)
	{
		sleep(1);
	}
	return 0;
}
#endif
