#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static FormantOscillator fo;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size){
  for(size_t i=0;i<size;i++){ float dry=0.5f*(in[0][i]+in[1][i]); float w=0.6f*fo.Process(); out[0][i]=w; out[1][i]=w; }
}

int main(void){
  hw.Init(); hw.seed.StartLog(); float sr=hw.SampleRate(); (void)sr;
  fo.Init(sr);
  hw.StartAudio(AudioCallback);
  while(1){ hw.ProcessControls(); fo.SetCarrierFreq(mtof(24.f+hw.GetPot(POT_1)*60.f+hw.GetCv(CV_1)*24.f));
    fo.SetFormantFreq(mtof(48.f+hw.GetPot(POT_2)*48.f));
    fo.SetPhaseShift(hw.GetPot(POT_3)); }
}
