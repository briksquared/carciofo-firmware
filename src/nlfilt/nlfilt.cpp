#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static NlFilt filt;
static volatile float a = 0.3f, b = 0.1f, d = 0.05f, level = 0.5f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        float dry = 0.5f * (in[0][i] + in[1][i]);
        float w   = 0.f;
        filt.ProcessBlock(&dry, &w, 1);
        out[0][i] = w * level;
        out[1][i] = w * level;
    }
}

int main(void)
{
    hw.Init();
    hw.seed.StartLog();
    filt.Init();
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessControls();
        a = (hw.GetPot(POT_1) - 0.5f) * 1.2f;
        b = (hw.GetPot(POT_2) - 0.5f) * 1.2f;
        d = hw.GetPot(POT_3) * 0.25f;
        level = 0.2f + hw.GetPot(POT_4) * 0.6f;
        filt.SetCoefficients(a, b, d, hw.GetCv(CV_1) * 0.2f, 1.f + hw.GetCv(CV_2) * 6.f);
    }
}
