#include "../../lib/carciofo.h"
#include "distortion_engine.h"
using namespace daisy;
using namespace carciofo;

static Carciofo hw;
static DistortionEngine engine;
static volatile float dist = 0.4f, low = 0.5f, high = 0.5f, level = 0.5f;

static float Clamp01(float v)
{
    if(v < 0.f) return 0.f;
    if(v > 1.f) return 1.f;
    return v;
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    engine.SetParameters(dist, low, high, level);
    engine.ProcessBlock(in[0], in[1], out[0], out[1], size);
}

int main(void)
{
    hw.Init();
    hw.seed.StartLog();
    engine.Init(hw.SampleRate());
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessControls();
        dist  = Clamp01(hw.GetPot(POT_1) + hw.GetCv(CV_1) * 0.5f);
        low   = hw.GetPot(POT_2);
        high  = Clamp01(hw.GetPot(POT_3) + hw.GetCv(CV_2) * 0.5f);
        level = hw.GetPot(POT_4);
    }
}
