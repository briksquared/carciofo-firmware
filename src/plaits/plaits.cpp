#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>

using namespace daisy;
using namespace daisysp;
using namespace carciofo;

/*
 * PLAITS-style macro voice on the Carciofo.
 * One firmware, six Mutable-class engines from DaisySP. B1 steps the engine.
 * This is a voice mimic, not a byte-for-byte Plaits port.
 *
 *   P1 (+CV1) pitch
 *   P2        timbre (shape / index / formant / tilt)
 *   P3        harmonics (PW / ratio / shape / first harmonic)
 *   P4        level
 *   B1        next engine
 */

static Carciofo hw;
static VariableShapeOscillator va;
static Fm2 fm;
static FormantOscillator form;
static VosimOscillator vox;
static ZOscillator zos;
static HarmonicOscillator<16> harm;
static float harmAmp[16];

enum Engine { VA = 0, FM = 1, FORMANT = 2, VOSIM = 3, ZOSC = 4, ADD = 5, N = 6 };
static volatile int engine = VA;
static volatile float level = 0.5f;

static const char* EngineName(int e) {
  switch (e) {
    case FM: return "fm";
    case FORMANT: return "formant";
    case VOSIM: return "vosim";
    case ZOSC: return "zosc";
    case ADD: return "additive";
    default: return "va";
  }
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out,
                   size_t size) {
  (void)in;
  for (size_t i = 0; i < size; i++) {
    float w = 0.f;
    switch (engine) {
      case FM: w = fm.Process(); break;
      case FORMANT: w = form.Process(); break;
      case VOSIM: w = vox.Process(); break;
      case ZOSC: w = zos.Process(); break;
      case ADD: w = harm.Process(); break;
      default: w = va.Process(); break;
    }
    w *= level;
    out[0][i] = w;
    out[1][i] = w;
  }
}

int main(void) {
  hw.Init();
  hw.seed.StartLog(false);
  float sr = hw.SampleRate();
  va.Init(sr);
  fm.Init(sr);
  form.Init(sr);
  vox.Init(sr);
  zos.Init(sr);
  harm.Init(sr);
  hw.StartAudio(AudioCallback);
  hw.seed.PrintLine("plaits %s", EngineName(engine));

  while (1) {
    hw.ProcessControls();
    if (hw.button[BUTTON_1].RisingEdge()) {
      engine = (engine + 1) % N;
      hw.seed.PrintLine("plaits %s", EngineName(engine));
    }

    float f = mtof(24.f + hw.GetPot(POT_1) * 60.f + hw.GetCv(CV_1) * 24.f);
    float timbre = hw.GetPot(POT_2);
    float harmkn = hw.GetPot(POT_3);
    level = 0.15f + hw.GetPot(POT_4) * 0.7f;

    va.SetFreq(f);
    va.SetWaveshape(timbre);
    va.SetPW(0.05f + harmkn * 0.9f);

    fm.SetFrequency(f);
    fm.SetRatio(0.5f + harmkn * 7.5f);
    fm.SetIndex(timbre * 8.f);

    form.SetCarrierFreq(f);
    form.SetFormantFreq(mtof(48.f + timbre * 48.f));
    form.SetPhaseShift(harmkn);

    vox.SetFreq(f);
    vox.SetForm1Freq(mtof(48.f + timbre * 36.f));
    vox.SetForm2Freq(mtof(60.f + harmkn * 36.f));
    vox.SetShape(timbre);

    zos.SetFreq(f);
    zos.SetFormantFreq(mtof(48.f + timbre * 48.f));
    zos.SetShape(harmkn);
    zos.SetMode(timbre);

    harm.SetFreq(f);
    harm.SetFirstHarmIdx(1 + (int)(harmkn * 7.f));
    float g = 1.f;
    float tilt = 0.35f + timbre * 0.6f;
    for (int k = 0; k < 16; k++) {
      harmAmp[k] = g;
      g *= tilt;
    }
    harm.SetAmplitudes(harmAmp);
  }
}
