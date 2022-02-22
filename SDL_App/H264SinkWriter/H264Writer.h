#pragma once

#include <Windows.h>

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <atlbase.h>
#include <atlstr.h>

#include <mfapi.h>
#include <mfidl.h>
#include <Mfreadwrite.h>
#include <mferror.h>
#include <Codecapi.h>
#include <strmif.h>
#include <vector>
#include <string>

extern int check_config_file(const wchar_t* file, int* width, int* height, int* fps);
extern int encoder_start(UINT** pixels, HANDLE evtRequest, HANDLE evtReply, HANDLE evtExit, HANDLE evtVideoEnded, const WCHAR* szUrl);

#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid")

// All video decoders and encoders support at least one of these video formats - basically
// decoded frames (in some cases variants of bitmaps)
static GUID intermediateVideoFormats[] =
{
	MFVideoFormat_NV12,
	MFVideoFormat_YV12,
	MFVideoFormat_YUY2,
	MFVideoFormat_RGB32
};
int nIntermediateVideoFormats = 4;


// audio stream formats that every audio decoder and encoder should
// be able to agree on - uncompressed audio data
static GUID intermediateAudioFormats[] =
{
	MFAudioFormat_Float,
	MFAudioFormat_PCM,
};
int nIntermediateAudioFormats = 2;

#define BREAK_ON_FAIL(value)            if(FAILED(value)) break;
#define BREAK_ON_NULL(value, newHr)     if(value == NULL) { hr = newHr; break; }

DWORD WINAPI ThreadOpenGLProc(LPVOID pParam);

