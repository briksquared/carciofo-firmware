#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>

using namespace daisy;
using namespace daisysp;
using namespace carciofo;

/*
 * Carciofo WARPS - meta-modulator (Mutable Warps-style).
 *   IN L = carrier, IN R = external modulator. An internal oscillator can stand
 *   in for the modulator when nothing is patched (raise P4).
 *   OUT = processed (mono to both).
 *
 *   P1 (+CV1) Drive / depth        B1  cycle algorithm: Ring / AM / Fold / Diode
 *   P2 (+CV2) Internal osc pitch   B2  cycle internal osc wave: sin/tri/saw/sqr
 *   P3        Dry / Wet
 *   P4        Internal osc level (0 = external R only)
 */

static Carciofo hw;
static Wavefolder folder;
static Oscillator osc;

enum Algo { RING = 0, AM = 1, FOLD = 2, DIODE = 3, NUM_ALGO = 4 };
static volatile int algo = RING;
static volatile float drive = 0.f, wet = 0.5f, oscLevel = 0.f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out,
                   size_t size) {
  for (size_t i = 0; i < size; i++) {
    float car = in[0][i];
    float mod = in[1][i] + oscLevel * osc.Process();
    float w;
    switch (algo) {
      case AM:    w = car * (0.5f + 0.5f * mod); break;
      case FOLD:  folder.SetGain(1.f + drive * 8.f);
                  w = folder.Process(car * (1.f + mod)); break;
      case DIODE: w = fabsf(car) * (mod >= 0.f ? 1.f : -1.f); break;
      case RING:
      default:    w = car * mod * (1.f + drive * 3.f); break;
    }
    float y = (1.f - wet) * car + wet * w;
    out[0][i] = y;
    out[1][i] = y;
  }
}

int main(void) {
  hw.Init();
  hw.seed.StartLog();
  osc.Init(hw.SampleRate());
  osc.SetWaveform(Oscillator::WAVE_SIN);
  osc.SetAmp(1.f);
  folder.Init();
  hw.StartAudio(AudioCallback);

  static const uint8_t waves[4] = {
      Oscillator::WAVE_SIN, Oscillator::WAVE_TRI,
      Oscillator::WAVE_POLYBLEP_SAW, Oscillator::WAVE_POLYBLEP_SQUARE};
  int wave = 0;
  uint32_t lastPrint = System::GetNow();

  while (1) {
    hw.ProcessControls();
    if (hw.button[BUTTON_1].RisingEdge()) { algo = (algo + 1) % NUM_ALGO; hw.seed.PrintLine("algo %d", algo); }
    if (hw.button[BUTTON_2].RisingEdge()) { wave = (wave + 1) % 4; osc.SetWaveform(waves[wave]); }

    drive = Clamp(hw.GetPot(POT_1) + hw.GetCv(CV_1), 0.f, 1.f);
    osc.SetFreq(mtof(24.f + hw.GetPot(POT_2) * 60.f + hw.GetCv(CV_2) * 24.f));
    wet = hw.GetPot(POT_3);
    oscLevel = hw.GetPot(POT_4);

    static const float hue[4] = {0.0f, 0.13f, 0.55f, 0.85f};
    hw.led.SetHsv(hue[algo], 0.9f, 0.15f + 0.7f * wet);

    uint32_t now = System::GetNow();
    if (now - lastPrint >= 400) {
      lastPrint = now;
      hw.seed.PrintLine("WARPS algo %d drive " FLT_FMT3 " wet " FLT_FMT3 " oscLvl " FLT_FMT3,
                        algo, FLT_VAR3(drive), FLT_VAR3(wet), FLT_VAR3(oscLevel));
    }
  }
}
