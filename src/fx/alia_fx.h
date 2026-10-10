#pragma once
// ---------------------------------------------------------------------------
// alia_fx.h  --  a switchable multi-effect for the WGD boards, in the spirit of
// the Noise Engineering effects modules.  Processes the mono-summed audio input
// through one of kNumFx effects, chosen live.  Wraps DaisySP effects/filters +
// a large clean-room DSP set.  Board-agnostic; a host main drives it.
//
//   SetFx(i) SetParams(a,b,c) -> float Process(float in)
//
// The bank holds many delay/reverb buffers, so the host MUST place its instance
// in SDRAM (DSY_SDRAM_BSS).
// ---------------------------------------------------------------------------
#include "daisysp.h"
#include <cmath>
#include <cstdint>

namespace afx {

struct FxInfo { const char* name; float r, g, b; };
static const FxInfo kFx[] = {
  {"CHORUS",  0.20f, 0.80f, 1.00f},  //  0
  {"FLANGER", 0.40f, 0.90f, 0.80f},  //  1
  {"PHASER",  0.70f, 0.40f, 1.00f},  //  2
  {"TREMOLO", 1.00f, 0.70f, 0.20f},  //  3
  {"AUTOWAH", 0.90f, 0.90f, 0.10f},  //  4
  {"DRIVE",   1.00f, 0.30f, 0.00f},  //  5
  {"FOLD",    0.00f, 1.00f, 0.50f},  //  6
  {"CRUSH",   0.00f, 0.85f, 1.00f},  //  7
  {"LADDER",  1.00f, 0.20f, 0.30f},  //  8
  {"SVFILT",  0.60f, 0.60f, 1.00f},  //  9
  {"REVERB",  0.50f, 0.80f, 0.90f},  // 10
  {"DELAY",   0.90f, 0.50f, 0.30f},  // 11
  {"PITCH",   1.00f, 0.40f, 0.70f},  // 12
  {"RING",    0.80f, 1.00f, 0.00f},  // 13
  {"COMB",    0.40f, 0.90f, 0.60f},  // 14
  {"LOFI",    0.70f, 0.60f, 0.40f},  // 15
  {"GATE",    1.00f, 0.60f, 0.00f},  // 16
  {"WSHAPE",  1.00f, 0.30f, 0.50f},  // 17
  {"OCTAVE",  0.50f, 1.00f, 0.30f},  // 18
  {"SOAP",    0.80f, 0.40f, 0.90f},  // 19 bandpass / band-reject (DaisySP)
  {"STIFF",   0.90f, 0.80f, 0.50f},  // 20 stiff-string resonator (DaisySP)
  {"LIMIT",   0.95f, 0.75f, 0.20f},  // 21 brickwall limiter (DaisySP)
  {"ALLPASS", 0.60f, 0.70f, 0.90f},  // 22 modulatable allpass smear
  {"TONE",    0.70f, 0.85f, 0.95f},  // 23 tilt tone LP<->HP
  {"CLIP",    1.00f, 0.25f, 0.10f},  // 24 hard clip / saturate
  {"RESON",   0.30f, 0.95f, 0.85f},  // 25 tuned resonant comb
  {"COMBS",   0.20f, 0.80f, 0.55f},  // 26 comb chord (3 voices)
  {"FORMANT", 0.95f, 0.85f, 0.35f},  // 27 vowel formant filter
  {"LPG",     0.55f, 0.95f, 0.45f},  // 28 lowpass gate
  {"VIBRATO", 0.45f, 0.65f, 1.00f},  // 29 pitch vibrato
  {"ENSEMBL", 0.35f, 0.75f, 1.00f},  // 30 3-voice ensemble chorus
  {"TAPE",    0.85f, 0.65f, 0.45f},  // 31 wow/flutter + saturation
  {"STUTTER", 0.90f, 0.20f, 0.80f},  // 32 buffer stutter / glitch
  {"REVERSE", 0.75f, 0.30f, 0.95f},  // 33 reverse-chunk playback
  {"DIFFUSE", 0.55f, 0.75f, 0.95f},  // 34 allpass diffuser
  {"SPRING",  0.60f, 0.85f, 0.80f},  // 35 spring reverb
  {"FUZZ",    1.00f, 0.15f, 0.25f},  // 36 asymmetric fuzz
  {"RINGSWP", 0.90f, 1.00f, 0.20f},  // 37 swept ring mod
  {"PUMP",    1.00f, 0.50f, 0.60f},  // 38 sidechain-style pump
  {"DRONE",   0.65f, 0.45f, 0.95f},  // 39 self-oscillating resonator
  {"HPF",     0.50f, 0.60f, 1.00f},  // 40 resonant highpass
  {"PEAK",    0.95f, 0.70f, 0.30f},  // 41 parametric peak EQ
  {"NOTCH",   0.60f, 0.55f, 0.85f},  // 42 sweep notch
  {"TILT",    0.75f, 0.80f, 0.70f},  // 43 tilt EQ
  {"SHIMMER", 0.70f, 0.90f, 1.00f},  // 44 octave reverb
  {"SPACE",   0.55f, 0.75f, 0.95f},  // 45 delay + reverb
  {"GRAIN",   0.85f, 0.45f, 0.95f},  // 46 granular cloud
  {"ROTARY",  0.90f, 0.75f, 0.40f},  // 47 leslie rotary
  {"PWARP",   1.00f, 0.45f, 0.75f},  // 48 pitch-LFO warp
  {"SWELL",   0.45f, 0.85f, 0.75f},  // 49 auto volume swell
  {"NGATE",   0.80f, 0.80f, 0.30f},  // 50 noise gate
  {"SAT",     0.95f, 0.55f, 0.30f},  // 51 saturator
  {"BITFOLD", 0.30f, 0.90f, 0.70f},  // 52 bitcrush + fold
  {"SNH",     0.65f, 0.70f, 0.95f},  // 53 sample & hold
  {"SLEW",    0.70f, 0.65f, 0.55f},  // 54 slew limiter
  {"REZ",     1.00f, 0.30f, 0.40f},  // 55 acid resonant LP
  {"SUBOCT",  0.40f, 0.30f, 0.90f},  // 56 sub-octave
  {"CHEBY",   1.00f, 0.60f, 0.20f},  // 57 chebyshev exciter
  {"KILLEQ",  0.80f, 0.85f, 0.55f},  // 58 3-band kill EQ
  {"WOBBLE",  0.55f, 0.35f, 0.95f},  // 59 LFO filter wobble
  {"MULTITAP",0.85f, 0.65f, 0.35f},  // 60 3-tap delay
  {"DUCKDLY", 0.70f, 0.55f, 0.40f},  // 61 ducked delay
  {"GRITVRB", 0.60f, 0.70f, 0.60f},  // 62 crushed reverb
  {"RNDFILT", 0.95f, 0.40f, 0.85f},  // 63 random stepped filter
};
static const int kNumFx = (int)(sizeof(kFx) / sizeof(kFx[0]));

static inline float clampf(float x, float lo, float hi){ return x < lo ? lo : (x > hi ? hi : x); }
static inline float fracf(float x){ x -= (float)(long)x; return x < 0 ? x + 1.f : x; }
static inline float sinf01(float p){ return sinf(6.2831853f * p); }
static inline float mtof(float m){ return 440.f * powf(2.f, (m - 69.f) * (1.f / 12.f)); }

static const int RVC_LEN[4] = {1116, 1188, 1277, 1356};
static const int RVC_OFF[4] = {0, 1116, 2304, 3581};
static const int RVA_LEN[2] = {556, 441};
static const int RVA_OFF[2] = {0, 556};

// vowel formant frequencies (A E I O U) x 3 formants
static const float kVowel[5][3] = {
  {730.f, 1090.f, 2440.f}, {530.f, 1840.f, 2480.f}, {270.f, 2290.f, 3010.f},
  {570.f,  840.f, 2410.f}, {300.f,  870.f, 2240.f},
};

struct BQ {
  float b0, b1, b2, a1, a2, x1, x2, y1, y2;
  void Reset(){ b0 = 1.f; b1 = b2 = a1 = a2 = 0.f; x1 = x2 = y1 = y2 = 0.f; }
  float Proc(float x){ float y = b0*x + b1*x1 + b2*x2 - a1*y1 - a2*y2;
                       x2 = x1; x1 = x; y2 = y1; y1 = y; return y; }
};

class FxBank {
 public:
  void Init(float sr){
    sr_ = sr;
    chorus_.Init(sr); flanger_.Init(sr); phaser_.Init(sr); tremolo_.Init(sr);
    autowah_.Init(sr); drive_.Init(); fold_.Init(); crush_.Init();
    ladder_.Init(sr); svf_.Init(sr); pitch_.Init(sr);
    shPitch_.Init(sr); shPitch_.SetTransposition(12.f);
    dl_.Init(); dlTime_ = 12000.f; dlFb_ = 0.4f; dlLp_ = 0.f; dlTone_ = 0.5f; ringph_ = 0.f;
    ringInc_ = 220.f / sr;
    for(int i = 0; i < kCombSum; i++) cbuf_[i] = 0.f;
    for(int i = 0; i < kApSum;   i++) abuf_[i] = 0.f;
    for(int k = 0; k < 4; k++){ cidx_[k] = 0; cfs_[k] = 0.f; }
    aidx_[0] = aidx_[1] = 0; rvFb_ = 0.85f; rvDamp_ = 0.3f;
    svfMode_ = 0; fx_ = 0;
    combLen_ = 200.f; combFb_ = 0.7f; lofiLp_ = 0.f; lofiCut_ = 0.5f;
    gph_ = 0.f; gateRate_ = 6.f; gateWidth_ = 0.5f; gateSm_ = 0.f; gateSmC_ = 0.05f;
    wsDrive_ = 3.f; wsAsym_ = 0.f; wsCut_ = 0.5f; wsLp_ = 0.f;
    octLp_ = 0.f; octCut_ = 0.2f; octGain_ = 2.f;
    soap_.Init(sr); string_.Init(sr); limiter_.Init();
    soapReject_ = false; limPre_ = 2.f;
    toneLp_ = 0.f; toneCut_ = 0.3f; toneHp_ = false; clipPre_ = 4.f;
    ZeroAll();
  }
  void SetFreeze(bool f){ frozen_ = f; }
  void SetFx(int i){ fx_ = i < 0 ? 0 : (i >= kNumFx ? kNumFx - 1 : i); }
  int  Fx() const { return fx_; }
  const FxInfo& Info() const { return kFx[fx_]; }

