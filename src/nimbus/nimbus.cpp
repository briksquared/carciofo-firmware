#include <math.h>

#include "../../lib/carciofo.h"
#include "daisysp.h"
#include "granular_processor.h"
#include "resources.h"

using namespace daisy;
using namespace carciofo;

/*
 * Clouds / Nimbus on the Carciofo (Daisy Seed3).
 * DSP is the Löwenzahnhonig Nimbus port (Electro-Smith / Mutable Clouds),
 * unchanged. This file only maps the Carciofo panel.
 *
 *   Audio IN   recorded into the buffer while not frozen
 *   Audio OUT  fully wet, soft-clipped
 *
 *   P1 (+CV1)  position in the buffer
 *   P2         size (grain length / window / loop)
 *   P3         pitch, ±24 st, unison dead-zone at centre
 *   P4         density (granular: clockwise half also opens texture + reverb)
 *   CV2        freeze gate (rising edge also triggers one grain)
 *   B1         cycle mode: granular → stretch → looping delay
 *   B2         latch freeze (works with nothing patched to CV2)
 *   LED        colour = mode, white = frozen (panel LED is dark until reflow)
 */

namespace {

constexpr float kPitchDeadZone = 0.04f;
constexpr float kPitchRangeSemitones = 24.f;
constexpr float kFreezeOn = 0.55f;
constexpr float kFreezeOff = 0.45f;

enum Mode { GRANULAR = 0, STRETCH = 1, LOOPING = 2, NUM_MODES = 3 };

float Clamp01(float v) {
  if (v < 0.f) return 0.f;
  if (v > 1.f) return 1.f;
  return v;
}

float PitchFromPot(float pot) {
  float offset = pot - 0.5f;
  float magnitude = fabsf(offset) - kPitchDeadZone;
  if (magnitude <= 0.f) return 0.f;
  float x = magnitude / (0.5f - kPitchDeadZone);
  float semitones = kPitchRangeSemitones * x * x;
  return offset < 0.f ? -semitones : semitones;
}

PlaybackMode PlaybackFor(int mode) {
  if (mode == STRETCH) return PLAYBACK_MODE_STRETCH;
  if (mode == LOOPING) return PLAYBACK_MODE_LOOPING_DELAY;
  return PLAYBACK_MODE_GRANULAR;
}

}  // namespace

static Carciofo hw;
static GranularProcessorClouds processor;
static uint8_t block_mem[118784];
static uint8_t block_ccm[65536 - 128];
static Parameters* parameters;
static volatile int mode = GRANULAR;
static volatile bool freeze_latch = false;
static bool frozen = false;

static void PaintLed(bool freeze) {
  if (freeze) {
    hw.led.Set(1.f, 1.f, 1.f);
    return;
  }
  if (mode == STRETCH) hw.led.Set(1.f, 0.45f, 0.05f);
  else if (mode == LOOPING) hw.led.Set(0.7f, 0.05f, 0.9f);
  else hw.led.Set(0.05f, 0.55f, 0.9f);
}

static void ReadControls() {
  parameters->position = hw.GetPot(POT_1);
  parameters->size = hw.GetPot(POT_2);
  parameters->pitch = PitchFromPot(hw.GetPot(POT_3));
  parameters->feedback = (mode == LOOPING) ? hw.GetPot(POT_4) : 0.15f;
  parameters->texture = 0.5f;
  parameters->reverb = 0.2f;
  parameters->density = 0.f;

  if (mode == GRANULAR) {
    float density = hw.GetPot(POT_4);
    parameters->density = density;
    float wash = Clamp01((density - 0.5f) * 2.f);
    parameters->texture = 0.5f + 0.5f * wash;
    parameters->reverb = 0.2f + 0.4f * wash;
  } else if (mode == STRETCH) {
    parameters->texture = hw.GetPot(POT_4);
  }

  parameters->position = Clamp01(parameters->position + hw.GetCv(CV_1));
}

static void AudioCallback(AudioHandle::InputBuffer in,
                          AudioHandle::OutputBuffer out, size_t size) {
  ReadControls();

  const float gate = hw.GetCv(CV_2);
  const bool was_frozen = frozen;
  if (freeze_latch || gate > kFreezeOn) frozen = true;
  else if (gate < kFreezeOff) frozen = false;

  parameters->freeze = frozen;
  parameters->trigger = frozen && !was_frozen;

  FloatFrame input[48];
  FloatFrame output[48];
  if (size > 48) size = 48;

  for (size_t i = 0; i < size; i++) {
    input[i].l = in[0][i];
    input[i].r = in[1][i];
    output[i].l = output[i].r = 0.f;
  }

  processor.Process(input, output, size);

  for (size_t i = 0; i < size; i++) {
    out[0][i] = daisysp::SoftClip(output[i].l);
    out[1][i] = daisysp::SoftClip(output[i].r);
  }
}

int main(void) {
  hw.Init(32);  // Clouds does not work with larger blocks
  hw.seed.StartLog(false);
  float sample_rate = hw.SampleRate();

  InitResources(sample_rate);
  processor.Init(sample_rate, block_mem, sizeof(block_mem), block_ccm,
                 sizeof(block_ccm));
  processor.set_playback_mode(PLAYBACK_MODE_GRANULAR);
  processor.set_quality(0);

  parameters = processor.mutable_parameters();
  parameters->dry_wet = 1.f;
  parameters->stereo_spread = 0.5f;
  parameters->freeze = false;
  parameters->trigger = false;
  parameters->gate = false;

  hw.StartAudio(AudioCallback);
  PaintLed(false);
  hw.seed.PrintLine("nimbus granular");

  while (1) {
    hw.ProcessControls();
    processor.Prepare();

    if (hw.button[BUTTON_1].RisingEdge()) {
      mode = (mode + 1) % NUM_MODES;
      processor.set_playback_mode(PlaybackFor(mode));
      const char* name = mode == STRETCH     ? "stretch"
                         : mode == LOOPING    ? "looping-delay"
                                              : "granular";
      hw.seed.PrintLine("mode %s", name);
    }
    if (hw.button[BUTTON_2].RisingEdge()) {
      freeze_latch = !freeze_latch;
      hw.seed.PrintLine("freeze latch %d", freeze_latch ? 1 : 0);
    }
    PaintLed(frozen || freeze_latch);
  }
}
