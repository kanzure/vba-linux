#include "stdafx.h"
#include <cstdio>
#include <cassert>

#include "AVIWrite.h"
#include "../common/System.h"

AVIWrite::AVIWrite()
{
	m_failed = false;
	m_file   = NULL;
	m_stream = NULL;
	m_streamCompressed = NULL;
	m_streamSound      = NULL;
	m_videoFrames      = 0;
	m_samplesSound     = 0;
	m_videoFramesTotal = 0;
	m_samplesSoundTotal= 0;
	m_totalBytes       = 0;
	m_segmentNumber    = 0;
	m_usePrevOptions   = false;
	m_pauseRecording   = false;

	AVIFileInit();
}

void AVIWrite::CleanUp()
{
	if (m_streamSound)
	{
		AVIStreamClose(m_streamSound);
		m_streamSound = NULL;
	}
	if (m_streamCompressed)
	{
		AVIStreamClose(m_streamCompressed);
		m_streamCompressed = NULL;
	}
	if (m_stream)
	{
		AVIStreamClose(m_stream);
		m_stream = NULL;
	}
	if (m_file)
	{
		AVIFileClose(m_file);
		m_file = NULL;
	}
}

AVIWrite::~AVIWrite()
{
	CleanUp();
	AVIFileExit();
}

void AVIWrite::SetVideoFormat(BITMAPINFOHEADER *bh)
{
	// force size to 0x28 to avoid extra fields
	memcpy(&m_bitmap, bh, 0x28);
}

void AVIWrite::SetSoundFormat(WAVEFORMATEX *format)
{
	memcpy(&m_soundFormat, format, sizeof(WAVEFORMATEX));
	ZeroMemory(&m_soundHeader, sizeof(AVISTREAMINFO));
	// setup the sound stream header
	m_soundHeader.fccType         = streamtypeAUDIO;
	m_soundHeader.dwQuality       = (DWORD)-1;
	m_soundHeader.dwScale         = format->nBlockAlign;
	m_soundHeader.dwInitialFrames = 1;
	m_soundHeader.dwRate       = format->nAvgBytesPerSec;
	m_soundHeader.dwSampleSize = format->nBlockAlign;

	// create the sound stream
	if (FAILED(AVIFileCreateStream(m_file, &m_streamSound, &m_soundHeader)))
	{
		m_failed = true;
		return;
	}

	// setup the sound stream format
	if (FAILED(AVIStreamSetFormat(m_streamSound, 0, (void *)&m_soundFormat,
	                              sizeof(WAVEFORMATEX))))
	{
		m_failed = true;
		return;
	}
}

