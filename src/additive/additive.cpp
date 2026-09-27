#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static HarmonicOscillator<16> ho; static float amp[16];

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size){
  for(size_t i=0;i<size;i++){ float dry=0.5f*(in[0][i]+in[1][i]); float w=0.5f*ho.Process(); out[0][i]=w; out[1][i]=w; }
}

int main(void){
  hw.Init(); hw.seed.StartLog(); float sr=hw.SampleRate(); (void)sr;
  ho.Init(sr);
  hw.StartAudio(AudioCallback);
  while(1){ hw.ProcessControls(); ho.SetFreq(mtof(24.f+hw.GetPot(POT_1)*48.f+hw.GetCv(CV_1)*24.f));
    ho.SetFirstHarmIdx((int)(hw.GetPot(POT_3)*8.f));
    float tilt=hw.GetPot(POT_2); float g=1.f;
    for(int k=0;k<16;k++){ amp[k]=g; g*=(0.3f+tilt*0.68f);}
    ho.SetAmplitudes(amp); }
}
