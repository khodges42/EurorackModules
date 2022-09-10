# "Ugly Love Machine"

My attempt to make a Trent Reznor-y style synth. Square-Saw with an octave lower square + noise with a bandpass.

Code structure based on [DaisyExamples](https://github.com/electro-smith/DaisyExamples). Follow the instructions there to build this, you can drop this directory into /patch_sm

Designed for the ElectroSmith Patch Submodule. Majorly a work in progress.
## Controls

| Pin Name | Pin Location | Function | Comment |
| --- | --- | --- | --- |
| CV_1 | C5 | Coarse Tuning | Tunes the base frequency of all three oscillators |
| CV_2 | C2 | Noise Amount | Bring (or don't bring) the noise. |
| CV_3 | C3 | Pulse Width | On all three OSCs, one is thicker than the others. |
| CV_4 | C4 | Filter for Noise | Bandpass |
| CV_5 | C6 | 1V/Octave Input A | Musical pitch tracking input for Osc A|
| CV_6 | C7 | 1V/Octave Input B | Musical pitch tracking input for Osc B |
| CV_7 | C8 | 1V/Octave Input C | Musical pitch tracking input for Osc C|
| B8 | B8 | Toggle Switch | Switch between hard sync tracking and desync for indivdual pitch control |
| Output 2 | | This is a sub osc.

