#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static Metro clock;
static PolyPluck<4> voice;
static volatile float note = 60.f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    (void)in;
    for(size_t i = 0; i < size; i++)
    {
        float trig = clock.Process() ? 1.f : 0.f;
        float w    = voice.Process(trig, note);
        out[0][i]  = w;
        out[1][i]  = w;
    }
}

int main(void)
{
    hw.Init();
    hw.seed.StartLog();
    float sr = hw.SampleRate();
    clock.Init(2.f, sr);
    voice.Init(sr);
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessControls();
        note = 36.f + hw.GetPot(POT_1) * 24.f + hw.GetPot(POT_4) * 12.f + hw.GetCv(CV_1) * 12.f;
        voice.SetDecay(0.5f + hw.GetPot(POT_2) * 0.49f);
        clock.SetFreq(0.4f + hw.GetPot(POT_3) * 8.f);
    }
}
