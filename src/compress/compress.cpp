#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static Compressor left;
static Compressor right;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        out[0][i] = left.Process(in[0][i]);
        out[1][i] = right.Process(in[1][i]);
    }
}

int main(void)
{
    hw.Init();
    hw.seed.StartLog();
    float sr = hw.SampleRate();
    left.Init(sr);
    right.Init(sr);
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessControls();
        float thresh = -60.f * hw.GetPot(POT_1);
        float ratio  = 1.f + hw.GetPot(POT_2) * 19.f;
        float atk    = 0.001f + hw.GetPot(POT_3) * 0.2f;
        float rel    = 0.01f + hw.GetPot(POT_4) * 0.5f;
        left.SetThreshold(thresh);
        right.SetThreshold(thresh);
        left.SetRatio(ratio);
        right.SetRatio(ratio);
        left.SetAttack(atk);
        right.SetAttack(atk);
        left.SetRelease(rel);
        right.SetRelease(rel);
    }
}
