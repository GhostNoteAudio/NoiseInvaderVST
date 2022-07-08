#pragma once
#include <cmath>

template<typename T>
inline void Copy(T* dest, T* source, int len)
{
    memcpy(dest, source, len * sizeof(T));
}

template<typename T>
inline double DB2Gain(T input)
{
    return std::pow(10, input / 20.0);
}

template<typename T>
inline double Gain2DB(T input)
{
    //if (input < 0.0000001)
    //    return -100000;

    return 20.0 * std::log10(input);
}

inline double Response2Dec(double input)
{
    return std::fmin((std::pow(100, input) - 1.0) * 0.01010101010101, 1.0);
}

class Expander
{
    int samplerate;

    float accHipass = 0.0;
    float fVal = -150.0;
    float slewVal = 0.0;
    float expanderOut = -150.0;
    float expanderSigfiltOut = 0.0;

    float bl = 0.0;
    float bu = 0.0;
    bool resetBands = false;

    float buffer[16384];

public:
    // Comments show unit and suggested range of input values
    float BandUpper; // dB [-120, -20]
    float BandGap; // dB [0, 20]
    float Expansion; // db [0, 60]
    float DecayMs; // milliseconds [1, 300]
    float Hysteresis; // db [0, 10]
    float Knee; // fraction [0, 1]

    float p0x, p0y, p1x, p1y, p2x, p2y;

    // Readonly values for debugging
    float GainSensorRO, UpperBandRO, ExpansionRO;

    Expander()
    {
        this->samplerate = 48000;
        SetDefaults();
    }

    void SetSamplerate(int samplerate)
    {
        this->samplerate = samplerate;
    }

    // Apply some decent defaults for a high-gain guitar sound
    void SetDefaults()
    {
        Hysteresis = 6.0;
        BandUpper = -75;
        BandGap = 5;
        Expansion = 30;
        DecayMs = 50;
        Knee = 0.8;
    }

    void ResetBands()
    {
        resetBands = true;
    }

    void SlewLimit(float* ins, int len)
    {
        float decaySample = 30.0 / (DecayMs / 1000.0) / samplerate; // 90db decay per second

        for (int i = 0; i < len; i++)
        {
            if (ins[i] < slewVal - decaySample)
                ins[i] = slewVal - decaySample;
            slewVal = ins[i];
        }
    }

    void Hipass(float* ins, int len)
    {
        for (int i = 0; i < len; i++)
        {
            accHipass = 0.8 * accHipass + 0.2 * ins[i];
            ins[i] = ins[i] - accHipass;
        }
    }

    void AbsOffset(float* ins, int len)
    {
        float offs = DB2Gain(-150);
        for (int i = 0; i < len; i++)
        {
            ins[i] = fabsf(ins[i]) + offs;
        }
    }

    void Gain2db(float* ins, int len)
    {
        for (int i = 0; i < len; i++)
        {
            ins[i] = Gain2DB(ins[i]);
        }
    }

    void GainDetect(float* ins, int len)
    {
        Hipass(ins, len);
        AbsOffset(ins, len);
        Gain2db(ins, len);
        SlewLimit(ins, len);
        for (int i = 0; i < len; i++)
        {
            fVal = fVal * 0.998 + ins[i] * 0.002;
            ins[i] = fVal;
        }
    }

    void Process(float* inData, float* sensorData, int len)
    {
        float BandLower = BandUpper - BandGap;
        float BandDelta = BandGap;
        auto dbData = buffer;
        Copy(dbData, sensorData, len);
        GainDetect(dbData, len);

        // needed when you tweak the bands, otherwise the effect can get stuck in a weird state.
        if (resetBands)
        {
            bl = BandLower;
            bu = BandUpper;
            resetBands = false;
        }

        // calculate knee bezier points
        float knee = Knee * BandDelta;
        float k = Expansion / BandDelta;
        p0x = BandUpper + knee;
        p0y = BandUpper + knee;
        p1x = BandUpper;
        p1y = BandUpper;
        p2x = BandUpper - knee;
        p2y = BandUpper - (1 + k) * knee;

        float diff = 0;

        for (int i = 0; i < len; i++)
        {
            float sample = dbData[i];
            float value = 0.0;

            // Apply hysteresis to bands
            if (sample < BandLower)
            {
                bl = BandLower + Hysteresis;
                bu = BandUpper + Hysteresis;
            }
            else if (sample > (BandUpper + Hysteresis))
            {
                bl = BandLower;
                bu = BandUpper;
            }

            // Apply expansion based on whether we are:
            // Above the upper band (no expansion)
            if (sample >= bu + knee)
                value = sample;
            else if (sample <= bl)
                value = sample - Expansion;
            else
            {
                if (sample >= bu - knee)
                {
                    float t = (sample - p0x) / (p2x - p0x);
                    value = p1y + (1 - t) * (1 - t) * (p0y - p1y) + t * t * (p2y - p1y);
                }
                else
                {
                    value = bu - (sample - bu) / (bl - bu) * (BandDelta + Expansion);
                }
            }

            // post below-threshold filtering. This applies different low-pass filtering to the signal
            // above, below and between the bands. It also filters upwards moves more heavily than downwards moves.
            // Note: the filter coefficients are chosen based on assumed 48Khz sampling rate

            if (value < bl)
            {
                if (value > expanderOut)
                {
                    expanderOut = expanderOut * 0.99998 + value * 0.000002;
                    expanderSigfiltOut = expanderSigfiltOut * 0.99998 + sample * 0.000002;
                }
                else
                {
                    expanderOut = expanderOut * 0.9999 + value * 0.0001;
                    expanderSigfiltOut = expanderSigfiltOut * 0.9999 + sample * 0.0001;
                }
            }
            else if (value < bu)
            {
                if (value > expanderOut)
                {
                    expanderOut = expanderOut * 0.999 + value * 0.001;
                    expanderSigfiltOut = expanderSigfiltOut * 0.999 + sample * 0.001;
                }
                else
                {
                    expanderOut = expanderOut * 0.998 + value * 0.002;
                    expanderSigfiltOut = expanderSigfiltOut * 0.998 + sample * 0.002;
                }
            }
            else
            {
                expanderOut = expanderOut * 0.8 + value * 0.2;
                expanderSigfiltOut = expanderSigfiltOut * 0.8 + sample * 0.2;
            }

            diff = expanderSigfiltOut - expanderOut;
            float diffGain = DB2Gain(-diff);
            inData[i] *= diffGain;
        }

        GainSensorRO = expanderSigfiltOut;
        ExpansionRO = diff;
        UpperBandRO = bu;
    }

    inline int GetSamplerate()
    {
        return this->samplerate;
    }

    inline void Initialize()
    {
        // run some zeros through the algorithm to initialize all the filters and followers
        for (int i = 0; i < 1000; i++)
        {
            float dummyData[1024] = { 0.0001f };
            Process(dummyData, dummyData, 1024);
        }
    }
};
