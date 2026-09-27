# Carciofo module library — "$500 cart → free flashes"

Eurorack/DSP modules re-implemented as Carciofo (Daisy Seed) firmware, built on
DaisySP. Flash: enter DFU (hold BOOT, tap RESET), then
`dfu-util -a 0 -s 0x08000000:leave -D <name>.bin -d ,0483:df11`, then tap RESET.
Pitch voices: P1 (+CV1) = pitch. Effects process the stereo audio in → out.

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

## Synth voices (free-run drone, pitch = P1+CV1)
| va_osc | virtual-analog + sub | pitch | waveshape | pulse width | sub level | — |
| fm | 2-op FM | pitch | ratio | index | level | — |
| formant | formant / vowel | pitch | formant | phase shift | — | — |
| additive | 16-harmonic additive | pitch | harmonic tilt | first harmonic | — | — |

## Triggered voices (gate on CV2)
| karplus | plucked string | pitch | structure | brightness | damping | B2 sustain · CV2 pluck |
| kick | analog bass drum | pitch | tone | decay | FM amount | CV2 trigger |

**Total: 22 modules.** Source in `wgd-ports/carciofo-firmware/src/<name>/`.
