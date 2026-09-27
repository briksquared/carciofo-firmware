#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static Phaser ph; static volatile float mix=0.5f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size){
  for(size_t i=0;i<size;i++){ float dry=0.5f*(in[0][i]+in[1][i]); float w=ph.Process(dry); out[0][i]=0.5f*dry+0.5f*w; out[1][i]=0.5f*dry+0.5f*w; }
}

int main(void){
  hw.Init(); hw.seed.StartLog(); float sr=hw.SampleRate(); (void)sr;
  ph.Init(sr); ph.SetPoles(4);
  hw.StartAudio(AudioCallback);
  while(1){ hw.ProcessControls(); ph.SetLfoFreq(0.05f+hw.GetPot(POT_1)*2.f);
    ph.SetLfoDepth(hw.GetPot(POT_2));
    ph.SetFeedback(hw.GetPot(POT_3)*0.95f);
    ph.SetFreq(200.f+hw.GetPot(POT_4)*4000.f);
    if(hw.button[BUTTON_1].RisingEdge()){ static int p=4; p=(p%8)+1; ph.SetPoles(p);}
    mix=0.5f; }
}
