#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static Phasor clock;
static AdEnv env;
static VariableSawOscillator osc;
static float prev = 0.f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        float ph = clock.Process();
        if(ph < prev)
            env.Trigger();
        prev = ph;
        float w   = osc.Process() * env.Process();
        out[0][i] = w;
        out[1][i] = w;
    }
}

int main(void)
{
    hw.Init();
    hw.seed.StartLog();
    float sr = hw.SampleRate();
    clock.Init(sr, 2.f);
    env.Init(sr);
    env.SetMin(0.f);
    env.SetMax(0.45f);
    osc.Init(sr);
    osc.SetWaveshape(0.2f);
    osc.SetPW(0.5f);
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessControls();
        osc.SetFreq(mtof(24.f + hw.GetPot(POT_1) * 48.f + hw.GetCv(CV_1) * 24.f));
        clock.SetFreq(0.2f + hw.GetPot(POT_2) * 12.f);
        env.SetTime(ADENV_SEG_ATTACK, 0.001f + hw.GetPot(POT_3) * 0.4f);
        env.SetTime(ADENV_SEG_DECAY, 0.02f + hw.GetPot(POT_4) * 1.5f);
    }
}
