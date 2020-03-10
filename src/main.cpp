
//#include "pch.h"
#include <iostream>
#include <string>
#include <map>
#include <dsound.h>
#include <detours/detours.h>
#include "rdll.h"
#include <Shlwapi.h>
#include <Tlhelp32.h>
//#pragma comment(lib,"detours.lib")
typedef void(*dcom_init_func)();
int test_run()
{
	//MessageBoxA(0, "a", "tsxlsx", MB_OKCANCEL);
	auto tdstr = R"(E:\code\c\td\bin\td1.dll)";
	//auto dslib = LoadLibraryA("dsound.dll");
	//auto td = LoadLibraryA(tdstr);
	//dcom_init_func dcom_init = (dcom_init_func)GetProcAddress(td, "dcom_init");
	//dcom_init();
	//rd_t::init("kugou.exe", tdstr);
	//rd_t::init("qq.exe", tdstr);
	rd_t::init("notepad.exe", tdstr);
	//auto kg=read_json( "akg.json");
	//printf();
	while (0)
	{
		Sleep(1);
	}
	return 0;
}

int main()
{
#ifdef _WIN32
	system("color 00");
	SetConsoleTitleA("td.exe");
#endif
	test_run();
	std::cout << "Hello World!\n";
	system("pause");
	return 0;
}
