#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static Svf f; static volatile float morph=0.f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size){
  for(size_t i=0;i<size;i++){ float dry=0.5f*(in[0][i]+in[1][i]); f.Process(dry); float lo=f.Low(),bd=f.Band(),hi=f.High();
    float m=morph*2.f; float w=(m<1.f)?(lo*(1.f-m)+bd*m):(bd*(2.f-m)+hi*(m-1.f));
    out[0][i]=w; out[1][i]=w; }
}

int main(void){
  hw.Init(); hw.seed.StartLog(); float sr=hw.SampleRate(); (void)sr;
  f.Init(sr);
  hw.StartAudio(AudioCallback);
  while(1){ hw.ProcessControls(); f.SetFreq(mtof(20.f+hw.GetPot(POT_1)*90.f+hw.GetCv(CV_1)*24.f));
    f.SetRes(hw.GetPot(POT_2)*0.95f);
    f.SetDrive(hw.GetPot(POT_3));
    morph=hw.GetPot(POT_4); }
}
