# Carciofo module library — "$500 cart → free flashes"

Eurorack/DSP modules re-implemented as Carciofo (Daisy Seed) firmware, built on
DaisySP. Flash: enter DFU (hold BOOT, tap RESET), then
`dfu-util -a 0 -s 0x08000000:leave -D <name>.bin -d ,0483:df11`, then tap RESET.
Pitch voices: P1 (+CV1) = pitch. Effects process the stereo audio in → out.

## Granular (Clouds DSP, not a DaisySP stub)
| nimbus | Clouds / Nimbus | position (+CV1) | size | pitch ±2 oct | density / filter / feedback | B1 mode · B2 freeze latch · CV2 freeze |

## Resonators / meta (hand-wrapped Mutable ports)
| bin | module | P1 | P2 | P3 | P4 | B1 / B2 / CV2 |
|---|---|---|---|---|---|---|
| rings | Rings resonator | freq | structure | brightness | damping | B1 model · B2 sustain · CV2 strum |
| warps | Warps meta-mod | drive | osc pitch | dry/wet | osc level | B1 algo · B2 wave |

## Filters
| ladder | Moog ladder | cutoff | resonance | drive | dry/wet | B1 mode LP24/LP12/BP/HP |
| svf | state-variable | cutoff | resonance | drive | morph LP→BP→HP | — |

## Modulation
| chorus | stereo chorus | rate | depth | delay | dry/wet | — |
| flanger | flanger | rate | depth | feedback | dry/wet | — |
| phaser | phaser | rate | depth | feedback | freq | B1 poles |
| tremolo | tremolo | rate | depth | — | — | B1 wave |
| autowah | auto-wah | wah(+CV1) | dry/wet | level | — | — |

## Drive / lo-fi
| overdrive | overdrive | drive(+CV1) | tone | dry/wet | level | — |
| folder | wavefolder fuzz | fold gain(+CV1) | offset | dry/wet | level | — |
| crush | bitcrush/decimate | downsample | bit depth | dry/wet | — | B1 smooth |

## Pitch / time
| pitch | ±24st harmonizer | transpose(+CV1) | grain size | dry/wet | — | — |
| delay | delay (1s) | time | feedback | tone | dry/wet | — |
| pingpong | ping-pong delay | time | feedback | — | dry/wet | — |
| limiter | limiter/leveler | pre-gain | — | — | — | — |

## Macro voices
| plaits | Plaits-class | pitch (+CV1) | timbre | harmonics | level | B1 engine: VA / FM / formant / VOSIM / ZOsc / additive |
| drums | 808/909 drums | pitch (+CV1) | tone | decay | snap | B1 voice: kick / snare808 / snare909 / hat · CV2 or B2 trigger |

## Synth voices (free-run drone, pitch = P1+CV1)
| va_osc | virtual-analog + sub | pitch | waveshape | pulse width | sub level | — |
| fm | 2-op FM | pitch | ratio | index | level | — |
| formant | formant / vowel | pitch | formant | phase shift | — | — |
| additive | 16-harmonic additive | pitch | harmonic tilt | first harmonic | — | — |

## Triggered voices (gate on CV2)
| karplus | plucked string | pitch | structure | brightness | damping | B2 sustain · CV2 pluck |
| kick | analog bass drum | pitch | tone | decay | FM amount | CV2 trigger |
| modal | modal resonator | pitch (+CV1) | structure | brightness | damping | B1 sustain · B2 or CV2 strike |

## West coast
| westcoast | env-follow → wavefolder → LPG → space | fold gain (+CV1) | offset | LPG depth | space | ReverbSc in SDRAM (LGPL) |

## Plaits-class voices, one engine each
| vosim | VOSIM | pitch (+CV1) | formant 1 | formant 2 | shape | — |
| zosc | Z-oscillator | pitch (+CV1) | formant | shape | mode | — |
| grainlet | grainlet | pitch (+CV1) | formant | shape | bleed | — |
| particle | particle noise | freq (+CV1) | resonance | density | gain | — |
| dust | dust through SVF | density | cutoff (+CV1) | resonance | drive | — |

## Elements / filters / clocks
| elements | blow + strike + clocked noise into a resonator | pitch (+CV1) | structure | brightness | damping | CV2 blow density |
| stiff | guitar-excited Karplus string | pitch (+CV1) | bridge ↔ dispersion | brightness | damping | audio in is the exciter |
| soap | second-order allpass (bandpass / reject) | center (+CV1) | bandwidth | dry/wet | level | B1 held = band reject |
| fractal | fractal noise through Soap | noise rate | color | center (+CV1) | bandwidth | B1 held = band reject |
| compress | stereo compressor (LGPL) | threshold | ratio | attack | release | — |

