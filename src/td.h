// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 TD_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// TD_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef TD_EXPORTS
#define TD_C_API extern"C" __declspec(dllexport)
#define TD_CPP_API __declspec(dllexport)
#define TD_N_API __declspec(dllexport)
#else
#define TD_C_API extern"C" __declspec(dllimport)
#define TD_CPP_API __declspec(dllimport)
#endif


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	void td_on();
	void td_off();

	LRESULT HookProc_(int nCode, WPARAM wParam, LPARAM lParam);

	int msbox_ok(const char* str, const char* title);
	int msbox_okcancel(const char* str, const char* title);
	void push_fun(void** old, void* fun);
	void out_data(const char* key, void* data, int len);

	TD_N_API void dcom_init();
	char* i2s(int d, int idx);
	void get_format(void* vt, int* out, int c);
	void outRelease(void* t);
	void outUnlock(void* t, LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2);
#ifdef __cplusplus
}
#endif // __cplusplus
