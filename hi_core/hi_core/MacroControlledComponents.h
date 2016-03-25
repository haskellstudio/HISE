/*
  ==============================================================================

    MacroControlledComponents.h
    Created: 23 Sep 2014 11:36:38am
    Author:  Christoph

  ==============================================================================
*/


#ifndef MACROCONTROLLEDCOMPONENTS_H_INCLUDED
#define MACROCONTROLLEDCOMPONENTS_H_INCLUDED

class Processor;

/** A class that handles temposyncing.
*	@ingroup utility
*
*	All methods are static and it holds no data, so you have to get the host bpm before
*	you can use this class.
*
*	If the supplied hostTempo is invalid (= 0.0), a default tempo of 120.0 is used.
*/
class TempoSyncer
{
public:

	/** The note values. */
	enum Tempo
	{
		Whole = 0, ///< a whole note
		Half, ///< a half note
		Quarter, ///< a quarter note
		Eighth, ///< a eighth note
		Sixteenth, ///< a sixteenth note
		HalfTriplet, ///< a half triplet note
		QuarterTriplet, ///< a quarter triplet note
		EighthTriplet, ///< a eighth triplet note
		SixteenthTriplet, ///< a sixteenth triplet
		numTempos
	};

	/** Returns the sample amount for the specified tempo. */
	static int getTempoInSamples(double hostTempoBpm, double sampleRate, Tempo t)
	{
		if(hostTempoBpm == 0.0) hostTempoBpm = 120.0;

		const float seconds = (60.0f / (float)hostTempoBpm) * getTempoFactor(t);
		return (int)(seconds * (float)sampleRate);
	};

	/** Returns the time for the specified tempo in milliseconds. */
	static float getTempoInMilliSeconds(double hostTempoBpm, Tempo t)
	{
		if(hostTempoBpm == 0.0) hostTempoBpm = 120.0;
		
		const float seconds = (60.0f / (float)hostTempoBpm) * getTempoFactor(t);
		return seconds * 1000.0f;
	};

	/** Returns the tempo as frequency (in Hertz). */
	static float getTempoInHertz(double hostTempoBpm, Tempo t)
	{
		if(hostTempoBpm == 0.0) hostTempoBpm = 120.0;

		const float seconds = (60.0f / (float)hostTempoBpm) * getTempoFactor(t);

		return 1.0f / seconds;
	}

	/** Returns the name of the tempo with the index 't'. */
	static const String & getTempoName(int t)
	{
		jassert(t < numTempos);
		return tempoNames[t];
	};

	/** Returns the index of the tempo with the name 't'. */
	static Tempo getTempoIndex(const String &t) { return (Tempo)tempoNames.indexOf(t); };

	/** Fills the internal arrays. Call this on application start. */
	static void initTempoData()
	{
		tempoNames.add("1/1");		tempoFactors[Whole]			 = 4.0f;
		tempoNames.add("1/2");		tempoFactors[Half]			 = 2.0f;
		tempoNames.add("1/4");		tempoFactors[Quarter]		 = 1.0f;
		tempoNames.add("1/8");		tempoFactors[Eighth]		 = 0.5f;
		tempoNames.add("1/16");		tempoFactors[Sixteenth]		 = 0.25f;
		tempoNames.add("1/2T");		tempoFactors[HalfTriplet]	 = 4.0f / 3.0f;
		tempoNames.add("1/4T");		tempoFactors[QuarterTriplet] = 2.0f / 3.0f;
		tempoNames.add("1/8T");		tempoFactors[EighthTriplet]  = 1.0f / 3.0f;
		tempoNames.add("1/16T");	tempoFactors[SixteenthTriplet] = 0.5f / 3.0f;
	}

private:

	

	static float getTempoFactor(Tempo t) {return tempoFactors[(int)t];};

	static StringArray tempoNames;
	static float tempoFactors[numTempos];

};


/** A base class for all control widgets that can be controlled by a MacroControlBroadcaster.
*	@ingroup macroControl
*
*	Subclass your component from this class, and set the Processor's Attribute in its specified callback.
*	
*/
class MacroControlledObject
{
public:
	
	/** Creates a new MacroControlledObject. 
	*
	*	You have to call setup() before you use the object!
	*/
	MacroControlledObject():
        parameter(-1),
		processor(nullptr),
		macroIndex(-1),
		name(""),
		numberTag(new NumberTag(3, 14.0f, Colours::darkred)),
		macroControlledComponentEnabled(true)
	{};
    