template <class T> void SafeRelease(T **ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

enum class VideoCodec
{
	H264,
	HEVC
};
enum class Processing
{
	HardwareAcceleration,
	Software
};
enum class RateControlMode
{
	None,
	UnconstrainedVBR, // Variable Bitrate
	Quality,
	CBR // Constant Bitrate
};
class H264Writer
{
public:
	// Format constants
	const GUID   VIDEO_ENCODING_FORMAT = MFVideoFormat_H264;
	const GUID   VIDEO_INPUT_FORMAT = MFVideoFormat_RGB32;

	UINT32 getVideoFPS() const { return m_VideoFPS; }
	void setVideoFPS(UINT32 val) {
		m_VideoFPS = val; 
		m_FrameDuration = 10 * 1000 * 1000 / m_VideoFPS;
	}

	UINT32 getVideoBitrate() const { return m_VideoBitrate; }
	void setVideoBitrate(UINT32 val) { m_VideoBitrate = val; }

	int& GetWidth() { return m_Width; }
	int& GetHeight() { return m_Height; }

	const std::wstring& GetUrl()       const { return m_SrcFilename; }

	H264Writer(const wchar_t* mp3_file, const wchar_t* src_file, const wchar_t* dest_file, VideoCodec codec, Processing processing, UINT32 bitrate = 4000000,
		int numWorkerThreads = 0 /* 0 leaves to default */,
		int qualityVsSpeed = 100 /* [0:100] 0 for speed, 100 for quality */,
		RateControlMode mode = RateControlMode::Quality,
		int quality = 100 /* Only valid when mode is RateControlMode::Quality. [0:100] 0 for smaller file size and less quality, 100 for bigger file size and more quality */) :
		m_OpenSrcFileSuccess(false),
		m_MP3Filename(mp3_file),
		m_SrcFilename(src_file),
		m_DestFilename(dest_file),
		m_Width(0),
		m_Height(0),
		m_pImage(nullptr),
		m_cbWidth(4 * m_Width),
		m_cbBuffer(m_cbWidth * m_Height),
		m_pBuffer(nullptr),
		m_hThread(INVALID_HANDLE_VALUE),
		m_evtRequest(INVALID_HANDLE_VALUE),
		m_evtReply(INVALID_HANDLE_VALUE),
		m_evtExit(INVALID_HANDLE_VALUE),
		m_evtVideoEnded(INVALID_HANDLE_VALUE),
		m_CoInited(false),
		m_MFInited(false),
		m_pSinkWriter(nullptr),
		m_VideoFPS(60),
		m_FrameDuration(10 * 1000 * 1000 / m_VideoFPS),
		m_VideoBitrate(bitrate),
		m_VideoCodec(codec),
		m_Processing(processing),
		m_nStreams(0),
		m_NumWorkerThreads(numWorkerThreads),
		m_QualityVsSpeed(qualityVsSpeed),
		m_Mode(mode),
		m_Quality(quality)
	{
		int width = 0; int height = 0; int fps = 0;
		if (check_config_file(m_SrcFilename.c_str(), &width, &height, &fps))
		{
			m_OpenSrcFileSuccess = true;
			m_Width = width;
			m_Height = height;
			m_cbWidth = 4 * m_Width;
			m_cbBuffer = m_cbWidth * m_Height;
			m_VideoFPS = fps;
			m_FrameDuration = (10 * 1000 * 1000 / m_VideoFPS);
			m_pImage = new (std::nothrow) UINT32[m_Width * m_Height];
		}

		if (!m_OpenSrcFileSuccess)
			return;

		if (!m_pImage)
			return;



		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

		if (SUCCEEDED(hr))
		{
			m_CoInited = true;
			hr = MFStartup(MF_VERSION);
			if (SUCCEEDED(hr))
			{
				m_MFInited = true;
				// Create a new memory buffer.
				hr = MFCreateMemoryBuffer(m_cbBuffer, &m_pBuffer);
			}
		}

		if (IsValid() == false)
		{
			return;
		}

		if (SUCCEEDED(hr))
		{
			m_evtRequest = CreateEvent(NULL, FALSE, FALSE, NULL);

			m_evtReply = CreateEvent(NULL, FALSE, FALSE, NULL);

			m_evtExit = CreateEvent(NULL, FALSE, FALSE, NULL);

			m_evtVideoEnded = CreateEvent(NULL, FALSE, FALSE, NULL);

			m_hThread = CreateThread(NULL, 100000, ThreadOpenGLProc, this, 0, NULL);
		}
	}

	~H264Writer()
	{
		SetEvent(m_evtExit);
		Sleep(100);

		if (m_pImage)
		{
			delete[] m_pImage;
			m_pImage = nullptr;
		}

		m_pBuffer.Release();

		if (m_evtRequest != INVALID_HANDLE_VALUE)
			CloseHandle(m_evtRequest);

		if(m_evtReply != INVALID_HANDLE_VALUE)
			CloseHandle(m_evtReply);

		if(m_evtExit != INVALID_HANDLE_VALUE)
			CloseHandle(m_evtExit);

		if(m_evtVideoEnded!=INVALID_HANDLE_VALUE)
			CloseHandle(m_evtVideoEnded);

		m_pSourceReader = nullptr;
		m_pSinkWriter = nullptr;

		if(m_MFInited)
			MFShutdown();

		if(m_CoInited)
			CoUninitialize();
	}

	const bool IsValid() const
	{
		return m_CoInited && m_MFInited;
	}

	HRESULT GetSourceDuration(IMFMediaSource *pSource, MFTIME *pDuration)
	{
		*pDuration = 0;

		IMFPresentationDescriptor *pPD = NULL;

		HRESULT hr = pSource->CreatePresentationDescriptor(&pPD);
		if (SUCCEEDED(hr))
		{
			hr = pPD->GetUINT64(MF_PD_DURATION, (UINT64*)pDuration);
			pPD->Release();
		}
		return hr;
	}

	HRESULT InitializeWriter(DWORD *videoStreamIndex, DWORD *audioStreamIndex)
	{
		HRESULT hr = S_OK;
		m_pSourceReader = nullptr;
		m_pSinkWriter = nullptr;
		*videoStreamIndex = 1;
		*audioStreamIndex = 0;

		IMFMediaType    *pMediaTypeOut = nullptr;
		IMFMediaType    *pMediaTypeIn = nullptr;
		DWORD           streamIndex=1;
		CComPtr<IMFAttributes> pConfigAttrs;

		do
		{
			// create an attribute store
			hr = MFCreateAttributes(&pConfigAttrs, 1);
			BREAK_ON_FAIL(hr);

			// set MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS property in the store
			hr = pConfigAttrs->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);
			BREAK_ON_FAIL(hr);

			if (m_MP3Filename.empty() == false)
			{
				// create a source reader
				hr = MFCreateSourceReaderFromURL(m_MP3Filename.c_str(), pConfigAttrs, &m_pSourceReader);
				BREAK_ON_FAIL(hr);

				IMFMediaSource* pSource = nullptr;
				hr = m_pSourceReader->GetServiceForStream(
					MF_SOURCE_READER_MEDIASOURCE,
					GUID_NULL, //MF_MEDIASOURCE_SERVICE,
					IID_IMFMediaSource,
					(void**)&pSource
				);
				BREAK_ON_FAIL(hr);

				MFTIME duration = 0;
				hr = GetSourceDuration(pSource, &duration);
				BREAK_ON_FAIL(hr);
				if (pSource)
					pSource->Release();

				printf("Audio duration:%lld\n", duration);

				MFTIME total_seconds = duration / 10000000;
				MFTIME minute = total_seconds / 60;
				MFTIME second = total_seconds % 60;
				printf("Audio duration:%lld:%lld\n", minute, second);
			}
			if (m_Processing == Processing::HardwareAcceleration)
			{

				CComPtr<IMFAttributes> attrs;
				MFCreateAttributes(&attrs, 1);
				attrs->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);

				hr = MFCreateSinkWriterFromURL(m_DestFilename.c_str(), nullptr, attrs, &m_pSinkWriter);
			}
			else
			{
				hr = MFCreateSinkWriterFromURL(m_DestFilename.c_str(), nullptr, nullptr, &m_pSinkWriter);
			}
			BREAK_ON_FAIL(hr);

			// map the streams found in the source file from the source reader to the
			// sink writer, while negotiating media types
			hr = MapStreams();
			BREAK_ON_FAIL(hr);

			hr = SetVideoOutputType(&pMediaTypeOut, streamIndex);
			BREAK_ON_FAIL(hr);
			hr = SetVideoInputType(&pMediaTypeIn, streamIndex);
			BREAK_ON_FAIL(hr);
			hr = SetQuality(streamIndex);
			BREAK_ON_FAIL(hr);

			hr = m_pSinkWriter->BeginWriting();
			BREAK_ON_FAIL(hr);

			*videoStreamIndex = streamIndex;
		} while (false);

		SafeRelease(&pMediaTypeOut);
		SafeRelease(&pMediaTypeIn);
		return hr;
	}
	HRESULT SetQuality(DWORD& streamIndex)
	{
		if (m_Processing == Processing::Software)
			return S_FALSE;

		CComPtr<ICodecAPI> ca;
		HRESULT hr = m_pSinkWriter->GetServiceForStream(streamIndex, GUID_NULL, __uuidof(ICodecAPI), (void**)&ca);
		if (ca)
		{
			if (m_NumWorkerThreads)
			{
				VARIANT v = {};
				v.vt = VT_UI4;
				v.ulVal = m_NumWorkerThreads;
				ca->SetValue(&CODECAPI_AVEncNumWorkerThreads, &v);
			}
			if (m_QualityVsSpeed >= 0 && m_QualityVsSpeed <= 100)
			{
				VARIANT v = {};
				v.vt = VT_UI4;
				v.ulVal = m_QualityVsSpeed;
				ca->SetValue(&CODECAPI_AVEncCommonQualityVsSpeed, &v);
			}

			if (m_Mode != RateControlMode::None)
			{
				VARIANT v = {};
				v.vt = VT_UI4;
				if (m_Mode == RateControlMode::UnconstrainedVBR)
					v.ulVal = eAVEncCommonRateControlMode_UnconstrainedVBR;
				if (m_Mode == RateControlMode::Quality)
					v.ulVal = eAVEncCommonRateControlMode_Quality;
				if (m_Mode == RateControlMode::CBR)
					v.ulVal = eAVEncCommonRateControlMode_CBR;
				ca->SetValue(&CODECAPI_AVEncCommonRateControlMode, &v);
				if (m_Mode == RateControlMode::Quality)
				{
					VARIANT v7 = {};
					v7.vt = VT_UI4;
					if (m_Quality >= 0 && m_Quality <= 100)
						v7.ulVal = m_Quality;
					else
						v7.ulVal = 100;

					ca->SetValue(&CODECAPI_AVEncCommonQuality, &v7);
				}
			}
		}
		return hr;
	}

	HRESULT SetVideoOutputType(IMFMediaType** pMediaTypeOut, DWORD& streamIndex)
	{
		HRESULT hr = S_OK;
		do 
		{
		hr = MFCreateMediaType(pMediaTypeOut);
		BREAK_ON_FAIL(hr);
		hr = (*pMediaTypeOut)->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
		BREAK_ON_FAIL(hr);
		if (m_VideoCodec==VideoCodec::HEVC)
		{
			hr = (*pMediaTypeOut)->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_HEVC);
			BREAK_ON_FAIL(hr);
			hr = (*pMediaTypeOut)->SetUINT32(MF_MT_MPEG2_PROFILE, eAVEncH265VProfile_Main_420_8);
			BREAK_ON_FAIL(hr);
		}
		else
		{
			hr = (*pMediaTypeOut)->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
			BREAK_ON_FAIL(hr);
			hr = (*pMediaTypeOut)->SetUINT32(MF_MT_MPEG2_PROFILE, eAVEncH264VProfile_High);
			BREAK_ON_FAIL(hr);
		}

		//(*pMediaTypeOut)->SetUINT32(MF_MT_VIDEO_NOMINAL_RANGE, MFNominalRange_Wide);
		//BREAK_ON_FAIL(hr);

		hr = (*pMediaTypeOut)->SetUINT32(MF_MT_AVG_BITRATE, m_VideoBitrate);
		BREAK_ON_FAIL(hr);
		hr = (*pMediaTypeOut)->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
		BREAK_ON_FAIL(hr);
		hr = MFSetAttributeSize((*pMediaTypeOut), MF_MT_FRAME_SIZE, m_Width, m_Height);
		BREAK_ON_FAIL(hr);
		hr = MFSetAttributeRatio((*pMediaTypeOut), MF_MT_FRAME_RATE, m_VideoFPS, 1);
		BREAK_ON_FAIL(hr);
		hr = MFSetAttributeRatio((*pMediaTypeOut), MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
		BREAK_ON_FAIL(hr);
		hr = m_pSinkWriter->AddStream((*pMediaTypeOut), &streamIndex);
	} while (false);
	return hr;
}
	HRESULT SetVideoInputType(IMFMediaType** pMediaTypeIn, DWORD& streamIndex)
	{
		HRESULT hr = S_OK;
		do 
		{
			hr = MFCreateMediaType(pMediaTypeIn);
			BREAK_ON_FAIL(hr);
			hr = (*pMediaTypeIn)->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
			BREAK_ON_FAIL(hr);
			hr = (*pMediaTypeIn)->SetGUID(MF_MT_SUBTYPE, VIDEO_INPUT_FORMAT);
			BREAK_ON_FAIL(hr);
			hr = (*pMediaTypeIn)->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
			BREAK_ON_FAIL(hr);
			hr = MFSetAttributeSize(*pMediaTypeIn, MF_MT_FRAME_SIZE, m_Width, m_Height);
			BREAK_ON_FAIL(hr);
			hr = MFSetAttributeRatio(*pMediaTypeIn, MF_MT_FRAME_RATE, m_VideoFPS, 1);
			BREAK_ON_FAIL(hr);
			hr = MFSetAttributeRatio(*pMediaTypeIn, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
			BREAK_ON_FAIL(hr);
			hr = m_pSinkWriter->SetInputMediaType(streamIndex, *pMediaTypeIn, NULL);
		} while (false);

		return hr;
	}
	//
	// Attempt to find an uncompressed media type for the specified stream that both the source 
	// and sink can agree on
	//
	HRESULT ConnectStream(DWORD dwStreamIndex,
		const GUID& streamMajorType)
	{
		HRESULT hr = S_OK;

		CComPtr<IMFMediaType> pPartialMediaType;
		CComPtr<IMFMediaType> pFullMediaType;

		BOOL fConfigured = FALSE;
		GUID* intermediateFormats = NULL;
		int nFormats = 0;

		do
		{
			// create a media type container object that will be used to match stream input
			// and output media types
			hr = MFCreateMediaType(&pPartialMediaType);
			BREAK_ON_FAIL(hr);

			// set the major type of the partial match media type container
			hr = pPartialMediaType->SetGUID(MF_MT_MAJOR_TYPE, streamMajorType);
			BREAK_ON_FAIL(hr);

			// Get the appropriate list of intermediate formats - formats that every decoder and
			// encoder of that type should agree on.  Essentially these are the uncompressed 
			// formats that correspond to decoded frames for video, and uncompressed audio 
			// formats
			if (streamMajorType == MFMediaType_Video)
			{
				intermediateFormats = intermediateVideoFormats;
				nFormats = nIntermediateVideoFormats;
			}
			else if (streamMajorType == MFMediaType_Audio)
			{
				intermediateFormats = intermediateAudioFormats;
				nFormats = nIntermediateAudioFormats;
			}
			else
			{
				hr = E_UNEXPECTED;
				break;
			}


			// loop through every intermediate format that you have for this major type, and
			// try to find one on which both the source stream and sink stream can agree on
			for (int x = 0; x < nFormats; x++)
			{
				// set the format of the partial media type
				hr = pPartialMediaType->SetGUID(MF_MT_SUBTYPE, intermediateFormats[x]);
				BREAK_ON_FAIL(hr);

				// set the partial media type on the source stream
				hr = m_pSourceReader->SetCurrentMediaType(
					dwStreamIndex,                      // stream index
					NULL,                               // reserved - always NULL
					pPartialMediaType);                // media type to try to set

													   // if the source stream (i.e. the decoder) is not happy with this media type -
													   // if it cannot decode the data into this media type, restart the loop in order 
													   // to try the next format on the list
				if (FAILED(hr))
				{
					hr = S_OK;
					continue;
				}

				pFullMediaType = NULL;

				// if you got here, the source stream is happy with the partial media type you set
				// - extract the full media type for this stream (with all internal fields 
				// filled in)
				hr = m_pSourceReader->GetCurrentMediaType(dwStreamIndex, &pFullMediaType);

				// Now try to match the full media type to the corresponding sink stream
				hr = m_pSinkWriter->SetInputMediaType(
					dwStreamIndex,             // stream index
					pFullMediaType,            // media type to match
					NULL);                    // configuration attributes for the encoder

											  // if the sink stream cannot accept this media type - i.e. if no encoder was
											  // found that would accept this media type - restart the loop and try the next
											  // format on the list
				if (FAILED(hr))
				{
					hr = S_OK;
					continue;
				}

				// you found a media type that both the source and sink could agree on - no need
				// to try any other formats
				fConfigured = TRUE;
				break;
			}
			BREAK_ON_FAIL(hr);

			// if you didn't match any formats return an error code
			if (!fConfigured)
			{
				hr = MF_E_INVALIDMEDIATYPE;
				break;
			}

		} while (false);

		return hr;
	}


	HRESULT MapStreams(void)
	{
		if (m_MP3Filename.empty())
			return S_OK;

		HRESULT hr = S_OK;
		BOOL isStreamSelected = FALSE;
		DWORD sourceStreamIndex = 0;
		DWORD sinkStreamIndex = 0;
		GUID streamMajorType;
		CComPtr<IMFMediaType> pStreamMediaType;


		do
		{
			m_nStreams = 0;

			while (SUCCEEDED(hr))
			{
				// check whether you have a stream with the right index - if you don't, the 
				// IMFSourceReader::GetStreamSelection() function will fail, and you will drop
				// out of the while loop
				hr = m_pSourceReader->GetStreamSelection(sourceStreamIndex, &isStreamSelected);
				if (FAILED(hr))
				{
					hr = S_OK;
					break;
				}

				// count the total number of streams for later
				m_nStreams++;

				// get the source media type of the stream
				hr = m_pSourceReader->GetNativeMediaType(
					sourceStreamIndex,           // index of the stream you are interested in
					0,                           // index of the media type exposed by the 
												 //    stream decoder
					&pStreamMediaType);          // media type
				BREAK_ON_FAIL(hr);

				// extract the major type of the source stream from the media type
				hr = pStreamMediaType->GetMajorType(&streamMajorType);
				BREAK_ON_FAIL(hr);

				// select a stream, indicating that the source should send out its data instead
				// of dropping all of the samples
				hr = m_pSourceReader->SetStreamSelection(sourceStreamIndex, TRUE);
				BREAK_ON_FAIL(hr);

				// if this is a video or audio stream, transcode it and negotiate the media type
				// between the source reader stream and the corresponding sink writer stream.  
				// If this is a some other stream format (e.g. subtitles), just pass the media 
				// type unchanged.
				if (streamMajorType == MFMediaType_Audio || streamMajorType == MFMediaType_Video)
				{
					// get the target media type - the media type into which you will transcode
					// the data of the current source stream
					hr = GetTranscodeMediaType(pStreamMediaType);
					BREAK_ON_FAIL(hr);

					// add the stream to the sink writer - i.e. tell the sink writer that a 
					// stream with the specified index will have the target media type
					hr = m_pSinkWriter->AddStream(pStreamMediaType, &sinkStreamIndex);
					BREAK_ON_FAIL(hr);

					// hook up the source and sink streams - i.e. get them to agree on an
					// intermediate media type that will be used to pass data between source 
					// and sink
					hr = ConnectStream(sourceStreamIndex, streamMajorType);
					BREAK_ON_FAIL(hr);
				}
				else
				{
					// add the stream to the sink writer with the exact same media type as the
					// source stream
					hr = m_pSinkWriter->AddStream(pStreamMediaType, &sinkStreamIndex);
					BREAK_ON_FAIL(hr);
				}

				// make sure that the source stream index is equal to the sink stream index
				if (sourceStreamIndex != sinkStreamIndex)
				{
					hr = E_UNEXPECTED;
					break;
				}

				// increment the source stream index, so that on the next loop you are analyzing
				// the next stream
				sourceStreamIndex++;

				// release the media type
				pStreamMediaType = NULL;
			}

			BREAK_ON_FAIL(hr);

		} while (false);

		return hr;
	}


	//
	// Get the target audio media type - use the AAC media format.
	//
	HRESULT GetTranscodeAudioType(
		CComPtr<IMFMediaType>& pStreamMediaType)
	{
		HRESULT hr = S_OK;

		do
		{
			BREAK_ON_NULL(pStreamMediaType, E_POINTER);

			// wipe out existing data from the media type
			hr = pStreamMediaType->DeleteAllItems();
			BREAK_ON_FAIL(hr);

			// reset the major type to audio since we just wiped everything out
			pStreamMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
			BREAK_ON_FAIL(hr);

			// set the audio subtype
			hr = pStreamMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_AAC);
			BREAK_ON_FAIL(hr);

			// set the number of audio bits per sample
			hr = pStreamMediaType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
			BREAK_ON_FAIL(hr);

			// set the number of audio samples per second
			hr = pStreamMediaType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, 44100);
			BREAK_ON_FAIL(hr);

			// set the number of audio channels
			hr = pStreamMediaType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, 2);
			BREAK_ON_FAIL(hr);

			// set the Bps of the audio stream
			hr = pStreamMediaType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 16000);
			BREAK_ON_FAIL(hr);

			// set the block alignment of the samples
			hr = pStreamMediaType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, 1);
			BREAK_ON_FAIL(hr);
		} while (false);

		return hr;
	}



	//
	// Get the target video media type - use the H.264 media format.
	//
	HRESULT GetTranscodeVideoType(
		CComPtr<IMFMediaType>& pStreamMediaType)
	{
		HRESULT hr = S_OK;

		do
		{
			BREAK_ON_NULL(pStreamMediaType, E_POINTER);

			// wipe out existing data from the media type
			hr = pStreamMediaType->DeleteAllItems();
			BREAK_ON_FAIL(hr);

			// reset the major type to video since we just wiped everything out
			pStreamMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
			BREAK_ON_FAIL(hr);

			// set the video subtype
			if (m_VideoCodec == VideoCodec::H264)
			{
				hr = pStreamMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
			}
			else
			{
				hr = pStreamMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_HEVC);
			}
			BREAK_ON_FAIL(hr);

			// set the frame size to 720p as a 64-bit packed value
			hr = MFSetAttributeSize(
				pStreamMediaType,           // attribute store on which to set the value
				MF_MT_FRAME_SIZE,           // value ID GUID
				m_Width, m_Height);                 // frame width and height
			BREAK_ON_FAIL(hr);

			// Set the frame rate to 30/1.001 - the standard frame rate of NTSC television - as 
			// a 64-bit packed value consisting of a fraction of two integers
			hr = MFSetAttributeRatio(
				pStreamMediaType,           // attribute store on which to set the value
				MF_MT_FRAME_RATE,           // value
				m_VideoFPS, 1);               // frame rate ratio
			BREAK_ON_FAIL(hr);

			// set the average bitrate of the video in bits per second - in this case 10 Mbps
			hr = pStreamMediaType->SetUINT32(MF_MT_AVG_BITRATE, m_VideoBitrate);
			BREAK_ON_FAIL(hr);

			// set the interlace mode to progressive
			hr = pStreamMediaType->SetUINT32(MF_MT_INTERLACE_MODE,
				MFVideoInterlace_Progressive);
			BREAK_ON_FAIL(hr);

			// set the pixel aspect ratio to 1x1 - square pixels
			hr = MFSetAttributeSize(pStreamMediaType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
			BREAK_ON_FAIL(hr);
		} while (false);

		return hr;
	}




	//
	// Set the target target audio and video media types to hard-coded values.  In this case you
	// are setting audio to AAC, and video to 720p H.264
	//
	HRESULT GetTranscodeMediaType(
		CComPtr<IMFMediaType>& pStreamMediaType)
	{
		HRESULT hr = S_OK;
		GUID streamMajorType;

		do
		{
			// extract the major type of the source stream from the media type
			hr = pStreamMediaType->GetMajorType(&streamMajorType);
			BREAK_ON_FAIL(hr);

			// if this is an audio stream, configure a hard-coded AAC profile.  If this is a
			// video stream, configure an H.264 profile
			if (streamMajorType == MFMediaType_Audio)
			{
				hr = GetTranscodeAudioType(pStreamMediaType);
			}
			else if (streamMajorType == MFMediaType_Video)
			{
				hr = GetTranscodeVideoType(pStreamMediaType);
			}
		} while (false);

		return hr;
	}

	HRESULT WriteVideoFrame(
		DWORD streamIndex,
		const LONGLONG& rtStart
	)
	{
		IMFSample *pSample = NULL;

		BYTE *pData = NULL;
		// Lock the buffer and copy the video frame to the buffer.
		HRESULT hr = m_pBuffer->Lock(&pData, NULL, NULL);
		if (SUCCEEDED(hr))
		{
			hr = MFCopyImage(
				pData,              // Destination buffer.
				m_cbWidth,          // Destination stride.
				(BYTE*)m_pImage,    // First row in source image.
				m_cbWidth,          // Source stride.
				m_cbWidth,          // Image width in bytes.
				m_Height            // Image height in pixels.
			);
		}
		if (m_pBuffer)
		{
			m_pBuffer->Unlock();
		}

		// Set the data length of the buffer.
		if (SUCCEEDED(hr))
		{
			hr = m_pBuffer->SetCurrentLength(m_cbBuffer);
		}

		// Create a media sample and add the buffer to the sample.
		if (SUCCEEDED(hr))
		{
			hr = MFCreateSample(&pSample);
		}
		if (SUCCEEDED(hr))
		{
			hr = pSample->AddBuffer(m_pBuffer);
		}

		// Set the time stamp and the duration.
		if (SUCCEEDED(hr))
		{
			hr = pSample->SetSampleTime(rtStart);
		}
		if (SUCCEEDED(hr))
		{
			hr = pSample->SetSampleDuration(m_FrameDuration);
		}

		// Send the sample to the Sink Writer.
		if (SUCCEEDED(hr))
		{
			hr = m_pSinkWriter->WriteSample(streamIndex, pSample);
		}

		SafeRelease(&pSample);
		return hr;
	}
	HRESULT WriteAudioFrame(DWORD streamIndex, LONGLONG& timestamp)
	{
		HRESULT hr = S_OK;
		DWORD flags = 0;
		CComPtr<IMFSample> pSample;

		do
		{
			// pull a sample out of the source reader
			hr = m_pSourceReader->ReadSample(
				(DWORD)MF_SOURCE_READER_ANY_STREAM,     // get a sample from any stream
				0,                                      // no source reader controller flags
				&streamIndex,                         // get index of the stream
				&flags,                               // get flags for this sample
				&timestamp,                           // get the timestamp for this sample
				&pSample);                             // get the actual sample
			BREAK_ON_FAIL(hr);

			// The sample can be null if you've reached the end of stream or encountered a
			// data gap (AKA a stream tick).  If you got a sample, send it on.  Otherwise,
			// if you got a stream gap, send information about it to the sink.
			if (pSample != NULL)
			{
				// push the sample to the sink writer
				hr = m_pSinkWriter->WriteSample(streamIndex, pSample);
				BREAK_ON_FAIL(hr);
			}
			else if (flags & MF_SOURCE_READERF_STREAMTICK)
			{
				// signal a stream tick
				hr = m_pSinkWriter->SendStreamTick(streamIndex, timestamp);
				BREAK_ON_FAIL(hr);
			}

			// if a stream reached the end, notify the sink, and increment the number of
			// finished streams
			if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
			{
				hr = m_pSinkWriter->NotifyEndOfSegment(streamIndex);
				BREAK_ON_FAIL(hr);
			}
			// release sample
			pSample = NULL;
		} while (false);

		return hr;
	}
	static const bool EnumVideoEncoder(std::vector<std::wstring>& encoders, Processing proc, VideoCodec codec)
	{
		MFT_REGISTER_TYPE_INFO output_MFT_type;
		output_MFT_type.guidMajorType = MFMediaType_Video;
		output_MFT_type.guidSubtype = (codec == VideoCodec::HEVC) ? MFVideoFormat_HEVC : MFVideoFormat_H264;
		IMFActivate **ppMFTActivate = NULL;
		UINT32 count = 0;
		HRESULT hr;
		if (proc == Processing::HardwareAcceleration)
		{
			hr = MFTEnumEx(MFT_CATEGORY_VIDEO_ENCODER, MFT_ENUM_FLAG_HARDWARE, NULL, &output_MFT_type, &ppMFTActivate, &count);
		}
		else
		{
			hr = MFTEnumEx(MFT_CATEGORY_VIDEO_ENCODER, MFT_ENUM_FLAG_SYNCMFT | MFT_ENUM_FLAG_ASYNCMFT | MFT_ENUM_FLAG_SORTANDFILTER, NULL, &output_MFT_type, &ppMFTActivate, &count);
		}
		if (SUCCEEDED(hr))
		{
			for (UINT32 i = 0; i < count; i++)
			{
				LPWSTR name = 0;

				hr = ppMFTActivate[i]->GetAllocatedString(MFT_FRIENDLY_NAME_Attribute, &name, 0);

				if (SUCCEEDED(hr))
				{
					//printf("Video Encoder: %S\n", name);
					encoders.push_back(name);
				}

				CoTaskMemFree(name);

				ppMFTActivate[i]->Release();
			}
			CoTaskMemFree(ppMFTActivate);
		}
		return SUCCEEDED(hr);
	}
	static bool HasH264(Processing processing)
	{
		std::vector<std::wstring> encoders;
		if (H264Writer::EnumVideoEncoder(encoders, processing, VideoCodec::H264))
		{
			return encoders.size() > 0;
		}
		return false;
	}
	static bool HasHEVC(Processing processing)
	{
		std::vector<std::wstring> encoders;
		if (H264Writer::EnumVideoEncoder(encoders, processing, VideoCodec::HEVC))
		{
			return encoders.size() > 0;
		}
		return false;
	}
	const bool Process()
	{
		if (IsValid())
		{
			DWORD video_stream = 0;
			DWORD audio_stream = 0;

			HRESULT hr = InitializeWriter(&video_stream, &audio_stream);
			if (SUCCEEDED(hr))
			{
				// Send frames to the sink writer.
				LONGLONG rtStart = 0;

				bool success = true;

				HRESULT hr = S_OK;

				bool audio_done = false;

				DWORD audio_stream = 0;
				LONGLONG audio_timestamp = 0;

				while (true)
				{
					SetEvent(m_evtRequest);

					success = true;
					HANDLE arr[2];
					arr[0] = m_evtVideoEnded;
					arr[1] = m_evtReply;
					DWORD dw = WaitForMultipleObjects(2, arr, FALSE, INFINITE);

					if (WAIT_OBJECT_0 == dw)
					{
						OutputDebugStringA("VideoEnded");
						break;
					}
					if (WAIT_OBJECT_0 + 1 != dw)
					{
						OutputDebugStringA("E_FAIL");
						success = false;
						break;
					}

					if (success)
					{
						hr = WriteVideoFrame(video_stream, rtStart);
						if (FAILED(hr))
						{
							success = false;
							break;
						}
						rtStart += m_FrameDuration;

						if (m_MP3Filename.empty() == false)
						{
							if (rtStart < audio_timestamp)
								continue;

							if (!audio_done)
							{
								hr = WriteAudioFrame(audio_stream, audio_timestamp);
								if (FAILED(hr))
								{
									audio_done = true;
								}
							}
						}
					}
				}

				

				if (success)
				{
					hr = m_pSinkWriter->Finalize();
				}
				m_pSinkWriter.Release();
				return success;
			}
		}
		return false;
	}

	UINT32 ** GetImagePtr()      { return &m_pImage;       }
	HANDLE GetRequestEvent()    const { return m_evtRequest;    }
	HANDLE GetReplyEvent()      const { return m_evtReply;      }
	HANDLE GetExitEvent()       const { return m_evtExit;       }
	HANDLE GetVideoEndedEvent() const { return m_evtVideoEnded; }

