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
#include "circle.h"
#include "config.h"
#include <circle/sound/pwmsoundbasedevice.h>
#include <circle/sound/i2ssoundbasedevice.h>
#include <circle/sound/hdmisoundbasedevice.h>
#include <circle/sound/usbsoundbasedevice.h>
#include <circle/machineinfo.h>
#include <circle/util.h>
#include <assert.h>

#include <circle/synchronize.h>


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

#ifdef USE_VCHIQ_SOUND
	#include <vc4/sound/vchiqsoundbasedevice.h>
#endif

CFaust2Circle::CFaust2Circle(CInterruptSystem *pInterrupt, unsigned nSampleRate, unsigned nChunkSize) 
:	CPWMSoundBaseDevice (pInterrupt, nSampleRate, nChunkSize),
	
	m_nMaxLevel (GetRangeMax ()-1), // GetRangeMax import
	m_nNullLevel (m_nMaxLevel / 2),
	m_fVolume (1.0) // Constant but could be changed
{
	#ifdef NVOICES
    int nvoices = NVOICES;
    fDSP = new mydsp_poly(new mydsp(), nvoices, true, true);
#else
    fDSP = new mydsp();
#endif
    
    fDSP->init(AUDIO_SAMPLE_RATE_EXACT);
    
    fUI = new MapUI();
    fDSP->buildUserInterface(fUI);
    
    // allocating Faust inputs
    if (fDSP->getNumInputs() > 0) {
        fInChannel = new float*[fDSP->getNumInputs()];
        for (int i = 0; i < fDSP->getNumInputs(); i++) {
            fInChannel[i] = new float[AUDIO_BLOCK_SAMPLES];
        }
    } else {
        fInChannel = NULL;
    }
    
    // allocating Faust outputs
    if (fDSP->getNumOutputs() > 0) {
        fOutChannel = new float*[fDSP->getNumOutputs()];
        for (int i = 0; i < fDSP->getNumOutputs(); i++) {
            fOutChannel[i] = new float[AUDIO_BLOCK_SAMPLES];
        }
    } else {
        fOutChannel = NULL;
    }
    
#if MIDICTRL
    fMIDIHandler = new teensy_midi();
    fMIDIInterface = new MidiUI(fMIDIHandler);
    fDSP->buildUserInterface(fMIDIInterface);
    fMIDIInterface->run();
#endif

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

