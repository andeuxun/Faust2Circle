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

	COscillator m_VFO;
	COscillator m_LFO;

protected:
	float m_fVolume;
};