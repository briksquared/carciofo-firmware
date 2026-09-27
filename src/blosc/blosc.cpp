#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static BlOsc osc;
static volatile int wave = BlOsc::WAVE_SAW;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        float w   = osc.Process();
        out[0][i] = w;
        out[1][i] = w;
    }
}

int main(void)
{
    hw.Init();
    hw.seed.StartLog();
    float sr = hw.SampleRate();
    osc.Init(sr);
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessControls();
        if(hw.button[BUTTON_1].RisingEdge())
            wave = (wave + 1) % 3;
        float oct = floorf(hw.GetPot(POT_3) * 2.99f);
        osc.SetFreq(mtof(24.f + hw.GetPot(POT_1) * 36.f + oct * 12.f + hw.GetCv(CV_1) * 24.f));
        osc.SetPw(0.05f + hw.GetPot(POT_2) * 0.9f);
        osc.SetAmp(0.15f + hw.GetPot(POT_4) * 0.6f);
        osc.SetWaveform(wave);
    }
}