    enum HiBackgroundColours
    {
        upperBgColour = 0xFF123532,
        lowerBgColour,
        outlineBgColour,
        numHiBackgroundColours
    };

    virtual ~MacroControlledObject() {};

	/** returns the name. */
	const String getName() const noexcept { return name; };

	/** Initializes the control widget.
	*
	*	It connects to a Processor's parameter and automatically updates its value and changes the attribute. 
	*	@param p the Processor that is controlled by the widget.
	*	@param parameter_ the parameter that is controlled by the widget
	*	@param name_ the name of the widget. This will also be the displayed name in the macro control panel.
	*/
	virtual void setup(Processor *p, int parameter_, const String &name_)
	{
		processor = p;
		parameter = parameter_;
		name = name_;
	}

	virtual void addToMacroController(int newMacroIndex)
	{ 
		numberTag->setNumber(newMacroIndex+1);
		macroIndex = newMacroIndex; 
	};

	virtual void removeFromMacroController() 
	{ 
		numberTag->setNumber(0);
		macroIndex = -1;	
	}

	/** overwrite this method and update the widget to display the current value of the controlled attribute. */
	virtual void updateValue() = 0;

	/** overwrite this method and return the range that the parameter can have. */
	virtual NormalisableRange<double> getRange() const = 0;

	bool isLocked();

	/** Since the original setEnabled() is overwritten in the updateValue, use this method instead to enable / disable MacroControlledComponents. */
	void enableMacroControlledComponent(bool shouldBeEnabled) noexcept
	{
		macroControlledComponentEnabled = shouldBeEnabled;
	}

	bool isReadOnly();

	int getMacroIndex() const;
	
	int getParameter() const { return parameter; };

protected:

	/** checks if the macro learn mode is active.
	*
	*	If it is active, it adds this control to the macro and returns true.
	*	You can use this in the widget callback like:
	*
	*		if(!checkLearnMode())
	*		{
	*			getProcessor()->setAttribute(parameter, value, dontSendNotification);
	*		}
	*
	*/
	bool checkLearnMode();
	
	Processor *getProcessor() {return processor.get(); };

	const Processor *getProcessor() const {return processor.get(); };

	int parameter;

protected:

	void removeParameterWithPopup();

	ScopedPointer<NumberTag> numberTag;

private:

	WeakReference<Processor> processor;
	
	int macroIndex;
	String name;

	bool macroControlledComponentEnabled;
	
};

/** A combobox which can be controlled by the macro system. */
class HiComboBox: public ComboBox,
				  public ComboBox::Listener,
				  public MacroControlledObject
{
public:

	HiComboBox(const String &name):
        ComboBox(name),
		MacroControlledObject()
	{
		addListener(this);

		setWantsKeyboardFocus(false);
        
        setColour(HiBackgroundColours::upperBgColour, Colour(0x66333333));
        setColour(HiBackgroundColours::lowerBgColour, Colour(0xfb111111));
        setColour(HiBackgroundColours::outlineBgColour, Colours::white.withAlpha(0.3f));
	};

	void setup(Processor *p, int parameter, const String &name) override;

	void updateValue() override;

	void comboBoxChanged(ComboBox *c) override;

	void paint(Graphics &g) override
	{
		ComboBox::paint(g);

		Image img(Image::PixelFormat::ARGB, getWidth(), getHeight(), true);

		numberTag->applyEffect(img, g, 1.0f, 1.0f);

		g.drawImageAt(img, 0, 0);

	}

	void mouseDown(const MouseEvent &e) override
	{
		if(e.mods.isLeftButtonDown())
		{
			checkLearnMode();
			ComboBox::mouseDown(e);
		}
		else
		{
			removeParameterWithPopup();
		}
	}

	NormalisableRange<double> getRange() const override { return NormalisableRange<double>(1.0, (double)getNumItems()); };
	
};

