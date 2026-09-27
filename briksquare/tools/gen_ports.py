#!/usr/bin/env python3
# Generate 20 Carciofo (Daisy Seed) DSP-module port firmwares + Makefiles.
import os
SRC = r"C:/Users/briks/Desktop/GitHub/hardware/daisy/wgd-ports/carciofo-firmware/src"

PRE = '''#include "../../lib/carciofo.h"
#include "daisysp.h"
#include <math.h>
using namespace daisy;
using namespace daisysp;
using namespace carciofo;
static Carciofo hw;
'''

MK = ("TARGET = {n}\nCPP_SOURCES = {n}.cpp\nLIBDAISY_DIR = ../../lib/libDaisy\n"
      "DAISYSP_DIR = ../../lib/DaisySP\nSYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core\n"
      "include $(SYSTEM_FILES_DIR)/Makefile\n")

# Each entry: name -> (globals, audio_body, main_setup, control_body)
# audio uses: float dry=0.5f*(in[0][i]+in[1][i]); writes out[0][i],out[1][i].
apps = {}

def fx(name, glob, setup, ctrl, audio):
    apps[name] = (glob, setup, ctrl, audio)

# ---------------- Filters ----------------
fx("ladder",
   "static LadderFilter flt; static volatile float mix=0.5f; static volatile int mode=0;",
   "flt.Init(sr);",
   '''flt.SetFreq(mtof(20.f+hw.GetPot(POT_1)*90.f+hw.GetCv(CV_1)*24.f));
    flt.SetRes(hw.GetPot(POT_2)*0.95f);
    flt.SetInputDrive(1.f+hw.GetPot(POT_3)*4.f);
    mix=hw.GetPot(POT_4);
    if(hw.button[BUTTON_1].RisingEdge()) mode=(mode+1)%4;
    static const LadderFilter::FilterMode M[4]={LadderFilter::FilterMode::LP24,LadderFilter::FilterMode::LP12,LadderFilter::FilterMode::BP24,LadderFilter::FilterMode::HP24};
    flt.SetFilterMode(M[mode]);''',
   "float w=flt.Process(dry); float y=(1.f-mix)*dry+mix*w; out[0][i]=y; out[1][i]=y;")

fx("svf",
   "static Svf f; static volatile float morph=0.f;",
   "f.Init(sr);",
   '''f.SetFreq(mtof(20.f+hw.GetPot(POT_1)*90.f+hw.GetCv(CV_1)*24.f));
    f.SetRes(hw.GetPot(POT_2)*0.95f);
    f.SetDrive(hw.GetPot(POT_3));
    morph=hw.GetPot(POT_4);''',
   '''f.Process(dry); float lo=f.Low(),bd=f.Band(),hi=f.High();
    float m=morph*2.f; float w=(m<1.f)?(lo*(1.f-m)+bd*m):(bd*(2.f-m)+hi*(m-1.f));
    out[0][i]=w; out[1][i]=w;''')

# ---------------- Modulation FX ----------------
fx("chorus",
   "static Chorus ch; static volatile float mix=0.5f;",
   "ch.Init(sr);",
   '''ch.SetLfoFreq(0.1f+hw.GetPot(POT_1)*4.f);
    ch.SetLfoDepth(hw.GetPot(POT_2));
    ch.SetDelay(0.1f+hw.GetPot(POT_3)*0.9f);
    mix=hw.GetPot(POT_4);''',
   '''ch.Process(dry); float l=ch.GetLeft(), r=ch.GetRight();
    out[0][i]=(1.f-mix)*dry+mix*l; out[1][i]=(1.f-mix)*dry+mix*r;''')

fx("flanger",
   "static Flanger fl; static volatile float mix=0.5f;",
   "fl.Init(sr);",
   '''fl.SetLfoFreq(0.05f+hw.GetPot(POT_1)*2.f);
    fl.SetLfoDepth(hw.GetPot(POT_2));
    fl.SetFeedback(hw.GetPot(POT_3)*0.95f);
    mix=hw.GetPot(POT_4);''',
   "float w=fl.Process(dry); float y=(1.f-mix)*dry+mix*w; out[0][i]=y; out[1][i]=y;")

