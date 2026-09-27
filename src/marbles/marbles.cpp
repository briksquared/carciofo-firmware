#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static Metro clock;
static SmoothRandomGenerator walk;
static SampleHold hold;
static Maytrig coin;
static AdEnv env;
static VariableSawOscillator osc;
static volatile float spread = 12.f, prob = 0.8f, base = 48.f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        bool tick = clock.Process() != 0;
        float held = hold.Process(tick, walk.Process());
        if(tick && coin.Process(prob))
            env.Trigger();
        osc.SetFreq(mtof(base + held * spread));
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
    clock.Init(2.f, sr);
    walk.Init(sr);
    env.Init(sr);
    env.SetTime(ADENV_SEG_ATTACK, 0.005f);
    env.SetTime(ADENV_SEG_DECAY, 0.2f);
    env.SetMin(0.f);
    env.SetMax(0.4f);
    osc.Init(sr);
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessControls();
        float rate = 0.25f + hw.GetPot(POT_1) * 8.f;
        clock.SetFreq(rate);
        walk.SetFreq(rate);
        spread = 2.f + hw.GetPot(POT_2) * 24.f + hw.GetCv(CV_1) * 12.f;
        prob   = hw.GetPot(POT_3);
        base   = 36.f + hw.GetPot(POT_4) * 24.f;
        osc.SetWaveshape(0.5f);
        osc.SetPW(0.5f);
        env.SetTime(ADENV_SEG_DECAY, 0.05f + (1.f - hw.GetPot(POT_3)) * 0.4f);
    }
}