class HiToggleButton: public ToggleButton,
					  public Button::Listener,
				      public MacroControlledObject
{
public:

	HiToggleButton(const String &name):
		ToggleButton(name),
        MacroControlledObject(),
		notifyEditor(dontSendNotification)
	{
		addListener(this);
		setWantsKeyboardFocus(false);
        
        setColour(HiBackgroundColours::upperBgColour, Colour(0x66333333));
        setColour(HiBackgroundColours::lowerBgColour, Colour(0xfb111111));
        setColour(HiBackgroundColours::outlineBgColour, Colours::white.withAlpha(0.3f));
	};

	void setup(Processor *p, int parameter, const String &name) override;

	void updateValue() override;

	void buttonClicked(Button *b) override;

	void setNotificationType(NotificationType notify)
	{
		notifyEditor = notify;
	}

	void setLookAndFeelOwned(LookAndFeel *fslaf);

	void mouseDown(const MouseEvent &e) override
	{
		if(e.mods.isLeftButtonDown())
		{
			checkLearnMode();
			ToggleButton::mouseDown(e);
		}
		else
		{
			removeParameterWithPopup();
		}
	}

	void paint(Graphics &g) override
	{
		ToggleButton::paint(g);

		Image img(Image::PixelFormat::ARGB, getWidth(), getHeight(), true);

		numberTag->applyEffect(img, g, 1.0f, 1.0f);

		g.drawImageAt(img, 0, 0);

	}

	NormalisableRange<double> getRange() const override { return NormalisableRange<double>(0.0, 1.0); };
	
private:

	NotificationType notifyEditor;
	ScopedPointer<LookAndFeel> laf;
};




