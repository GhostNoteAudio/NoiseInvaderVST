#include <stdio.h>
#include "NoiseGateVst.h"

const char* PluginName = "Noise Invader v3.0";
const char* DeveloperName = "Ghost Note Audio";

AudioEffect* createEffectInstance (audioMasterCallback audioMaster)
{
	return new NoiseGateVst(audioMaster);
}

// ------------------------------------------------------------------------------------

NoiseGateVst::NoiseGateVst(audioMasterCallback audioMaster)
	: AudioEffectX (audioMaster, 1, (int)Parameters::Count)
{
	detectorInput = 0;
	setNumInputs(1); // Mono in
	setNumOutputs(1); // Mono out
	setUniqueID(91572786); // Random ID
	canProcessReplacing();
	canDoubleReplacing(false);
	isSynth(false);
	vst_strncpy (programName, PluginName, kVstMaxProgNameLen);
	sampleRate = 48000;

	parameters[(int)Parameters::BandUpper] = 0.2;
	parameters[(int)Parameters::BandGap] = 0.3;
	parameters[(int)Parameters::Expansion] = 0.7;
	parameters[(int)Parameters::DecayMs] = 0.5;
	parameters[(int)Parameters::Hysteresis] = 0.5;

	expander.Knee = 0.3;
	for (int i = 0; i < (int)Parameters::Count; i++)
		setParameter(i, parameters[i]);
}

bool NoiseGateVst::getInputProperties(VstInt32 index, VstPinProperties* properties)
{
	if (index == 0 || index == 1) // 0-1 = Main in
	{
		properties->arrangementType = kSpeakerArrMono;
		//properties->flags = kVstPinIsStereo;
		sprintf(properties->shortLabel, "Input");
		sprintf(properties->label, "Input");
		return true;
	}
	else
	{
		return false;
	}
}

bool NoiseGateVst::getOutputProperties(VstInt32 index, VstPinProperties* properties)
{
	if (index == 0 || index == 1) // 0-1 = Main out
	{
		properties->arrangementType = kSpeakerArrMono;
		//properties->flags = kVstPinIsStereo;
		sprintf(properties->shortLabel, "Output");
		sprintf(properties->label, "Output");
		return true;
	}
	else
	{
		return false;
	}
}

void NoiseGateVst::setProgramName(char* name)
{
	vst_strncpy(programName, name, kVstMaxProgNameLen);
}

void NoiseGateVst::getProgramName(char* name)
{
	vst_strncpy(name, programName, kVstMaxProgNameLen);
}

void NoiseGateVst::setParameter(VstInt32 index, float value)
{
	parameters[index] = value;

	switch ((Parameters)index)
	{
	case Parameters::BandUpper:
		expander.BandUpper = -120 + parameters[index] * 120;
		break;
	case Parameters::BandGap:
		expander.BandGap = 1 + parameters[index] * 19;
		break;
	case Parameters::Expansion:
		expander.Expansion = parameters[index] * 50;
		break;
	case Parameters::DecayMs:
		expander.DecayMs = 10 + parameters[index] * 290;
		break;
	case Parameters::Hysteresis:
		expander.Hysteresis = parameters[index] * 10;
		break;
	}
}

float NoiseGateVst::getParameter(VstInt32 index)
{
	return parameters[index];
}

void NoiseGateVst::getParameterName(VstInt32 index, char* label)
{
	switch ((Parameters)index)
	{
	case Parameters::BandUpper:
		strcpy(label, "Threshold");
		break;
	case Parameters::BandGap:
		strcpy(label, "Transition");
		break;
	case Parameters::Expansion:
		strcpy(label, "Expansion");
		break;
	case Parameters::DecayMs:
		strcpy(label, "Decay");
		break;
	case Parameters::Hysteresis:
		strcpy(label, "Hysteresis");
		break;
	}
}

void NoiseGateVst::getParameterDisplay(VstInt32 index, char* text)
{
	switch ((Parameters)index)
	{
	case Parameters::BandUpper:
		sprintf(text, "%.1f", expander.BandUpper);
		break;
	case Parameters::BandGap:
		sprintf(text, "%.1f", expander.BandGap);
		break;
	case Parameters::Expansion:
		sprintf(text, "%.1f", expander.Expansion);
		break;
	case Parameters::DecayMs:
		sprintf(text, "%.0f", expander.DecayMs);
		break;
	case Parameters::Hysteresis:
		sprintf(text, "%.1f", expander.Hysteresis);
		break;
	}
}

void NoiseGateVst::getParameterLabel(VstInt32 index, char* label)
{
	switch ((Parameters)index)
	{
	/*case Parameters::DetectorInput:
		strcpy(label, "");
		break;*/
	case Parameters::BandUpper:
		strcpy(label, "dB");
		break;
	case Parameters::BandGap:
		strcpy(label, "dB");
		break;
	case Parameters::Expansion:
		strcpy(label, "dB");
		break;
	case Parameters::DecayMs:
		strcpy(label, "ms");
		break;
	case Parameters::Hysteresis:
		strcpy(label, "dB");
		break;
	}
}

bool NoiseGateVst::getEffectName(char* name)
{
	vst_strncpy(name, PluginName, kVstMaxEffectNameLen);
	return true;
}

bool NoiseGateVst::getProductString(char* text)
{
	vst_strncpy(text, PluginName, kVstMaxProductStrLen);
	return true;
}

bool NoiseGateVst::getVendorString(char* text)
{
	vst_strncpy(text, DeveloperName, kVstMaxVendorStrLen);
	return true;
}

VstInt32 NoiseGateVst::getVendorVersion()
{ 
	return 2000; 
}

void NoiseGateVst::processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames)
{
	expander.Process(inputs[0], inputs[0], sampleFrames);
	Copy(outputs[0], inputs[0], sampleFrames);
}

void NoiseGateVst::setSampleRate(float sampleRate)
{
	this->sampleRate = sampleRate;
	expander.SetSamplerate(sampleRate);
}
