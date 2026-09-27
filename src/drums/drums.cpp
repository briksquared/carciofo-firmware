#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>

using namespace daisy;
using namespace daisysp;
using namespace carciofo;

/*
 * DRUMS — Plaits/808-class drum voices on one Carciofo flash.
 * CV2 (or B2) triggers. B1 steps the voice.
 *
 *   P1 (+CV1) pitch
 *   P2        tone
 *   P3        decay
 *   P4        snap / FM / noise
 */

static Carciofo hw;
static AnalogBassDrum kick;
static AnalogSnareDrum snare808;
static SyntheticSnareDrum snare909;
static HiHat<> hat;

enum Voice { KICK = 0, SNARE808 = 1, SNARE909 = 2, HAT = 3, N = 4 };
static volatile int voice = KICK;
static volatile bool trig = false;

static const char* VoiceName(int v) {
  switch (v) {
    case SNARE808: return "snare808";
    case SNARE909: return "snare909";
    case HAT: return "hat";
    default: return "kick";
  }
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out,
                   size_t size) {
  (void)in;
  bool t = trig;
  trig = false;
  for (size_t i = 0; i < size; i++) {
    bool edge = t && i == 0;
    float w = 0.f;
    switch (voice) {
      case SNARE808: w = snare808.Process(edge); break;
      case SNARE909: w = snare909.Process(edge); break;
      case HAT: w = hat.Process(edge); break;
      default: w = kick.Process(edge); break;
    }
    out[0][i] = w;
    out[1][i] = w;
  }
}

int main(void) {
  hw.Init();
  hw.seed.StartLog(false);
  float sr = hw.SampleRate();
  kick.Init(sr);
  snare808.Init(sr);
  snare909.Init(sr);
  hat.Init(sr);
  hw.StartAudio(AudioCallback);
  hw.seed.PrintLine("drums %s", VoiceName(voice));

  bool prevGate = false;
  while (1) {
    hw.ProcessControls();
    if (hw.button[BUTTON_1].RisingEdge()) {
      voice = (voice + 1) % N;
      hw.seed.PrintLine("drums %s", VoiceName(voice));
    }
    if (hw.button[BUTTON_2].RisingEdge()) trig = true;

    bool gate = hw.GetCv(CV_2) > 0.5f;
    if (gate && !prevGate) trig = true;
    prevGate = gate;

    float f = mtof(20.f + hw.GetPot(POT_1) * 48.f + hw.GetCv(CV_1) * 12.f);
    float tone = hw.GetPot(POT_2);
    float decay = hw.GetPot(POT_3);
    float snap = hw.GetPot(POT_4);

    kick.SetFreq(f * 0.5f);
    kick.SetTone(tone);
    kick.SetDecay(decay);
    kick.SetSelfFmAmount(snap);

    snare808.SetFreq(f);
    snare808.SetTone(tone);
    snare808.SetDecay(decay);
    snare808.SetSnappy(snap);

    snare909.SetFreq(f);
    snare909.SetFmAmount(tone);
    snare909.SetDecay(decay);
    snare909.SetSnappy(snap);

    hat.SetFreq(800.f + hw.GetPot(POT_1) * 6000.f);
    hat.SetTone(tone);
    hat.SetDecay(0.05f + decay * 0.6f);
    hat.SetNoisiness(snap);
  }
}
