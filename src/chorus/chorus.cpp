#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static Chorus ch; static volatile float mix=0.5f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size){
  for(size_t i=0;i<size;i++){ float dry=0.5f*(in[0][i]+in[1][i]); ch.Process(dry); float l=ch.GetLeft(), r=ch.GetRight();
    out[0][i]=(1.f-mix)*dry+mix*l; out[1][i]=(1.f-mix)*dry+mix*r; }
}

int main(void){
  hw.Init(); hw.seed.StartLog(); float sr=hw.SampleRate(); (void)sr;
  ch.Init(sr);
  hw.StartAudio(AudioCallback);
  while(1){ hw.ProcessControls(); ch.SetLfoFreq(0.1f+hw.GetPot(POT_1)*4.f);
    ch.SetLfoDepth(hw.GetPot(POT_2));
    ch.SetDelay(0.1f+hw.GetPot(POT_3)*0.9f);
    mix=hw.GetPot(POT_4); }
}
