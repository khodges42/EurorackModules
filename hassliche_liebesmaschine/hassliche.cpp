#include "daisy_patch_sm.h"
#include "daisysp.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

DaisyPatchSM patch;
Oscillator   osc_a, osc_b, osc_c, osc_sub;
WhiteNoise noise;
Switch toggle;
Svf svf;



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

    float knob_drywet  = patch.GetAdcValue(CV_2);
    float drywet = fmap(knob_drywet, 0, 1); 

    // Handle the hardsync switch
    toggle.Debounce();
    bool state = toggle.Pressed();

    // Filters
    float knob_cutoff = patch.GetAdcValue(CV_4); //20 to 20000, Parameter::LOGARITHMIC
    float cutoff = fmap(knob_cutoff, 20, 20000, Mapping::LOG);

    svf.SetFreq(cutoff);
    svf.SetDrive(0.7f); // todo find a value for this
    //todo, do we want to invert freq for highpass? Does it do that already?





    // Oscillator Pitches
    float knob_coarse = patch.GetAdcValue(CV_1);
    float coarse_tune = fmap(knob_coarse, 12, 84);

    float fine_tune = 0.f;

    float cv_vocta = patch.GetAdcValue(CV_5);
    float vocta    = fmap(cv_vocta, 0, 60);

    float cv_voctb = patch.GetAdcValue(CV_6);
    float voctb    = fmap(cv_voctb, 0, 60);

    float cv_voctc = patch.GetAdcValue(CV_7);
    float voctc    = fmap(cv_voctc, 0, 60);

    /** Convert from MIDI note number to frequency */
    //float midi_nna = fclamp(coarse_tune + fine_tune + vocta, 0.f, 127.f);
    float midi_nna = fclamp(coarse_tune + fine_tune + vocta, 0.f, 127.f);
    float freq_a  = mtof(midi_nna);

    // Set frequences for B and C either via hardsync or voct
    float freq_b     = (freq_a * 0.5);

    if (state){
        float midi_nnb = fclamp(coarse_tune + fine_tune + voctb, 0.f, 127.f);
        freq_b  = mtof(midi_nnb);
    }

    float freq_c     = freq_b;

    if(state){
        float midi_nnc = fclamp(coarse_tune + fine_tune + voctc, 0.f, 127.f);
        freq_c  = mtof(midi_nnc);
    }

    float freq_sub = freq_a * 0.5; //Always follows A, maybe add more if room?

    float knob_pw  = patch.GetAdcValue(CV_3);
    float pw = fmap(knob_pw, 0, 1); 
    osc_a.SetFreq(freq_a);
    osc_a.SetPw(pw+0.3f);
    osc_b.SetFreq(freq_b);
    osc_b.SetPw(pw);
    osc_c.SetFreq(freq_c);
    osc_c.SetPw(pw);
    osc_sub.SetFreq(freq_sub);

    /** Process each sample of the oscillator and send to the hardware outputs */
    for(size_t i = 0; i < size; i++)
    {
        float sig = osc_a.Process() + osc_b.Process() + osc_c.Process();
        float sig_sub = osc_sub.Process();

        // idk if this is going to work lol
        float wet = sig;
        //wet *= 101.2f; // this is maybe not usable
        //wet = softClip(wet);
        //wet = hardClip(wet);

        float noise_raw = noise.Process();
        svf.Process(noise_raw);

        //send the different filter types to the different outputs
        float noise_sig = svf.Band();
        


        OUT_L[i] = (wet + noise_sig) * drywet + sig * (1 - drywet);
        OUT_R[i] = sig_sub;

    }
}

int main(void)
{
    patch.Init();
    noise.Init();
    toggle.Init(patch.B8);
    float samplerate = patch.AudioSampleRate();
    svf.Init(samplerate);


    osc_a.Init(patch.AudioSampleRate());
    osc_b.Init(patch.AudioSampleRate());
    osc_c.Init(patch.AudioSampleRate());

    osc_a.SetWaveform(Oscillator::WAVE_POLYBLEP_SQUARE);
    osc_b.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
    osc_c.SetWaveform(Oscillator::WAVE_POLYBLEP_SQUARE);

    patch.StartAudio(AudioCallback);
    while(1) {}
}
