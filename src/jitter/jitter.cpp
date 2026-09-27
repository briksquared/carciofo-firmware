#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static Jitter jit;
static VariableSawOscillator osc;
static volatile float base = 60.f, amt = 2.f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    (void)in;
    for(size_t i = 0; i < size; i++)
    {
        osc.SetFreq(mtof(base + jit.Process() * amt));
        float w   = osc.Process() * 0.3f;
        out[0][i] = w;
        out[1][i] = w;
    }
}

int main(void)
{
    hw.Init();
    hw.seed.StartLog();
    float sr = hw.SampleRate();
    jit.Init(sr);
    osc.Init(sr);
    osc.SetWaveshape(0.3f);
    osc.SetPW(0.5f);
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessControls();
        base = 36.f + hw.GetPot(POT_1) * 36.f + hw.GetCv(CV_1) * 12.f;
        amt  = hw.GetPot(POT_2) * 12.f;
        jit.SetCpsMin(0.2f + hw.GetPot(POT_3) * 4.f);
        jit.SetCpsMax(1.f + hw.GetPot(POT_4) * 20.f);
        jit.SetAmp(1.f);
    }
}
