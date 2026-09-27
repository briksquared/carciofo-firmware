#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static DelayLine<float, 24000> DSY_SDRAM_BSS dL; static DelayLine<float, 24000> DSY_SDRAM_BSS dR;
static volatile float tsamp=8000.f, fb=0.5f, mix=0.5f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size){
  for(size_t i=0;i<size;i++){ float dry=0.5f*(in[0][i]+in[1][i]); dL.SetDelay(tsamp); dR.SetDelay(tsamp);
    float sl=dL.Read(), sr2=dR.Read();
    dL.Write(in[0][i] + sr2*fb); dR.Write(in[1][i] + sl*fb);
    out[0][i]=(1.f-mix)*in[0][i]+mix*sl; out[1][i]=(1.f-mix)*in[1][i]+mix*sr2; }
}

int main(void){
  hw.Init(); hw.seed.StartLog(); float sr=hw.SampleRate(); (void)sr;
  dL.Init(); dR.Init();
  hw.StartAudio(AudioCallback);
  while(1){ hw.ProcessControls(); tsamp=0.02f*sr + hw.GetPot(POT_1)*0.48f*sr;
    fb=hw.GetPot(POT_2)*0.95f; mix=hw.GetPot(POT_4); }
}
