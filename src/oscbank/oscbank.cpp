#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static OscillatorBank bank;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        float w   = bank.Process();
        out[0][i] = w;
        out[1][i] = w;
    }
}

int main(void)
{
    hw.Init();
    hw.seed.StartLog();
    float sr = hw.SampleRate();
    bank.Init(sr);
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessControls();
        bank.SetFreq(mtof(24.f + hw.GetPot(POT_1) * 48.f + hw.GetCv(CV_1) * 12.f));
        float center = hw.GetPot(POT_2);
        float width  = 0.4f + hw.GetPot(POT_3) * 4.f;
        float raw[7];
        float sum = 0.f;
        for(int i = 0; i < 7; i++)
        {
            float d = fabsf((i / 6.f) - center);
            raw[i]  = expf(-d * width);
            sum += raw[i];
        }
        float inv = 1.f / sum;
        for(int i = 0; i < 7; i++)
            raw[i] *= inv;
        bank.SetAmplitudes(raw);
        bank.SetGain(0.15f + hw.GetPot(POT_4) * 0.7f);
    }
}
