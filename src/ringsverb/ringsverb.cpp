#include "../../lib/carciofo.h"
#include "dev/sdram.h"
#include "rings/dsp/fx/reverb.h"
using namespace daisy;
using namespace carciofo;
using namespace rings;

static Carciofo hw;
static Reverb reverb;
static uint16_t DSY_SDRAM_BSS revbuf[32768];
static volatile float wet = 0.5f, lp = 0.7f, diffusion = 0.625f, time = 0.6f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    reverb.set_amount(wet);
    reverb.set_lp(lp);
    reverb.set_diffusion(diffusion);
    reverb.set_time(time);
    reverb.set_input_gain(0.5f);
    for(size_t i = 0; i < size; i++)
    {
        float l = in[0][i];
        float r = in[1][i];
        reverb.Process(&l, &r, 1);
        out[0][i] = l;
        out[1][i] = r;
    }
}

int main(void)
{
    hw.Init();
    hw.seed.StartLog();
    reverb.Init(revbuf);
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessControls();
        wet        = hw.GetPot(POT_1);
        lp         = hw.GetPot(POT_2);
        diffusion  = hw.GetPot(POT_3) + hw.GetCv(CV_1) * 0.5f;
        time       = hw.GetPot(POT_4) + hw.GetCv(CV_2) * 0.5f;
        if(diffusion < 0.f) diffusion = 0.f;
        if(diffusion > 1.f) diffusion = 1.f;
        if(time < 0.f) time = 0.f;
        if(time > 1.f) time = 1.f;
    }
}
