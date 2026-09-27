#include "../../lib/carciofo.h"
#include "daisysp.h"
#include "dev/sdram.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static float DSY_SDRAM_BSS loop_mem[96000];
static Looper loop;
static volatile float mix = 0.5f, speed = 1.f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        float dry = 0.5f * (in[0][i] + in[1][i]);
        float w   = loop.Process(dry);
        float y   = (1.f - mix) * dry + mix * w;
        out[0][i] = y;
        out[1][i] = y;
    }
}

int main(void)
{
    hw.Init();
    hw.seed.StartLog();
    loop.Init(loop_mem, 96000);
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessControls();
        if(hw.button[BUTTON_1].RisingEdge())
            loop.TrigRecord();
        if(hw.button[BUTTON_2].RisingEdge())
            loop.IncrementMode();
        speed = 0.5f + hw.GetPot(POT_1) * 1.5f;
        loop.SetIncrementSize(speed);
        loop.SetHalfSpeed(hw.GetPot(POT_2) < 0.5f);
        loop.SetReverse(hw.GetPot(POT_3) > 0.5f);
        mix = hw.GetPot(POT_4);
    }
}
