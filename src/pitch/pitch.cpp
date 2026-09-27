#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static PitchShifter ps; static volatile float mix=0.5f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size){
  for(size_t i=0;i<size;i++){ float dry=0.5f*(in[0][i]+in[1][i]); float x=dry; float w=ps.Process(x); float y=(1.f-mix)*dry+mix*w; out[0][i]=y; out[1][i]=y; }
}

int main(void){
  hw.Init(); hw.seed.StartLog(); float sr=hw.SampleRate(); (void)sr;
  ps.Init(sr);
  hw.StartAudio(AudioCallback);
  while(1){ hw.ProcessControls(); ps.SetTransposition((hw.GetPot(POT_1)-0.5f)*48.f + hw.GetCv(CV_1)*24.f);
    ps.SetDelSize((uint32_t)(256.f+hw.GetPot(POT_2)*4000.f));
    mix=hw.GetPot(POT_3); }
}
