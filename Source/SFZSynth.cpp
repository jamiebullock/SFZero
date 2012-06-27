#include "SFZSynth.h"
#include "SFZVoice.h"
#include "SFZSound.h"
#include "SFZDebug.h"


SFZSynth::SFZSynth()
	: Synthesiser()
{
}


void SFZSynth::noteOn(
	const int midiChannel, const int midiNoteNumber, const float velocity)
{
	int i;

	const ScopedLock locker(lock);

	int midiVelocity = (int) (velocity * 127);

	// First, stop any currently-playing sounds in the group.
	//*** Currently, this only pays attention to the first matching region.
	int group = 0;
	SFZSound* sound = dynamic_cast<SFZSound*>(getSound(0));
	if (sound) {
		SFZRegion* region = sound->getRegionFor(midiNoteNumber, midiVelocity);
		if (region)
			group = region->group;
		}
	if (group != 0) {
		for (i = voices.size(); --i >= 0;) {
			SFZVoice* voice = dynamic_cast<SFZVoice*>(voices.getUnchecked(i));
			if (voice == NULL)
				continue;
			if (voice->getOffBy() == group)
				voice->stopNoteForGroup();
			}
		}

	// Are any notes playing?  (Needed for first/legato trigger handling.)
	// Also stop any voices still playing this note.
	bool anyNotesPlaying = false;
	for (i = voices.size(); --i >= 0;) {
		SFZVoice* voice = dynamic_cast<SFZVoice*>(voices.getUnchecked(i));
		if (voice == NULL)
			continue;
		if (voice->isPlayingChannel(midiChannel)) {
			if (voice->isPlayingNoteDown()) {
				if (voice->getCurrentlyPlayingNote() == midiNoteNumber)
					voice->stopNoteQuick();
				else
					anyNotesPlaying = true;
				}
			}
		}

	// Play *all* matching regions.
	SFZRegion::Trigger trigger =
		(anyNotesPlaying ? SFZRegion::legato : SFZRegion::first);
	if (sound) {
		int numRegions = sound->getNumRegions();
		for (i = 0; i < numRegions; ++i) {
			SFZRegion* region = sound->regionAt(i);
			if (region->matches(midiNoteNumber, midiVelocity, trigger)) {
				SFZVoice* voice =
					dynamic_cast<SFZVoice*>(
						findFreeVoice(sound, isNoteStealingEnabled()));
				if (voice) {
					voice->setRegion(region);
					startVoice(voice, sound, midiChannel, midiNoteNumber, velocity);
					}
				}
			}
		}

	noteVelocities[midiNoteNumber] = midiVelocity;
}


void SFZSynth::noteOff(
	const int midiChannel, const int midiNoteNumber,
	const bool allowTailOff)
{
	const ScopedLock locker(lock);

	Synthesiser::noteOff(midiChannel, midiNoteNumber, allowTailOff);

	// Start release region.
	SFZSound* sound = dynamic_cast<SFZSound*>(getSound(0));
	if (sound) {
		SFZRegion* region =
			sound->getRegionFor(
				midiNoteNumber, noteVelocities[midiNoteNumber], SFZRegion::release);
		if (region) {
			SFZVoice* voice = dynamic_cast<SFZVoice*>(findFreeVoice(sound, false));
			if (voice) {
				// Synthesiser is too locked-down (ivars are private rt protected), so
				// we have to use a "setRegion()" mechanism.
				voice->setRegion(region);
				startVoice(
					voice, sound,
					midiChannel, midiNoteNumber,
					noteVelocities[midiNoteNumber] / 127.0);
				}
			}
		}
}


int SFZSynth::numVoicesUsed()
{
	int numUsed = 0;
	for (int i = voices.size(); --i >= 0;) {
		if (voices.getUnchecked(i)->getCurrentlyPlayingNote() >= 0)
			numUsed += 1;
		}
	return numUsed;
}



