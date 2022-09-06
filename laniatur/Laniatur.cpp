#include "daisy_patch_sm.h"
#include "daisysp.h"

/** TODO: ADD CALIBRATION */

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

DaisyPatchSM patch;
Oscillator   osc_a, osc_b, osc_c;
Parameter  cutoff_ctrl, res_ctrl, drive_ctrl;
MoogLadder flt;
Switch toggle, button;
Decimator decimator_l;

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    patch.ProcessAllControls();
    /** Debounce the switch */
    toggle.Debounce();
    button.Debounce();

    /** Get the current toggle state */
    bool state = toggle.Pressed();

    bool env_state = patch.gate_in_1.State();

   // if(patch.gate_in_1.State())
      //  env_state = true;
  //  else
       // env_state = false;

    //get new control values
    float knob_cutoff = patch.GetAdcValue(CV_3);
    float cutoff = fmap(knob_cutoff, 20, 20000);
    flt.SetFreq(cutoff);

    decimator_l.SetBitcrushFactor(patch.GetAdcValue(CV_4));
    decimator_l.SetDownsampleFactor(patch.GetAdcValue(CV_8));

    /**float knob_drive = patch.GetAdcValue(CV_4);
    float drive = fmap(knob_drive, .3, 1);
    flt.SetDrive(drive)**/

    //float knob_res = patch.GetAdcValue(CV_4);
    //float res = fmap(knob_res, .3, 1);
    float res = 0.5f;
    flt.SetRes(res);

    /** Get Coarse, Fine, and V/OCT inputs from hardware 
     *  MIDI Note number are easy to use for defining ranges */
    float knob_coarse = patch.GetAdcValue(CV_1);
    float coarse_tune = fmap(knob_coarse, 12, 84);

    //float knob_fine = patch.GetAdcValue(CV_8);
    //float fine_tune = fmap(knob_fine, 0, 10);
    float fine_tune = 0.f;

    float cv_vocta = patch.GetAdcValue(CV_5);
    float vocta    = fmap(cv_vocta, 0, 60);

    float cv_voctb = patch.GetAdcValue(CV_6);
    float voctb    = fmap(cv_voctb, 0, 60);

    float cv_voctc = patch.GetAdcValue(CV_7);
    float voctc    = fmap(cv_voctc, 0, 60);

    /** Convert from MIDI note number to frequency */
    float midi_nna = fclamp(coarse_tune + fine_tune + vocta, 0.f, 127.f);
    float freq_a  = mtof(midi_nna);

    /** Calculate a detune amount */
    float detune_amt = patch.GetAdcValue(CV_2);
    float freq_b     = freq_a + (0.05 * freq_a * detune_amt);
    //if (voctb <= 2) { // if not patched, Using 2 here but maybe should be 0
    if (state){
        float midi_nnb = fclamp(coarse_tune + fine_tune + voctb, 0.f, 127.f);
        freq_b  = mtof(midi_nnb);
    }

    float freq_c     = freq_a - (0.05 * freq_a * detune_amt);
    //if (voctc <= 2) { // If not patched, Using 2 here but maybe should be 0
    if(state){
        /** Convert from MIDI note number to frequency */
        float midi_nnc = fclamp(coarse_tune + fine_tune + voctc, 0.f, 127.f);
        freq_c  = mtof(midi_nnc);
    }
    patch.SetLed(state);
    /** Set all three oscillators' frequencies */
    osc_a.SetFreq(freq_a);
    osc_b.SetFreq(freq_b);
    osc_c.SetFreq(freq_c);

    /** Process each sample of the oscillator and send to the hardware outputs */
    for(size_t i = 0; i < size; i++)
    {
        float sig = osc_a.Process() + osc_b.Process() + osc_c.Process();
        //flt.Process(sig);
        //OUT_L[i]  = flt.Process(sig);
        //if (env_state) { // gate
        if (patch.gate_in_1.State()){ // todo i hate this
            OUT_L[i] = decimator_l.Process(flt.Process(sig));
        } else {
            OUT_L[i] = sig * 0.f;
        }
        
    }
}

int main(void)
{
    float samplerate;
    patch.Init();
    toggle.Init(patch.B8);
    samplerate = patch.AudioSampleRate();

    //Initialize filter
    flt.Init(samplerate);

    /** Initialize the button input to pin B7 (Button on the MicroPatch Eval board) */
    button.Init(patch.B7, 1000);

    /** Initialize the ADSR */
    //envelope.Init(48000);

    decimator_l.Init();

    osc_a.Init(patch.AudioSampleRate());
    osc_b.Init(patch.AudioSampleRate());
    osc_c.Init(patch.AudioSampleRate());

    osc_a.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
    osc_b.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
    osc_c.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);

    patch.StartAudio(AudioCallback);
    while(1) {}
}
