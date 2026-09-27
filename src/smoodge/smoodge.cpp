#include "../../lib/carciofo.h"
#include "daisysp.h"
#include "dev/sdram.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;

static const size_t kMaxDelay = 48000 * 16;
static DelayLine<float, kMaxDelay> DSY_SDRAM_BSS dell;
static DelayLine<float, kMaxDelay> DSY_SDRAM_BSS delr;
static Chorus chorus;
static Tremolo bcL;
static Tremolo bcR;
static Oscillator flutter;
static Carciofo hw;

static float currentDelayL = 1.f, currentDelayR = 1.f;
static float clockSecs = 0.4f;
static volatile float targetL = 19200.f, targetR = 19200.f;
static volatile float feedback = 0.4f, pot4 = 0.f, wet = 0.5f, dry = 0.5f, gainLevel = 0.5f;

static float Clamp01(float v)
{
    if(v < 0.f) return 0.f;
    if(v > 1.f) return 1.f;
    return v;
}

static float Divisor(float pot)
{
    if(pot == 0.f) return 8.f;
    if(pot <= 0.1f) return 4.f;
    if(pot <= 0.2f) return 3.f;
    if(pot <= 0.3f) return 2.f;
    if(pot <= 0.4f) return 1.5f;
    if(pot <= 0.5f) return 1.f;
    if(pot <= 0.6f) return 0.5f;
    if(pot <= 0.7f) return 0.33f;
    if(pot <= 0.8f) return 0.25f;
    if(pot <= 0.9f) return 0.2f;
    return 0.125f;
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        float dryL = in[0][i];
        float dryR = in[1][i];
        chorus.Process((dryL + dryR) * gainLevel);
        float chorusL = chorus.GetLeft();
        float chorusR = chorus.GetRight();
        fonepole(currentDelayL, targetL, 0.0001f);
        fonepole(currentDelayR, targetR * (1.f - (pot4 / 14.f)), 0.0001f);
        if(currentDelayL < 1.f) currentDelayL = 1.f;
        if(currentDelayR < 1.f) currentDelayR = 1.f;
        if(currentDelayL > (float)(kMaxDelay - 2)) currentDelayL = (float)(kMaxDelay - 2);
        if(currentDelayR > (float)(kMaxDelay - 2)) currentDelayR = (float)(kMaxDelay - 2);
        delr.SetDelay(currentDelayL);
        dell.SetDelay(currentDelayR);
        float delayOutL = dell.Read();
        float delayOutR = delr.Read();
        dell.Write((feedback * delayOutL) + dryL + (chorusL * pot4));
        delr.Write((feedback * delayOutR) + dryR + (chorusR * pot4));
        delayOutL = bcL.Process(delayOutL);
        delayOutR = bcR.Process(delayOutR);
        out[0][i] = SoftLimit((dryL * dry) + (delayOutL * wet));
        out[1][i] = SoftLimit((dryR * dry) + (delayOutR * wet));
    }
}

int main(void)
{
    hw.Init();
    hw.seed.StartLog();
    float sr = hw.SampleRate();
    dell.Init();
    delr.Init();
    chorus.Init(sr);
    bcL.Init(sr);
    bcR.Init(sr);
    bcL.SetWaveform(Oscillator::WAVE_SIN);
    bcR.SetWaveform(Oscillator::WAVE_SIN);
    flutter.Init(sr);
    flutter.SetWaveform(Oscillator::WAVE_SIN);
    flutter.SetAmp(0.f);
    flutter.SetFreq(0.05f);
    hw.StartAudio(AudioCallback);

    float cv2 = 0.f;
    bool oldClk = false;
    int numClks = 0;
    uint32_t previousTs = 0;
    uint32_t durationArr[3] = {0, 0, 0};
    while(1)
    {
        hw.ProcessControls();
        float cv1 = hw.GetCv(CV_1);
        float cv2Temp = hw.GetCv(CV_2);
        if(fabsf(cv2Temp - cv2) > 0.01f)
            cv2 = cv2Temp;
        float pot1 = hw.GetPot(POT_1);
        wet = hw.GetPot(POT_2);
        dry = 1.f - wet;
        float pot3 = hw.GetPot(POT_3);
        pot4 = hw.GetPot(POT_4);
        feedback = Clamp01(cv2 + pot1);

        bool clk = cv1 >= 0.5f;
        if(clk != oldClk && clk)
        {
            numClks++;
            if(previousTs != 0)
            {
                uint32_t now = System::GetNow();
                durationArr[numClks % 3] = now - previousTs;
                if(numClks >= 3)
                {
                    float sum = 0.f;
                    for(int i = 0; i < 3; i++)
                        sum += (float)durationArr[i];
                    clockSecs = (sum / 3.f) / 1000.f;
                }
                if(numClks % 4 == 0)
                {
                    float rndTrem = (1 + (numClks % 10)) / 10.f;
                    bcL.SetFreq(rndTrem - 0.1f);
                    bcR.SetFreq(rndTrem);
                }
                previousTs = now;
            }
            else
                previousTs = System::GetNow();
        }
        oldClk = clk;

        float samples = (clockSecs * 48000.f) / Divisor(pot3);
        float wow = flutter.Process();
        flutter.SetAmp(9.f * pot4);
        float flutterAmt = (20.f * wow) * (pot4 * pot4);
        targetL = samples + flutterAmt;
        targetR = samples + flutterAmt;
        gainLevel = 0.5f + pot4 * 0.4f;
        chorus.SetLfoFreq(pot4);
        chorus.SetLfoDepth(0.2f);
        chorus.SetFeedback(pot4 * pot4 * 0.5f);
        bcL.SetDepth(pot4 * 0.61f);
        bcR.SetDepth(pot4 * 0.6f);
    }
}
