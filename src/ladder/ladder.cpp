#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static LadderFilter flt; static volatile float mix=0.5f; static volatile int mode=0;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size){
  for(size_t i=0;i<size;i++){ float dry=0.5f*(in[0][i]+in[1][i]); float w=flt.Process(dry); float y=(1.f-mix)*dry+mix*w; out[0][i]=y; out[1][i]=y; }
}

int main(void){
  hw.Init(); hw.seed.StartLog(); float sr=hw.SampleRate(); (void)sr;
  flt.Init(sr);
  hw.StartAudio(AudioCallback);
  while(1){ hw.ProcessControls(); flt.SetFreq(mtof(20.f+hw.GetPot(POT_1)*90.f+hw.GetCv(CV_1)*24.f));
    flt.SetRes(hw.GetPot(POT_2)*0.95f);
    flt.SetInputDrive(1.f+hw.GetPot(POT_3)*4.f);
    mix=hw.GetPot(POT_4);
    if(hw.button[BUTTON_1].RisingEdge()) mode=(mode+1)%4;
    static const LadderFilter::FilterMode M[4]={LadderFilter::FilterMode::LP24,LadderFilter::FilterMode::LP12,LadderFilter::FilterMode::BP24,LadderFilter::FilterMode::HP24};
    flt.SetFilterMode(M[mode]); }
}
