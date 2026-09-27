#include "../../lib/carciofo.h"
#include <math.h>
using namespace daisy;
using namespace carciofo;
static Carciofo hw;
static volatile float levelL = 1.f, levelR = 1.f;

static float Clamp01(float v)
{
    if(v < 0.f) return 0.f;
    if(v > 1.f) return 1.f;
    return v;
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        out[0][i] = in[0][i] * levelL;
        out[1][i] = in[1][i] * levelR;
    }
}

int main(void)
{
    hw.Init();
    hw.seed.StartLog();
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessControls();
        levelL = Clamp01(hw.GetPot(POT_1) + hw.GetPot(POT_3) * hw.GetCv(CV_1));
        levelR = Clamp01(hw.GetPot(POT_2) + hw.GetPot(POT_4) * hw.GetCv(CV_2));
    }
}
