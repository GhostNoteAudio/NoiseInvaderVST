#include "ExternalEditor.h"
#include <Windows.h>
#include <string>

ExternalEditor::ExternalEditor(AudioEffect* effect) : AEffEditor(effect)
{
	rect.top = 0;
	rect.left = 0;
	rect.right = 800;
	rect.bottom = 800;
}

ExternalEditor::~ExternalEditor()
{
}

bool ExternalEditor::open(void *ptr)
{
	AEffEditor::open(ptr);

	rect.top = 0;
	rect.left = 0;
	rect.right = 800;
	rect.bottom = 800;
	Open = true;
	
	std::string str = "1234 ";
	str += std::to_string((int)ptr);
	ShellExecute(NULL, "open", "C:\\Src\\_Tree\\Audio\\VstNoiseGate2\\Gui\\bin\\Debug\\Gui.exe", str.c_str(), NULL, SW_SHOWDEFAULT);
		
	return false;
}

void ExternalEditor::close()
{
	Open = false;
}

bool ExternalEditor::getRect(ERect **ppErect)
{
	*ppErect = &rect;
	return false;
}

