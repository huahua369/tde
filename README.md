# tde
### hook win32 pe
exe使用OpenProcess、CreateRemoteThread在目标进程创建线程执行LoadLibrary加载td.dll。

> dll使用detours来hook函数，支持com接口。

> 本例子源码dcom.c是hook dsound.dll的com接口例子，可以获取播放器音频数据。

可用于实现hook进程的音频数据、纹理数据、网络数据，只要知道函数声明和地址都可以hook。