fx("phaser",
   "static Phaser ph; static volatile float mix=0.5f;",
   "ph.Init(sr); ph.SetPoles(4);",
   '''ph.SetLfoFreq(0.05f+hw.GetPot(POT_1)*2.f);
    ph.SetLfoDepth(hw.GetPot(POT_2));
    ph.SetFeedback(hw.GetPot(POT_3)*0.95f);
    ph.SetFreq(200.f+hw.GetPot(POT_4)*4000.f);
    if(hw.button[BUTTON_1].RisingEdge()){ static int p=4; p=(p%8)+1; ph.SetPoles(p);}
    mix=0.5f;''',
   "float w=ph.Process(dry); out[0][i]=0.5f*dry+0.5f*w; out[1][i]=0.5f*dry+0.5f*w;")

fx("tremolo",
   "static Tremolo tr; static volatile int wave=0;",
   "tr.Init(sr);",
   '''tr.SetFreq(0.1f+hw.GetPot(POT_1)*15.f);
    tr.SetDepth(hw.GetPot(POT_2));
    if(hw.button[BUTTON_1].RisingEdge()){ wave=(wave+1)%4; tr.SetWaveform(wave);} ''',
   "float w=tr.Process(dry); out[0][i]=w; out[1][i]=w;")

fx("autowah",
   "static Autowah aw;",
   "aw.Init(sr);",
   '''aw.SetWah(Clamp(hw.GetPot(POT_1)+hw.GetCv(CV_1),0.f,1.f));
    aw.SetDryWet(hw.GetPot(POT_2));
    aw.SetLevel(0.3f+hw.GetPot(POT_3)*0.7f);''',
   "float w=aw.Process(dry); out[0][i]=w; out[1][i]=w;")

# ---------------- Drive / crush ----------------
fx("overdrive",
   "static Overdrive od; static OnePole tone; static volatile float mix=1.f,lvl=0.6f;",
   "od.Init(); tone.Init(); tone.SetFilterMode(OnePole::FILTER_MODE_LOW_PASS);",
   '''od.SetDrive(Clamp(hw.GetPot(POT_1)+hw.GetCv(CV_1),0.f,1.f));
    tone.SetFrequency(0.02f+hw.GetPot(POT_2)*0.4f);
    mix=hw.GetPot(POT_3); lvl=0.2f+hw.GetPot(POT_4)*0.8f;''',
   "float w=tone.Process(od.Process(dry)); float y=((1.f-mix)*dry+mix*w)*lvl; out[0][i]=y; out[1][i]=y;")

fx("folder",
   "static Wavefolder wf; static volatile float mix=1.f,lvl=0.6f;",
   "wf.Init();",
   '''wf.SetGain(1.f+(hw.GetPot(POT_1)+hw.GetCv(CV_1))*8.f);
    wf.SetOffset((hw.GetPot(POT_2)-0.5f)*2.f);
    mix=hw.GetPot(POT_3); lvl=0.2f+hw.GetPot(POT_4)*0.8f;''',
   "float w=wf.Process(dry); float y=((1.f-mix)*dry+mix*w)*lvl; out[0][i]=y; out[1][i]=y;")

fx("crush",
   "static Decimator dec; static volatile float mix=1.f;",
   "dec.Init();",
   '''dec.SetDownsampleFactor(hw.GetPot(POT_1));
    dec.SetBitsToCrush((uint8_t)(hw.GetPot(POT_2)*16.f));
    dec.SetSmoothCrushing(hw.button[BUTTON_1].Pressed());
    mix=hw.GetPot(POT_3);''',
   "float w=dec.Process(dry); float y=(1.f-mix)*dry+mix*w; out[0][i]=y; out[1][i]=y;")

fx("pitch",
   "static PitchShifter ps; static volatile float mix=0.5f;",
   "ps.Init(sr); ps.SetSemitones();",
   '''ps.SetTransposition((hw.GetPot(POT_1)-0.5f)*48.f + hw.GetCv(CV_1)*24.f);
    ps.SetDelSize((uint32_t)(256.f+hw.GetPot(POT_2)*4000.f));
    mix=hw.GetPot(POT_3);''',
   "float x=dry; float w=ps.Process(x); float y=(1.f-mix)*dry+mix*w; out[0][i]=y; out[1][i]=y;")

