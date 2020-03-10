
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

int test_run()
{
	auto tdstr = R"(E:\code\c\td\bin\td1.dll)";
	rd_t::init("notepad.exe", tdstr);
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
