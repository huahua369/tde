// td.cpp : 定义 DLL 的导出函数。
//

#include "pch.h"
#include "framework.h"
#include <string>
#include <map>
#include <set>
#include <unordered_set>
#include <thread>
#include <mmreg.h>
#define CINTERFACE

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
#include <dsound.h>

#include <Shlwapi.h>
#include <Tlhelp32.h>
#ifdef __cplusplus
}
#endif // __cplusplus
#include <detours/detours.h>
#include "rdll.h"
#include "td.h"
//#pragma comment(lib,"detours.lib")

std::thread mthr;
std::map<PVOID*, PVOID> funs;
// old void**,my void*


int msbox_oks(const std::string& text, const std::string& title)
{
	return MessageBoxA(GetForegroundWindow(), text.c_str(), title.c_str(), MB_OK);
}
int msbox_okcancels(const std::string& text, const std::string& title)
{
	return MessageBoxA(GetForegroundWindow(), text.c_str(), title.c_str(), MB_OKCANCEL);
}
void push_fun(void** old, void* fun)
{
	funs[old] = fun;
}
//njson kg;
void out_data(const char* key, void* data, int len)
{
	//kg[key] = hz::Base64::encode((unsigned char*)data, len);
	std::string logs;
	if (key && *key)
	{
		logs = key;
		logs += "\r\n";
	}
	if (data)
	{
		if (len > 0)
		{
			logs.append((char*)data, len);
		}
		else
		{
			logs.append((char*)data);
		}
		logs += "\r\n";
	}
	if (logs.size())
	{
		// TODO 保存到日志
		//File::save_binary_file("td.log", (char*)logs.c_str(), logs.size(), 0, true);
		//save_json(kg, "akg.json");
	}
}
char* i2s(int d, int idx)
{
	static char buf[128] = {};
	memset(buf, 0, 128);
	return itoa(d, buf, idx);
}
void td_main();
void td_on()
{
	std::thread t([]() {td_main(); });
	mthr.swap(t);
	mthr.detach();
}

void td_off()
{
	HookOff(funs);
}

int msbox_ok(const char* str, const char* title)
{
	return msbox_oks(str, title);
}

int msbox_okcancel(const char* str, const char* title)
{
	return msbox_okcancels(str, title);
}
// 区别每个播放对象
std::unordered_set<void*> dsbt;
//LockS lock;
#define XA (x++)

void outUnlock(void* t, LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2)
{
	out_data("Unlock", 0, 0);
#if 0
	char buf[1024] = { 0 };
	njson info;
	if (dsbt.find(t) == dsbt.end())
	{
		LOCK_W(lock);
		dsbt.insert(t);
		// 第一次创建播放
		int sout[20] = { 0 };
		int x = 0;
		get_format(t, sout, 9);
		info["format"] = sout[XA];        /* format type */
		info["channels"] = sout[XA];         /* number of channels (i.e. mono, stereo...) */
		info["sample_rate"] = sout[XA];    /* sample rate */
		info["AvgBytesPerSec"] = sout[XA];   /* for buffer estimation */
		info["BlockAlign"] = sout[XA];       /* block size of data */
		info["BitsPerSample"] = sout[XA];    /* Number of bits per sample of mono data */
		info["cbSize"] = sout[XA];
		out_data(0, (void*)info.dump(2).c_str(), 0);
	}
	sprintf(buf, "%p\taptr1\t%p,%d aptr2\t%p,%d\r\n", t, pvAudioPtr1, (int)dwAudioBytes1, pvAudioPtr2, (int)dwAudioBytes2);
	out_data(0, buf, 0);
	if (dwAudioBytes1 > 0)
	{
		std::string fn = i2s((size_t)t, 16);
		File::save_binary_file("temp/" + fn + ".pcm", (char*)pvAudioPtr1, dwAudioBytes1, 0, true);
	}
#endif
}
void outRelease(void* t)
{
	out_data("Release", i2s((size_t)t, 16), 0);
}
void td_main()
{
	auto hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_SPEED_OVER_MEMORY);
	auto fw = GetForegroundWindow();
	auto aw = GetActiveWindow();
	//msbox_ok(i2s(__LINE__), "td_on");
	try
	{
		//msbox_ok(i2s(__LINE__), "td_on");
		dcom_init();
	}
	catch (const std::exception & e)
	{
		//msbox_oks(e.what(), "dcom_init error");
		exit(-1);
	}
	// 替换hook函数
	HookOn(funs);
	while (1)
	{
		Sleep(1);
	}
}
