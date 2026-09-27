#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static Particle osc;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        float w   = osc.Process();
        out[0][i] = w;
        out[1][i] = w;
    }
}

int main(void)
{
    hw.Init();
    hw.seed.StartLog();
    float sr = hw.SampleRate();
    osc.Init(sr);
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessControls();
        float f = mtof(36.f + hw.GetPot(POT_1) * 48.f + hw.GetCv(CV_1) * 12.f);
        osc.SetFreq(f);
        osc.SetResonance(hw.GetPot(POT_2));
        osc.SetDensity(0.05f + hw.GetPot(POT_3) * 0.9f);
        osc.SetGain(0.2f + hw.GetPot(POT_4) * 0.8f);
        osc.SetRandomFreq(f * 0.01f);
    }
}
