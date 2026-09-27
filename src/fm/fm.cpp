#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static Fm2 fm; static volatile float lvl=0.6f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size){
  for(size_t i=0;i<size;i++){ float dry=0.5f*(in[0][i]+in[1][i]); float w=fm.Process()*lvl; out[0][i]=w; out[1][i]=w; }
}

int main(void){
  hw.Init(); hw.seed.StartLog(); float sr=hw.SampleRate(); (void)sr;
  fm.Init(sr);
  hw.StartAudio(AudioCallback);
  while(1){ hw.ProcessControls(); fm.SetFrequency(mtof(24.f+hw.GetPot(POT_1)*60.f+hw.GetCv(CV_1)*24.f));
    fm.SetRatio(0.5f+hw.GetPot(POT_2)*8.f);
    fm.SetIndex(hw.GetPot(POT_3)*10.f);
    lvl=0.2f+hw.GetPot(POT_4)*0.8f; }
}