bool AVIWrite::Open(const char *filename)
{
	// this is only here because AVIFileOpen doesn't seem to do it for us
	FILE*fd = fopen(filename, "wb");
	if (!fd)
	{
		systemMessage(0, "AVI recording failed: file is read-only or already in use.");
		m_failed = true;
		return false;
	}
	fclose(fd);

	// create the AVI file
	if (FAILED(AVIFileOpen(&m_file,
	                       filename,
	                       OF_WRITE | OF_CREATE,
	                       NULL)))
	{
		m_failed = true;
		return false;
	}
	// setup the video stream information
	ZeroMemory(&m_header, sizeof(AVISTREAMINFO));
	m_header.fccType = streamtypeVIDEO;
	m_header.dwScale = 1;
	m_header.dwRate  = m_fps;
	m_header.dwSuggestedBufferSize = m_bitmap.biSizeImage;

	// create the video stream
	if (FAILED(AVIFileCreateStream(m_file,
	                               &m_stream,
	                               &m_header)))
	{
		m_failed = true;
		return false;
	}

	if (!m_usePrevOptions)
	{
		ZeroMemory(&m_options, sizeof(AVICOMPRESSOPTIONS));
		m_arrayOptions[0] = &m_options;

		// call the dialog to setup the compress options to be used
		if (!AVISaveOptions(AfxGetApp()->m_pMainWnd->GetSafeHwnd(), 0, 1, &m_stream, m_arrayOptions))
		{
			m_failed = true;
			return false;
		}
	}

	// create the compressed stream
	if (FAILED(AVIMakeCompressedStream(&m_streamCompressed, m_stream, &m_options, NULL)))
	{
		m_failed = true;
		return false;
	}

	// setup the video stream format
	if (FAILED(AVIStreamSetFormat(m_streamCompressed, 0,
	                              &m_bitmap,
	                              m_bitmap.biSize +
	                              m_bitmap.biClrUsed * sizeof(RGBQUAD))))
	{
		m_failed = true;
		return false;
	}

	m_videoFrames  = 0;
	m_samplesSound = 0;
	m_totalBytes   = 0;
	if (!m_usePrevOptions) {
		m_videoFramesTotal  = 0;
		m_samplesSoundTotal = 0;
	}

	strncpy(m_aviFileName, filename, MAX_PATH);
	strncpy(m_aviBaseName, filename, MAX_PATH);
	m_aviFileName[MAX_PATH - 1] = '\0';
	m_aviBaseName[MAX_PATH - 1] = '\0';
	char *dot = strrchr(m_aviBaseName, '.');
	if (dot && dot > strrchr(m_aviBaseName, '/') && dot > strrchr(m_aviBaseName, '\\'))
	{
		strcpy(m_aviExtension, dot);
		dot[0] = '\0';
	}

	return true;
}

bool AVIWrite::AddSound(const u8 *sound, int len)
{
	LONG byteBuffer;

	// return if we failed somewhere already
	if (m_failed)
		return false;

	assert(len % m_soundFormat.nBlockAlign == 0);
	int samples = len / m_soundFormat.nBlockAlign;

	if (FAILED(AVIStreamWrite(m_streamSound,
	                          m_samplesSound,
	                          samples,
	                          (LPVOID)sound,
	                          len,
	                          0,
	                          NULL,
	                          &byteBuffer)))
	{
		m_failed = true;
		return false;
	}
	m_samplesSound += samples;
	m_samplesSoundTotal += samples;
	m_totalBytes   += byteBuffer;
	return true;
}

bool AVIWrite::NextSegment()
{
	char avi_fname[MAX_PATH];
	strcpy(avi_fname, m_aviBaseName);
	char avi_fname_temp[MAX_PATH];
	sprintf(avi_fname_temp, "%s_part%d%s", avi_fname, m_segmentNumber+2, m_aviExtension);
	m_segmentNumber++;

	CleanUp();

	m_usePrevOptions = true;
	bool ret = Open(avi_fname_temp);
	m_usePrevOptions = false;
	strcpy(m_aviBaseName, avi_fname);

	return ret;
}

bool AVIWrite::AddFrame(const u8 *bmp)
{
	LONG byteBuffer;

	if (m_failed)
		return false;

	// write the frame to the video stream
	if (FAILED(AVIStreamWrite(m_streamCompressed,
	                          m_videoFrames,
	                          1,
	                          (LPVOID)bmp,
	                          m_bitmap.biSizeImage,
	                          AVIIF_KEYFRAME,
	                          NULL,
	                          &byteBuffer)))
	{
		m_failed = true;
		return false;
	}
	m_videoFrames++;
	m_videoFramesTotal++;
	m_totalBytes += byteBuffer;

	// segment / split AVI when it's almost 2 GB (2000MB, to be precise)
	if (!(m_videoFrames % 60) && m_totalBytes > 2097152000)
		return NextSegment();
	else
		return true;
}

bool AVIWrite::IsSoundAdded()
{
	return m_streamSound != NULL;
}

void AVIWrite::SetFPS(int f)
{
	m_fps = f;
}

int AVIWrite::videoFrames()
{
	return m_videoFramesTotal;
}

void AVIWrite::Pause(bool pause)
{
	m_pauseRecording = pause;
}

bool AVIWrite::IsPaused()
{
	return m_pauseRecording;
}