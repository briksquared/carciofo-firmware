#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static Tremolo tr; static volatile int wave=0;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size){
  for(size_t i=0;i<size;i++){ float dry=0.5f*(in[0][i]+in[1][i]); float w=tr.Process(dry); out[0][i]=w; out[1][i]=w; }
}

int main(void){
  hw.Init(); hw.seed.StartLog(); float sr=hw.SampleRate(); (void)sr;
  tr.Init(sr);
  hw.StartAudio(AudioCallback);
  while(1){ hw.ProcessControls(); tr.SetFreq(0.1f+hw.GetPot(POT_1)*15.f);
    tr.SetDepth(hw.GetPot(POT_2));
    if(hw.button[BUTTON_1].RisingEdge()){ wave=(wave+1)%4; tr.SetWaveform(wave);}  }
}
