#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static Particle blow;
static Dust strike;
static ClockedNoise noise;
static Resonator body;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        float ex = 0.45f * blow.Process() + 0.35f * strike.Process() + 0.25f * noise.Process();
        float w  = body.Process(ex);
        out[0][i] = w;
        out[1][i] = w;
    }
}

int main(void)
{
    hw.Init();
    hw.seed.StartLog();
    float sr = hw.SampleRate();
    blow.Init(sr);
    strike.Init();
    noise.Init(sr);
    body.Init(0.3f, 24, sr);
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessControls();
        float f = mtof(24.f + hw.GetPot(POT_1) * 48.f + hw.GetCv(CV_1) * 12.f);
        body.SetFreq(f);
        body.SetStructure(hw.GetPot(POT_2));
        body.SetBrightness(hw.GetPot(POT_3));
        body.SetDamping(0.2f + hw.GetPot(POT_4) * 0.75f);
        blow.SetFreq(f);
        blow.SetDensity(0.15f + hw.GetCv(CV_2) * 0.7f);
        blow.SetResonance(0.6f);
        strike.SetDensity(0.02f);
        noise.SetFreq(f * 2.f);
    }
}