  void SetParams(float a, float b, float c){
    a = clampf(a,0,1); b = clampf(b,0,1); c = clampf(c,0,1);
    switch(fx_){
      case 0: chorus_.SetLfoFreq(0.1f + b * 4.f); chorus_.SetLfoDepth(0.2f + a * 0.8f);
              chorus_.SetFeedback(c * 0.7f); break;
      case 1: flanger_.SetLfoFreq(0.05f + b * 2.f); flanger_.SetLfoDepth(0.2f + a * 0.8f);
              flanger_.SetFeedback(c * 0.9f); break;
      case 2: phaser_.SetLfoFreq(0.1f + b * 4.f); phaser_.SetLfoDepth(0.3f + a * 0.7f);
              phaser_.SetFreq(mtof(30.f + a * 60.f)); phaser_.SetFeedback(c * 0.8f); break;
      case 3: tremolo_.SetFreq(0.5f + a * 20.f); tremolo_.SetDepth(b);
              tremolo_.SetWaveform((int)(c * 3.99f)); break;
      case 4: autowah_.SetWah(a); autowah_.SetDryWet(100.f); autowah_.SetLevel(0.3f + c * 0.7f); break;
      case 5: drive_.SetDrive(0.2f + a * 0.75f); break;
      case 6: fold_.SetGain(1.f + a * 8.f); fold_.SetOffset((b - 0.5f)); break;
      case 7: crush_.SetDownsampleFactor(clampf(1.f - a * 0.97f, 0.02f, 1.f));
              crush_.SetBitsToCrush((uint8_t)(1 + (int)(b * 15.f))); break;
      case 8: ladder_.SetFreq(mtof(30.f + a * 95.f)); ladder_.SetRes(b * 0.95f);
              ladder_.SetInputDrive(1.f + c * 3.f); break;
      case 9: svf_.SetFreq(mtof(30.f + a * 95.f)); svf_.SetRes(0.2f + b * 0.7f);
              svfMode_ = (int)(c * 2.99f); break;
      case 10: rvFb_ = 0.7f + a * 0.28f; rvDamp_ = clampf(b, 0.f, 0.95f); break;
      case 11: dlTime_ = clampf((0.02f + a * 0.97f) * sr_, 1.f, (float)(kDelayMax - 1));
               dlFb_ = b * 0.92f; dlTone_ = 0.05f + c * 0.9f; break;
      case 12: pitch_.SetTransposition((a - 0.5f) * 24.f); pitch_.SetFun(b); break;
      case 13: ringInc_ = mtof(24.f + a * 84.f) / sr_; break;
      case 14: combLen_ = 2.f + a * 800.f; combFb_ = b * 0.97f; break;
      case 15: crush_.SetDownsampleFactor(clampf(1.f - a * 0.9f, 0.03f, 1.f));
               crush_.SetBitsToCrush((uint8_t)(1 + (int)((1.f - b) * 14.f)));
               lofiCut_ = 0.03f + c * 0.9f; break;
      case 16: gateRate_ = 1.f + a * 16.f; gateWidth_ = 0.1f + b * 0.85f;
               gateSmC_ = 0.003f + c * 0.2f; break;
      case 17: wsDrive_ = 1.f + a * 10.f; wsAsym_ = (b - 0.5f); wsCut_ = 0.05f + c * 0.9f; break;
      case 18: octCut_ = 0.05f + a * 0.4f; octGain_ = 1.f + c * 3.f; break;
      case 19: soap_.SetCenterFreq(mtof(20.f + a * 100.f)); soap_.SetFilterBandwidth(40.f + b * 4000.f);
               soapReject_ = c > 0.5f; break;
      case 20: string_.SetFreq(mtof(24.f + a * 48.f)); string_.SetNonLinearity((b - 0.5f) * 2.f);
               string_.SetBrightness(c); string_.SetDamping(0.5f); break;
      case 21: limPre_ = 1.f + a * 7.f; break;
      case 22: apLen_ = (int)(8.f + a * (float)(kApLen - 10)); apFb_ = -(0.2f + b * 0.75f); break;
      case 23: toneCut_ = 0.02f + a * 0.9f; toneHp_ = b > 0.5f; break;
      case 24: clipPre_ = 1.f + a * 24.f; break;
      case 25: rLen_ = (int)clampf(sr_ / mtof(36.f + a * 48.f), 4.f, (float)(kBufN - 2));
               rFb_ = 0.80f + b * 0.19f; rDampC_ = 0.05f + c * 0.9f; break;
      case 26: { float root = mtof(30.f + a * 36.f); float sp = 0.5f + c;
                 const float semis[3] = {0.f, 4.f, 7.f};
                 for(int k = 0; k < 3; k++)
                   cmL_[k] = (int)clampf(sr_ / (root * powf(2.f, semis[k] * sp / 12.f)), 4.f, (float)(kBufN - 2));
                 cmFb_ = 0.80f + b * 0.19f; } break;
      case 27: { float vp = a * 4.f; int v0 = (int)vp; int v1 = v0 + 1 > 4 ? 4 : v0 + 1; float vf = vp - v0;
                 float res = 0.95f + b * 0.049f; float shift = 0.5f + c * 1.5f;
                 for(int k = 0; k < 3; k++){
                   float f = (kVowel[v0][k] * (1.f - vf) + kVowel[v1][k] * vf) * shift;
                   float w = 6.2831853f * clampf(f, 20.f, sr_ * 0.45f) / sr_;
                   fa1_[k] = -2.f * res * cosf(w); fa2_[k] = res * res; fb0_[k] = 1.f - res; } } break;
      case 28: lpgDec_ = 0.0004f + a * 0.02f; lpgBase_ = 0.01f + b * 0.3f; lpgAmt_ = 0.1f + c * 0.8f; break;
      case 29: vRate_ = 0.5f + a * 8.f; vDepth_ = 10.f + b * 300.f; break;
      case 30: eDepth_ = 4.f + a * 24.f; eRateB_ = 0.3f + b * 2.f; break;
      case 31: tDrive_ = 1.f + a * 6.f; wowAmt_ = b * 250.f; tTone_ = 0.1f + c * 0.8f; break;
      case 32: stLen_ = (int)clampf(480.f + a * 20000.f, 32.f, (float)(kStutN - 2)); stFreeze_ = b > 0.5f; break;
      case 33: rvLen_ = (int)clampf(2400.f + a * 20000.f, 64.f, (float)(kRevN - 2)); break;
      case 34: dfSize_ = 0.3f + a * 0.7f; dfG_ = 0.4f + b * 0.5f; break;
      case 35: spLen_ = (int)clampf(600.f + a * 2600.f, 64.f, (float)(kSpN - 2)); spFb_ = 0.5f + b * 0.45f; break;
      case 36: fzGain_ = 2.f + a * 30.f; fzBias_ = (b - 0.5f) * 0.5f; fzTone_ = 0.05f + c * 0.9f; break;
      case 37: rsBaseInc_ = mtof(24.f + a * 72.f) / sr_; rsRate_ = 0.1f + b * 8.f; rsDepth_ = c * 0.9f; break;
      case 38: puRate_ = 0.5f + a * 8.f; puDepth_ = b; puShape_ = 0.2f + c * 4.f; break;
      case 39: dnLen_ = (int)clampf(sr_ / mtof(28.f + a * 36.f), 8.f, (float)(kBufN - 2));
               dnFb_ = 0.95f + b * 0.05f; dnLpC_ = 0.1f + c * 0.8f; break;
      case 40: { float w = 6.2831853f * mtof(30.f + a * 90.f) / sr_; float cw = cosf(w), sw = sinf(w);
                 float al = sw / (2.f * (0.5f + b * 8.f)); float a0 = 1.f + al;
                 bqH_.b0 = (1.f + cw) * 0.5f / a0; bqH_.b1 = -(1.f + cw) / a0; bqH_.b2 = bqH_.b0;
                 bqH_.a1 = -2.f * cw / a0; bqH_.a2 = (1.f - al) / a0; } break;
      case 41: { float w = 6.2831853f * mtof(30.f + a * 90.f) / sr_; float cw = cosf(w), sw = sinf(w);
                 float al = sw / (2.f * (0.7f + b * 8.f)); float A = powf(10.f, ((c - 0.5f) * 30.f) / 40.f);
                 float a0 = 1.f + al / A;
                 bqP_.b0 = (1.f + al * A) / a0; bqP_.b1 = -2.f * cw / a0; bqP_.b2 = (1.f - al * A) / a0;
                 bqP_.a1 = -2.f * cw / a0; bqP_.a2 = (1.f - al / A) / a0; } break;
      case 42: { float w = 6.2831853f * mtof(30.f + a * 90.f) / sr_; float cw = cosf(w), sw = sinf(w);
                 float al = sw / (2.f * (0.7f + b * 10.f)); float a0 = 1.f + al;
                 bqN_.b0 = 1.f / a0; bqN_.b1 = -2.f * cw / a0; bqN_.b2 = 1.f / a0;
                 bqN_.a1 = -2.f * cw / a0; bqN_.a2 = (1.f - al) / a0; } break;
      case 43: tiltAmt_ = (a - 0.5f) * 2.f; tiltCut_ = 0.02f + b * 0.4f; break;
      case 44: rvFb_ = 0.80f + a * 0.18f; rvDamp_ = 0.3f; shAmt_ = b * 0.75f; shPitch_.SetTransposition(12.f); break;
      case 45: dlTime_ = clampf((0.08f + a * 0.5f) * sr_, 1.f, (float)(kDelayMax - 1)); dlFb_ = b * 0.6f;
               rvFb_ = 0.8f + c * 0.18f; rvDamp_ = 0.35f; break;
      case 46: glen_ = (int)clampf(1200.f + a * 12000.f, 256.f, (float)(kGrainN - 2));
               gspawnPer_ = (int)clampf((float)glen_ / (1.f + b * 3.f), 64.f, (float)(kGrainN - 2));
               gSpeed_ = 0.5f + c * 1.5f; break;
      case 47: rotRate_ = 0.5f + a * 8.f; rotDepth_ = 5.f + b * 60.f; break;
      case 48: pwRate_ = 0.05f + a * 4.f; pwDepth_ = b * 24.f; break;
      case 49: swAtk_ = 0.0003f + (1.f - a) * 0.01f; swRel_ = 0.005f + b * 0.05f; break;
      case 50: ngThr_ = a * 0.3f; ngRel_ = 0.004f + b * 0.05f; break;
      case 51: satDrive_ = 1.f + a * 12.f; break;
      case 52: bfLv_ = powf(2.f, 1.f + a * 5.f); bfG_ = 1.f + b * 5.f; break;
      case 53: snhPer_ = (int)clampf(1.f + a * 220.f, 1.f, 500.f); break;
      case 54: slwRate_ = 0.001f + a * 0.25f; break;
      case 55: rezF_ = 2.f * sinf(3.14159265f * clampf(mtof(28.f + a * 84.f), 20.f, sr_ * 0.45f) / sr_);
               rezQ_ = 0.5f - b * 0.47f; rezDrive_ = 1.f + c * 4.f; break;
      case 56: subTone_ = 0.02f + a * 0.3f; break;
      case 57: chDrive_ = 1.f + a * 4.f; chMix_ = b; break;
      case 58: keGLo_ = a * 2.f; keGMid_ = b * 2.f; keGHi_ = c * 2.f; break;
      case 59: wbRate_ = 0.1f + a * 8.f; wbDepth_ = b; wbQ_ = 0.5f - c * 0.45f; break;
      case 60: mtTime_ = clampf((0.05f + a * 0.4f) * sr_, 1.f, (float)(kDelayMax - 1)); mtFb_ = b * 0.7f; break;
      case 61: ddTime_ = clampf((0.05f + a * 0.5f) * sr_, 1.f, (float)(kDelayMax - 1)); ddFb_ = b * 0.7f; ddDepth_ = c; break;
      case 62: rvFb_ = 0.75f + a * 0.2f; rvDamp_ = 0.3f;
               crush_.SetDownsampleFactor(clampf(1.f - b * 0.8f, 0.05f, 1.f));
               crush_.SetBitsToCrush((uint8_t)(2 + (int)((1.f - c) * 10.f))); break;
      case 63: rfPer_ = (int)clampf(400.f + (1.f - a) * 12000.f, 64.f, 48000.f);
               rfQ_ = 0.5f - b * 0.45f; rfRng_ = 1000.f + c * 5000.f; break;
      default: break;
    }
    if(frozen_){                                   // B2: hold the tail at (near) unity feedback
      switch(fx_){
        case 10: rvFb_ = 0.999f; break; case 11: dlFb_ = 1.0f;     break;
        case 14: combFb_ = 0.995f; break; case 25: rFb_ = 0.999f;  break;
        case 26: cmFb_ = 0.997f;  break; case 32: stFreeze_ = true; break;
        case 35: spFb_ = 0.97f;   break; case 39: dnFb_ = 1.0f;    break;
        default: break;
      }
      killIn_ = (fx_==10||fx_==11||fx_==14||fx_==25||fx_==26||fx_==35||fx_==39);
    } else killIn_ = false;
  }

