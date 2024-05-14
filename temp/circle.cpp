#include "kernel.h"
#include "config.h"
#include <circle/sound/pwmsoundbasedevice.h>
#include <circle/sound/i2ssoundbasedevice.h>
#include <circle/sound/hdmisoundbasedevice.h>
#include <circle/sound/usbsoundbasedevice.h>
#include <circle/machineinfo.h>
#include <circle/util.h>
#include <assert.h>
#include "oscillator.h"
#include <circle/synchronize.h>


CFaust2Circle::CFaust2Circle(CInterruptSystem *pInterrupt, unsigned nSampleRate, unsigned nChunkSize) 
:	CPWMSoundBaseDevice (pInterrupt, nSampleRate, nChunkSize),
	m_VFO (&m_LFO),
	m_nMaxLevel (GetRangeMax ()-1), // GetRangeMax import
	m_nNullLevel (m_nMaxLevel / 2),
	m_fVolume (1.0) // Constant but could be changed
{
	// initialize oscillators
	m_LFO.SetWaveform (WaveformSine);
	m_LFO.SetFrequency (10.0);

	m_VFO.SetWaveform (WaveformSine);
	m_VFO.SetFrequency (440.0);
	m_VFO.SetModulationVolume (0.25);

}

void CFaust2Circle::GlobalLock (void)
{
	EnterCritical (IRQ_LEVEL);
}

void CFaust2Circle::GlobalUnlock (void)
{
	LeaveCritical ();
}

boolean CFaust2Circle::Start (void) 
{
	return CPWMSoundBaseDevice::Start ();
}

boolean CFaust2Circle::IsActive (void)
{
	return CPWMSoundBaseDevice::IsActive ();
}

unsigned CFaust2Circle::GetChunk (u32 *pBuffer, unsigned nChunkSize)
{
#ifdef SHOW_STATUS
	unsigned nTicks = CTimer::GetClockTicks ();
#endif

	GlobalLock ();

	unsigned nResult = nChunkSize;

	float fVolumeLevel = m_fVolume * m_nMaxLevel/2;

	for (; nChunkSize > 0; nChunkSize -= 2)		// fill the whole buffer
	{
		m_LFO.NextSample ();
		m_VFO.NextSample ();

		float fLevel = m_VFO.GetOutputLevel ();

		TYPE nLevel = (TYPE) (fLevel*fVolumeLevel + m_nNullLevel);

		*pBuffer++ = (u32) nLevel;
		*pBuffer++ = (u32) nLevel;
	}

#ifdef SHOW_STATUS
	nTicks = CTimer::GetClockTicks () - nTicks;
	if (nTicks > m_nMaxDelayTicks)
	{
		m_nMaxDelayTicks = nTicks;
	}
#endif

	GlobalUnlock ();

	return nResult;
}