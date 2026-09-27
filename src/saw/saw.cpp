#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static VariableSawOscillator osc;
static volatile float lvl = 0.4f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        float w   = osc.Process() * lvl;
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
        osc.SetFreq(mtof(24.f + hw.GetPot(POT_1) * 60.f + hw.GetCv(CV_1) * 24.f));
        osc.SetWaveshape(hw.GetPot(POT_2));
        osc.SetPW(0.05f + hw.GetPot(POT_3) * 0.9f);
        lvl = 0.15f + hw.GetPot(POT_4) * 0.6f;
    }
}