## Generators
| marbles | clocked sample-hold of smooth random into a saw | rate | spread (+CV1) | probability / decay | base pitch | — |
| stages | phasor retriggers an AD envelope on a saw | pitch (+CV1) | rate | attack | decay | — |
| oscbank | 7-voice divide-down organ | pitch (+CV1) | register center | register width | level | amplitudes sum to 1 |
| saw | variable saw | pitch (+CV1) | waveshape | pulse width | level | — |
| blosc | band-limited osc (LGPL) | pitch (+CV1) | pulse width | octave (0–2) | level | B1 triangle / saw / square |

**39 DaisySP ports, plus `nimbus` (Clouds DSP).** Source in `wgd-ports/carciofo-firmware/src/<name>/`.

## Grids
| grids | topographic drum map (GPL pattern bytes from Mutable Grids) | map X (euclidean length 1) | map Y (length 2) | chaos (length 3) | tempo | B1 drums map ↔ Euclidean · CV1 density. Kick, analog snare, hat. |

## LGPL filters
| moog | Moog ladder | cutoff (+CV1) | resonance | dry/wet | level | — |
| comb | comb, 8192-sample buffer in SDRAM | frequency (+CV1) | decay | dry/wet | level | audio in |
| mode | resonant mode | frequency (+CV1) | Q | dry/wet | level | audio in |

**43 ports, plus `nimbus`.** `grids` uses the Mutable pattern tables, not a DaisySP class.

## Sampler / looper
| looper | DaisySP looper, 2 s in SDRAM | speed | half speed (below noon) | reverse (above noon) | dry/wet | B1 record · B2 mode: normal / dub / replace / frippertronics |
| grains | one-second buffer into GranularPlayer | speed −1..1 | transposition ±12 st (+CV1) | grain size | dry/wet | B1 freezes the write |

## LGPL voices and effects
| polypluck | four Karplus voices | pitch (+CV1) | damp | rate | octave | — |
| pluck | single Karplus | pitch (+CV1) | damp | decay | level | B1 recursive / average · B2 or CV2 trigger |
| jitter | random line into a saw | pitch (+CV1) | depth | slowest rate | fastest rate | — |
| fold | sample-hold fold (not the wavefolder) | hold time (+CV1) | dry/wet | level | input gain | audio in |
| bits | bit depth and sample-rate crush | bit depth | crush rate (+CV1) | dry/wet | level | distinct from `crush` (decimator) |
| allpass | allpass delay | loop time | reverb time | dry/wet | level | buffer in SDRAM |
| nlfilt | Dobson/Fitch nonlinear filter | a | b | d | level | CV1 offset · CV2 delay length |

**52 ports, plus `nimbus`.** Oliverb and Resonestor are not in this source tree.

## Existing engines, Carciofo panel
| ringsverb | Mutable Rings Dattorro plate (`rings/dsp/fx/reverb.h`) | wet | lowpass | diffusion (+CV1) | time (+CV2) | 32,768 × 16-bit buffer in SDRAM |
| metal | Loewenzahnhonig oversampled distortion (MT-2 style) | distortion (+CV1) | low shelf | high shelf (+CV2) | level | engine source is `distortion_engine.cpp` |
| splooge | octave shimmer, chorus, 150 ms delay, reverb | reverb time | dry/wet | tone (+CV1) | shimmer (+CV2) | delays and reverb in SDRAM |

**55 ports, plus `nimbus`.**

## Löwenzahnhonig engines on the Carciofo panel
| cloudseed | Cloud Seed, ten programs | program | dry/wet | decay (+CV1) | tone | CV2 freeze. Block size 48. SDRAM delay pool. |
| smoodge | 16 s chorus delay with clock | feedback (+CV2) | dry/wet | clock divisor | smear | CV1 clock. Both lines in SDRAM. |
| ringmod | left × sine/tri/square carrier | shape | blend | carrier freq (+CV1) | carrier level (+CV2) | right out is the carrier |
| vca | dual VCA | left offset | right offset | left CV amount | right CV amount | level = offset + amount × CV |

**59 ports, plus `nimbus`.**
