# tde
exe使用用OpenProcess、CreateRemoteThread在目标进程创建线程执行LoadLibrary加载dll。
#
dll使用detours替换函数，支持com接口。
#
dcom.c是dsound.dll的com接口例子
