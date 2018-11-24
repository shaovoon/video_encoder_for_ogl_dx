// MP3.cpp
#ifdef _WIN32

#include "stdafx.h"
#include "DS_Mp3.h"
#include <uuids.h>

DS_Mp3::DS_Mp3()
{
	pigb = NULL;
	pimc = NULL;
	pimex = NULL;
	piba = NULL;
	pims = NULL;
	ready = false;
	duration = 0;
}

DS_Mp3::~DS_Mp3()
{
	Cleanup();
}

void DS_Mp3::Cleanup()
{
	if (pimc)
		pimc->Stop();

	if(pigb)
	{
		pigb->Release();
		pigb = NULL;
	}

	if(pimc)
	{
		pimc->Release();
		pimc = NULL;
	}

	if(pimex)
	{
		pimex->Release();
		pimex = NULL;
	}

	if(piba)
	{
		piba->Release();
		piba = NULL;
	}

	if(pims)
	{
		pims->Release();
		pims = NULL;
	}
	ready = false;
}

bool DS_Mp3::Load(LPCWSTR szFile)
{
	Cleanup();
	ready = false;
	if (SUCCEEDED(CoCreateInstance( CLSID_FilterGraph,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IGraphBuilder,
		(void **)&this->pigb)))
	{
		pigb->QueryInterface(IID_IMediaControl, (void **)&pimc);
		pigb->QueryInterface(IID_IMediaEventEx, (void **)&pimex);
		pigb->QueryInterface(IID_IBasicAudio, (void**)&piba);
		pigb->QueryInterface(IID_IMediaSeeking, (void**)&pims);

		HRESULT hr = pigb->RenderFile(szFile, NULL);
		if (SUCCEEDED(hr))
		{
			ready = true;
			if(pims)
			{
				pims->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
				pims->GetDuration(&duration); // returns 10,000,000 for a second.
				duration = duration;
			}
		}
	}
	return ready;
}

bool DS_Mp3::Play()
{
	if (ready&&pimc)
	{
		HRESULT hr = pimc->Run();
		return SUCCEEDED(hr);
	}
	return false;
}

bool DS_Mp3::Pause()
{
	if (ready&&pimc)
	{
		HRESULT hr = pimc->Pause();
		return SUCCEEDED(hr);
	}
	return false;
}

bool DS_Mp3::Stop()
{
	if (ready&&pimc)
	{
		HRESULT hr = pimc->Stop();
		return SUCCEEDED(hr);
	}
	return false;
}

bool DS_Mp3::WaitForCompletion(long msTimeout, long* EvCode)
{
	if (ready&&pimex)
	{
		HRESULT hr = pimex->WaitForCompletion(msTimeout, EvCode);
		return *EvCode > 0;
	}

	return false;
}

bool DS_Mp3::SetVolume(long vol)
{
	if (ready&&piba)
	{
		HRESULT hr = piba->put_Volume(vol);
		return SUCCEEDED(hr);
	}
	return false;
}

long DS_Mp3::GetVolume()
{
	if (ready&&piba)
	{
		long vol = -1;
		HRESULT hr = piba->get_Volume(&vol);

		if(SUCCEEDED(hr))
			return vol;
	}

	return -1;
}

__int64 DS_Mp3::GetDuration()
{
	return duration;
}

__int64 DS_Mp3::GetCurrentPosition()
{
	if (ready&&pims)
	{
		__int64 curpos = -1;
		HRESULT hr = pims->GetCurrentPosition(&curpos);

		if(SUCCEEDED(hr))
			return curpos;
	}

	return -1;
}

bool DS_Mp3::SetPositions(__int64* pCurrent, __int64* pStop, bool bAbsolutePositioning)
{
	if (ready&&pims)
	{
		DWORD flags = 0;
		if(bAbsolutePositioning)
			flags = AM_SEEKING_AbsolutePositioning | AM_SEEKING_SeekToKeyFrame;
		else
			flags = AM_SEEKING_RelativePositioning | AM_SEEKING_SeekToKeyFrame;

		HRESULT hr = pims->SetPositions(pCurrent, flags, pStop, flags);

		if(SUCCEEDED(hr))
			return true;
	}

	return false;
}

#endif // _WIN32

//Alan
//-----
//alankemp@bigfoot.com
