#include "../../lib/carciofo.h"
#include "daisysp.h"
#include "dev/sdram.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static float DSY_SDRAM_BSS comb_buf[8192];
static Comb filt;
static volatile float mix = 0.5f, lvl = 0.7f;

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
    filt.Init(sr, comb_buf, 8192);
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessControls();
        filt.SetFreq(mtof(24.f + hw.GetPot(POT_1) * 60.f + hw.GetCv(CV_1) * 12.f));
        filt.SetRevTime(0.05f + hw.GetPot(POT_2) * 4.f);
        mix = hw.GetPot(POT_3);
        lvl = 0.2f + hw.GetPot(POT_4) * 0.8f;
    }
}