  float Process(float in){
    float ki = killIn_ ? 0.f : 1.f;   // B2 freeze kills new input into feedback effects
    switch(fx_){
      case 0:  return chorus_.Process(in);
      case 1:  return flanger_.Process(in);
      case 2:  return phaser_.Process(in);
      case 3:  return tremolo_.Process(in);
      case 4:  return autowah_.Process(in);
      case 5:  return drive_.Process(in);
      case 6:  return fold_.Process(in);
      case 7:  return crush_.Process(in);
      case 8:  return ladder_.Process(in);
      case 9:  svf_.Process(in);
               return svfMode_ == 0 ? svf_.Low() : (svfMode_ == 1 ? svf_.Band() : svf_.High());
      case 10: return reverb(in * ki);
      case 11: return delayfx(in * ki);
      case 12: { float x = in; return pitch_.Process(x); }
      case 13: return in * sinf01(ringph_ = fracf(ringph_ + ringInc_));
      case 14: { dl_.SetDelay(combLen_); float rd = dl_.Read(); dl_.Write(in * ki + rd * combFb_); return (in + rd) * 0.7f; }
      case 15: { float d = crush_.Process(in); lofiLp_ += (d - lofiLp_) * lofiCut_; return lofiLp_; }
      case 16: { gph_ = fracf(gph_ + gateRate_ / sr_); float g = (gph_ < gateWidth_) ? 1.f : 0.f;
                 gateSm_ += (g - gateSm_) * gateSmC_; return in * gateSm_; }
      case 17: { float x = in * wsDrive_ + wsAsym_; float y = x - x * x * x * (1.f / 6.75f);
                 y = clampf(y, -1.f, 1.f); wsLp_ += (y - wsLp_) * wsCut_; return wsLp_; }
      case 18: { float r = fabsf(in) * 2.f * octGain_; octLp_ += (r - octLp_) * octCut_;
                 return clampf(r - octLp_, -1.f, 1.f); }
      case 19: { soap_.Process(in); return soapReject_ ? soap_.Bandreject() : soap_.Bandpass(); }
      case 20: return string_.Process(in);
      case 21: { float x = in; limiter_.ProcessBlock(&x, 1, limPre_); return x; }
      case 22: { int r = apIdx_ - apLen_; if(r < 0) r += kApLen; float d = apd_[r];
                 float y = -apFb_ * in + d; apd_[apIdx_] = in + apFb_ * y;
                 if(++apIdx_ >= kApLen) apIdx_ = 0; return y; }
      case 23: { toneLp_ += (in - toneLp_) * toneCut_; return toneHp_ ? (in - toneLp_) : toneLp_; }
      case 24: return clampf(in * clipPre_, -1.f, 1.f);
      case 25: { int rd = rIdx_ - rLen_; if(rd < 0) rd += kBufN; float y = rbuf_[rd];
                 rLp_ += (y - rLp_) * rDampC_; rbuf_[rIdx_] = in * ki * 0.5f + rLp_ * rFb_;
                 if(++rIdx_ >= kBufN) rIdx_ = 0; return y; }
      case 26: { float s = 0.f;
                 for(int k = 0; k < 3; k++){ int rd = cmi_[k] - cmL_[k]; if(rd < 0) rd += kBufN;
                   float y = cmb_[k][rd]; cmb_[k][cmi_[k]] = in * ki * 0.33f + y * cmFb_;
                   if(++cmi_[k] >= kBufN) cmi_[k] = 0; s += y; } return s * 0.5f; }
      case 27: { float s = 0.f;
                 for(int k = 0; k < 3; k++){ float y = fb0_[k] * in - fa1_[k] * fz1_[k] - fa2_[k] * fz2_[k];
                   fz2_[k] = fz1_[k]; fz1_[k] = y; s += y; } return clampf(s * 0.8f, -1.f, 1.f); }
      case 28: { float r = fabsf(in);
                 lpgEnv_ += (r > lpgEnv_) ? (r - lpgEnv_) * 0.05f : (r - lpgEnv_) * lpgDec_;
                 float cut = clampf(lpgBase_ + lpgEnv_ * lpgAmt_, 0.002f, 0.95f);
                 lpgLp_ += (in - lpgLp_) * cut; return lpgLp_ * clampf(lpgEnv_ * 3.f, 0.f, 1.f); }
      case 29: { float lfo = sinf01(vph_); vph_ = fracf(vph_ + vRate_ / sr_);
                 vbuf_[vIdx_] = in; float y = frd(vbuf_, kVibN, vIdx_, 400.f + vDepth_ * lfo);
                 if(++vIdx_ >= kVibN) vIdx_ = 0; return y; }
      case 30: { ebuf_[eIdx_] = in; float s = 0.f; const float rat[3] = {0.8f, 1.0f, 1.3f};
                 for(int k = 0; k < 3; k++){ float lfo = sinf01(eph_[k]);
                   eph_[k] = fracf(eph_[k] + eRateB_ * rat[k] / sr_);
                   s += frd(ebuf_, kEnsN, eIdx_, 200.f + k * 60.f + eDepth_ * lfo); }
                 if(++eIdx_ >= kEnsN) eIdx_ = 0; return s * 0.4f; }
      case 31: { float wow = sinf01(wowPh_); wowPh_ = fracf(wowPh_ + 0.5f / sr_);
                 float fl = sinf01(flPh_); flPh_ = fracf(flPh_ + 6.f / sr_);
                 tbuf_[tIdx_] = in; float y = frd(tbuf_, kTapeN, tIdx_, 600.f + wowAmt_ * wow + 30.f * fl);
                 if(++tIdx_ >= kTapeN) tIdx_ = 0; y = tanhf(y * tDrive_);
                 tLp_ += (y - tLp_) * tTone_; return tLp_; }
      case 32: { if(!stFreeze_) sbuf_[sIdx_] = in; int base = sIdx_ - stLen_;
                 int rp = base + stPh_; rp %= kStutN; if(rp < 0) rp += kStutN; float y = sbuf_[rp];
                 if(++stPh_ >= stLen_) stPh_ = 0; if(++sIdx_ >= kStutN) sIdx_ = 0; return y; }
      case 33: { rvbuf_[rvIdx_] = in; int rp = rvIdx_ - rvPh_; if(rp < 0) rp += kRevN; float y = rvbuf_[rp];
                 if(++rvPh_ >= rvLen_) rvPh_ = 0; if(++rvIdx_ >= kRevN) rvIdx_ = 0; return y; }
      case 34: { float y = in; const int bl[4] = {113, 211, 347, 479};
                 for(int k = 0; k < 4; k++){ int L = (int)(bl[k] * dfSize_); if(L < 2) L = 2;
                   y = apf(dbuf_[k], kDifN, di_[k], L, dfG_, y); } return y; }
      case 35: { int rd = spIdx_ - spLen_; if(rd < 0) rd += kSpN; float y = spbuf_[rd];
                 y = apf(spa_[0], kSpaN, spai_[0], 180, 0.6f, y);
                 y = apf(spa_[1], kSpaN, spai_[1], 120, 0.6f, y);
                 spbuf_[spIdx_] = in * ki * 0.5f + tanhf(y * spFb_);
                 if(++spIdx_ >= kSpN) spIdx_ = 0; return y; }
      case 36: { float x = in * fzGain_ + fzBias_; float y = x / (1.f + fabsf(x));
                 fzLp_ += (y - fzLp_) * fzTone_; return fzLp_; }
      case 37: { float lfo = sinf01(rsSwPh_); rsSwPh_ = fracf(rsSwPh_ + rsRate_ / sr_);
                 float inc = rsBaseInc_ * (1.f + rsDepth_ * lfo); rsPh_ = fracf(rsPh_ + inc);
                 return in * sinf01(rsPh_); }
      case 38: { puPh_ = fracf(puPh_ + puRate_ / sr_); float env = powf(puPh_, puShape_);
                 return in * ((1.f - puDepth_) + puDepth_ * env); }
      case 39: { int rd = dnIdx_ - dnLen_; if(rd < 0) rd += kBufN; float y = dnbuf_[rd];
                 dnLp_ += (y - dnLp_) * dnLpC_; dnbuf_[dnIdx_] = tanhf(in * ki * 0.3f + dnLp_ * dnFb_);
                 if(++dnIdx_ >= kBufN) dnIdx_ = 0; return y; }
      case 40: return bqH_.Proc(in);
      case 41: return bqP_.Proc(in);
      case 42: return bqN_.Proc(in);
      case 43: { tiltLp_ += (in - tiltLp_) * tiltCut_; float lo = tiltLp_, hi = in - lo;
                 return lo * (1.f - tiltAmt_) + hi * (1.f + tiltAmt_); }
      case 44: { float fb = shPitch_.Process(shPrev_) * shAmt_;
                 float wet = reverb(in * ki * 0.5f + fb); shPrev_ = wet; return wet; }
      case 45: { dl_.SetDelay(dlTime_); float rd = dl_.Read(); dl_.Write(in * ki + rd * dlFb_);
                 return reverb(rd * 0.5f + in * 0.3f); }
      case 46: { gbuf_[gw_] = in;
                 if(--gspawn_ <= 0){ gspawn_ = gspawnPer_;
                   int sl = -1; for(int k = 0; k < 4; k++){ if(grph_[k] >= 1.f){ sl = k; break; } }
                   if(sl >= 0){ grng_ = grng_ * 1664525u + 1013904223u;
                     float off = 1000.f + (float)((grng_ >> 9) % 12000u);
                     gp_[sl] = (float)gw_ - off; if(gp_[sl] < 0.f) gp_[sl] += (float)kGrainN;
                     grph_[sl] = 0.f; } }
                 float acc = 0.f;
                 for(int k = 0; k < 4; k++){ if(grph_[k] < 1.f){
                     int i0 = (int)gp_[k]; float fr = gp_[k] - (float)i0; int i1 = i0 + 1; if(i1 >= kGrainN) i1 -= kGrainN;
                     float win = 0.5f - 0.5f * cosf(6.2831853f * grph_[k]);
                     acc += (gbuf_[i0] * (1.f - fr) + gbuf_[i1] * fr) * win;
                     gp_[k] += gSpeed_; if(gp_[k] >= (float)kGrainN) gp_[k] -= (float)kGrainN;
                     grph_[k] += gSpeed_ / (float)glen_; } }
                 if(++gw_ >= kGrainN) gw_ = 0; return acc * 0.6f; }
      case 47: { float lfo = sinf01(hornPh_); hornPh_ = fracf(hornPh_ + rotRate_ / sr_);
                 rotBuf_[rotIdx_] = in; float y = frd(rotBuf_, kRotN, rotIdx_, 200.f + rotDepth_ * lfo);
                 if(++rotIdx_ >= kRotN) rotIdx_ = 0; return y * (0.7f + 0.3f * lfo); }
      case 48: { float lfo = sinf01(pwPh_); pwPh_ = fracf(pwPh_ + pwRate_ / sr_);
                 shPitch_.SetTransposition(pwDepth_ * lfo); return shPitch_.Process(in); }
      case 49: { float r = fabsf(in); swEnv_ += (r > swEnv_) ? (r - swEnv_) * swAtk_ : (r - swEnv_) * swRel_;
                 return in * clampf(swEnv_ * 4.f, 0.f, 1.f); }
      case 50: { float r = fabsf(in); ngEnv_ += (r > ngEnv_) ? (r - ngEnv_) * 0.05f : (r - ngEnv_) * ngRel_;
                 float tg = ngEnv_ > ngThr_ ? 1.f : 0.f; ngGain_ += (tg - ngGain_) * 0.02f; return in * ngGain_; }
      case 51: return tanhf(in * satDrive_);
      case 52: { float q = floorf(in * bfLv_ + 0.5f) / bfLv_; float x = q * bfG_;
                 x = fmodf(x + 1.f, 4.f); if(x < 0.f) x += 4.f; x = (x > 2.f) ? (4.f - x) : x; return x - 1.f; }
      case 53: { if(--snhCnt_ <= 0){ snhVal_ = in; snhCnt_ = snhPer_; } return snhVal_; }
      case 54: { float d = in - slw_; if(d > slwRate_) slw_ += slwRate_;
                 else if(d < -slwRate_) slw_ -= slwRate_; else slw_ = in; return slw_; }
      case 55: { rezLp_ += rezF_ * rezBp_; float hp = in - rezLp_ - rezQ_ * rezBp_;
                 rezBp_ += rezF_ * hp; return tanhf(rezLp_ * rezDrive_); }
      case 56: { bool pos = in > 0.f; if(pos && !subPrev_) subFlip_ = !subFlip_; subPrev_ = pos;
                 float r = fabsf(in); subEnv_ += (r - subEnv_) * 0.01f;
                 float sq = (subFlip_ ? 1.f : -1.f) * subEnv_ * 2.f;
                 subLp_ += (sq - subLp_) * subTone_; return clampf(subLp_, -1.f, 1.f); }
      case 57: { float x = clampf(in * chDrive_, -1.f, 1.f); float x2 = x * x;
                 float t2 = 2.f * x2 - 1.f; float t3 = 4.f * x2 * x - 3.f * x; float t4 = 8.f * x2 * x2 - 8.f * x2 + 1.f;
                 float h = (t2 + t3 + t4) * 0.33f; return x * (1.f - chMix_) + h * chMix_; }
      case 58: { keLp1_ += (in - keLp1_) * keFLo_; keLp2_ += (in - keLp2_) * keFHi_;
                 float low = keLp1_, high = in - keLp2_, mid = keLp2_ - keLp1_;
                 return clampf(low * keGLo_ + mid * keGMid_ + high * keGHi_, -1.f, 1.f); }
      case 59: { float lfo = 0.5f + 0.5f * sinf01(wbPh_); wbPh_ = fracf(wbPh_ + wbRate_ / sr_);
                 float hz = 150.f + wbDepth_ * lfo * 3500.f;
                 float f = 2.f * sinf(3.14159265f * clampf(hz, 40.f, sr_ * 0.45f) / sr_);
                 wbLp_ += f * wbBp_; float hp = in - wbLp_ - wbQ_ * wbBp_; wbBp_ += f * hp; return wbLp_; }
      case 60: { float t1 = dl_.Read(mtTime_); float t2 = dl_.Read(mtTime_ * 0.66f); float t3 = dl_.Read(mtTime_ * 0.33f);
                 mtLast_ = t1; dl_.Write(in + mtLast_ * mtFb_); return (t1 + t2 * 0.7f + t3 * 0.5f) * 0.5f; }
      case 61: { float rd = dl_.Read(ddTime_); float r = fabsf(in);
                 ddEnv_ += (r - ddEnv_) * (r > ddEnv_ ? 0.05f : 0.005f);
                 float duck = 1.f - clampf(ddEnv_ * 4.f, 0.f, 1.f) * ddDepth_;
                 dl_.Write(in + rd * ddFb_); return rd * duck; }
      case 62: return crush_.Process(reverb(in * ki * 0.5f));
      case 63: { if(--rfCnt_ <= 0){ rfCnt_ = rfPer_; rfSeed_ = rfSeed_ * 1664525u + 1013904223u;
                   rfTarget_ = (float)((rfSeed_ >> 9) & 0xFFFFu) / 65535.f; }
                 rfF_ += (rfTarget_ - rfF_) * 0.01f;
                 float hz = 150.f + rfF_ * rfRng_;
                 float f = 2.f * sinf(3.14159265f * clampf(hz, 40.f, sr_ * 0.45f) / sr_);
                 rfLp_ += f * rfBp_; float hp = in - rfLp_ - rfQ_ * rfBp_; rfBp_ += f * hp; return rfBp_; }
      default: return in;
    }
  }

