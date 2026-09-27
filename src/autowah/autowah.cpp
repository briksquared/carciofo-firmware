#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static Autowah aw;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size){
  for(size_t i=0;i<size;i++){ float dry=0.5f*(in[0][i]+in[1][i]); float w=aw.Process(dry); out[0][i]=w; out[1][i]=w; }
}

int main(void){
  hw.Init(); hw.seed.StartLog(); float sr=hw.SampleRate(); (void)sr;
  aw.Init(sr);
  hw.StartAudio(AudioCallback);
  while(1){ hw.ProcessControls(); aw.SetWah(Clamp(hw.GetPot(POT_1)+hw.GetCv(CV_1),0.f,1.f));
    aw.SetDryWet(hw.GetPot(POT_2));
    aw.SetLevel(0.3f+hw.GetPot(POT_3)*0.7f); }
}
