#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static DelayLine<float, 48000> DSY_SDRAM_BSS dl; static OnePole tlp;
static volatile float tsamp=12000.f, fb=0.5f, mix=0.5f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size){
  for(size_t i=0;i<size;i++){ float dry=0.5f*(in[0][i]+in[1][i]); dl.SetDelay(tsamp); float s=dl.Read(); dl.Write(dry + tlp.Process(s)*fb);
    float y=(1.f-mix)*dry+mix*s; out[0][i]=y; out[1][i]=y; }
}

int main(void){
  hw.Init(); hw.seed.StartLog(); float sr=hw.SampleRate(); (void)sr;
  dl.Init(); tlp.Init(); tlp.SetFilterMode(OnePole::FILTER_MODE_LOW_PASS);
  hw.StartAudio(AudioCallback);
  while(1){ hw.ProcessControls(); tsamp=0.02f*sr + hw.GetPot(POT_1)*0.95f*sr;
    fb=hw.GetPot(POT_2)*0.95f;
    tlp.SetFrequency(0.05f+hw.GetPot(POT_3)*0.35f);
    mix=hw.GetPot(POT_4); }
}