 private:
  // generic fractional read: `d` samples behind the newest sample at widx
  static inline float frd(const float* buf, int N, int widx, float d){
    float rp = (float)widx - d; while(rp < 0.f) rp += (float)N;
    int i0 = (int)rp; float f = rp - (float)i0; int i1 = i0 + 1; if(i1 >= N) i1 -= N;
    return buf[i0] * (1.f - f) + buf[i1] * f;
  }
  // schroeder allpass stage over a fixed circular buffer
  static inline float apf(float* buf, int N, int& idx, int len, float g, float x){
    int rd = idx - len; if(rd < 0) rd += N; float by = buf[rd];
    float out = -g * x + by; buf[idx] = x + g * out;
    if(++idx >= N) idx = 0; return out;
  }

  // ---- clean-room reverb (freeverb-style) ----------------------------------
  static const int kC0 = 1116, kC1 = 1188, kC2 = 1277, kC3 = 1356;
  static const int kA0 = 556,  kA1 = 441;
  static const int kCombSum = kC0 + kC1 + kC2 + kC3;
  static const int kApSum   = kA0 + kA1;
  float cbuf_[kCombSum];
  float abuf_[kApSum];
  int   cidx_[4]; float cfs_[4];
  int   aidx_[2];
  float rvFb_, rvDamp_;
  float comb(int k, float in){
    float* buf = cbuf_ + RVC_OFF[k]; int& idx = cidx_[k];
    float y = buf[idx];
    cfs_[k] = y * (1.f - rvDamp_) + cfs_[k] * rvDamp_;
    buf[idx] = in + cfs_[k] * rvFb_;
    if(++idx >= RVC_LEN[k]) idx = 0;
    return y;
  }
  float allpass(int k, float in){
    float* buf = abuf_ + RVA_OFF[k]; int& idx = aidx_[k];
    float y = buf[idx]; float out = -in + y;
    buf[idx] = in + y * 0.5f;
    if(++idx >= RVA_LEN[k]) idx = 0;
    return out;
  }
  float reverb(float in){
    float x = in * 0.25f;
    float s = comb(0, x) + comb(1, x) + comb(2, x) + comb(3, x);
    s = allpass(0, s); s = allpass(1, s);
    return s;
  }

