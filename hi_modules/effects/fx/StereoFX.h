/*
  ==============================================================================

    StereoFX.h
    Created: 3 Aug 2014 1:25:37pm
    Author:  Chrisboy

  ==============================================================================
*/

#ifndef STEREOFX_H_INCLUDED
#define STEREOFX_H_INCLUDED

class MidSideDecoder
{
public:
    
    MidSideDecoder():
    width(1.0f)
    {};
    
    void calculateStereoValues(float &left, float &right)
    {
        const float m = (left + right) * 0.5f;
        const float s = (right - left) * width * 0.5f;
        
        left  = m - s;
        right = m + s;
    }
    
    void setWidth(float newValue) noexcept { width = newValue; }
    
    float getWidth() const noexcept { return width; }
    
private:
    
    float width;
};

/** A simple stereo panner which can be modulated using all types of Modulators
*	@ingroup effectTypes
*
*/
class StereoEffect: public VoiceEffectProcessor
{
public:

	SET_PROCESSOR_NAME("StereoFX", "Stereo FX");

	enum InternalChains
	{
		BalanceChain = 0,
		numInternalChains
	};

	/** The parameters */
	enum Parameters
	{
		Pan = 0, ///< the balance of the effect. If you want to modulate this parameter, set it to the biggest value and add Modulators to the Chain.
		Width ///< the stereo width using a M/S matrix
	};

	enum EditorStates
	{
		PanChainShown = Processor::numEditorStates,
		numEditorStates
	};

	StereoEffect(MainController *mc, const String &uid, int numVoices): 
		VoiceEffectProcessor(mc, uid, numVoices),
		balanceChain(new ModulatorChain(mc, "Pan Modulation", numVoices, Modulation::GainMode, this)),
		pan(0.5f)
	{
		parameterNames.add("Pan");
		parameterNames.add("Width");

		editorStateIdentifiers.add("PanChainShown");

	};

	float getAttribute(int parameterIndex) const override
	{
		switch ( parameterIndex )
		{
		case Pan:							return pan * 200.0f - 100.0f;
		case Width:							return msDecoder.getWidth() * 100.0f;
		default:							jassertfalse; return 1.0f;
		}
	};

	int getNumInternalChains() const override { return numInternalChains; };

	void setInternalAttribute(int parameterIndex, float newValue) override 
	{
		switch ( parameterIndex )
		{
		case Pan:							pan = (newValue + 100.0f) / 200.0f; ; break;
		case Width:							msDecoder.setWidth(newValue / 100.0f); break;
		default:							jassertfalse; return;
		}
	};

	

	void restoreFromValueTree(const ValueTree &v) override
	{
		EffectProcessor::restoreFromValueTree(v);

		loadAttribute(Pan, "Pan");
		loadAttribute(Width, "Width");
	};

	ValueTree exportAsValueTree() const override
	{
		ValueTree v = EffectProcessor::exportAsValueTree();

		saveAttribute(Pan, "Pan");
		saveAttribute(Width, "Width");

		return v;
	}

	

	virtual void renderNextBlock(AudioSampleBuffer &buffer, int startSample, int numSamples)
	{
		float *l = buffer.getWritePointer(0, 0);
		float *r = buffer.getWritePointer(1, 0);

		while(--numSamples >= 0)
		{
            msDecoder.calculateStereoValues(l[startSample], r[startSample++]);
            
			//const float m = (l[startSample]  + r[startSample])*0.5f;
			//const float s = (r[startSample] - l[startSample]) * width * 0.5f;

			//l[startSample]  = m - s;
			//r[startSample++] = m + s;
		}
	};

	bool hasTail() const override { return false; };

	Processor *getChildProcessor(int /*processorIndex*/) override { return balanceChain; };

	const Processor *getChildProcessor(int /*processorIndex*/) const override { return balanceChain; };

	int getNumChildProcessors() const override { return numInternalChains; };

	ProcessorEditorBody *createEditor(BetterProcessorEditor *parentEditor)  override;

	AudioSampleBuffer &getBufferForChain(int /*index*/) override
	{
		return panBuffer;
	}

	void preVoiceRendering(int voiceIndex, int startSample, int numSamples)
	{
		calculateChain(BalanceChain, voiceIndex, startSample, numSamples);
	}

	void applyEffect(int voiceIndex, AudioSampleBuffer &b, int startSample, int numSamples) override
	{
		const float panModValue = 2.0f * getCurrentModulationValue(BalanceChain, voiceIndex, startSample) - 1.0f;

		const float panL = 0.5f + ((pan-0.5f) * panModValue);
		const float panR = 1.0f - panL;

		FloatVectorOperations::multiply(b.getWritePointer(0, startSample), panR, numSamples);
		FloatVectorOperations::multiply(b.getWritePointer(1, startSample), panL, numSamples);
	}

private:

    MidSideDecoder msDecoder;
    
	float pan;
	float width;
	
	ScopedPointer<ModulatorChain> balanceChain;

	AudioSampleBuffer panBuffer;
};







#endif  // STEREOFX_H_INCLUDED
