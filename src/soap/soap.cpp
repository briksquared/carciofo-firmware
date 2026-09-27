#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static Soap filt;
static volatile float mix = 1.f, lvl = 0.8f, reject = 0.f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        float dry = 0.5f * (in[0][i] + in[1][i]);
        filt.Process(dry);
        float wet = reject > 0.5f ? filt.Bandreject() : filt.Bandpass();
        float y   = ((1.f - mix) * dry + mix * wet) * lvl;
        out[0][i] = y;
        out[1][i] = y;
    }
}

int main(void)
{
    hw.Init();
    hw.seed.StartLog();
    float sr = hw.SampleRate();
    filt.Init(sr);
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessControls();
        filt.SetCenterFreq(mtof(20.f + hw.GetPot(POT_1) * 100.f + hw.GetCv(CV_1) * 24.f));
        filt.SetFilterBandwidth(40.f + hw.GetPot(POT_2) * 4000.f);
        mix    = hw.GetPot(POT_3);
        lvl    = 0.2f + hw.GetPot(POT_4) * 0.8f;
        reject = hw.button[BUTTON_1].Pressed() ? 1.f : 0.f;
    }
}
