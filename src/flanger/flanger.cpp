#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static Flanger fl; static volatile float mix=0.5f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size){
  for(size_t i=0;i<size;i++){ float dry=0.5f*(in[0][i]+in[1][i]); float w=fl.Process(dry); float y=(1.f-mix)*dry+mix*w; out[0][i]=y; out[1][i]=y; }
}

int main(void){
  hw.Init(); hw.seed.StartLog(); float sr=hw.SampleRate(); (void)sr;
  fl.Init(sr);
  hw.StartAudio(AudioCallback);
  while(1){ hw.ProcessControls(); fl.SetLfoFreq(0.05f+hw.GetPot(POT_1)*2.f);
    fl.SetLfoDepth(hw.GetPot(POT_2));
    fl.SetFeedback(hw.GetPot(POT_3)*0.95f);
    mix=hw.GetPot(POT_4); }
}
