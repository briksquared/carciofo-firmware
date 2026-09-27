#include "../../lib/carciofo.h"
#include "daisysp.h"
#include "dev/sdram.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static const size_t kDelay = 120000;
static PitchShifter DSY_SDRAM_BSS shifter;
static Chorus chorus;
static ReverbSc DSY_SDRAM_BSS verb;
static DelayLine<float, kDelay> DSY_SDRAM_BSS dell;
static DelayLine<float, kDelay> DSY_SDRAM_BSS delr;
static Svf filtL;
static Svf filtR;
static volatile float fb = 0.5f, wet = 0.5f, tone = 0.5f, shim = 0.4f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    float delaySamps = hw.SampleRate() * 0.15f;
    for(size_t i = 0; i < size; i++)
    {
        float dryL = in[0][i];
        float dryR = in[1][i];
        float mono = (dryL * 0.7f) + (dryR * 0.7f);
        shifter.SetTransposition(24.f);
        float up = mono;
        float pitch1 = shifter.Process(up);
        shifter.SetTransposition(12.f);
        float src = mono;
        float pitch2 = shifter.Process(src);
        float shimAmt = shim * shim;
        float pitched = (pitch1 * (0.5f * shimAmt)) + (pitch2 * (0.2f * shimAmt));
        chorus.Process(pitched);
        float chorusL = chorus.GetLeft();
        float chorusR = chorus.GetRight();
        delr.SetDelay(delaySamps);
        dell.SetDelay(delaySamps);
        float delayOutL = dell.Read();
        float delayOutR = delr.Read();
        float feedback = shim * 0.91f;
        dell.Write((feedback * delayOutL) + (dryL + (chorusL * 0.5f)));
        delayOutL = (feedback * delayOutL) + ((1.f - feedback) * (dryL + (chorusL * 0.5f)));
        delr.Write((feedback * delayOutR) + (dryL + (chorusR * 0.5f)));
        delayOutR = (feedback * delayOutR) + ((1.f - feedback) * (dryR + (chorusR * 0.5f)));
        float verbL, verbR;
        verb.Process((dryL * 0.5f) + (delayOutL * shim), (dryL * 0.5f) + (delayOutR * shim), &verbL, &verbR);
        filtL.Process(verbL * wet);
        filtR.Process(verbR * wet);
        out[0][i] = (dryL * (1.f - wet)) + filtL.High();
        out[1][i] = (dryR * (1.f - wet)) + filtR.High();
        (void)fb;
    }
}

int main(void)
{
    hw.Init();
    hw.seed.StartLog();
    float sr = hw.SampleRate();
    shifter.Init(sr);
    chorus.Init(sr);
    verb.Init(sr);
    dell.Init();
    delr.Init();
    filtL.Init(sr);
    filtR.Init(sr);
    chorus.SetLfoFreq(0.33f, 0.2f);
    chorus.SetDelay(0.75f, 0.9f);
    chorus.SetLfoDepth(1.f, 1.f);
    chorus.SetFeedback(0.75f, 0.75f);
    dell.SetDelay(sr * 0.15f);
    delr.SetDelay(sr * 0.15f - 0.01f);
    filtL.SetRes(0.1f);
    filtR.SetRes(0.1f);
    filtL.SetDrive(0.6f);
    filtR.SetDrive(0.6f);
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessControls();
        fb   = hw.GetPot(POT_1);
        wet  = hw.GetPot(POT_2);
        tone = hw.GetPot(POT_3) + hw.GetCv(CV_1);
        shim = hw.GetPot(POT_4) + hw.GetCv(CV_2);
        if(tone < 0.f) tone = 0.f;
        if(tone > 1.f) tone = 1.f;
        if(shim < 0.f) shim = 0.f;
        if(shim > 1.f) shim = 1.f;
        verb.SetFeedback(0.1f + 0.8f * fb);
        float lp = expf(logf(100.f) + tone * (logf(28000.f) - logf(100.f)));
        verb.SetLpFreq(lp);
        filtL.SetFreq(500.f * tone);
        filtR.SetFreq(500.f * tone);
    }
}
