// Blinks the Daisy Seed's ONBOARD LED (independent of the module's WS2812) so it's
// obvious when the app is running vs sitting in DFU. Also toggles the WS2812 red/green
// in case it's alive. No audio.
#include "../../lib/carciofo.h"

using namespace carciofo;
using namespace daisy;

Carciofo hw;

int main(void) {
  hw.Init();
  bool on = false;
  while (1) {
    on = !on;
    hw.seed.SetLed(on);                       // onboard Seed LED (always works)
    hw.led.SetHsv(on ? 0.f : 0.33f, 1.f, 1.f);// module WS2812 too, if it's alive
    hw.led.Update();
    System::Delay(250);                        // ~2 Hz blink
  }
}
