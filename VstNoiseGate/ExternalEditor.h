#pragma once

#ifndef __aeffeditor__
#include "aeffeditor.h"
#endif

#include "NoiseGateVst.h"
#include <windows.h>

class ExternalEditor : public AEffEditor
{
public:
	NoiseGateVst* instance;

	ExternalEditor(AudioEffect* effect);
	virtual ~ExternalEditor();

	virtual bool open(void* ptr);
	virtual void close();
	virtual bool isOpen() { return Open; }

	bool Open;
	bool getRect(ERect** rect);

protected:
	ERect rect;
};
