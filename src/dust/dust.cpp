#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static Dust noise;
static Svf tone;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        tone.Process(noise.Process());
        float w   = tone.Low();
        out[0][i] = w;
        out[1][i] = w;
    }
}

int main(void)
{
    hw.Init();
    hw.seed.StartLog();
    float sr = hw.SampleRate();
    noise.Init();
    tone.Init(sr);
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessControls();
        noise.SetDensity(0.001f + hw.GetPot(POT_1) * 0.2f);
        tone.SetFreq(mtof(20.f + hw.GetPot(POT_2) * 90.f + hw.GetCv(CV_1) * 12.f));
        tone.SetRes(hw.GetPot(POT_3) * 0.9f);
        tone.SetDrive(0.2f + hw.GetPot(POT_4));
    }
}
