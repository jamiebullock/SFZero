#ifndef SFZVoice_h
#define SFZVoice_h

#include "../JuceLibraryCode/JuceHeader.h"

class SFZRegion;


class JUCE_API SFZVoice : public SynthesiserVoice {
	public:
		SFZVoice();
		~SFZVoice();

    bool	canPlaySound(SynthesiserSound* sound);
    void	startNote(
			const int midiNoteNumber,
			const float velocity,
			SynthesiserSound* sound,
			const int currentPitchWheelPosition);
    void	stopNote(const bool allowTailOff);
    void	pitchWheelMoved(const int newValue);
    void	controllerMoved(
			const int controllerNumber,
			const int newValue);
    void	renderNextBlock(
			AudioSampleBuffer& outputBuffer, int startSample, int numSamples);

	protected:
		SFZRegion*	region;
		double    	pitchRatio;
		float     	gainLeft, gainRight;
		double    	sourceSamplePosition;
	};


#endif 	// SFZVoice_h

