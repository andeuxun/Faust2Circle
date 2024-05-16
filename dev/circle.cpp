/* ------------------------------------------------------------
name: "example"
version: "2.72.16"
Code generated with Faust 2.72.16 (https://faust.grame.fr)
Compilation options: -a faust2circle.cpp -lang cpp -ct 1 -fm def -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0
------------------------------------------------------------ */

#ifndef  __mydsp_H__
#define  __mydsp_H__

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

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <math.h>

#ifndef FAUSTCLASS 
#define FAUSTCLASS mydsp
#endif

#ifdef __APPLE__ 
#define exp10f __exp10f
#define exp10 __exp10
#endif

#if defined(_WIN32)
#define RESTRICT __restrict
#else
#define RESTRICT __restrict__
#endif


class mydsp : public dsp {
	
 private:
	
	FAUSTFLOAT fHslider0;
	int fSampleRate;
	float fConst0;
	float fConst1;
	float fRec0[2];
	FAUSTFLOAT fHslider1;
	
 public:
	mydsp() {}

	void metadata(Meta* m) { 
		m->declare("compile_options", "-a faust2circle.cpp -lang cpp -ct 1 -fm def -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0");
		m->declare("filename", "expanded.dsp");
		m->declare("library_path0", "/usr/local/share/faust/stdfaust.lib");
		m->declare("library_path1", "/usr/local/share/faust/oscillators.lib");
		m->declare("library_path2", "/usr/local/share/faust/maths.lib");
		m->declare("library_path3", "/usr/local/share/faust/platform.lib");
		m->declare("maths_lib_author", "GRAME");
		m->declare("maths_lib_copyright", "GRAME");
		m->declare("maths_lib_license", "LGPL with exception");
		m->declare("maths_lib_name", "Faust Math Library");
		m->declare("maths_lib_version", "2.8.0");
		m->declare("name", "example");
		m->declare("oscillators_lib_name", "Faust Oscillator Library");
		m->declare("oscillators_lib_saw2ptr_author", "Julius O. Smith III");
		m->declare("oscillators_lib_saw2ptr_license", "STK-4.3");
		m->declare("oscillators_lib_version", "1.5.1");
		m->declare("platform_lib_name", "Generic Platform Library");
		m->declare("platform_lib_version", "1.3.0");
		m->declare("version", "2.72.16");
	}

	virtual int getNumInputs() {
		return 0;
	}
	virtual int getNumOutputs() {
		return 2;
	}
	
	static void classInit(int sample_rate) {
	}
	
	virtual void instanceConstants(int sample_rate) {
		fSampleRate = sample_rate;
		fConst0 = std::min<float>(1.92e+05f, std::max<float>(1.0f, float(fSampleRate)));
		fConst1 = 1.0f / fConst0;
	}
	
	virtual void instanceResetUserInterface() {
		fHslider0 = FAUSTFLOAT(2e+02f);
		fHslider1 = FAUSTFLOAT(0.5f);
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			fRec0[l0] = 0.0f;
		}
	}
	
	virtual void init(int sample_rate) {
		classInit(sample_rate);
		instanceInit(sample_rate);
	}
	
	virtual void instanceInit(int sample_rate) {
		instanceConstants(sample_rate);
		instanceResetUserInterface();
		instanceClear();
	}
	
	virtual mydsp* clone() {
		return new mydsp();
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("example");
		ui_interface->addHorizontalSlider("freq", &fHslider0, FAUSTFLOAT(2e+02f), FAUSTFLOAT(5e+01f), FAUSTFLOAT(1e+03f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("gain", &fHslider1, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* output0 = outputs[0];
		FAUSTFLOAT* output1 = outputs[1];
		float fSlow0 = std::max<float>(1.1920929e-07f, fast_fabsf(float(fHslider0)));
		float fSlow1 = fConst1 * fSlow0;
		float fSlow2 = 1.0f - fConst0 / fSlow0;
		float fSlow3 = float(fHslider1);
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			float fTemp0 = fSlow1 + fRec0[1] + -1.0f;
			int iTemp1 = fTemp0 < 0.0f;
			float fTemp2 = fSlow1 + fRec0[1];
			fRec0[0] = ((iTemp1) ? fTemp2 : fTemp0);
			float fRec1 = ((iTemp1) ? fTemp2 : fSlow1 + fRec0[1] + fSlow2 * fTemp0);
			float fTemp3 = fSlow3 * (2.0f * fRec1 + -1.0f);
			output0[i0] = FAUSTFLOAT(fTemp3);
			output1[i0] = FAUSTFLOAT(fTemp3);
			fRec0[1] = fRec0[0];
		}
	}

};

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


#endif
