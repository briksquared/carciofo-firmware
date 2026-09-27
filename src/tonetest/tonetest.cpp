// Triple test: constant 440Hz tone on both outputs + blink onboard Seed LED + drive WS2812.
// If you HEAR a tone, the app is definitely running.
#include "../../lib/carciofo.h"
#include "daisysp.h"

using namespace carciofo;
using namespace daisy;
using namespace daisysp;

Carciofo hw;
Oscillator osc;

static void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {
  for (size_t i = 0; i < size; i++) {
    float s = osc.Process();
    out[0][i] = s;
    out[1][i] = s;
  }
}

int main(void) {
  hw.Init();
  osc.Init(hw.SampleRate());
  osc.SetWaveform(Oscillator::WAVE_SIN);
  osc.SetFreq(440.f);
  osc.SetAmp(0.4f);
  hw.StartAudio(AudioCallback);

  bool on = false;
  while (1) {
    on = !on;
    hw.seed.SetLed(on);
    hw.led.SetHsv(on ? 0.f : 0.5f, 1.f, 1.f);
    hw.led.Update();
    System::Delay(250);
  }
}
