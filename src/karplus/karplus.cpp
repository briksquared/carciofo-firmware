#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static StringVoice sv; static volatile bool trg=false;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size){
  for(size_t i=0;i<size;i++){ float dry=0.5f*(in[0][i]+in[1][i]); bool t=(i==0&&trg); if(t)trg=false; float w=sv.Process(t); out[0][i]=w; out[1][i]=w; }
}

int main(void){
  hw.Init(); hw.seed.StartLog(); float sr=hw.SampleRate(); (void)sr;
  sv.Init(sr);
  hw.StartAudio(AudioCallback);
  while(1){ hw.ProcessControls(); sv.SetFreq(mtof(24.f+hw.GetPot(POT_1)*48.f+hw.GetCv(CV_1)*24.f));
    sv.SetStructure(hw.GetPot(POT_2)); sv.SetBrightness(hw.GetPot(POT_3)); sv.SetDamping(hw.GetPot(POT_4));
    sv.SetSustain(hw.button[BUTTON_2].Pressed());
    { static bool pv=false; bool g=hw.GetCv(CV_2)>0.5f; if(g&&!pv)trg=true; pv=g; } }
}
