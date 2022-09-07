#include "daisy_patch_sm.h"
#include "daisysp.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

DaisyPatchSM patch;
Oscillator   osc_a, osc_b, osc_c;
Switch toggle;
Decimator decimator_l;

float hardClip(float in)
{
    in = in > 1.f ? 1.f : in;
    in = in < -1.f ? -1.f : in;
    return in;
}

float softClip(float in)
{
    if(in > 0)
        return 1 - expf(-in);
    return -1 + expf(in);
}

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    patch.ProcessAllControls();

    // Distortion variables
    //float Pregain = 0.5f * 10 + 1.2; //to be cv?
    
    float knob_drywet  = patch.GetAdcValue(CV_3);
    float drywet = fmap(knob_drywet, 0, 1); 

    // Handle the hardsync switch
    toggle.Debounce();
    bool state = toggle.Pressed();

    // Setup Bitcrush
    float knob_crush = patch.GetAdcValue(CV_4);
    decimator_l.SetBitcrushFactor(fmap(knob_crush, 0, 1));
    float knob_downsample = patch.GetAdcValue(CV_8);
    decimator_l.SetDownsampleFactor(fmap(knob_downsample, 0, 1));
    //SetBitsToCrush

    // Oscillator Pitches
    //float knob_coarse = patch.GetAdcValue(CV_1);
    //float coarse_tune = fmap(knob_coarse, 12, 84);

    float fine_tune = 0.f;

    float cv_vocta = patch.GetAdcValue(CV_5);
    float vocta    = fmap(cv_vocta, 0, 60);

    float cv_voctb = patch.GetAdcValue(CV_6);
    float voctb    = fmap(cv_voctb, 0, 60);

    float cv_voctc = patch.GetAdcValue(CV_7);
    float voctc    = fmap(cv_voctc, 0, 60);

    /** Convert from MIDI note number to frequency */
    //float midi_nna = fclamp(coarse_tune + fine_tune + vocta, 0.f, 127.f);
    float midi_nna = fclamp(fine_tune + vocta, 0.f, 127.f);
    float freq_a  = mtof(midi_nna);

    /** Calculate a detune amount */
    float knob_detune_amt = patch.GetAdcValue(CV_2);
    float detune_amt = fmap(knob_detune_amt, 0, 5, Mapping::EXP);

    // Set frequences for B and C either via hardsync or voct
    float freq_b     = freq_a + (0.05 * freq_a * detune_amt);

    if (state){
        float midi_nnb = fclamp(fine_tune + voctb, 0.f, 127.f);
        freq_b  = mtof(midi_nnb);
    }

    float freq_c     = freq_a - (0.05 * freq_a * detune_amt);

    if(state){
        float midi_nnc = fclamp(fine_tune + voctc, 0.f, 127.f);
        freq_c  = mtof(midi_nnc);
    }

    osc_a.SetFreq(freq_a);
    osc_b.SetFreq(freq_b);
    osc_c.SetFreq(freq_c);

    /** Process each sample of the oscillator and send to the hardware outputs */
    for(size_t i = 0; i < size; i++)
    {
        float sig = osc_a.Process() + osc_b.Process() + osc_c.Process();

        // idk if this is going to work lol
        float w_sig = decimator_l.Process(osc_a.Process()) + decimator_l.Process(osc_b.Process()) + decimator_l.Process(osc_c.Process());

        float wet = w_sig;
        //wet = decimator_l.Process(wet);
        wet *= 101.2f; // this is maybe not usable
        wet = softClip(wet);
        wet = hardClip(wet);
        


        OUT_L[i] = wet * drywet + sig * (1 - drywet);
        //OUT_R[i]

    }
}

int main(void)
{
    patch.Init();
    toggle.Init(patch.B8);
    //float samplerate = patch.AudioSampleRate();

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