  // ---- delay ---------------------------------------------------------------
  static const size_t kDelayMax = 48000;
  daisysp::DelayLine<float, kDelayMax> dl_;
  float dlTime_, dlFb_, dlLp_, dlTone_ = 0.5f;
  float delayfx(float in){
    dl_.SetDelay(dlTime_);
    float rd = dl_.Read();
    dlLp_ += (rd - dlLp_) * dlTone_;
    dl_.Write(in + dlLp_ * dlFb_);
    return rd;
  }

  // existing extra-fx state
  float combLen_, combFb_, lofiLp_, lofiCut_;
  float gph_, gateRate_, gateWidth_, gateSm_, gateSmC_;
  float wsDrive_, wsAsym_, wsCut_, wsLp_;
  float octLp_, octCut_, octGain_;
  bool  soapReject_;
  float limPre_;
  static const int kApLen = 2048;
  float apd_[kApLen];
  int   apIdx_, apLen_;
  float apFb_;
  float toneLp_, toneCut_;
  bool  toneHp_;
  float clipPre_;
  bool  frozen_, killIn_;
  // ---- wave-3 state ----
  BQ bqH_, bqP_, bqN_;
  float tiltLp_, tiltCut_, tiltAmt_;
  float shPrev_, shAmt_;
  static const int kGrainN = 24000;
  float gbuf_[kGrainN]; int gw_; float gp_[4], grph_[4]; int glen_; float gSpeed_; int gspawn_, gspawnPer_;
  uint32_t grng_;
  static const int kRotN = 1600;
  float rotBuf_[kRotN]; int rotIdx_; float hornPh_, rotRate_, rotDepth_;
  float pwPh_, pwRate_, pwDepth_;
  float swEnv_, swAtk_, swRel_;
  float ngEnv_, ngThr_, ngRel_, ngGain_;
  float satDrive_;
  float bfLv_, bfG_;
  float snhVal_; int snhCnt_, snhPer_;
  float slw_, slwRate_;
  float rezLp_, rezBp_, rezF_, rezQ_, rezDrive_;
  // ---- wave-4 state ----
  float subEnv_, subLp_, subTone_; bool subPrev_, subFlip_;
  float chDrive_, chMix_;
  float keLp1_, keLp2_, keGLo_, keGMid_, keGHi_, keFLo_, keFHi_;
  float wbLp_, wbBp_, wbPh_, wbRate_, wbDepth_, wbQ_;
  float mtTime_, mtFb_, mtLast_;
  float ddTime_, ddFb_, ddEnv_, ddDepth_;
  float rfLp_, rfBp_, rfF_, rfQ_, rfTarget_, rfRng_; int rfCnt_, rfPer_; uint32_t rfSeed_;

