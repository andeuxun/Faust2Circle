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

/************************BEGIN DSP SECTION **************************/

<<includeIntrinsic>>
<<includeclass>>

/**************************END DSP SECTION **************************/

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
    
    fDSP->init(nSampleRate);
    
    fUI = new MapUI();
    fDSP->buildUserInterface(fUI);
    
    // allocating Faust inputs
    if (fDSP->getNumInputs() > 0) {
        fInChannel = new float*[fDSP->getNumInputs()];
        for (int i = 0; i < fDSP->getNumInputs(); i++) {
            fInChannel[i] = new float[nChunkSize/2];
        }
    } else {
        fInChannel = NULL;
    }
    
    // allocating Faust outputs
    if (fDSP->getNumOutputs() > 0) {
        fOutChannel = new float*[fDSP->getNumOutputs()];
        for (int i = 0; i < fDSP->getNumOutputs(); i++) {
            fOutChannel[i] = new float[nChunkSize/2];
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

	// Manage input, remove comment to test after completing
	/*
	for (int i=0; i < nChunkSize/2; i++)		// fill the input buffers
	{
		// Get sound from inputs
		s16 sLevelLeft = (s16) m_nNullLevel;
		s16 sLevelRight = (s16) m_nNullLevel;

		// Transition to floats
		fLevelLeft = 0 ; // CONVERSION NEEDED
		fLevelRight = 0 ; // COVERSION NEEDED

		// Normalise levels
		float nLevelLeft = (float) (fLevelLeft*fVolumeLevel + m_nNullLevel);
		float nLevelRight = (float) (fLevelRight*fVolumeLevel + m_nNullLevel);

		// Test L levels
		if (nLevelLeft > (float) m_nMaxLevel){nLevelLeft = m_nMaxLevel;}
		else if (nLevelLeft < 0.0){nLevelLeft = 0.0;}

		// Test R levels 
		if (nLevelRight > (float) m_nMaxLevel){nLevelRight = m_nMaxLevel;}
		else if (nLevelRight < 0.0){nLevelRight = 0.0;}

		fInChannel[0][i] = nLevelLeft;
		fInChannel[1][i] = nLevelRight;

	}
	*/

	fDSP->compute(nChunkSize/2,fInChannel, fOutChannel);
	
	for (int j=0; j < nChunkSize/2; j++)		// fill the whole buffer
	{
		float fLevelLeft = fOutChannel[0][j];
		float fLevelRight = fOutChannel[1][j];

		int nLevelLeft = (int) (fLevelLeft*fVolumeLevel + m_nNullLevel);
		int nLevelRight = (int) (fLevelRight*fVolumeLevel + m_nNullLevel);

		// Test L levels
		if (nLevelLeft > (int) m_nMaxLevel){nLevelLeft = m_nMaxLevel;}
		else if (nLevelLeft < 0){nLevelLeft = 0;}

		// Test R levels 
		if (nLevelRight > (int) m_nMaxLevel){nLevelRight = m_nMaxLevel;}
		else if (nLevelRight < 0){nLevelRight = 0;}

		*pBuffer++ = (u32) nLevelLeft;
		*pBuffer++ = (u32) nLevelRight;
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

