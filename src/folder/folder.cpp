#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static Wavefolder wf; static volatile float mix=1.f,lvl=0.6f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size){
  for(size_t i=0;i<size;i++){ float dry=0.5f*(in[0][i]+in[1][i]); float w=wf.Process(dry); float y=((1.f-mix)*dry+mix*w)*lvl; out[0][i]=y; out[1][i]=y; }
}

int main(void){
  hw.Init(); hw.seed.StartLog(); float sr=hw.SampleRate(); (void)sr;
  wf.Init();
  hw.StartAudio(AudioCallback);
  while(1){ hw.ProcessControls(); wf.SetGain(1.f+(hw.GetPot(POT_1)+hw.GetCv(CV_1))*8.f);
    wf.SetOffset((hw.GetPot(POT_2)-0.5f)*2.f);
    mix=hw.GetPot(POT_3); lvl=0.2f+hw.GetPot(POT_4)*0.8f; }
}