# ---------------- Delays (DelayLine) ----------------
fx("delay",
   '''static DelayLine<float, 48000> DSY_SDRAM_BSS dl; static OnePole tlp;
static volatile float tsamp=12000.f, fb=0.5f, mix=0.5f;''',
   "dl.Init(); tlp.Init(); tlp.SetFilterMode(OnePole::FILTER_MODE_LOW_PASS);",
   '''tsamp=0.02f*sr + hw.GetPot(POT_1)*0.95f*sr;
    fb=hw.GetPot(POT_2)*0.95f;
    tlp.SetFrequency(0.05f+hw.GetPot(POT_3)*0.35f);
    mix=hw.GetPot(POT_4);''',
   '''dl.SetDelay(tsamp); float s=dl.Read(); dl.Write(dry + tlp.Process(s)*fb);
    float y=(1.f-mix)*dry+mix*s; out[0][i]=y; out[1][i]=y;''')

fx("pingpong",
   '''static DelayLine<float, 24000> DSY_SDRAM_BSS dL; static DelayLine<float, 24000> DSY_SDRAM_BSS dR;
static volatile float tsamp=8000.f, fb=0.5f, mix=0.5f;''',
   "dL.Init(); dR.Init();",
   '''tsamp=0.02f*sr + hw.GetPot(POT_1)*0.48f*sr;
    fb=hw.GetPot(POT_2)*0.95f; mix=hw.GetPot(POT_4);''',
   '''dL.SetDelay(tsamp); dR.SetDelay(tsamp);
    float sl=dL.Read(), sr2=dR.Read();
    dL.Write(in[0][i] + sr2*fb); dR.Write(in[1][i] + sl*fb);
    out[0][i]=(1.f-mix)*in[0][i]+mix*sl; out[1][i]=(1.f-mix)*in[1][i]+mix*sr2;''')

fx("limiter",
   "static Limiter lmL, lmR; static float bL[128], bR[128]; static volatile float pre=1.f;",
   "lmL.Init(); lmR.Init();",
   "pre=1.f+hw.GetPot(POT_1)*8.f;",
   None)  # limiter uses a custom block callback (handled below)

# ---------------- Synth voices (free-run) ----------------
fx("va_osc",
   "static VariableShapeOscillator osc; static Oscillator sub; static volatile float subl=0.f;",
   "osc.Init(sr); sub.Init(sr); sub.SetWaveform(Oscillator::WAVE_SIN);",
   '''float f=mtof(24.f+hw.GetPot(POT_1)*60.f+hw.GetCv(CV_1)*24.f);
    osc.SetFreq(f); osc.SetWaveshape(hw.GetPot(POT_2)); osc.SetPW(0.5f*hw.GetPot(POT_3)+0.25f);
    sub.SetFreq(f*0.5f); subl=hw.GetPot(POT_4);''',
   "float w=0.5f*osc.Process()+subl*0.4f*sub.Process(); out[0][i]=w; out[1][i]=w;")

fx("fm",
   "static Fm2 fm; static volatile float lvl=0.6f;",
   "fm.Init(sr);",
   '''fm.SetFrequency(mtof(24.f+hw.GetPot(POT_1)*60.f+hw.GetCv(CV_1)*24.f));
    fm.SetRatio(0.5f+hw.GetPot(POT_2)*8.f);
    fm.SetIndex(hw.GetPot(POT_3)*10.f);
    lvl=0.2f+hw.GetPot(POT_4)*0.8f;''',
   "float w=fm.Process()*lvl; out[0][i]=w; out[1][i]=w;")

