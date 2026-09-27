#include "../../lib/carciofo.h"
#include "daisysp.h"
#include "grids_lut.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static Metro clock;
static AnalogBassDrum kick;
static AnalogSnareDrum snare;
static HiHat<> hat;
static uint32_t rng = 1;
static int step = 0;
static int eucl_step[3] = {0, 0, 0};
static uint8_t perturb[3] = {0, 0, 0};
static volatile int mode = 0;
static volatile uint8_t map_x = 128, map_y = 128, chaos = 0, density = 160;
static volatile uint8_t euc_len[3] = {16, 12, 8};

static uint8_t RandByte()
{
    rng = rng * 1664525u + 1013904223u;
    return (uint8_t)(rng >> 24);
}

static uint8_t U8Mix(uint8_t a, uint8_t b, uint8_t balance)
{
    return (uint8_t)(((uint16_t)a * (255 - balance) + (uint16_t)b * balance) >> 8);
}

static uint8_t ReadDrumMap(uint8_t st, uint8_t instrument, uint8_t x, uint8_t y)
{
    uint8_t i = x >> 6;
    uint8_t j = y >> 6;
    const uint8_t* a = kGridsNode[kGridsMap[i][j]];
    const uint8_t* b = kGridsNode[kGridsMap[i + 1][j]];
    const uint8_t* c = kGridsNode[kGridsMap[i][j + 1]];
    const uint8_t* d = kGridsNode[kGridsMap[i + 1][j + 1]];
    uint8_t offset = instrument * kGridsSteps + st;
    uint8_t xb = (uint8_t)(x << 2);
    uint8_t yb = (uint8_t)(y << 2);
    return U8Mix(U8Mix(a[offset], b[offset], xb), U8Mix(c[offset], d[offset], xb), yb);
}

struct Hit { bool trig; float accent; };
static Hit hits[3];

static void Advance()
{
    hits[0] = hits[1] = hits[2] = Hit{false, 0.4f};
    if(step == 0)
    {
        uint8_t randomness = (uint8_t)(chaos >> 2);
        for(int i = 0; i < 3; i++)
            perturb[i] = (uint8_t)(((uint16_t)RandByte() * randomness) >> 8);
    }
    if(mode == 0)
    {
        for(uint8_t i = 0; i < 3; i++)
        {
            uint8_t level = ReadDrumMap((uint8_t)step, i, map_x, map_y);
            uint16_t raised = (uint16_t)level + perturb[i];
            if(raised > 255)
                level = 255;
            else
                level = (uint8_t)raised;
            if(level > (uint8_t)(~density))
            {
                hits[i].trig = true;
                hits[i].accent = level > 192 ? 1.f : 0.45f;
            }
        }
    }
    else if((step & 1) == 0)
    {
        for(uint8_t i = 0; i < 3; i++)
        {
            uint8_t length = euc_len[i];
            if(length < 1)
                length = 1;
            uint8_t dens = density >> 3;
            uint16_t address = (uint16_t)(length - 1) * 32u + dens;
            while(eucl_step[i] >= length)
                eucl_step[i] -= length;
            uint32_t bits = kGridsEuclidean[address];
            if(bits & (1u << eucl_step[i]))
            {
                hits[i].trig = true;
                hits[i].accent = 0.8f;
            }
        }
    }
    if((step & 1) == 0)
    {
        for(int i = 0; i < 3; i++)
            eucl_step[i]++;
    }
    step++;
    if(step >= kGridsSteps)
        step = 0;
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    (void)in;
    for(size_t i = 0; i < size; i++)
    {
        bool tick = clock.Process() != 0;
        if(tick)
            Advance();
        kick.SetAccent(hits[0].accent);
        snare.SetAccent(hits[1].accent);
        hat.SetAccent(hits[2].accent);
        float w = 0.55f * kick.Process(tick && hits[0].trig)
                + 0.45f * snare.Process(tick && hits[1].trig)
                + 0.35f * hat.Process(tick && hits[2].trig);
        hits[0].trig = hits[1].trig = hits[2].trig = false;
        out[0][i] = w;
        out[1][i] = w;
    }
}

int main(void)
{
    hw.Init();
    hw.seed.StartLog();
    float sr = hw.SampleRate();
    clock.Init(8.f, sr);
    kick.Init(sr);
    snare.Init(sr);
    hat.Init(sr);
    kick.SetFreq(49.f);
    kick.SetTone(0.4f);
    kick.SetDecay(0.45f);
    snare.SetFreq(180.f);
    snare.SetTone(0.5f);
    snare.SetDecay(0.4f);
    snare.SetSnappy(0.55f);
    hat.SetFreq(7000.f);
    hat.SetTone(0.5f);
    hat.SetDecay(0.35f);
    hw.StartAudio(AudioCallback);
    while(1)
    {
        hw.ProcessControls();
        if(hw.button[BUTTON_1].RisingEdge())
            mode = mode ? 0 : 1;
        map_x = (uint8_t)(hw.GetPot(POT_1) * 255.f);
        map_y = (uint8_t)(hw.GetPot(POT_2) * 255.f);
        chaos = (uint8_t)(hw.GetPot(POT_3) * 255.f);
        float bpm = 60.f + hw.GetPot(POT_4) * 160.f;
        clock.SetFreq(bpm * 8.f / 60.f);
        density = (uint8_t)(80.f + hw.GetCv(CV_1) * 175.f);
        if(mode)
        {
            euc_len[0] = (uint8_t)(1 + (int)(hw.GetPot(POT_1) * 31.f));
            euc_len[1] = (uint8_t)(1 + (int)(hw.GetPot(POT_2) * 31.f));
            euc_len[2] = (uint8_t)(1 + (int)(hw.GetPot(POT_3) * 31.f));
        }
    }
}
