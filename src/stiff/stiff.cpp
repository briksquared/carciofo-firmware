#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static String string;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        float dry = 0.5f * (in[0][i] + in[1][i]);
        float w   = string.Process(dry);
        out[0][i] = w;
        out[1][i] = w;
    }
}

int main(void)
{
    hw.Init();
    hw.seed.StartLog();
    float sr = hw.SampleRate();
    string.Init(sr);
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessControls();
        string.SetFreq(mtof(24.f + hw.GetPot(POT_1) * 48.f + hw.GetCv(CV_1) * 12.f));
        string.SetNonLinearity((hw.GetPot(POT_2) - 0.5f) * 2.f);
        string.SetBrightness(hw.GetPot(POT_3));
        string.SetDamping(hw.GetPot(POT_4));
    }
}