fx("formant",
   "static FormantOscillator fo;",
   "fo.Init(sr);",
   '''fo.SetCarrierFreq(mtof(24.f+hw.GetPot(POT_1)*60.f+hw.GetCv(CV_1)*24.f));
    fo.SetFormantFreq(mtof(48.f+hw.GetPot(POT_2)*48.f));
    fo.SetPhaseShift(hw.GetPot(POT_3));''',
   "float w=0.6f*fo.Process(); out[0][i]=w; out[1][i]=w;")

fx("additive",
   "static HarmonicOscillator<16> ho; static float amp[16];",
   "ho.Init(sr);",
   '''ho.SetFreq(mtof(24.f+hw.GetPot(POT_1)*48.f+hw.GetCv(CV_1)*24.f));
    ho.SetFirstHarmIdx((int)(hw.GetPot(POT_3)*8.f));
    float tilt=hw.GetPot(POT_2); float g=1.f;
    for(int k=0;k<16;k++){ amp[k]=g; g*=(0.3f+tilt*0.68f);}
    ho.SetAmplitudes(amp);''',
   "float w=0.5f*ho.Process(); out[0][i]=w; out[1][i]=w;")

# ---------------- Triggered voices (CV2 gate) ----------------
fx("karplus",
   "static StringVoice sv; static volatile bool trg=false;",
   "sv.Init(sr);",
   '''sv.SetFreq(mtof(24.f+hw.GetPot(POT_1)*48.f+hw.GetCv(CV_1)*24.f));
    sv.SetStructure(hw.GetPot(POT_2)); sv.SetBrightness(hw.GetPot(POT_3)); sv.SetDamping(hw.GetPot(POT_4));
    sv.SetSustain(hw.button[BUTTON_2].Pressed());
    { static bool pv=false; bool g=hw.GetCv(CV_2)>0.5f; if(g&&!pv)trg=true; pv=g; }''',
   "bool t=(i==0&&trg); if(t)trg=false; float w=sv.Process(t); out[0][i]=w; out[1][i]=w;")

fx("kick",
   "static AnalogBassDrum bd; static volatile bool trg=false;",
   "bd.Init(sr);",
   '''bd.SetFreq(mtof(12.f+hw.GetPot(POT_1)*36.f+hw.GetCv(CV_1)*12.f));
    bd.SetTone(hw.GetPot(POT_2)); bd.SetDecay(hw.GetPot(POT_3)); bd.SetSelfFmAmount(hw.GetPot(POT_4));
    { static bool pv=false; bool g=hw.GetCv(CV_2)>0.5f; if(g&&!pv)trg=true; pv=g; }''',
   "bool t=(i==0&&trg); if(t)trg=false; float w=bd.Process(t); out[0][i]=w; out[1][i]=w;")

# ---------- emit ----------
CB_STD = '''void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size){
  for(size_t i=0;i<size;i++){ float dry=0.5f*(in[0][i]+in[1][i]); __AUDIO__ }
}
'''
CB_LIMITER = '''void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size){
  for(size_t i=0;i<size;i++){ bL[i]=in[0][i]; bR[i]=in[1][i]; }
  lmL.ProcessBlock(bL,size,pre); lmR.ProcessBlock(bR,size,pre);
  for(size_t i=0;i<size;i++){ out[0][i]=bL[i]; out[1][i]=bR[i]; }
}
'''
MAIN = '''int main(void){
  hw.Init(); hw.seed.StartLog(); float sr=hw.SampleRate(); (void)sr;
  __SETUP__
  hw.StartAudio(AudioCallback);
  while(1){ hw.ProcessControls(); __CTRL__ }
}
'''

n_written=0
for name,(glob,setup,ctrl,audio) in apps.items():
    body = PRE + "\n" + glob + "\n\n"
    body += (CB_LIMITER if audio is None else CB_STD.replace("__AUDIO__", audio)) + "\n"
    body += MAIN.replace("__SETUP__", setup).replace("__CTRL__", ctrl)
    d = os.path.join(SRC, name)
    os.makedirs(d, exist_ok=True)
    open(os.path.join(d, name+".cpp"),"w",newline="\n").write(body)
    open(os.path.join(d, "Makefile"),"w",newline="\n").write(MK.format(n=name))
    n_written+=1
print("generated", n_written, "apps:", ", ".join(apps.keys()))
