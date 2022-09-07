# Triple Bitcrush Osc
 Three saws that can be hard synced or desynced and played individually. Each saw is individually bit crushed, and passed through hard/soft clipping distortion and overdrive.

Code structure based on [DaisyExamples](https://github.com/electro-smith/DaisyExamples). Follow the instructions there to build this, you can drop this directory into /patch_sm

Designed for the ElectroSmith Patch Submodule. Majorly a work in progress.
## Controls

| Pin Name | Pin Location | Function | Comment |
| --- | --- | --- | --- |
| CV_1 | C5 | Coarse Tuning | Tunes the base frequency of all three oscillators |
| CV_2 | C2 | Detune Amount | Adjusts the amount of detuning between all three voices, exponentially |
| CV_3 | C3 | Dry/Wet | Percentage of dry to wet output |
| CV_4 | C4 | Bitcrush | Crushes the bits into dust (exponentially!) |
| CV_5 | C6 | 1V/Octave Input A | Musical pitch tracking input for Osc A|
| CV_6 | C7 | 1V/Octave Input B | Musical pitch tracking input for Osc B |
| CV_7 | C8 | 1V/Octave Input C | Musical pitch tracking input for Osc C|
| CV_8 | C9 | Downsample Rate | Rate at which the downsampling is done|
| B8 | B8 | Toggle Switch | Switch between hard sync tracking and desync for indivdual pitch control |
| GATE_IN_1 | B10 | Gate | Kind of a crappy gate tbh |