/** A custom Slider class that automatically sets up its properties according to the specified mode.
*
*	You can call setup() after you created the Slider and it will be skinned using the global HiLookAndFeel
*	and its range, skew factor and suffix are specified.
*/
class HiSlider: public juce::Slider,
				public MacroControlledObject,
				public SliderListener
{
public:

	/** The HiSlider can be configured using one of these modes. */
	enum Mode
	{
		Frequency = 0, ///< Range 20 - 20 000, Suffix "Hz", Midpoint: 1.5kHz
		Decibel, ///< Range -100 - 0, Suffix "dB", Midpoint -18dB
		Time, ///< Range 0 - 20 000, Suffix "ms", Midpoint 1000ms
		TempoSync, ///< Range 0 - numTempos, @see TempoSyncer
		Linear, ///< Range min - max
		Discrete, ///< Range min - max, Stepsize integer
		Pan, ///< 100L - 100R, Stepsize integer
		NormalizedPercentage, ///< 0.0 - 1.0, Displayed as percentage
		numModes
	};

	static NormalisableRange<double> getRangeForMode(HiSlider::Mode m)
	{
		switch(m)
		{
		case Frequency:		return NormalisableRange<double>(20.0, 20000.0, 1);
							
		case Decibel:		return NormalisableRange<double>(-100.0, 0.0, 0.1);
							
		case Time:			return NormalisableRange<double>(0.0, 20000.0, 1);
							
		case TempoSync:		return NormalisableRange<double>(0, TempoSyncer::numTempos, 1);
							
		case Pan:			return NormalisableRange<double>(-100.0, 100.0, 1);
							
		case NormalizedPercentage:	return NormalisableRange<double>(0.0, 1.0, 0.01);									
									
		case Linear:		
		case Discrete:		return NormalisableRange<double>(0.0, 1.0, 0.01);
        case numModes: 
		default:			jassertfalse; return NormalisableRange<double>();
		}

	};

	/** Creates a Slider. The name will be displayed. 
	*
	*	The slider must be initialized using setup().
	*/
	HiSlider(const String &name):
		Slider(name),
   		MacroControlledObject(),
        mode(numModes),
		displayValue(1.0f),
		useModulatedRing(false)
	{
        FloatVectorOperations::clear(modeValues, numModes);
		addListener(this);
		setWantsKeyboardFocus(false);
        
        setColour(HiBackgroundColours::upperBgColour, Colour(0x66333333));
        setColour(HiBackgroundColours::lowerBgColour, Colour(0xfb111111));
        setColour(HiBackgroundColours::outlineBgColour, Colours::white.withAlpha(0.3f));
	};

	
	void mouseDown(const MouseEvent &e) override;

	void paint(Graphics &g) override
	{
		Slider::paint(g);

		Image img(Image::PixelFormat::ARGB, getWidth(), getHeight(), true);

		numberTag->applyEffect(img, g, 1.0f, 1.0f);

		g.drawImageAt(img, 0, 0);

	}
	

	/** sets the mode. */
	void setMode(Mode m, double min=-1.0, double max=-1.0, double mid=0.5) 
	{ 
		if(mode != m)
		{
			mode = m; 
			setModeRange(min, max, mid);
			setTextValueSuffix(getModeSuffix());

			setValue(modeValues[m], sendNotification);

			repaint();
		}
	};

	/* initialises the slider. You must call this after creation before you use this component! */
	void setup(Processor *p, int parameter, const String &name) override;

	void sliderValueChanged(Slider *s) override;

	/** If the slider represents a modulated attribute (eg. LFO Frequency), this can be used to set the displayed value. 
	*
	*	In order to use this functionality, add a timer callback to your editor and update the value using the ModulatorChain's getOutputValue().
	*/
	void setDisplayValue(float newDisplayValue)
	{
        if(newDisplayValue != displayValue)
        {
            displayValue = newDisplayValue;
            repaint();
        }
	}

	bool isUsingModulatedRing() const noexcept{ return useModulatedRing; };

	void setIsUsingModulatedRing(bool shouldUseModulatedRing) { useModulatedRing = shouldUseModulatedRing; };

	float getDisplayValue() const
	{
		return useModulatedRing ? displayValue : 1.0f;
	}

	void updateValue() override;

	/** Overrides the slider method to display the tempo names for the TempoSync mode. */
	String getTextFromValue(double value) override
	{
		if(mode == Pan) setTextValueSuffix(getModeSuffix());

		if(mode == TempoSync) return TempoSyncer::getTempoName((int)(value));
		else if(mode == NormalizedPercentage) return String((int)(value * 100)) + "%";
		else				  return Slider::getTextFromValue(value);
	};

	/** Overrides the slider method to set the value from the Tempo names */
	double getValueFromText(const String &text) override
	{
		if(mode == TempoSync) return TempoSyncer::getTempoIndex(text);
		else if (mode == NormalizedPercentage) return text.getDoubleValue() / 100.0;
		else				  return Slider::getValueFromText(text);
	};

	void setLookAndFeelOwned(LookAndFeel *fslaf);

	NormalisableRange<double> getRange() const override { return normRange; };

private:

	String getModeSuffix()
	{
		jassert(mode != numModes);

		switch (mode)
		{
		case Frequency:		return " Hz";
		case Decibel:		return " dB";
		case Time:			return " ms";
		case Pan:			return modeValues[Pan] > 0.0 ? "R" : "L";
		case TempoSync:		return String::empty;
		case Linear:		return String::empty;
		case Discrete:		return String::empty;
		case NormalizedPercentage:	return "%";
		default:			return String::empty;
		}
	};

	void setModeRange(double min, double max, double mid)
	{
		jassert(mode != numModes);

		if(min != -1.0 && max != -1.0)
		{
			setRange(min, max, 0.1);
			normRange = NormalisableRange<double>(min, max);


			if(mode == Discrete)
			{
				
				setRange(min, max, 1);
                setSkewFactor(1.0);
				return;
			}
			else if (mid != 0.5)
			{
				setSkewFactorFromMidPoint(mid);
				return;
			}

			
		}

		normRange = getRangeForMode(mode);

		switch(mode)
		{
		case Frequency:		setRange(20.0, 20000.0, 1);
							setSkewFactorFromMidPoint(1500.0);
							break;
		case Decibel:		setRange(-100.0, 0.0, 0.1);
							setSkewFactorFromMidPoint(-18.0);
							break;
		case Time:			setRange(0.0, 20000.0, 1);
							setSkewFactorFromMidPoint(1000.0);
							break;
		case TempoSync:		setRange(0, TempoSyncer::numTempos-1, 1);
							setSkewFactor(1.0);
							break;
		case Pan:			setRange(-100.0, 100.0, 1.0);
							setSkewFactor(1.0);
							break;
		case NormalizedPercentage:	setRange(0.0, 1.0, 0.01);
									setSkewFactor(1.0);
									break;
		case Linear:		break;
		case Discrete:		jassertfalse; break;
        case numModes:  jassertfalse; break;
		}
	};
	
	Mode mode;

	bool useModulatedRing;

	double modeValues[numModes];

	float displayValue;

	NormalisableRange<double> normRange;
	ScopedPointer<LookAndFeel> laf;
	//ScopedPointer<Component> stupidComponent;

};




#endif  // MACROCONTROLLEDCOMPONENTS_H_INCLUDED
