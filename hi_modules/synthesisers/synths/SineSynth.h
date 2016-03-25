/*
  ==============================================================================

    SineSynth.h
    Created: 10 Jul 2014 12:40:14pm
    Author:  Chrisboy

  ==============================================================================
*/

#ifndef SINESYNTH_H_INCLUDED
#define SINESYNTH_H_INCLUDED


class SineSynth;

class SineWaveSound : public ModulatorSynthSound
{
public:
    SineWaveSound() {}

    bool appliesToNote (int /*midiNoteNumber*/) override   { return true; }
    bool appliesToChannel (int /*midiChannel*/) override   { return true; }
	bool appliesToVelocity (int /*midiChannel*/) override  { return true; }
};

class SineSynthVoice: public ModulatorSynthVoice
{
public:

	SineSynthVoice(ModulatorSynth *ownerSynth):
		ModulatorSynthVoice(ownerSynth),
		octaveTransposeFactor(1.0)
	{
		for(int i = 0; i < 2048; i++)
		{
			sinTable[i] = sinf(i * float_Pi / 1024.0f);
		}
	};

	bool canPlaySound(SynthesiserSound *) override
	{
		return true;
	};

	void startNote (int midiNoteNumber, float /*velocity*/, SynthesiserSound* , int /*currentPitchWheelPosition*/) override
	{
		ModulatorSynthVoice::startNote(midiNoteNumber, 0.0f, nullptr, -1);

        voiceUptime = 0.0;
        
        const double cyclesPerSecond = MidiMessage::getMidiNoteInHertz (midiNoteNumber);
		const double cyclesPerSample = cyclesPerSecond / getSampleRate();

		uptimeDelta = cyclesPerSample * 2048.0 * octaveTransposeFactor;
    }

	void calculateBlock(int startSample, int numSamples) override;;

	

	void setOctaveTransposeFactor(double newFactor)
	{
		octaveTransposeFactor = newFactor;
	}

private:

	float sinTable[2048];

	double octaveTransposeFactor;


};

class SineSynth: public ModulatorSynth
{
public:

	SET_PROCESSOR_NAME("SineSynth", "Sine Wave Generator");

	enum SpecialParameters
	{
		OctaveTranspose = ModulatorSynth::numModulatorSynthParameters,
		SemiTones,
		UseFreqRatio,
		CoarseFreqRatio,
		FineFreqRatio,
		SaturationAmount,
		numSineSynthParameters
	};

	SineSynth(MainController *mc, const String &id, int numVoices):
		ModulatorSynth(mc, id, numVoices),
		octaveTranspose((int)getDefaultValue(OctaveTranspose)),
		semiTones((int)getDefaultValue(SemiTones)),
		useRatio(false),
		fineRatio(getDefaultValue(FineFreqRatio)),
		coarseRatio(getDefaultValue(CoarseFreqRatio)),
		saturationAmount(getDefaultValue(SaturationAmount))
	{
		parameterNames.add("OctaveTranspose");
		parameterNames.add("SemiTones");
		parameterNames.add("UseFreqRatio");
		parameterNames.add("CoarseFreqRatio");
		parameterNames.add("FineFreqRatio");
		parameterNames.add("SaturationAmount");

		for(int i = 0; i < numVoices; i++) addVoice(new SineSynthVoice(this));
		addSound (new SineWaveSound());	
	};

	void restoreFromValueTree(const ValueTree &v) override
	{
		ModulatorSynth::restoreFromValueTree(v);

		loadAttribute(OctaveTranspose, "OctaveTranspose");
		loadAttribute(SemiTones, "SemiTones");
		loadAttribute(UseFreqRatio, "UseFreqRatio");
		loadAttribute(CoarseFreqRatio, "CoarseFreqRatio");
		loadAttribute(FineFreqRatio, "FineFreqRatio");
		loadAttribute(SaturationAmount, "SaturationAmount");

	};

	ValueTree exportAsValueTree() const override
	{
		ValueTree v = ModulatorSynth::exportAsValueTree();

		saveAttribute(OctaveTranspose, "OctaveTranspose");
		saveAttribute(SemiTones, "SemiTones");
		saveAttribute(UseFreqRatio, "UseFreqRatio");
		saveAttribute(CoarseFreqRatio, "CoarseFreqRatio");
		saveAttribute(FineFreqRatio, "FineFreqRatio");
		saveAttribute(SaturationAmount, "SaturationAmount");

		return v;
	}

	float getDefaultValue(int parameterIndex) const override
	{
		if (parameterIndex < ModulatorSynth::numModulatorSynthParameters) return ModulatorSynth::getDefaultValue(parameterIndex);

		switch (parameterIndex)
		{
		case OctaveTranspose:		return 0.0f;
		case SemiTones:				return 0.0f;
		case UseFreqRatio:			return 0.0f;
		case CoarseFreqRatio:		return 1.0f;
		case FineFreqRatio:			return 0.0f;
		case SaturationAmount:		return 0.0f;
		default:					jassertfalse; return -1.0f;
		}
	};

	float getAttribute(int parameterIndex) const override 
	{
		if(parameterIndex < ModulatorSynth::numModulatorSynthParameters) return ModulatorSynth::getAttribute(parameterIndex);

		switch(parameterIndex)
		{
		case OctaveTranspose:		return (float)octaveTranspose;
		case SemiTones:				return (float)semiTones;
		case UseFreqRatio:			return (float)useRatio ? 1.0f : 0.0f;
		case CoarseFreqRatio:		return coarseRatio;
		case FineFreqRatio:			return fineRatio;
		case SaturationAmount:		return saturationAmount;
		default:					jassertfalse; return -1.0f;
		}
	};

	void setInternalAttribute(int parameterIndex, float newValue) override
	{
		if(parameterIndex < ModulatorSynth::numModulatorSynthParameters)
		{
			ModulatorSynth::setInternalAttribute(parameterIndex, newValue);
			return;
		}

		switch(parameterIndex)
		{
		case OctaveTranspose:		octaveTranspose = (int)newValue; break;
									
		case SemiTones:				semiTones = (int)newValue; break;
		case UseFreqRatio:			useRatio = newValue > 0.5f; break;
		case CoarseFreqRatio:		coarseRatio = newValue; break;
		case FineFreqRatio:			fineRatio = newValue; break;
		case SaturationAmount:		saturationAmount = newValue; 
									saturator.setSaturationAmount(newValue); 
									return; // skip the calculation of the pitch ratio
		default:					jassertfalse;
									break;
		}

		double factor;

		if (useRatio)
		{
			factor = coarseRatio + fineRatio;
		}
		else
		{
			factor = pow(2.0, (double)octaveTranspose + (double)semiTones / 12.0);
		}
	
		for (int i = 0; i < getNumVoices(); i++)
		{
			static_cast<SineSynthVoice*>(getVoice(i))->setOctaveTransposeFactor(factor);
		}
	};

	ProcessorEditorBody* createEditor(BetterProcessorEditor *parentEditor) override;

	float const * getSaturatedTableValues();

private:

	Saturator saturator;
	
	friend class SineSynthVoice;

	float saturatedTableValues[128];

	int octaveTranspose;
	int semiTones;

	float saturationAmount;
	float coarseRatio;
	float fineRatio;
	bool useRatio;
};




#endif  // SINESYNTH_H_INCLUDED
