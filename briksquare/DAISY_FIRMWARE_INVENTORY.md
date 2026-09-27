# Daisy Seed firmware inventory — both repo roots

Generated 2026-09-27. Unique **buildable** Daisy projects (a Makefile that pulls
in libDaisy + a `main`), with `.claude/worktrees/` duplicate copies and
library-internal Makefiles excluded. Every one targets a Daisy (Seed or a board
variant), so each can be built for the Carciofo's Seed and the knobs reassigned.

Full path list: `DAISY_FIRMWARE_INVENTORY.tsv` (drive, collection, path).

## Totals
- **668 unique projects** across both drives — C: **587**, F: **81**
- Of those: **341 real firmware projects** + **327 libDaisy/DaisySP example demos**

## By collection
| Count | Collection | What's in it |
|------:|---|---|
| 260 | `hardware/daisy` | vendor libDaisy, wgd-ports, briksquare-daisy, examples |
| 171 | `audio/firmwares` | chip-madness hoard (Daisy subset) |
| 116 | `audio/electrosmith` | official Electrosmith instruments + libDaisy |
|  47 | `briksquare-daisy` (F:) | our own Daisy apps (plaits, sampler, rings-reverb, drums…) |
|  43 | `wgd-official` (F:) | WGD's official module firmwares |
|  23 | `audio/pedals` | Daisy guitar-pedal firmwares (Hothouse / Terrarium) |
|   8 | `audio/DSP_DREAMS` | the DSP harvest + porting playbook (see below) |

## The porting reference: `audio/DSP_DREAMS`
Not just code — the map for turning the Carciofo into anything:
- `DAISY_HARVEST.md` — whole Daisy ecosystem: hardware models, full DaisySP API,
  community effect collections (GuitarML, Hothouse, bkshepherd, Tone3000, NAM).
- `EURORACK_DSP.md` — "what to lift": Clouds→Nimbus, Rings→ModalVoice/Resonator/
  StringVoice, Plaits→per-engine DaisySP classes, Warps, Ripples, Marbles/Tides/
  Stages — mapped to Daisy-native code, with MIT/GPL license notes.
- `firmware/daisy/` — a working C++ effect-module framework already present.
- JSON catalogs: community_daisy_catalog, effects_catalog, amp_models, hothouse_community.

## Already converted to flashable bins
`SEED_READY/converted/` — 188 built this session (incl. plaits, sampler,
rings-reverb, ringmod, smoodge-delay, drums, drummachine + the Electrosmith
instruments). See `converted/INDEX.tsv`.
