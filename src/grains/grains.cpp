#include "../../lib/carciofo.h"
#include "daisysp.h"
#include "dev/sdram.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static const int kLen = 48000;
static float DSY_SDRAM_BSS table[kLen];
static GranularPlayer play;
static int wrpos = 0;
static volatile bool freeze = false;
static volatile float speed = 1.f, cents = 0.f, grain = 80.f, mix = 0.7f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        float dry = 0.5f * (in[0][i] + in[1][i]);
        if(!freeze)
        {
            table[wrpos] = dry;
            wrpos++;
            if(wrpos >= kLen)
                wrpos = 0;
        }
        float w   = play.Process(speed, cents, grain);
        float y   = (1.f - mix) * dry + mix * w;
        out[0][i] = y;
        out[1][i] = y;
    }
}

int main(void)
{
    hw.Init();
    hw.seed.StartLog();
    float sr = hw.SampleRate();
    play.Init(table, kLen, sr);
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessControls();
        if(hw.button[BUTTON_1].RisingEdge())
            freeze = !freeze;
        speed = -1.f + hw.GetPot(POT_1) * 2.f;
        cents = (hw.GetPot(POT_2) - 0.5f) * 2400.f + hw.GetCv(CV_1) * 1200.f;
        grain = 5.f + hw.GetPot(POT_3) * 400.f;
        mix   = hw.GetPot(POT_4);
    }
}
