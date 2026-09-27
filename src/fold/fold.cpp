#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static Fold fold;
static volatile float mix = 1.f, lvl = 0.8f, gain = 1.f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        float dry = 0.5f * (in[0][i] + in[1][i]);
        float w   = fold.Process(dry * gain);
        float y   = ((1.f - mix) * dry + mix * w) * lvl;
        out[0][i] = y;
        out[1][i] = y;
    }
}

int main(void)
{
    hw.Init();
    hw.seed.StartLog();
    fold.Init();
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessControls();
        fold.SetIncrement(8.f + hw.GetPot(POT_1) * 4000.f + hw.GetCv(CV_1) * 2000.f);
        mix = hw.GetPot(POT_2);
        lvl  = 0.2f + hw.GetPot(POT_3) * 0.8f;
        gain = 0.25f + hw.GetPot(POT_4) * 3.f;
    }
}
