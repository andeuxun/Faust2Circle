//
// kernel.h
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

#include <circle/actled.h>
#include <circle/koptions.h>
#include <circle/devicenameservice.h>
#include <circle/screen.h>
#include <circle/serial.h>
#include <circle/exceptionhandler.h>
#include <circle/interrupt.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <circle/sched/scheduler.h>
#include <circle/i2cmaster.h>
#include <circle/usb/usbhcidevice.h>
#include <circle/sound/soundbasedevice.h>
#include <circle/types.h>
#include <circle/i2cmaster.h>
#include <circle/sound/pwmsoundbasedevice.h>
#include <circle/string.h>

#ifdef USE_VCHIQ_SOUND
	#include <vc4/vchiq/vchiqdevice.h>
#endif


class dsp;
class MapUI;

class CFaust2Circle : public CPWMSoundBaseDevice
{

public:
	CFaust2Circle (CInterruptSystem *pInterrupt,
			 		unsigned nSampleRate = 48000,
			    	unsigned nChunkSize  = 256);


	boolean Start (void);
	boolean IsActive (void);

protected:
	void GlobalLock (void);
	void GlobalUnlock (void);

private:
	unsigned GetChunk (u32 *pBuffer, unsigned nChunkSize);

private:
	unsigned m_nMaxLevel;
	unsigned m_nNullLevel;


    float** fInChannel;
    float** fOutChannel;
    MapUI* fUI;
    dsp* fDSP;

protected:
	float m_fVolume;
};