

#include "framework.h"

#include <mmreg.h>
#define CINTERFACE

#include <dsound.h>
#include <stdio.h>
#include <Shlwapi.h>
#include <Tlhelp32.h>


#include "td.h"
//#pragma comment(lib,"detours.lib")

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	typedef HRESULT(WINAPI* fnDirectSoundCreate8)(LPGUID, LPDIRECTSOUND*, LPUNKNOWN);
	typedef ULONG(WINAPI* fnRelease)(LPDIRECTSOUNDBUFFER);
	typedef HRESULT(WINAPI* fnSetFormat)(LPDIRECTSOUNDBUFFER, LPCWAVEFORMATEX pcfxFormat);
	typedef HRESULT(WINAPI* fnUnlock)(LPDIRECTSOUNDBUFFER, LPVOID pvAudioPtr1, DWORD dwAudioBytes1,
		_In_reads_bytes_opt_(dwAudioBytes2) LPVOID pvAudioPtr2, DWORD dwAudioBytes2);
	typedef HRESULT(WINAPI* fnLock)(LPDIRECTSOUNDBUFFER, DWORD dwOffset, DWORD dwBytes,
		_Outptr_result_bytebuffer_(*pdwAudioBytes1) LPVOID* ppvAudioPtr1, _Out_ LPDWORD pdwAudioBytes1,
		_Outptr_opt_result_bytebuffer_(*pdwAudioBytes2) LPVOID* ppvAudioPtr2, _Out_opt_ LPDWORD pdwAudioBytes2, DWORD dwFlags);

	fnDirectSoundCreate8 pDirectSoundCreate8 = 0;
	HMODULE dslib = 0;
	LPDIRECTSOUND ds = 0;

	LPDIRECTSOUNDBUFFER sbctx = 0;
	DSBUFFERDESC format = { 0 };
	WAVEFORMATEX wfmt = { 0 };

	fnLock pLock = 0;
	fnUnlock pUnlock = 0;
	fnSetFormat pSetFormat = 0;
	fnRelease pAddRef = 0;
	fnRelease pRelease = 0;

#ifdef __cplusplus
}
#endif // __cplusplus

ULONG WINAPI pmAddRef(LPDIRECTSOUNDBUFFER t)
{
	char buf[1024] = { 0 };
	sprintf(buf, "ptr\t%p\n", t);
	out_data("pmAddRef", buf, 0);
	return pRelease(t);
}
ULONG WINAPI pmRelease(LPDIRECTSOUNDBUFFER t)
{
	outRelease(t);
	return pRelease(t);
}
HRESULT WINAPI pmSetFormat(LPDIRECTSOUNDBUFFER t, LPCWAVEFORMATEX pcfxFormat)
{
	//out_data("pmSetFormat", pcfxFormat, sizeof(*pcfxFormat));
	return pSetFormat(t, pcfxFormat);
}
HRESULT WINAPI pmUnlock(LPDIRECTSOUNDBUFFER t, LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2)
{
	outUnlock(t, pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2);
	return pUnlock(t, pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2);
}
HRESULT WINAPI pmLock(LPDIRECTSOUNDBUFFER t, DWORD dwOffset, DWORD dwBytes,
	_Outptr_result_bytebuffer_(*pdwAudioBytes1) LPVOID* ppvAudioPtr1, _Out_ LPDWORD pdwAudioBytes1,
	_Outptr_opt_result_bytebuffer_(*pdwAudioBytes2) LPVOID* ppvAudioPtr2, _Out_opt_ LPDWORD pdwAudioBytes2, DWORD dwFlags)
{
	char buf[1024] = { 0 };
	sprintf(buf, "%p\t%p,%d,\taptr1\t%p,%d aptr2\t%p,%d", t, (void*)dwOffset, (int)dwBytes, ppvAudioPtr1, (int)pdwAudioBytes1, ppvAudioPtr2, (int)pdwAudioBytes2);
	//out_data("pmLock", buf, 0);
	return pLock(t, dwOffset, dwBytes, ppvAudioPtr1, pdwAudioBytes1, ppvAudioPtr2, pdwAudioBytes2, dwFlags);
}