  // ---- wave-2 buffers + state ----------------------------------------------
  static const int kBufN  = 2048;    // RESON / DRONE
  static const int kVibN  = 1600;
  static const int kEnsN  = 2400;
  static const int kTapeN = 4800;
  static const int kStutN = 48000;
  static const int kRevN  = 24000;
  static const int kDifN  = 512;
  static const int kSpN   = 4096;
  static const int kSpaN  = 256;

  float rbuf_[kBufN];    int rIdx_, rLen_;  float rFb_, rDampC_, rLp_;
  float cmb_[3][kBufN];  int cmi_[3], cmL_[3]; float cmFb_;
  float fz1_[3], fz2_[3], fa1_[3], fa2_[3], fb0_[3];
  float lpgEnv_, lpgLp_, lpgDec_, lpgBase_, lpgAmt_;
  float vbuf_[kVibN];    int vIdx_;  float vph_, vRate_, vDepth_;
  float ebuf_[kEnsN];    int eIdx_;  float eph_[3], eDepth_, eRateB_;
  float tbuf_[kTapeN];   int tIdx_;  float wowPh_, flPh_, wowAmt_, tDrive_, tTone_, tLp_;
  float sbuf_[kStutN];   int sIdx_, stLen_, stPh_;  bool stFreeze_;
  float rvbuf_[kRevN];   int rvIdx_, rvLen_, rvPh_;
  float dbuf_[4][kDifN]; int di_[4];  float dfSize_, dfG_;
  float spbuf_[kSpN];    int spIdx_, spLen_;  float spFb_;
  float spa_[2][kSpaN];  int spai_[2];
  float fzGain_, fzBias_, fzTone_, fzLp_;
  float rsBaseInc_, rsRate_, rsDepth_, rsPh_, rsSwPh_;
  float puPh_, puRate_, puDepth_, puShape_;
  float dnbuf_[kBufN];   int dnIdx_, dnLen_;  float dnFb_, dnLpC_, dnLp_;

