#include "../../lib/carciofo.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;
using namespace carciofo;

/*
 * Carciofo RINGS - modal / sympathetic-string resonator (Mutable Rings-style).
 * Built on DaisySP's MIT Mutable ports (Resonator / StringVoice / ModalVoice).
 *
 *   Audio IN  = exciter. Strum a guitar / send any audio and the body rings.
 *   Audio OUT = resonated voice (stereo: dry-blended L, wet R for width).
 *
 *   P1 (+CV1) Frequency / pitch      CV2  Strum in (gate fires an internal exciter)
 *   P2        Structure (harm->inharm)   B1  cycle model: Modal / String / Sympathetic
 *   P3        Brightness                 B2  hold = infinite sustain / max ring
 *   P4        Damping (decay)
 */

static Carciofo hw;
static Resonator reso;
static StringVoice str;
static ModalVoice modal;
static WhiteNoise noise;

enum Model { MODAL_RESO = 0, STRING = 1, SYMPATH = 2, NUM_MODELS = 3 };
static volatile int model = MODAL_RESO;
static volatile bool trigReq = false;   // one-shot from control loop -> audio
static volatile float wetMix = 1.f;
static float burst = 0.f;               // exciter-noise envelope (audio thread)

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out,
                   size_t size) {
  bool t = trigReq;
  trigReq = false;
  for (size_t i = 0; i < size; i++) {
    float dry = 0.5f * (in[0][i] + in[1][i]);
    float wet;
    if (model == MODAL_RESO) {
      if (t && i == 0) burst = 1.f;
      float ex = dry + burst * noise.Process() * 0.6f;
      burst *= 0.997f;
      wet = reso.Process(ex);
    } else if (model == STRING) {
      wet = str.Process(t && i == 0);
    } else {
      wet = modal.Process(t && i == 0);
    }
    out[0][i] = 0.5f * dry + wet;   // a little dry on the left for body
    out[1][i] = wet;                // pure resonated on the right
  }
}

int main(void) {
  hw.Init();
  hw.seed.StartLog();
  float sr = hw.SampleRate();

  reso.Init(0.3f, 32, sr);
  str.Init(sr);
  modal.Init(sr);
  noise.Init();

  hw.StartAudio(AudioCallback);

  bool prevGate = false;
  uint32_t lastPrint = System::GetNow();

  while (1) {
    hw.ProcessControls();

    if (hw.button[BUTTON_1].RisingEdge()) {
      model = (model + 1) % NUM_MODELS;
      hw.seed.PrintLine("model %d", model);
    }
    bool sustain = hw.button[BUTTON_2].Pressed();

    float f = mtof(24.f + hw.GetPot(POT_1) * 48.f + hw.GetCv(CV_1) * 24.f);
    float structure = hw.GetPot(POT_2);
    float brightness = hw.GetPot(POT_3);
    float damping = hw.GetPot(POT_4);
    if (sustain) damping = 1.f;   // max ring / infinite sustain

    reso.SetFreq(f);   reso.SetStructure(structure);   reso.SetBrightness(brightness);   reso.SetDamping(damping);
    str.SetFreq(f);    str.SetStructure(structure);    str.SetBrightness(brightness);    str.SetDamping(damping);
    modal.SetFreq(f);  modal.SetStructure(structure);  modal.SetBrightness(brightness);  modal.SetDamping(damping);
    str.SetSustain(sustain);
    modal.SetSustain(sustain);

    // CV2 = strum/trigger in (rising edge)
    bool gate = hw.GetCv(CV_2) > 0.5f;
    if (gate && !prevGate) trigReq = true;
    prevGate = gate;

    // LED: model colour, brightness on pitch (dark on this unit but harmless)
    static const float hue[3] = {0.33f, 0.08f, 0.55f};
    hw.led.SetHsv(hue[model], 0.9f, 0.15f + 0.7f * hw.GetPot(POT_1));

    uint32_t now = System::GetNow();
    if (now - lastPrint >= 400) {
      lastPrint = now;
      hw.seed.PrintLine("RINGS model %d  freq " FLT_FMT3 " struct " FLT_FMT3
                        " bright " FLT_FMT3 " damp " FLT_FMT3 " sus %d",
                        model, FLT_VAR3(f), FLT_VAR3(structure),
                        FLT_VAR3(brightness), FLT_VAR3(damping), sustain ? 1 : 0);
    }
  }
}