#define XA (x++)
void get_format(void* vt, int* out, int c)
{
	DWORD freq = 0;
	long v = 0;
	int x = 0;
	WAVEFORMATEX wfmt;
	LPDIRECTSOUNDBUFFER t = (LPDIRECTSOUNDBUFFER)vt;
	//IDirectSoundBuffer_GetVolume(t, &v);
	//IDirectSoundBuffer_GetFrequency(t, &freq);
	IDirectSoundBuffer_GetFormat(t, &wfmt, sizeof(wfmt), NULL);//得到主缓冲区格式

	out[XA] = wfmt.wFormatTag;        /* format type */
	out[XA] = wfmt.nChannels;         /* number of channels (i.e. mono, stereo...) */
	out[XA] = wfmt.nSamplesPerSec;    /* sample rate */
	out[XA] = wfmt.nAvgBytesPerSec;   /* for buffer estimation */
	out[XA] = wfmt.nBlockAlign;       /* block size of data */
	out[XA] = wfmt.wBitsPerSample;    /* Number of bits per sample of mono data */
	out[XA] = wfmt.cbSize;            /* The count in bytes of the size of
									extra information (after cbSize) */
									//out[XA] = freq;
									//out[XA] = v;
}
HRESULT WINAPI pmdsc8(LPGUID g, LPDIRECTSOUND* s, LPUNKNOWN u)
{
	msbox_ok(GetForegroundWindow(), atoll(s), "标题");
	return pDirectSoundCreate8(g, s, u);
}
int create_sound_buf(LPDIRECTSOUNDBUFFER* pc)
{
	/* Try to create the secondary buffer */
	WAVEFORMATEX wfx;
	DSBUFFERDESC dsbdesc;
	LPDIRECTSOUNDBUFFER pDsb = NULL;
	HRESULT hr;
	// Set up WAV format structure.
	memset(&wfx, 0, sizeof(WAVEFORMATEX));
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = 2;
	wfx.nSamplesPerSec = 22050;
	wfx.nBlockAlign = 4;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
	wfx.wBitsPerSample = 16;
	// Set up DSBUFFERDESC structure.
	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY; //  流buffer
	dsbdesc.dwBufferBytes = 3 * wfx.nAvgBytesPerSec;
	dsbdesc.lpwfxFormat = &wfx;
	return IDirectSound_CreateSoundBuffer(ds, &dsbdesc, pc, NULL);
}

void dcom_init()
{
	while (!dslib) {
		dslib = GetModuleHandleA("dsound.dll");
		if (dslib)
		{
			break;
		}
		dslib = LoadLibraryA("dsound.dll");
		break;
	}
	if (!dslib)
	{
		msbox_ok("找不到dsound.dll模块！", "错误提示");
		return;
	}
	if (!pDirectSoundCreate8)
		pDirectSoundCreate8 = (fnDirectSoundCreate8)GetProcAddress(dslib, "DirectSoundCreate8");
	//msbox_ok(i2s(__LINE__), "td_on");
	if (pDirectSoundCreate8 && !pUnlock)
	{
		auto result = pDirectSoundCreate8(0, &ds, NULL);
		if (result == DS_OK && ds)
		{
			result = create_sound_buf(&sbctx);
			if (result == DS_OK)
			{
				pLock = sbctx->lpVtbl->Lock;
				pUnlock = sbctx->lpVtbl->Unlock;
				pSetFormat = sbctx->lpVtbl->SetFormat;
				pRelease = sbctx->lpVtbl->Release;
				pAddRef = sbctx->lpVtbl->AddRef;
				if (pLock && pUnlock && pSetFormat && pRelease)
				{
					push_fun(&pUnlock, pmUnlock);
					push_fun(&pLock, pmLock);
					push_fun(&pSetFormat, pmSetFormat);
					push_fun(&pRelease, pmRelease);
					push_fun(&pAddRef, pmAddRef);
				}
			}
		}
	}
	if (pDirectSoundCreate8)
	{
		//push_fun((void**)&pDirectSoundCreate8, pmdsc8);
	}
}
