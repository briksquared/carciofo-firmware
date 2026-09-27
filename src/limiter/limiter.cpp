#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;

static Limiter lmL, lmR; static float bL[128], bR[128]; static volatile float pre=1.f;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size){
  for(size_t i=0;i<size;i++){ bL[i]=in[0][i]; bR[i]=in[1][i]; }
  lmL.ProcessBlock(bL,size,pre); lmR.ProcessBlock(bR,size,pre);
  for(size_t i=0;i<size;i++){ out[0][i]=bL[i]; out[1][i]=bR[i]; }
}

int main(void){
  hw.Init(); hw.seed.StartLog(); float sr=hw.SampleRate(); (void)sr;
  lmL.Init(); lmR.Init();
  hw.StartAudio(AudioCallback);
  while(1){ hw.ProcessControls(); pre=1.f+hw.GetPot(POT_1)*8.f; }
}
