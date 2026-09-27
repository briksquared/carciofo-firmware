#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;
static Oscillator oscTri, oscSin, oscSq;
static volatile float frequency = 440.f, level = 0.5f, shape = 0.f, blend = 1.f;

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
        float modulator = in[0][i];
        oscTri.SetFreq(frequency);
        oscSin.SetFreq(frequency);
        oscSq.SetFreq(frequency);
        oscTri.SetAmp(level);
        oscSin.SetAmp(level);
        oscSq.SetAmp(level);
        float carrier = (1.f - shape) * oscSin.Process() + shape * oscTri.Process();
        carrier = (1.f - shape) * carrier + shape * oscSq.Process();
        float output = (1.f - blend) * modulator + blend * (modulator * carrier);
        out[0][i] = output;
        out[1][i] = carrier;
    }
}

int main(void)
{
    hw.Init();
    hw.seed.StartLog();
    float sr = hw.SampleRate();
    oscTri.Init(sr);
    oscSin.Init(sr);
    oscSq.Init(sr);
    oscTri.SetWaveform(Oscillator::WAVE_TRI);
    oscSin.SetWaveform(Oscillator::WAVE_SIN);
    oscSq.SetWaveform(Oscillator::WAVE_SQUARE);
    hw.StartAudio(AudioCallback);
    const float freqMin = logf(1.f);
    const float freqMax = logf(16000.f);
    while(1)
    {
        hw.ProcessControls();
        shape = hw.GetPot(POT_1);
        blend = hw.GetPot(POT_2);
        float freqControl = Clamp01(hw.GetPot(POT_3) + hw.GetCv(CV_1));
        frequency = expf(freqMin + freqControl * (freqMax - freqMin));
        level = Clamp01(hw.GetPot(POT_4) + hw.GetCv(CV_2));
    }
}
