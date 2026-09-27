#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static Decimator dec; static volatile float mix=1.f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size){
  for(size_t i=0;i<size;i++){ float dry=0.5f*(in[0][i]+in[1][i]); float w=dec.Process(dry); float y=(1.f-mix)*dry+mix*w; out[0][i]=y; out[1][i]=y; }
}

int main(void){
  hw.Init(); hw.seed.StartLog(); float sr=hw.SampleRate(); (void)sr;
  dec.Init();
  hw.StartAudio(AudioCallback);
  while(1){ hw.ProcessControls(); dec.SetDownsampleFactor(hw.GetPot(POT_1));
    dec.SetBitsToCrush((uint8_t)(hw.GetPot(POT_2)*16.f));
    dec.SetSmoothCrushing(hw.button[BUTTON_1].Pressed());
    mix=hw.GetPot(POT_3); }
}
