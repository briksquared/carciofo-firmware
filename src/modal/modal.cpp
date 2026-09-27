#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static ModalVoice voice;
static volatile bool trg = false;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        bool t = (i == 0 && trg);
        if(t)
            trg = false;
        float w   = voice.Process(t);
        out[0][i] = w;
        out[1][i] = w;
    }
}

int main(void)
{
    hw.Init();
    hw.seed.StartLog();
    float sr = hw.SampleRate();
    voice.Init(sr);
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessControls();
        voice.SetFreq(mtof(24.f + hw.GetPot(POT_1) * 48.f + hw.GetCv(CV_1) * 12.f));
        voice.SetStructure(hw.GetPot(POT_2));
        voice.SetBrightness(hw.GetPot(POT_3));
        voice.SetDamping(hw.GetPot(POT_4));
        if(hw.button[BUTTON_1].RisingEdge())
        {
            static bool sus = false;
            sus = !sus;
            voice.SetSustain(sus);
        }
        static bool pv = false;
        bool        g  = hw.GetCv(CV_2) > 0.5f || hw.button[BUTTON_2].RisingEdge();
        if(g && !pv)
            trg = true;
        pv = g;
    }
}
