#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static Bitcrush crush;
static volatile float mix = 1.f, lvl = 0.8f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        float dry = 0.5f * (in[0][i] + in[1][i]);
        float w   = crush.Process(dry);
        float y   = ((1.f - mix) * dry + mix * w) * lvl;
        out[0][i] = y;
        out[1][i] = y;
    }
}

int main(void)
{
    hw.Init();
    hw.seed.StartLog();
    float sr = hw.SampleRate();
    crush.Init(sr);
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessControls();
        crush.SetBitDepth(1 + (int)(hw.GetPot(POT_1) * 15.f));
        crush.SetCrushRate(500.f + hw.GetPot(POT_2) * (sr - 500.f) + hw.GetCv(CV_1) * 8000.f);
        mix = hw.GetPot(POT_3);
        lvl = 0.2f + hw.GetPot(POT_4) * 0.8f;
    }
}
