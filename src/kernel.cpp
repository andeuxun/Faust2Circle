//
// kernel.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014-2023  R. Stange <rsta2@o2online.de>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include "kernel.h"
#include "config.h"
#include <circle/sound/pwmsoundbasedevice.h>
#include <circle/sound/i2ssoundbasedevice.h>
#include <circle/sound/hdmisoundbasedevice.h>
#include <circle/sound/usbsoundbasedevice.h>
#include <circle/machineinfo.h>
#include <circle/util.h>
#include <assert.h>

#ifdef USE_VCHIQ_SOUND
	#include <vc4/sound/vchiqsoundbasedevice.h>
#endif

#if WRITE_FORMAT == 0
	#define FORMAT		SoundFormatUnsigned8
	#define TYPE		u8
	#define TYPE_SIZE	sizeof (u8)
	#define FACTOR		((1 << 7)-1)
	#define NULL_LEVEL	(1 << 7)
#elif WRITE_FORMAT == 1
	#define FORMAT		SoundFormatSigned16
	#define TYPE		s16
	#define TYPE_SIZE	sizeof (s16)
	#define FACTOR		((1 << 15)-1)
	#define NULL_LEVEL	0
#elif WRITE_FORMAT == 2
	#define FORMAT		SoundFormatSigned24
	#define TYPE		s32
	#define TYPE_SIZE	(sizeof (u8)*3)
	#define FACTOR		((1 << 23)-1)
	#define NULL_LEVEL	0
#endif

static const char FromKernel[] = "kernel";

CKernel::CKernel (void)
:	m_Screen (m_Options.GetWidth (), m_Options.GetHeight ()),
	m_Timer (&m_Interrupt),
	m_Logger (m_Options.GetLogLevel (), &m_Timer),
#if RASPPI <= 4
	m_I2CMaster (CMachineInfo::Get ()->GetDevice (DeviceI2CMaster), TRUE),
#endif
	m_USBHCI (&m_Interrupt, &m_Timer, FALSE),
#ifdef USE_VCHIQ_SOUND
	m_VCHIQ (CMemorySystem::Get (), &m_Interrupt),
#endif
	m_pSound (0),
	m_VFO (&m_LFO)		// LFO modulates the VFO
{
	m_ActLED.Blink (5);	// show we are alive
}

CKernel::~CKernel (void)
{
}

boolean CKernel::Initialize (void)
{
	boolean bOK = TRUE;

	if (bOK)
	{
		bOK = m_Screen.Initialize ();
	}

	if (bOK)
	{
		bOK = m_Serial.Initialize (115200);
	}

	if (bOK)
	{
		CDevice *pTarget = m_DeviceNameService.GetDevice (m_Options.GetLogDevice (), FALSE);
		if (pTarget == 0)
		{
			pTarget = &m_Screen;
		}

		bOK = m_Logger.Initialize (pTarget);
	}

	if (bOK)
	{
		bOK = m_Interrupt.Initialize ();
	}

	if (bOK)
	{
		bOK = m_Timer.Initialize ();
	}
+
#if RASPPI <= 4
	if (bOK)
	{
		bOK = m_I2CMaster.Initialize ();
	}
#endif

	if (bOK)
	{
		bOK = m_USBHCI.Initialize ();
	}

#ifdef USE_VCHIQ_SOUND
	if (bOK)
	{
		bOK = m_VCHIQ.Initialize ();
	}
#endif

	return bOK;
}

TShutdownMode CKernel::Run (void)
{
	m_Logger.Write (FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);

	// initialize oscillators
	m_LFO.SetWaveform (WaveformSine);
	m_LFO.SetFrequency (10.0);

	m_VFO.SetWaveform (WaveformSine);
	m_VFO.SetFrequency (440.0);
	m_VFO.SetModulationVolume (0.25);

	// configure sound device
	
	m_Sound.start ();

	// main loop

	while (m_Sound.IsActive ()) {
        // Just loop here for as long as sound is active.
        // I know it's a bit weird, but no audio processing actually happens here;
        // It all takes place in CTest::GetChunk
    }

	return ShutdownHalt;
}

boolean CTest::Start (void)
{
	return CPWMSoundBaseDevice::Start ();
}

boolean CTest::IsActive (void)
{
	return CPWMSoundBaseDevice::IsActive ();
}

unsigned CTest::GetChunk (u32 *pBuffer, unsigned nChunkSize)
{
#ifdef SHOW_STATUS
	unsigned nTicks = CTimer::GetClockTicks ();
#endif

	//GlobalLock ();

	unsigned nResult = nChunkSize;

	float fVolumeLevel = m_fVolume * m_nMaxLevel/2;

	for (; nChunkSize > 0; nChunkSize -= 2)		// fill the whole buffer
	{
		m_LFO.NextSample ();
		m_VFO.NextSample ();

		float fLevelLeft = m_VFO.GetOutputLevelLeft ();
		int nLevelLeft = (int) (fLevelLeft*fVolumeLevel + m_nNullLevel);
		if (nLevelLeft > (int) m_nMaxLevel)
		{
			nLevelLeft = m_nMaxLevel;
		}
		else if (nLevelLeft < 0)
		{
			nLevelLeft = 0;
		}

		float fLevelRight = m_VFO.GetOutputLevelRight ();
		int nLevelRight = (int) (fLevelRight*fVolumeLevel + m_nNullLevel);
		if (nLevelRight > (int) m_nMaxLevel)
		{
			nLevelRight = m_nMaxLevel;
		}
		else if (nLevelRight < 0)
		{
			nLevelRight = 0;
		}

		// for 2 stereo channels
		if (!m_bChannelsSwapped)
		{
			*pBuffer++ = (u32) nLevelLeft;
			*pBuffer++ = (u32) nLevelRight;
		}
		else
		{
			*pBuffer++ = (u32) nLevelRight;
			*pBuffer++ = (u32) nLevelLeft;
		}
	}

#ifdef SHOW_STATUS
	nTicks = CTimer::GetClockTicks () - nTicks;
	if (nTicks > m_nMaxDelayTicks)
	{
		m_nMaxDelayTicks = nTicks;
	}
#endif

	//GlobalUnlock ();

	return nResult;
}