  void ZeroAll(){
    apIdx_ = 0; apLen_ = 512; apFb_ = -0.5f;
    for(int i = 0; i < kBufN; i++){ rbuf_[i] = 0.f; dnbuf_[i] = 0.f; }
    rIdx_ = 0; rLen_ = 256; rFb_ = 0.9f; rDampC_ = 0.4f; rLp_ = 0.f;
    for(int k = 0; k < 3; k++){ cmi_[k] = 0; cmL_[k] = 256; for(int i = 0; i < kBufN; i++) cmb_[k][i] = 0.f; }
    cmFb_ = 0.9f;
    for(int k = 0; k < 3; k++){ fz1_[k] = fz2_[k] = 0.f; fa1_[k] = 0.f; fa2_[k] = 0.f; fb0_[k] = 0.05f; }
    lpgEnv_ = lpgLp_ = 0.f; lpgDec_ = 0.005f; lpgBase_ = 0.05f; lpgAmt_ = 0.4f;
    for(int i = 0; i < kVibN; i++) vbuf_[i] = 0.f; vIdx_ = 0; vph_ = 0.f; vRate_ = 3.f; vDepth_ = 120.f;
    for(int i = 0; i < kEnsN; i++) ebuf_[i] = 0.f; eIdx_ = 0; eDepth_ = 12.f; eRateB_ = 0.8f;
    eph_[0] = 0.f; eph_[1] = 0.33f; eph_[2] = 0.66f;
    for(int i = 0; i < kTapeN; i++) tbuf_[i] = 0.f; tIdx_ = 0; wowPh_ = flPh_ = 0.f;
    wowAmt_ = 120.f; tDrive_ = 2.f; tTone_ = 0.5f; tLp_ = 0.f;
    for(int i = 0; i < kStutN; i++) sbuf_[i] = 0.f; sIdx_ = 0; stLen_ = 4800; stPh_ = 0; stFreeze_ = false;
    for(int i = 0; i < kRevN; i++) rvbuf_[i] = 0.f; rvIdx_ = 0; rvLen_ = 8000; rvPh_ = 0;
    for(int k = 0; k < 4; k++){ di_[k] = 0; for(int i = 0; i < kDifN; i++) dbuf_[k][i] = 0.f; }
    dfSize_ = 0.7f; dfG_ = 0.6f;
    for(int i = 0; i < kSpN; i++) spbuf_[i] = 0.f; spIdx_ = 0; spLen_ = 1500; spFb_ = 0.7f;
    for(int k = 0; k < 2; k++){ spai_[k] = 0; for(int i = 0; i < kSpaN; i++) spa_[k][i] = 0.f; }
    fzGain_ = 8.f; fzBias_ = 0.f; fzTone_ = 0.5f; fzLp_ = 0.f;
    rsBaseInc_ = 200.f / sr_; rsRate_ = 1.f; rsDepth_ = 0.5f; rsPh_ = 0.f; rsSwPh_ = 0.f;
    puPh_ = 0.f; puRate_ = 2.f; puDepth_ = 0.7f; puShape_ = 2.f;
    dnIdx_ = 0; dnLen_ = 256; dnFb_ = 0.97f; dnLpC_ = 0.4f; dnLp_ = 0.f;
    frozen_ = false; killIn_ = false;
    bqH_.Reset(); bqP_.Reset(); bqN_.Reset();
    tiltLp_ = 0.f; tiltCut_ = 0.2f; tiltAmt_ = 0.f;
    shPrev_ = 0.f; shAmt_ = 0.4f;
    for(int i = 0; i < kGrainN; i++) gbuf_[i] = 0.f; gw_ = 0;
    for(int k = 0; k < 4; k++){ gp_[k] = 0.f; grph_[k] = 1.f; }
    glen_ = 4000; gSpeed_ = 1.f; gspawn_ = 0; gspawnPer_ = 2000; grng_ = 22222u;
    for(int i = 0; i < kRotN; i++) rotBuf_[i] = 0.f; rotIdx_ = 0; hornPh_ = 0.f; rotRate_ = 4.f; rotDepth_ = 30.f;
    pwPh_ = 0.f; pwRate_ = 1.f; pwDepth_ = 0.f;
    swEnv_ = 0.f; swAtk_ = 0.002f; swRel_ = 0.02f;
    ngEnv_ = 0.f; ngThr_ = 0.05f; ngRel_ = 0.02f; ngGain_ = 0.f;
    satDrive_ = 3.f; bfLv_ = 16.f; bfG_ = 2.f;
    snhVal_ = 0.f; snhCnt_ = 1; snhPer_ = 40;
    slw_ = 0.f; slwRate_ = 0.05f;
    rezLp_ = rezBp_ = 0.f; rezF_ = 0.2f; rezQ_ = 0.2f; rezDrive_ = 2.f;
    subEnv_ = subLp_ = 0.f; subTone_ = 0.1f; subPrev_ = false; subFlip_ = false;
    chDrive_ = 2.f; chMix_ = 0.5f;
    keLp1_ = keLp2_ = 0.f; keGLo_ = keGMid_ = keGHi_ = 1.f; keFLo_ = 0.02f; keFHi_ = 0.3f;
    wbLp_ = wbBp_ = 0.f; wbPh_ = 0.f; wbRate_ = 2.f; wbDepth_ = 0.7f; wbQ_ = 0.2f;
    mtTime_ = 12000.f; mtFb_ = 0.4f; mtLast_ = 0.f;
    ddTime_ = 12000.f; ddFb_ = 0.4f; ddEnv_ = 0.f; ddDepth_ = 0.8f;
    rfLp_ = rfBp_ = 0.f; rfF_ = 0.3f; rfQ_ = 0.2f; rfTarget_ = 0.3f; rfRng_ = 3000.f;
    rfCnt_ = 1; rfPer_ = 4000; rfSeed_ = 7u;
  }

  float sr_;
  int fx_, svfMode_;
  float ringph_ = 0.f, ringInc_;
  daisysp::Chorus     chorus_;
  daisysp::Flanger    flanger_;
  daisysp::Phaser     phaser_;
  daisysp::Tremolo    tremolo_;
  daisysp::Autowah    autowah_;
  daisysp::Overdrive  drive_;
  daisysp::Wavefolder fold_;
  daisysp::Decimator  crush_;
  daisysp::LadderFilter ladder_;
  daisysp::Svf        svf_;
  daisysp::PitchShifter pitch_;
  daisysp::PitchShifter shPitch_;
  daisysp::Soap       soap_;
  daisysp::String     string_;
  daisysp::Limiter    limiter_;
};

}  // namespace afx
