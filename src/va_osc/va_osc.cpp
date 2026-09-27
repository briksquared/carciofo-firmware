#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static VariableShapeOscillator osc; static Oscillator sub; static volatile float subl=0.f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size){
  for(size_t i=0;i<size;i++){ float dry=0.5f*(in[0][i]+in[1][i]); float w=0.5f*osc.Process()+subl*0.4f*sub.Process(); out[0][i]=w; out[1][i]=w; }
}

int main(void){
  hw.Init(); hw.seed.StartLog(); float sr=hw.SampleRate(); (void)sr;
  osc.Init(sr); sub.Init(sr); sub.SetWaveform(Oscillator::WAVE_SIN);
  hw.StartAudio(AudioCallback);
  while(1){ hw.ProcessControls(); float f=mtof(24.f+hw.GetPot(POT_1)*60.f+hw.GetCv(CV_1)*24.f);
    osc.SetFreq(f); osc.SetWaveshape(hw.GetPot(POT_2)); osc.SetPW(0.5f*hw.GetPot(POT_3)+0.25f);
    sub.SetFreq(f*0.5f); subl=hw.GetPot(POT_4); }
}
