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
#ifndef _kernel_h
#define _kernel_h

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
#include "oscillator.h"
#include <circle/i2cmaster.h>
#include <circle/sound/pwmsoundbasedevice.h>
#include <circle/string.h>

#ifdef USE_VCHIQ_SOUND
	#include <vc4/vchiq/vchiqdevice.h>
#endif

enum TShutdownMode
{
	ShutdownNone,
	ShutdownHalt,
	ShutdownReboot
};

class CTest : public CPWMSoundBaseDevice
{

public:
	CTest (CInterruptSystem *pInterrupt,
			     unsigned	       nSampleRate = 48000,
			     unsigned	       nChunkSize  = 256);

	boolean Start (void);
	boolean IsActive (void);

private:
	unsigned GetChunk (u32 *pBuffer, unsigned nChunkSize);

private:
	CInterruptSystem *m_pInterrupt;
	unsigned m_nChunkSize;
	unsigned m_nSampleRate;

	unsigned m_nMaxLevel;
	unsigned m_nNullLevel;
	boolean m_bChannelsSwapped;

	COscillator m_LFO;
	COscillator m_VFO;
};


class CKernel
{
public:
	CKernel (void);
	~CKernel (void);

	boolean Initialize (void);

	TShutdownMode Run (void);

private:
	// do not change this order
	CActLED			m_ActLED;
	CKernelOptions		m_Options;
	CDeviceNameService	m_DeviceNameService;
	CScreenDevice		m_Screen;
	CSerialDevice		m_Serial;
	CExceptionHandler	m_ExceptionHandler;
	CInterruptSystem	m_Interrupt;
	CTimer			m_Timer;
	CLogger			m_Logger;
	CScheduler		m_Scheduler;
#if RASPPI <= 4
	CI2CMaster		m_I2CMaster;
#endif
	CUSBHCIDevice		m_USBHCI;

#ifdef USE_VCHIQ_SOUND
	CVCHIQDevice		m_VCHIQ;
#endif
	CTest m_Sound;
};


#endif
