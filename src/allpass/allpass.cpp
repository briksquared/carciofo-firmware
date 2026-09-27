#include "../../lib/carciofo.h"
#include "daisysp.h"
#include "dev/sdram.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static float DSY_SDRAM_BSS ap_buf[4096];
static Allpass filt;
static volatile float mix = 0.6f, lvl = 0.8f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        float dry = 0.5f * (in[0][i] + in[1][i]);
        float w   = filt.Process(dry);
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
    filt.Init(sr, ap_buf, 4096);
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessControls();
        filt.SetFreq(0.001f + hw.GetPot(POT_1) * 0.04f);
        filt.SetRevTime(0.05f + hw.GetPot(POT_2) * 2.f);
        mix = hw.GetPot(POT_3);
        lvl = 0.2f + hw.GetPot(POT_4) * 0.8f;
    }
}
