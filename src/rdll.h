#pragma once 
#ifdef __DLL_EXPORT__
#define ADAPTER_C_API extern"C" __declspec(dllexport)
#define ADAPTER_CPP_API __declspec(dllexport)
#else
#define ADAPTER_C_API extern"C" __declspec(dllimport)
#define ADAPTER_CPP_API __declspec(dllimport)
#endif
ADAPTER_C_API BOOL WINAPI PathFileExistsW(LPCWSTR pszPath);
int test_run();

void HookOff(std::map<PVOID*, PVOID>& funs);
void HookOn(std::map<PVOID*, PVOID>& funs);

class rd_t
{
public:
	rd_t()
	{
	}

	~rd_t()
	{
	}
	static DWORD GetProcessIdByName(const std::string& szProcessName);
	static int init(std::string pen, std::string dllpath);
	static void writedll(HANDLE hProcess, const wchar_t* lpszDll);
};
