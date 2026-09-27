// Minimal LED-only test for the WGD Carciofo. No audio. Just drives the WS2812B
// on D5 through a continuous full-brightness rainbow so a working LED/solder is unmistakable.
#include "../../lib/carciofo.h"

using namespace carciofo;
using namespace daisy;

Carciofo hw;

int main(void) {
  hw.Init();
  float hue = 0.f;
  while (1) {
    hw.led.SetHsv(hue, 1.f, 1.f);   // full saturation + brightness
    hw.led.Update();
    System::Delay(20);
    hue += 0.01f;
    if (hue >= 1.f) hue -= 1.f;
  }
}
