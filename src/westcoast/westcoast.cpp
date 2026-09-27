#include "../../lib/carciofo.h"
#include "daisysp.h"
#include "dev/sdram.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static OnePole follow;
static Wavefolder fold;
static Svf lpg;
static ReverbSc DSY_SDRAM_BSS space;
static volatile float gain = 2.f, offset = 0.f, open = 0.5f, wet = 0.3f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        float dry  = 0.5f * (in[0][i] + in[1][i]);
        float env  = follow.Process(fabsf(dry));
        float folded = fold.Process(dry);
        lpg.SetFreq(80.f + env * open * 8000.f);
        lpg.Process(folded);
        float gated = lpg.Low() * (0.08f + env);
        float l, r;
        space.Process(gated, gated, &l, &r);
        out[0][i] = (1.f - wet) * gated + wet * l;
        out[1][i] = (1.f - wet) * gated + wet * r;
    }
}

int main(void)
{
    hw.Init();
    hw.seed.StartLog();
    float sr = hw.SampleRate();
    follow.Init();
    follow.SetFrequency(0.004f);
    fold.Init();
    lpg.Init(sr);
    lpg.SetRes(0.45f);
    space.Init(sr);
    space.SetLpFreq(6000.f);
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessControls();
        gain   = 1.f + (hw.GetPot(POT_1) + hw.GetCv(CV_1)) * 8.f;
        offset = (hw.GetPot(POT_2) - 0.5f) * 2.f;
        open   = hw.GetPot(POT_3);
        wet    = hw.GetPot(POT_4);
        fold.SetGain(gain);
        fold.SetOffset(offset);
        space.SetFeedback(0.55f + wet * 0.4f);
    }
}
