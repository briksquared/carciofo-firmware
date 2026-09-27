#include "../../lib/carciofo.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;
using namespace carciofo;

/*
 * Carciofo full I/O test - everything is audible, no screen needed.
 *
 *   OUT L   oscillator, pitch = P1 (+ CV1), tone/waveform = P2
 *   OUT R   oscillator, pitch = P3 (+ CV2), tremolo speed = P4
 *   B1      octave up      (both oscillators)     range -3..+3
 *   B2      octave down
 *   BOTH    held together  -> toggle INPUT MONITOR: audio IN L/R passed
 *                             straight to OUT L/R (plug a source in to test
 *                             the input jacks). Onboard LED goes solid.
 *   RGB LED hue follows P1, brightness follows P3 (proves it if it ever lights)
 *   USB serial prints every control value 4x/second.
 *
 * Onboard (back) LED: slow blink = tone mode is running; solid = input monitor.
 */

static Carciofo hw;
static Oscillator oscL, oscR, tremLfo;

static int octave = 0;
static bool monitorInput = false;
static bool bothPrev = false;

static float QuantHz(float norm, int oct) {
  // C1..C5 in semitone steps, shifted by the octave buttons.
  float midi = 24.f + roundf(norm * 48.f) + static_cast<float>(oct * 12);
  return mtof(Clamp(midi, 0.f, 120.f));
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out,
                   size_t size) {
  for (size_t i = 0; i < size; i++) {
    if (monitorInput) {
      out[0][i] = in[0][i];
      out[1][i] = in[1][i];
    } else {
      float trem = 0.35f * (0.5f + 0.5f * tremLfo.Process());
      out[0][i] = oscL.Process() * 0.4f;
      out[1][i] = oscR.Process() * trem;
    }
  }
}

int main(void) {
  hw.Init();
  hw.seed.StartLog();

  oscL.Init(hw.SampleRate());
  oscL.SetWaveform(Oscillator::WAVE_SIN);
  oscR.Init(hw.SampleRate());
  oscR.SetWaveform(Oscillator::WAVE_SIN);
  tremLfo.Init(hw.SampleRate());
  tremLfo.SetWaveform(Oscillator::WAVE_SIN);

  hw.StartAudio(AudioCallback);

  static const uint8_t waves[4] = {
      Oscillator::WAVE_SIN, Oscillator::WAVE_TRI,
      Oscillator::WAVE_POLYBLEP_SAW, Oscillator::WAVE_POLYBLEP_SQUARE};

  uint32_t lastPrint = System::GetNow();
  uint32_t lastBlink = System::GetNow();
  bool blinkOn = false;

  while (1) {
    hw.ProcessControls();

    bool bothNow = hw.button[BUTTON_1].Pressed() && hw.button[BUTTON_2].Pressed();
    if (bothNow && !bothPrev) {
      monitorInput = !monitorInput;
      hw.seed.PrintLine("input monitor %s", monitorInput ? "ON" : "off");
    }
    bothPrev = bothNow;

    if (!bothNow) {
      if (hw.button[BUTTON_1].RisingEdge()) {
        octave = static_cast<int>(Clamp(octave + 1, -3, 3));
        hw.seed.PrintLine("octave %d", octave);
      }
      if (hw.button[BUTTON_2].RisingEdge()) {
        octave = static_cast<int>(Clamp(octave - 1, -3, 3));
        hw.seed.PrintLine("octave %d", octave);
      }
    }

    float p1 = hw.GetPotWithCv(POT_1, CV_1);
    float p2 = hw.GetPot(POT_2);
    float p3 = hw.GetPotWithCv(POT_3, CV_2);
    float p4 = hw.GetPot(POT_4);

    oscL.SetFreq(QuantHz(p1, octave));
    oscL.SetWaveform(waves[static_cast<int>(Clamp(p2 * 4.f, 0.f, 3.f))]);
    oscR.SetFreq(QuantHz(p3, octave));
    tremLfo.SetFreq(p4 * 12.f);  // 0..12 Hz; near 0 = steady

    // RGB LED - will stay dark if the part is mis-soldered, but reacts if fixed.
    hw.led.SetHsv(p1, 1.f, monitorInput ? 1.f : (0.1f + 0.9f * p3));

    // Onboard LED: heartbeat in tone mode, solid when monitoring input.
    uint32_t now = System::GetNow();
    if (monitorInput) {
      hw.seed.SetLed(true);
    } else if (now - lastBlink >= 400) {
      lastBlink = now;
      blinkOn = !blinkOn;
      hw.seed.SetLed(blinkOn);
    }

    if (now - lastPrint >= 250) {
      lastPrint = now;
      hw.seed.PrintLine("P1 " FLT_FMT3 " P2 " FLT_FMT3 " P3 " FLT_FMT3
                        " P4 " FLT_FMT3 " CV1 " FLT_FMT3 " CV2 " FLT_FMT3
                        " B1 %d B2 %d oct %d mon %d",
                        FLT_VAR3(hw.GetPot(POT_1)), FLT_VAR3(p2),
                        FLT_VAR3(hw.GetPot(POT_3)), FLT_VAR3(p4),
                        FLT_VAR3(hw.GetCv(CV_1)), FLT_VAR3(hw.GetCv(CV_2)),
                        hw.button[BUTTON_1].Pressed() ? 1 : 0,
                        hw.button[BUTTON_2].Pressed() ? 1 : 0, octave,
                        monitorInput ? 1 : 0);
    }
  }
}
