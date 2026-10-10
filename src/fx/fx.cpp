// ---------------------------------------------------------------------------
// fx (Carciofo) -- Alia-style multi-effect. kNumFx effects in banks of 8,
// processing the stereo audio input (mono-summed).
//
//   P1 param A (+CV1)  P2 param B  P3 param C (+CV2)  P4 dry/wet mix
//   B1 : TAP = next effect ; HOLD = jump bank of 8
//   B2 : HOLD = FREEZE + full wet
//   Onboard Seed LED: heartbeat = alive; flash on B1 step; blinks bank number on
//   bank jump; SOLID while B2 freeze is held.
//
//   NOTE: the FxBank is large and lives in SDRAM. SDRAM is only powered up inside
//   hw.Init(), so the bank is CONSTRUCTED (placement-new) AFTER hw.Init() -- never
//   as a plain global, whose constructor would run at C++ startup before the SDRAM
//   is alive and hard-fault. Serial log via USB (StartLog) for bring-up.
// ---------------------------------------------------------------------------
#include "../../lib/carciofo.h"
#include "alia_fx.h"
#include <math.h>
#include <new>

using namespace daisy;
using namespace carciofo;

static Carciofo hw;
static uint8_t DSY_SDRAM_BSS fxmem[sizeof(afx::FxBank)] __attribute__((aligned(16)));
static afx::FxBank* fx = nullptr;         // constructed after hw.Init()
static volatile float g_mix = 0.5f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size){
  if(!fx){ for(size_t i = 0; i < size; i++){ out[0][i] = in[0][i]; out[1][i] = in[1][i]; } return; }
  for(size_t i = 0; i < size; i++){
    float dry = 0.5f * (in[0][i] + in[1][i]);
    float wet = fx->Process(dry);
    float y   = (1.f - g_mix) * dry + g_mix * wet;
    out[0][i] = y; out[1][i] = y;
  }
}

int main(void){
  hw.Init();
  hw.seed.StartLog(false);                // non-blocking USB serial log
  fx = new (fxmem) afx::FxBank();          // construct NOW (SDRAM is up)
  fx->Init(hw.SampleRate());
  hw.StartAudio(AudioCallback);

  const int nBanks = (afx::kNumFx + 7) / 8;
  int f = 0; fx->SetFx(f);

  uint32_t hbT = System::GetNow(); bool hb = false;
  uint32_t flashUntil = 0, logT = 0, aliveCtr = 0;
  int   blinksLeft = 0; uint32_t blinkT = 0; bool blinkOn = false;
  uint32_t b1Down = 0; bool b1Banked = false;

  while(1){
    hw.ProcessControls();
    uint32_t now = System::GetNow();

    if(hw.button[BUTTON_1].RisingEdge()){ b1Down = now; b1Banked = false; }
    if(hw.button[BUTTON_1].Pressed() && !b1Banked && (now - b1Down) >= 400){
      int bank = (f / 8 + 1) % nBanks;
      f = bank * 8; if(f >= afx::kNumFx) f = 0;
      fx->SetFx(f);
      blinksLeft = bank + 1; blinkOn = false; blinkT = now; b1Banked = true;
    }
    if(hw.button[BUTTON_1].FallingEdge() && !b1Banked){
      f = (f + 1) % afx::kNumFx; fx->SetFx(f); flashUntil = now + 70;
    }

    bool frz = hw.button[BUTTON_2].Pressed();
    fx->SetFreeze(frz);

    fx->SetParams(hw.GetPotWithCv(POT_1, CV_1),
                  hw.GetPot(POT_2),
                  afx::clampf(hw.GetPot(POT_3) + hw.GetCv(CV_2), 0.f, 1.f));
    g_mix = frz ? 1.f : hw.GetPot(POT_4);

    const afx::FxInfo& e = fx->Info();
    float v = 0.15f + 0.85f * g_mix;
    hw.led.Set(e.r * v, e.g * v, e.b * v);

    bool led;
    if(frz){ led = true; }
    else if(blinksLeft > 0){
      uint32_t dt = now - blinkT;
      if(!blinkOn){ if(dt >= 160){ blinkOn = true;  blinkT = now; } }
      else        { if(dt >=  90){ blinkOn = false; blinkT = now; blinksLeft--; } }
      led = blinkOn;
    }
    else if(now < flashUntil){ led = true; }
    else { if(now - hbT >= 350){ hbT = now; hb = !hb; } led = hb; }
    hw.seed.SetLed(led);

    if(now - logT >= 500){ logT = now; aliveCtr++;
      hw.seed.PrintLine("alive=%d fx=%d %s mix=%d", (int)aliveCtr, f, e.name, (int)(g_mix * 100)); }
  }
}
