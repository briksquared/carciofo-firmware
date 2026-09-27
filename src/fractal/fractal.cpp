#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static FractalRandomGenerator<ClockedNoise, 4> noise;
static Soap tone;
static volatile float lvl = 0.5f, use_br = 0.f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        tone.Process(noise.Process());
        float w   = (use_br > 0.5f ? tone.Bandreject() : tone.Bandpass()) * lvl;
        out[0][i] = w;
        out[1][i] = w;
    }
}

int main(void)
{
    hw.Init();
    hw.seed.StartLog();
    float sr = hw.SampleRate();
    noise.Init(sr);
    tone.Init(sr);
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessControls();
        noise.SetFreq(20.f + hw.GetPot(POT_1) * 4000.f);
        noise.SetColor(hw.GetPot(POT_2));
        tone.SetCenterFreq(mtof(24.f + hw.GetPot(POT_3) * 84.f + hw.GetCv(CV_1) * 12.f));
        tone.SetFilterBandwidth(50.f + hw.GetPot(POT_4) * 5000.f);
        lvl    = 0.4f;
        use_br = hw.button[BUTTON_1].Pressed() ? 1.f : 0.f;
    }
}