private:
	bool m_OpenSrcFileSuccess;
	std::wstring m_MP3Filename;
	std::wstring m_SrcFilename;
	std::wstring m_DestFilename;
	int m_Width;
	int m_Height;
	UINT32 * m_pImage;
	LONG m_cbWidth;
	DWORD m_cbBuffer;
	CComPtr<IMFMediaBuffer> m_pBuffer;
	HANDLE m_hThread;
	HANDLE m_evtRequest;
	HANDLE m_evtReply;
	HANDLE m_evtExit;
	HANDLE m_evtVideoEnded;
	bool m_CoInited;
	bool m_MFInited;
	CComPtr<IMFSourceReader> m_pSourceReader;
	CComPtr<IMFSinkWriter> m_pSinkWriter;
	UINT32 m_VideoBitrate;
	UINT32 m_VideoFPS;
	UINT64 m_FrameDuration;
	VideoCodec m_VideoCodec;
	Processing m_Processing;
	int m_nStreams;
	int m_NumWorkerThreads;
	int m_QualityVsSpeed;
	RateControlMode m_Mode;
	int m_Quality;
};

DWORD WINAPI ThreadOpenGLProc(LPVOID pParam)
{
	H264Writer* pWriter = (H264Writer*)(pParam);
	return ::encoder_start((UINT**)(pWriter->GetImagePtr()), pWriter->GetRequestEvent(), 
		pWriter->GetReplyEvent(), pWriter->GetExitEvent(), 
		pWriter->GetVideoEndedEvent(), pWriter->GetUrl().c_str());
}
