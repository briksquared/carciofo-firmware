#include "../../lib/carciofo.h"
#include "daisysp.h"
#include "dev/sdram.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static float DSY_SDRAM_BSS buf[4096];
static Pluck string;
static Metro clock;
static volatile bool gated = false;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    (void)in;
    for(size_t i = 0; i < size; i++)
    {
        float trig = (clock.Process() || gated) ? 1.f : 0.f;
        gated      = false;
        float w    = string.Process(trig);
        out[0][i]  = w;
        out[1][i]  = w;
    }
}

int main(void)
{
    hw.Init();
    hw.seed.StartLog();
    float sr = hw.SampleRate();
    string.Init(sr, buf, 4096, PLUCK_MODE_RECURSIVE);
    clock.Init(1.f, sr);
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessControls();
        string.SetFreq(mtof(36.f + hw.GetPot(POT_1) * 36.f + hw.GetCv(CV_1) * 12.f));
        string.SetDamp(0.2f + hw.GetPot(POT_2) * 0.75f);
        string.SetDecay(0.4f + hw.GetPot(POT_3) * 0.55f);
        string.SetAmp(0.15f + hw.GetPot(POT_4) * 0.5f);
        clock.SetFreq(0.3f + hw.GetCv(CV_2) * 6.f);
        if(hw.button[BUTTON_1].RisingEdge())
        {
            string.SetMode(string.GetMode() == PLUCK_MODE_RECURSIVE ? PLUCK_MODE_WEIGHTED_AVERAGE
                                                                     : PLUCK_MODE_RECURSIVE);
        }
        if(hw.button[BUTTON_2].RisingEdge())
            gated = true;
    }
}
