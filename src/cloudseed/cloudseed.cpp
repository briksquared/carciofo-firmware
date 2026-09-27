#include "../../lib/carciofo.h"
#include "cloudseed/utils.h"
#include "cloudseed_daisy/engine.h"
#include "daisysp.h"
#include "mix.h"
#include "programs.h"
#include <math.h>
using namespace daisy;
using namespace carciofo;
using cloudseed_daisy::Engine;
using cloudseed_firmware::CrossfadeCos;
using cloudseed_firmware::CrossfadeSin;
using cloudseed_firmware::kNumPrograms;
using cloudseed_firmware::kPrograms;
using cloudseed_firmware::MixPosition;

static Carciofo hw;
CLOUDSEED_DAISY_DTCM static Engine engine;
CLOUDSEED_DAISY_DTCM static float wet_l[cloudseed::kMaxBlockSize];
CLOUDSEED_DAISY_DTCM static float wet_r[cloudseed::kMaxBlockSize];
static cloudseed_daisy::Program engine_programs[kNumPrograms];
static int quantized_program = 0;
static bool frozen = false;
static float current_dry = 1.f;
static float current_wet = 0.f;

static inline float SoftClip(float x)
{
    const int32_t bits = cloudseed::FloatBits(x);
    constexpr int32_t kThreeBits = 0x40400000;
    if((bits & 0x7fffffff) > kThreeBits)
        x = bits < 0 ? -3.f : 3.f;
    return daisysp::SoftLimit(x);
}

static float Clamp01(float v)
{
    if(v < 0.f) return 0.f;
    if(v > 1.f) return 1.f;
    return v;
}

static void OnProgramLoaded(cloudseed::ReverbController& reverb, void*)
{
    reverb.SetParameter(cloudseed::Parameter::DryOut, 0.0);
    reverb.SetParameter(cloudseed::Parameter::CutoffEnabled, 1.0);
}

static int ProgramZone(float pot)
{
    int zone = (int)floorf(pot * kNumPrograms);
    if(zone < 0) zone = 0;
    if(zone >= kNumPrograms) zone = kNumPrograms - 1;
    return zone;
}

static int QuantizeProgram(float pot)
{
    const float value = pot * kNumPrograms - 0.5f;
    const float sign = value > (float)quantized_program ? -1.f : 1.f;
    int zone = (int)floorf(value + sign * 0.25f + 0.5f);
    if(zone < 0) zone = 0;
    if(zone >= kNumPrograms) zone = kNumPrograms - 1;
    quantized_program = zone;
    return zone;
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    engine.BeginBlock();
    hw.ProcessControls();
    engine.RequestProgram(QuantizeProgram(hw.GetPot(POT_1)));
    const float mix = MixPosition(hw.GetPot(POT_2));
    const float decay = Clamp01(hw.GetPot(POT_3) + hw.GetCv(CV_1));
    engine.SetParameter(cloudseed::Parameter::LineDecay, decay, 0.001f);
    engine.SetParameter(cloudseed::Parameter::PostCutoffFrequency, hw.GetPot(POT_4), 0.001f);
    const float gate = hw.GetCv(CV_2);
    if(gate > 0.55f) frozen = true;
    else if(gate < 0.45f) frozen = false;
    engine.SetFrozen(frozen);
    const float dry_gain = CrossfadeCos(mix);
    const float wet_gain = CrossfadeSin(mix);
    const float dry_step = size ? (dry_gain - current_dry) / (float)size : 0.f;
    const float wet_step = size ? (wet_gain - current_wet) / (float)size : 0.f;
    const bool wet = engine.Process(in[0], in[1], wet_l, wet_r, size);
    if(!wet)
    {
        for(size_t i = 0; i < size; i++)
        {
            current_dry += dry_step;
            out[0][i] = current_dry * in[0][i];
            out[1][i] = current_dry * in[1][i];
        }
    }
    else
    {
        for(size_t i = 0; i < size; i++)
        {
            current_dry += dry_step;
            current_wet += wet_step;
            out[0][i] = current_dry * in[0][i] + current_wet * SoftClip(wet_l[i]);
            out[1][i] = current_dry * in[1][i] + current_wet * SoftClip(wet_r[i]);
        }
    }
    current_dry = dry_gain;
    current_wet = wet_gain;
    engine.EndBlock();
}

int main(void)
{
    hw.Init(cloudseed::kMaxBlockSize);
    hw.seed.StartLog();
    for(int i = 0; i < kNumPrograms; i++)
    {
        engine_programs[i].preset = kPrograms[i].preset;
        engine_programs[i].lines = kPrograms[i].lines;
    }
    Engine::Config config;
    config.programs = engine_programs;
    config.program_count = kNumPrograms;
    config.sample_rate = hw.SampleRate();
    config.block_size = cloudseed::kMaxBlockSize;
    config.on_program_loaded = OnProgramLoaded;
    if(!engine.Init(config) || !engine.Start(ProgramZone(0.f)))
    {
        while(1) {}
    }
    hw.StartAudio(AudioCallback);
    while(1)
        engine.Service();
}
