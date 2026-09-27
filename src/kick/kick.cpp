#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static AnalogBassDrum bd; static volatile bool trg=false;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size){
  for(size_t i=0;i<size;i++){ float dry=0.5f*(in[0][i]+in[1][i]); bool t=(i==0&&trg); if(t)trg=false; float w=bd.Process(t); out[0][i]=w; out[1][i]=w; }
}

int main(void){
  hw.Init(); hw.seed.StartLog(); float sr=hw.SampleRate(); (void)sr;
  bd.Init(sr);
  hw.StartAudio(AudioCallback);
  while(1){ hw.ProcessControls(); bd.SetFreq(mtof(12.f+hw.GetPot(POT_1)*36.f+hw.GetCv(CV_1)*12.f));
    bd.SetTone(hw.GetPot(POT_2)); bd.SetDecay(hw.GetPot(POT_3)); bd.SetSelfFmAmount(hw.GetPot(POT_4));
    { static bool pv=false; bool g=hw.GetCv(CV_2)>0.5f; if(g&&!pv)trg=true; pv=g; } }
}
