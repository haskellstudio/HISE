/*  ===========================================================================
*
*   This file is part of HISE.
*   Copyright 2016 Christoph Hart
*
*   HISE is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   HISE is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*
*   Commercial licences for using HISE in an closed source project are
*   available on request. Please visit the project's website to get more
*   information about commercial licencing:
*
*   http://www.hartinstruments.net/hise/
*
*   HISE is based on the JUCE library,
*   which also must be licenced for commercial applications:
*
*   http://www.juce.com
*
*   ===========================================================================
*/

#ifndef HI_LOOKANDFEELS_H_INCLUDED
#define HI_LOOKANDFEELS_H_INCLUDED


/** A class that provides the image data from the binary resources. It selects the correct image size based on the dpi scaling of the display to support retina displays. */
class ImageProvider
{
public:

	enum ImageType
	{
		KnobEmpty = 0,
		KnobUnmodulated,
		KnobModulated,
		MacroKnob,
		BalanceKnob,
		ToggleButton
	};

	enum DisplayScaleFactor
	{
		OneHundred = 0,
		OneHundredTwentyFive,
		OneHundredFifty,
		TwoHundred
	};

	static DisplayScaleFactor getScaleFactor();

	static Image getImage(ImageType type);
};


class SafeChangeBroadcaster;

/** A class for message communication between objects.
*
*	This class has the same functionality as the JUCE ChangeListener class, but it uses a weak reference for the internal list, 
*	so deleted listeners will not crash the application. 
*/
class SafeChangeListener
{
public:

	virtual ~SafeChangeListener()
	{
		masterReference.clear();

		
	}

	/** Overwrite this and handle the message. */
	virtual void changeListenerCallback(SafeChangeBroadcaster *b) = 0;

private:

	friend class WeakReference<SafeChangeListener>;

	WeakReference<SafeChangeListener>::Master masterReference;
};



/** A drop in replacement for the ChangeBroadcaster class from JUCE but with weak references.
*
*	If you use the normal class and forget to unregister a listener in its destructor, it will crash the application.
*	This class uses a weak reference (but still throws an assertion so you still recognize if something is funky), so it handles this case much more gracefully.
*
*	Also you can add a string to your message for debugging purposes (with the JUCE class you have no way of knowing what caused the message if you call it asynchronously.
*/
class SafeChangeBroadcaster
{
public:

	SafeChangeBroadcaster():
		dispatcher(this)
	{};

	virtual ~SafeChangeBroadcaster()
	{
		dispatcher.cancelPendingUpdate();
	};

	/** Sends a synchronous change message to all the registered listeners.
	*
	*	This will immediately call all the listeners that are registered. For thread-safety reasons, you must only call this method on the main message thread.
	*/
	void sendSynchronousChangeMessage(const String &identifier = String::empty)
	{
		currentString = identifier;

		// Ooops, only call this in the message thread.
		// Use sendChangeMessage() if you need to send a message from elsewhere.
		jassert(MessageManager::getInstance()->isThisTheMessageThread());

		ScopedLock sl(listeners.getLock());

		for(int i = 0; i < listeners.size(); i++)
		{
			if(listeners[i].get() != nullptr)
			{
				listeners[i]->changeListenerCallback(this);
			}
			else
			{
				// Ooops, you called an deleted listener. 
				// Normally, it would crash now, but since this is really lame, this class only throws an assert!
				jassertfalse;

				listeners.remove(i--);
			}
		}
	};

	/** Registers a listener to receive change callbacks from this broadcaster.
	*
	*	Trying to add a listener that's already on the list will have no effect.
	*/
	void addChangeListener(SafeChangeListener *listener)
	{
		ScopedLock sl(listeners.getLock());

		listeners.addIfNotAlreadyThere(listener);
	}

	/**	Unregisters a listener from the list.
	*
	*	If the listener isn't on the list, this won't have any effect.
	*/
	void removeChangeListener(SafeChangeListener *listener)
	{
		ScopedLock sl(listeners.getLock());

		listeners.removeAllInstancesOf(listener);
	}

	/** Removes all listeners from the list. */
	void removeAllChangeListeners()
	{
		dispatcher.cancelPendingUpdate();

		ScopedLock sl(listeners.getLock());

		listeners.clear();
	}

	/** Causes an asynchronous change message to be sent to all the registered listeners.
	*
	*	The message will be delivered asynchronously by the main message thread, so this method will return immediately. 
	*	To call the listeners synchronously use sendSynchronousChangeMessage().
	*/
	void sendChangeMessage(const String &identifier=String::empty)
	{
		currentString = identifier;

		dispatcher.triggerAsyncUpdate();
	};

private:

	class AsyncBroadcaster: public AsyncUpdater
	{
	public:
		AsyncBroadcaster(SafeChangeBroadcaster *parent_):
			parent(parent_)
		{}

		void handleAsyncUpdate() override
		{
			parent->sendSynchronousChangeMessage(parent->currentString);
		}

		SafeChangeBroadcaster *parent;

	};

	AsyncBroadcaster dispatcher;

	String currentString;

	Array<WeakReference<SafeChangeListener>, CriticalSection> listeners;
};



struct HiToolbarIcons
{
	static Path createSettingsPath()
	{
		static const unsigned char pathData[] = { 110,109,35,219,53,65,18,131,180,65,108,19,131,74,65,86,14,166,65,108,84,227,51,65,119,190,154,65,108,219,249,22,65,111,18,165,65,98,178,157,17,65,117,147,163,65,59,223,11,65,66,96,162,65,10,215,5,65,31,133,161,65,108,0,0,0,65,0,0,144,65,108,0,0,192,64,
		0,0,144,65,108,236,81,180,64,31,133,161,65,98,138,65,168,64,66,96,162,65,156,196,156,64,105,145,163,65,74,12,146,64,111,18,165,65,108,176,114,48,64,119,190,154,65,108,108,231,171,63,86,14,166,65,108,116,147,40,64,18,131,180,65,98,165,155,28,64,38,49,
		183,65,12,2,19,64,98,16,186,65,87,57,12,64,122,20,189,65,108,0,8,131,58,255,255,191,65,108,0,8,131,58,255,255,207,65,108,87,57,12,64,132,235,210,65,98,110,18,19,64,157,239,213,65,165,155,28,64,216,206,216,65,214,163,40,64,236,124,219,65,108,47,8,172,
		63,168,241,233,65,108,18,131,48,64,135,65,245,65,108,123,20,146,64,143,237,234,65,98,205,204,156,64,137,108,236,65,186,73,168,64,188,159,237,65,29,90,180,64,223,122,238,65,108,50,8,192,64,254,255,255,65,108,25,4,0,65,254,255,255,65,108,35,219,5,65,223,
		122,238,65,98,84,227,11,65,188,159,237,65,203,161,17,65,149,110,236,65,244,253,22,65,143,237,234,65,108,109,231,51,65,135,65,245,65,108,44,135,74,65,168,241,233,65,108,60,223,53,65,236,124,219,65,98,48,221,56,65,216,206,216,65,150,67,59,65,156,239,213,
		65,220,249,60,65,132,235,210,65,108,26,4,96,65,255,255,207,65,108,26,4,96,65,255,255,191,65,108,220,249,60,65,122,20,189,65,98,150,67,59,65,97,16,186,65,73,225,56,65,38,49,183,65,60,223,53,65,18,131,180,65,99,109,0,0,224,64,0,0,216,65,98,215,163,188,
		64,0,0,216,65,0,0,160,64,10,215,208,65,0,0,160,64,0,0,200,65,98,0,0,160,64,246,40,191,65,215,163,188,64,0,0,184,65,0,0,224,64,0,0,184,65,98,20,174,1,65,0,0,184,65,0,0,16,65,246,40,191,65,0,0,16,65,0,0,200,65,98,0,0,16,65,10,215,208,65,20,174,1,65,0,0,
		216,65,0,0,224,64,0,0,216,65,99,109,0,0,0,66,0,0,64,65,108,0,0,0,66,0,0,32,65,108,234,38,239,65,59,223,25,65,98,11,215,238,65,34,219,21,65,177,114,238,65,84,227,17,65,207,247,237,65,231,251,13,65,108,41,92,252,65,71,225,246,64,108,113,61,246,65,118,190,
		187,64,108,6,129,229,65,63,53,202,64,98,176,114,228,65,198,75,195,64,235,81,227,65,116,147,188,64,196,32,226,65,122,20,182,64,108,71,225,235,65,186,116,123,64,108,104,145,224,65,192,245,32,64,108,225,122,210,65,216,249,110,64,98,35,219,208,65,161,112,
		101,64,14,45,207,65,124,106,92,64,176,114,205,65,204,247,83,64,108,98,16,209,65,240,40,156,63,108,174,71,194,65,172,245,232,62,108,12,2,185,65,134,65,16,64,98,86,14,183,65,222,122,12,64,110,18,181,65,171,71,9,64,98,16,179,65,177,200,6,64,108,0,0,176,
		65,0,0,64,181,108,0,0,160,65,0,0,64,181,108,158,239,156,65,177,200,6,64,98,146,237,154,65,171,71,9,64,170,241,152,65,124,106,12,64,244,253,150,65,134,65,16,64,108,82,184,141,65,168,245,232,62,108,60,223,125,65,239,40,156,63,108,80,141,130,65,204,247,
		83,64,98,242,210,128,65,124,106,92,64,187,73,126,65,161,112,101,64,62,10,123,65,216,249,110,64,108,48,221,94,65,192,245,32,64,108,114,61,72,65,186,116,123,64,108,120,190,91,65,122,20,182,64,98,42,92,89,65,116,147,188,64,161,26,87,65,198,75,195,64,245,
		253,84,65,63,53,202,64,108,32,133,51,65,118,190,187,64,108,175,71,39,65,71,225,246,64,108,99,16,68,65,231,251,13,65,98,185,30,67,65,84,227,17,65,236,81,66,65,34,219,21,65,46,178,65,65,59,223,25,65,108,1,0,32,65,0,0,32,65,108,1,0,32,65,0,0,64,65,108,46,
		178,65,65,197,32,70,65,98,236,81,66,65,222,36,74,65,161,26,67,65,172,28,78,65,99,16,68,65,25,4,82,65,108,175,71,39,65,93,143,100,65,108,32,133,51,65,99,16,129,65,108,245,253,84,65,98,229,122,65,98,161,26,87,65,30,90,126,65,42,92,89,65,36,219,128,65,120,
		190,91,65,226,122,130,65,108,114,61,72,65,105,145,144,65,108,48,221,94,65,72,225,155,65,108,62,10,123,65,197,32,146,65,98,187,73,126,65,236,81,147,65,242,210,128,65,177,114,148,65,80,141,130,65,7,129,149,65,108,60,223,125,65,114,61,166,65,108,82,184,
		141,65,42,92,172,65,108,244,253,150,65,208,247,157,65,98,170,241,152,65,165,112,158,65,146,237,154,65,11,215,158,65,158,239,156,65,235,38,159,65,108,0,0,160,65,2,0,176,65,108,0,0,176,65,2,0,176,65,108,98,16,179,65,236,38,159,65,98,110,18,181,65,13,215,
		158,65,86,14,183,65,179,114,158,65,12,2,185,65,209,247,157,65,108,174,71,194,65,43,92,172,65,108,98,16,209,65,115,61,166,65,108,176,114,205,65,8,129,149,65,98,14,45,207,65,178,114,148,65,35,219,208,65,237,81,147,65,225,122,210,65,198,32,146,65,108,104,
		145,224,65,73,225,155,65,108,71,225,235,65,106,145,144,65,108,196,32,226,65,227,122,130,65,98,235,81,227,65,37,219,128,65,176,114,228,65,32,90,126,65,6,129,229,65,100,229,122,65,108,112,61,246,65,100,16,129,65,108,40,92,252,65,96,143,100,65,108,206,247,
		237,65,28,4,82,65,98,163,112,238,65,175,28,78,65,9,215,238,65,225,36,74,65,233,38,239,65,200,32,70,65,108,0,0,0,66,3,0,64,65,99,109,0,0,168,65,154,153,117,65,98,180,200,148,65,154,153,117,65,51,51,133,65,152,110,86,65,51,51,133,65,0,0,48,65,98,51,51,
		133,65,104,145,9,65,180,200,148,65,205,204,212,64,0,0,168,65,205,204,212,64,98,76,55,187,65,205,204,212,64,205,204,202,65,104,145,9,65,205,204,202,65,0,0,48,65,98,205,204,202,65,152,110,86,65,76,55,187,65,154,153,117,65,0,0,168,65,154,153,117,65,99,101,
		0,0 };

		Path path;
		path.loadPathFromData (pathData, sizeof (pathData));
		
		return path;
		
	};
};



class NumberTag: public juce::ImageEffectFilter
{
public:

	NumberTag(float offset_=0.f, float size_=13.0f, Colour c_=Colours::black):
		number(0),
		offset(offset_),
		size(size_),
		c(c_)
	{};

	void applyEffect (Image &image, Graphics &g, float /*scaleFactor*/, float /*alpha*/) override
	{
		const int w = image.getBounds().getWidth();


		g.drawImageAt(image, 0, 0);

		if(number == 0) return;

		Rectangle<float> rect = Rectangle<float>((float)w-offset-size, offset, size, size);

		DropShadow d;

		d.colour = c.withAlpha(0.3f);
		d.radius = (int)offset*2;
		d.offset = Point<int>();

		d.drawForRectangle(g, Rectangle<int>((int)rect.getX(), (int)rect.getY(), (int)rect.getWidth(), (int)rect.getHeight()));

		g.setColour(Colours::black.withAlpha(0.3f));

		g.setColour(Colours::white.withAlpha(0.7f));

		g.drawRoundedRectangle(rect.reduced(1.0f), 4.0f, 1.0f);

#if JUCE_WINDOWS
        
		g.setFont(GLOBAL_BOLD_FONT().withHeight(12.0f));
        
#else
        g.setFont(GLOBAL_BOLD_FONT());
#endif

		g.drawText(String(number), rect, Justification::centred, false);


	};
 	
	void setNumber(int newNumber)
	{
		number = newNumber;
	}

private:

	
	int number;

	float offset;
	float size;
    Colour c;
};

class PopupLookAndFeel : public LookAndFeel_V3
{
public:
	PopupLookAndFeel()
	{
		setColour(PopupMenu::highlightedBackgroundColourId, Colour(0xff680000));

		Colour dark(0xFF333333);

		Colour bright(0xFF999999);

		setColour(PopupMenu::ColourIds::backgroundColourId, dark);
		setColour(PopupMenu::ColourIds::textColourId, bright);
		setColour(PopupMenu::ColourIds::highlightedBackgroundColourId, bright);
		setColour(PopupMenu::ColourIds::highlightedTextColourId, dark);
		setColour(PopupMenu::ColourIds::headerTextColourId, bright);

	};

private:

	Font getPopupMenuFont() override
	{
#if JUCE_WINDOWS
		return GLOBAL_FONT().withHeight(15.0f);
#else
		return GLOBAL_FONT();
#endif
	};

	void drawMenuBarBackground(Graphics& g, int width, int height,
		bool, MenuBarComponent& menuBar);

	void drawMenuBarItem(Graphics& g, int width, int height,
		int /*itemIndex*/, const String& itemText,
		bool isMouseOverItem, bool isMenuOpen,
		bool /*isMouseOverBar*/, MenuBarComponent& menuBar)
	{
		if (!menuBar.isEnabled())
		{
			g.setColour(menuBar.findColour(PopupMenu::textColourId)
				.withMultipliedAlpha(0.5f));
		}
		else if (isMenuOpen || isMouseOverItem)
		{
			Colour dark(0xFF444444);

			Colour bright(0xFF999999);

			g.fillAll(dark);
			g.setColour(bright);
		}
		else
		{
			g.setColour(Colour(0xFF222222));
		}

		g.setFont(GLOBAL_BOLD_FONT());
		
		g.drawFittedText(itemText, 0, 0, width, height, Justification::centred, 1);

		
	}

	void drawPopupMenuBackground(Graphics& g, int width, int height) override
	{
		g.setColour((findColour(PopupMenu::backgroundColourId)));
		g.fillRect(0.0f, 0.0f, (float)width, (float)height);
		(void)width; (void)height;

#if ! JUCE_MAC
		g.setColour(findColour(PopupMenu::textColourId));
		//g.drawRoundedRectangle(0.0f, 0.0f, (float)width, (float)height, 4.0f, 0.5f);
#endif


	}
};


class TableHeaderLookAndFeel: public PopupLookAndFeel
{
	void drawTableHeaderBackground (Graphics &, TableHeaderComponent &)
	{
		
		
	}
 
	void drawTableHeaderColumn (Graphics &g, const String &columnName, int /*columnId*/, int width, int height, bool /*isMouseOver*/, bool /*isMouseDown*/, int /*columnFlags*/)
	{
		g.setColour(Colour(0xff474747));

		g.fillRect(0.0f, 0.0f, (float)width-1.0f, (float)height);

        
		g.setFont(GLOBAL_FONT());
        g.setColour(Colour(0xa2ffffff));

		g.drawText(columnName, 3, 0, width - 3, height, Justification::centredLeft, true);
	}
};


/* A collection of all LookAndFeels used by the HI engine. */

/* A VU style for ModulatorSynth headers. */
class VUSliderLookAndFeel: public LookAndFeel_V3
{
	void drawLinearSlider(Graphics &g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle style, Slider &slider);
};

class FileBrowserLookAndFeel : public LookAndFeel_V3
{
	void drawFileBrowserRow(Graphics&, int width, int height,
		const String& filename, Image* icon,
		const String& fileSizeDescription, const String& fileTimeDescription,
		bool isDirectory, bool isItemSelected, int itemIndex,
		DirectoryContentsDisplayComponent&) override;
};

class ConcertinaPanelHeaderLookAndFeel : public LookAndFeel_V3
{
public:

	void drawConcertinaPanelHeader(Graphics& g, const Rectangle<int>& area,
		bool isMouseOver, bool /*isMouseDown*/,
		ConcertinaPanel&, Component& panel) override;

};

class HiPropertyPanelLookAndFeel: public LookAndFeel_V3
{
public:

	HiPropertyPanelLookAndFeel()
	{
		setColour(PopupMenu::highlightedBackgroundColourId, Colour(0xff680000));

		Colour dark(0xFF333333);

		Colour bright(0xFF999999);

		setColour(PopupMenu::ColourIds::backgroundColourId, dark);
		setColour(PopupMenu::ColourIds::textColourId, bright);
		setColour(PopupMenu::ColourIds::highlightedBackgroundColourId, bright);
		setColour(PopupMenu::ColourIds::highlightedTextColourId, dark);
		setColour(PopupMenu::ColourIds::headerTextColourId, bright);

	};

	void drawPropertyPanelSectionHeader (Graphics& g, const String& name, bool isOpen, int width, int height) override
	{
		g.setColour(Colour(0xbbC1C1C1));

		g.fillRoundedRectangle(1.0f, 0.0f, (float)width-2.0f, (float)height, 1.0f);

		const float buttonSize = height * 0.75f;
		const float buttonIndent = (height - buttonSize) * 0.5f;

		drawTreeviewPlusMinusBox (g, Rectangle<float> (buttonIndent, buttonIndent, buttonSize, buttonSize), Colours::white, isOpen, false);

		const int textX = (int) (buttonIndent * 2.0f + buttonSize + 2.0f);

		g.setColour (Colours::black);
		g.setFont (GLOBAL_BOLD_FONT());
		g.drawText (name, textX, 0, width - textX - 4, height, Justification::centredLeft, true);
	}

    void drawPropertyComponentBackground (Graphics& g, int /*width*/, int /*height*/, PropertyComponent& /*component*/) override
	{
        g.setColour (Colours::grey);
        g.fillAll();
		
	}

    void drawPropertyComponentLabel (Graphics& g, int /*width*/, int /*height*/, PropertyComponent& component) override
	{
		 g.setColour (component.findColour (PropertyComponent::labelTextColourId)
                    .withMultipliedAlpha (component.isEnabled() ? 1.0f : 0.6f));

		 g.setFont (GLOBAL_MONOSPACE_FONT());

		const Rectangle<int> r (getPropertyComponentContentPosition (component));

		g.drawFittedText (component.getName(),
						  3, r.getY(), r.getX() - 5, r.getHeight(),
						  Justification::centredLeft, 2);
	};

	void drawLinearSlider (Graphics &g, int /*x*/, int /*y*/, int width, 
						   int height, float /*sliderPos*/, float /*minSliderPos*/, 
						   float /*maxSliderPos*/, const Slider::SliderStyle, Slider &s) override
	{
		const bool isBiPolar = s.getMinimum() < 0.0 && s.getMaximum() > 0.0;

		float leftX;
		float actualWidth;

		const float max = (float)s.getMaximum();
		const float min = (float)s.getMinimum();

		g.fillAll(Colour (0xff606060));

		if(isBiPolar)
		{
			const float value = ((float)s.getValue() - min) / (max - min);

			leftX = 2.0f + (value < 0.5f ? value * (float)(width-2) : 0.5f * (float)(width-2));
			actualWidth = fabs(0.5f - value) * (float)(width-2);
		}
		else
		{
			const double value = s.getValue();
			const double normalizedValue = (value - s.getMinimum()) / (s.getMaximum() - s.getMinimum());
			const double proportion = pow(normalizedValue, s.getSkewFactor());

			//const float value = ((float)s.getValue() - min) / (max - min);

			leftX = 2;
			actualWidth = (float)proportion * (float)(width-2);
		}

		g.setGradientFill (ColourGradient (Colour (0xffDDDDDD).withAlpha(s.isEnabled() ? 0.8f : 0.4f),
                                       0.0f, 0.0f,
                                       Colour (0xffAAAAAA).withAlpha(s.isEnabled() ? 0.8f : 0.4f),
                                       0.0f, (float)height,
                                       false));
		g.fillRect(leftX, 2.0f, actualWidth , (float)(height-2));

	}

	Font getLabelFont(Label &) override
	{
		return GLOBAL_MONOSPACE_FONT();
	}

	Font getComboBoxFont (ComboBox&) override
	{
		return GLOBAL_FONT();
	}

	Font getPopupMenuFont () override
	{
		return GLOBAL_FONT();
	};

	

	void drawPopupMenuBackground (Graphics& g, int width, int height) override
	{
		g.setColour( (findColour (PopupMenu::backgroundColourId)));
		g.fillRect(0.0f, 0.0f, (float)width, (float)height);
	}

	Font getTextButtonFont(TextButton &, int) override
	{
		return GLOBAL_FONT();
	};


   
};


/* A Bipolar Slider. */
class BiPolarSliderLookAndFeel: public LookAndFeel_V3
{
	void drawLinearSlider (Graphics &g, int /*x*/, int /*y*/, int width, 
						   int height, float /*sliderPos*/, float /*minSliderPos*/, 
						   float /*maxSliderPos*/, const Slider::SliderStyle style, Slider &s) override
	{

		if (style == Slider::SliderStyle::LinearBarVertical)
		{
			const bool isBiPolar = s.getMinimum() < 0.0 && s.getMaximum() > 0.0;

			float leftY;
			float actualHeight;

			const float max = (float)s.getMaximum();
			const float min = (float)s.getMinimum();

			g.fillAll(s.findColour(Slider::ColourIds::backgroundColourId));

			if (isBiPolar)
			{
				const float value = (-1.0f * (float)s.getValue() - min) / (max - min);

				leftY = (value < 0.5f ? value * (float)(height) : 0.5f * (float)(height));
				actualHeight = fabs(0.5f - value) * (float)(height);
			}
			else
			{
				const double value = s.getValue();
				const double normalizedValue = (value - s.getMinimum()) / (s.getMaximum() - s.getMinimum());
				const double proportion = pow(normalizedValue, s.getSkewFactor());

				//const float value = ((float)s.getValue() - min) / (max - min);

				actualHeight = (float)proportion * (float)(height);
				leftY = (float)height - actualHeight ;
			}

			Colour c = s.findColour(Slider::ColourIds::thumbColourId);

			g.setGradientFill(ColourGradient(c.withMultipliedAlpha(s.isEnabled() ? 1.0f : 0.4f),
				0.0f, 0.0f,
				c.withMultipliedAlpha(s.isEnabled() ? 1.0f : 0.3f).withMultipliedBrightness(0.9f),
				0.0f, (float)height,
				false));

			g.fillRect(0.0f, leftY, (float)(width+1), actualHeight+1.0f);

			if (width > 4)
			{
				//g.setColour(Colours::white.withAlpha(0.3f));
				g.setColour(s.findColour(Slider::trackColourId));
				g.drawRect(0.0f, leftY, (float)(width + 1), actualHeight + 1.0f, 1.0f);
			}

			
		}
		else
		{
			const bool isBiPolar = s.getMinimum() < 0.0 && s.getMaximum() > 0.0;

			float leftX;
			float actualWidth;

			const float max = (float)s.getMaximum();
			const float min = (float)s.getMinimum();

			g.fillAll(Colour(0xfb333333));

			if (isBiPolar)
			{
				const float value = ((float)s.getValue() - min) / (max - min);

				leftX = 2.0f + (value < 0.5f ? value * (float)(width - 2) : 0.5f * (float)(width - 2));
				actualWidth = fabs(0.5f - value) * (float)(width - 2);
			}
			else
			{
				const double value = s.getValue();
				const double normalizedValue = (value - s.getMinimum()) / (s.getMaximum() - s.getMinimum());
				const double proportion = pow(normalizedValue, s.getSkewFactor());

				//const float value = ((float)s.getValue() - min) / (max - min);

				leftX = 2;
				actualWidth = (float)proportion * (float)(width - 2);
			}

			g.setGradientFill(ColourGradient(Colour(0xff888888).withAlpha(s.isEnabled() ? 0.8f : 0.4f),
				0.0f, 0.0f,
				Colour(0xff666666).withAlpha(s.isEnabled() ? 0.8f : 0.4f),
				0.0f, (float)height,
				false));
			g.fillRect(leftX, 2.0f, actualWidth, (float)(height - 2));
		}

		

	}

	void drawLinearSliderBackground(Graphics&, int /*x*/, int /*y*/, int /*width*/, int /*height*/,
		float /*sliderPos*/, float /*minSliderPos*/, float /*maxSliderPos*/,
		const Slider::SliderStyle, Slider&) override {};

	Font getLabelFont(Label &) override
	{
		return GLOBAL_FONT();
	}
};



class AutocompleteLookAndFeel: public LookAndFeel_V3
{
public:
	
private:

	Font getPopupMenuFont () override
	{
		return GLOBAL_MONOSPACE_FONT();
	};

	void drawPopupMenuBackground (Graphics& g, int /*width*/, int /*height*/) override
	{
		g.setColour(Colours::grey);
        g.fillAll();
	}

};


class NoiseBackgroundDrawer
{
public:
    
    
private:
    
    static NoiseBackgroundDrawer instance;
    
};


class ProcessorEditorLookAndFeel
{
public:

    ProcessorEditorLookAndFeel();
    
	static void drawBackground(Graphics &g, int width, int height, Colour bgColour, bool folded, int intendationLevel = 0);
    
    static void drawNoiseBackground(Graphics &g, Rectangle<int> area, Colour c=Colours::lightgrey);
    
private:
    
    Image img;
};


//	===================================================================================================
/* This class is used to style a ProcessorEditorHeader. */
class ProcessorEditorHeaderLookAndFeel
{
public:

	enum ColourIds
	{
		HeaderBackgroundColour = 0

	};

	ProcessorEditorHeaderLookAndFeel():
		isChain(false)
	{};
    
    virtual ~ProcessorEditorHeaderLookAndFeel() {};
    

	virtual Colour getColour(int ColourId) const = 0;

	void drawBackground(Graphics &g, float width, float height, bool /*isFolded*/)
	{
		g.excludeClipRegion(Rectangle<int>(0, 35, (int)width, 10));

		height = 30.0f;

		g.setGradientFill (ColourGradient (getColour(HeaderBackgroundColour),
										288.0f, 8.0f,
										getColour(HeaderBackgroundColour).withMultipliedBrightness(0.9f),
										288.0f, 30.0f,
										false));
		g.fillRoundedRectangle (0.0f, 0.0f, width, 33.0f, 3.0f);

	}

	bool isChain;

};

/* A dark background with a normal slider. */
class ModulatorEditorHeaderLookAndFeel: public ProcessorEditorHeaderLookAndFeel
{
public:

	

    virtual ~ModulatorEditorHeaderLookAndFeel() {};
    
	Colour getColour(int /*ColourId*/) const override { return isChain ? Colour(0xFF3A3A3A) : Colour(0xFF222222); };

	

};

/* A bright background and a VU meter slider. */
class ModulatorSynthEditorHeaderLookAndFeel: public ProcessorEditorHeaderLookAndFeel
{
public:
    
    virtual ~ModulatorSynthEditorHeaderLookAndFeel() {};

	Colour getColour(int /*ColourId*/) const override { return Colour(0xFFEEEEEE); };
};



class AlertWindowLookAndFeel: public LookAndFeel_V3
{
public:

	AlertWindowLookAndFeel()
	{
		dark = Colour(0xFF252525);
		bright = Colour(0xFFAAAAAA);

		setColour(PopupMenu::ColourIds::backgroundColourId, dark);
		setColour(PopupMenu::ColourIds::textColourId, bright);
		setColour(PopupMenu::ColourIds::highlightedBackgroundColourId, bright);
		setColour(PopupMenu::ColourIds::highlightedTextColourId, dark);
		setColour(PopupMenu::ColourIds::headerTextColourId, bright);
	}

	Font getAlertWindowMessageFont () override
	{
		return GLOBAL_FONT();
	}

	Font getTextButtonFont (TextButton &, int /*buttonHeight*/) override
	{
		return getAlertWindowFont();
	}

	Font getComboBoxFont(ComboBox&) override
	{
		return GLOBAL_FONT();
	}

	Font getPopupMenuFont() override
	{
		return GLOBAL_FONT();
	};

	void drawPopupMenuBackground(Graphics& g, int width, int height) override
	{
		g.setColour((findColour(PopupMenu::backgroundColourId)));
		g.fillRect(0.0f, 0.0f, (float)width, (float)height);
	}
 
	void drawButtonText (Graphics &g, TextButton &button, bool /*isMouseOverButton*/, bool /*isButtonDown*/) override
	{
		Font font (getTextButtonFont (button, button.getHeight()));
		g.setFont (font);
		g.setColour (dark.withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));

		const int yIndent = jmin (4, button.proportionOfHeight (0.3f));
		const int cornerSize = jmin (button.getHeight(), button.getWidth()) / 2;

		const int fontHeight = roundToInt (font.getHeight() * 0.6f);
		const int leftIndent  = jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
		const int rightIndent = jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));

		g.drawFittedText (button.getButtonText(),
						  leftIndent,
						  yIndent,
						  button.getWidth() - leftIndent - rightIndent,
						  button.getHeight() - yIndent * 2,
						  Justification::centred, 2);
	}
 	

	void drawButtonBackground (Graphics& g, Button& button, const Colour& /*backgroundColour*/,
                                           bool isMouseOverButton, bool isButtonDown) override
	{
		Colour baseColour (bright.withMultipliedSaturation (button.hasKeyboardFocus (true) ? 1.3f : 0.9f)
										   .withMultipliedAlpha (button.isEnabled() ? 0.9f : 0.5f));

		if (isButtonDown || isMouseOverButton)
			baseColour = baseColour.contrasting (isButtonDown ? 0.2f : 0.1f);

		g.setColour(baseColour);

		const float width  = (float)button.getWidth();
		const float height = (float)button.getHeight();

		g.fillRoundedRectangle(0.f, 0.f, width, height, 3.0f);

	}
 
	Font getAlertWindowFont () override
	{
		return GLOBAL_FONT();
	};

	void setColourIdsForAlertWindow(AlertWindow &window)
	{
		window.setColour(AlertWindow::ColourIds::backgroundColourId, dark);
		window.setColour(AlertWindow::ColourIds::textColourId, bright);
		window.setColour(AlertWindow::ColourIds::outlineColourId, bright);
	}

	void drawProgressBar (Graphics &g, ProgressBar &/*progressBar*/, int width, int height, double progress, const String &textToShow) override
	{
		const Colour background = bright;
		const Colour foreground = dark;

		g.fillAll (dark);

		if (progress >= 0.0f && progress < 1.0f)
		{
			ColourGradient grad(bright, 0.0f, 0.0f, bright.withAlpha(0.6f), 0.0f, (float)height, false);

			g.setColour(bright);

			g.drawRect(0, 0, width, height);

			int x = 2;
			int y = 2;
			int w = (int)((width - 4) * progress);
			int h = height - 4;

			g.setGradientFill(grad);

			g.fillRect(x,y,w,h);
		}
		else
		{
			
			
		}

		if (textToShow.isNotEmpty())
		{
			g.setColour (Colour::contrasting (background, foreground));
			g.setFont (GLOBAL_FONT());

			g.drawText (textToShow, 0, 0, width, height, Justification::centred, false);
		}

	}

	void drawAlertBox (Graphics &g, AlertWindow &alert, const Rectangle< int > &textArea, juce::TextLayout &textLayout) override
	{
		ColourGradient grad(dark.withMultipliedBrightness(1.4f), 0.0f, 0.0f,
						dark, 0.0f, (float)alert.getHeight(), false);

		g.setGradientFill(grad);
		g.fillAll ();

		g.setColour (bright);

		for(int i = 0; i < textLayout.getNumLines(); i++)
		{
			textLayout.getLine(i).runs.getUnchecked(0)->colour = bright;
		}

		textLayout.draw (g, Rectangle<int> (textArea.getX(),
											textArea.getY(),
											textArea.getWidth(),
											textArea.getHeight()).toFloat());

		g.setColour (bright);
		g.drawRect (0, 0, alert.getWidth(), alert.getHeight());


	};

private:

	Colour dark, bright, special;

};



class KnobLookAndFeel: public LookAndFeel_V3
{
public:

	KnobLookAndFeel();

	Label *createSliderTextBox (Slider &s) override
	{


		Label *label = new Label("Textbox");
		label->setFont (GLOBAL_FONT());
		label->setEditable (false, false, false);
		label->setColour (Label::textColourId, Colour (0x66ffffff));
		

		if(s.getSliderStyle() == Slider::RotaryHorizontalVerticalDrag)
		{
			
			label->setJustificationType (Justification::centredBottom);
			label->setEditable (false, false, false);
			label->setColour (Label::textColourId, Colour (0x66ffffff));
			
		}
		else
		{
			
			label->setJustificationType (Justification::centred);
			
			label->setColour (Label::textColourId, s.findColour(Slider::textBoxTextColourId));
			


		}

		return label;
	}

	Font getPopupMenuFont () override
	{
		return GLOBAL_FONT();
	};

	void drawPopupMenuBackground (Graphics& g, int width, int height) override
	{
		g.setColour( (findColour (PopupMenu::backgroundColourId)));
		g.fillRect(0.0f, 0.0f, (float)width, (float)height);
		(void) width; (void) height;

	   #if ! JUCE_MAC
		g.setColour (findColour (PopupMenu::textColourId));
		//g.drawRoundedRectangle(0.0f, 0.0f, (float)width, (float)height, 4.0f, 0.5f);
	   #endif
	}


    static void drawHiBackground(Graphics &g, int x, int y, int width, int height, Component *c=nullptr, bool isMouseOverButton=false);
	

	void 	drawComboBox (Graphics &g, int width, int height, bool isButtonDown, int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/, ComboBox &c) override;
 
	Font 	getComboBoxFont (ComboBox &) override
	{
		

		return GLOBAL_FONT();
	}

	void 	positionComboBoxText (ComboBox &c, Label &labelToPosition) override
	{
		labelToPosition.setBounds(5, 5, c.getWidth() - 25, c.getHeight() - 10);

		labelToPosition.setFont (getComboBoxFont (c));


	}

	static void fillPathHiStyle(Graphics &g, Path &p, int width, int height, bool drawBorders=true)
	{
		

		if(drawBorders)
		{
			g.setColour(Colours::lightgrey.withAlpha(0.8f));
			g.strokePath (p, PathStrokeType(1.0f));

			g.setColour(Colours::lightgrey.withAlpha(0.1f));
			g.drawRect(0, 0, width, height, 1);
		}
	
		g.setGradientFill (ColourGradient (Colour (0x88ffffff),
										0.0f, 0.0f,
										Colour (0x11ffffff),
										0.0f, (float) height,
										false));

		g.fillPath(p);

		DropShadow d(Colours::white.withAlpha(drawBorders ? 0.2f : 0.1f), 5, Point<int>());

		d.drawForPath(g, p);

	};

	int getSliderThumbRadius(Slider& ) override { return 0; }

	void drawLinearSlider (Graphics &g, int /*x*/, int /*y*/, int width, int height, float /*sliderPos*/, float minSliderPos, float maxSliderPos, const Slider::SliderStyle style, Slider &s) override
	{
		if (style == Slider::TwoValueHorizontal)
		{
			g.fillAll(s.findColour(Slider::backgroundColourId));

			//width += 18;
			
			const float leftBoxPos = minSliderPos;// *(float)(width - 6.0f) + 3.0f;
			const float rightBoxPos = maxSliderPos;// *(float)(width - 6.0f) + 3.0f;

			Rectangle<float> area(leftBoxPos, 0.0f, rightBoxPos - leftBoxPos, (float)height);

			area.reduce(-1.0f, -1.0f);

			
			g.setColour(s.findColour(Slider::ColourIds::thumbColourId));
			g.fillRect(area);

			g.setColour(s.findColour(Slider::ColourIds::trackColourId));
			g.drawRect(0, 0, width, height, 1);

			g.drawLine(leftBoxPos, 0.0f, leftBoxPos, (float)height, 1.0f);
			g.drawLine(rightBoxPos, 0.0f, rightBoxPos, (float)height, 1.0f);

			g.setColour(s.findColour(Slider::ColourIds::textBoxTextColourId));

			const int decimal = jmax<int>(0, (int)-log10(s.getInterval()));

			const String text = String(s.getMinValue(), decimal) + " - " + String(s.getMaxValue(), decimal);
			g.setFont(GLOBAL_FONT());
			g.drawText(text, 0, 0, width, height, Justification::centred, false);
		}
		else
		{
			height = s.getHeight();
			width = s.getWidth();

			if (!s.isEnabled()) g.setOpacity(0.4f);

			//drawHiBackground(g, 12, 6, width-18, 32);

			const double value = s.getValue();
			const double normalizedValue = (value - s.getMinimum()) / (s.getMaximum() - s.getMinimum());
			const double proportion = pow(normalizedValue, s.getSkewFactor());

			g.fillAll(s.findColour(Slider::backgroundColourId));

			g.setColour(s.findColour(Slider::ColourIds::thumbColourId));

			if (style == Slider::SliderStyle::LinearBar)
			{
				g.fillRect(0.0f, 0.0f, (float)proportion * (float)width, (float)height);
			}
			else if (style == Slider::SliderStyle::LinearBarVertical)
			{
				g.fillRect(0.0f, (float)(1.0 - proportion)*height, (float)width, (float)(proportion * height));
			}

			g.setColour(s.findColour(Slider::ColourIds::trackColourId));
			g.drawRect(0, 0, width, height, 1);
		}

		

	}


	void drawRotarySlider (Graphics &g, int /*x*/, int /*y*/, int width, int height, float /*sliderPosProportional*/, float /*rotaryStartAngle*/, float /*rotaryEndAngle*/, Slider &s) override;
	

	void drawToggleButton (Graphics &g, ToggleButton &b, bool isMouseOverButton, bool /*isButtonDown*/) override;
	
	static const char* smalliKnob_png;
    static const int smalliKnob_pngSize;

	static const char* knobRing_png;
	static const int knobRing_size;

	static const char* toggle_png;
    static const int toggle_pngSize;
	static const char* slider_strip2_png;
    static const int slider_strip2_pngSize;
	static const char* slider2_bipolar_png;
    static const int slider2_bipolar_pngSize;
    
    

private:
    
    Image cachedImage_smalliKnob_png;
	Image cachedImage_knobRing_png;
	Image cachedImage_toggle_png;
	Image cachedImage_slider_strip2_png;
	Image cachedImage_slider2_bipolar_png;
	
	Image ring_red;
	Image ring_green;
	Image ring_yellow;
	Image ring_blue;
	Image ring_modulated;

};



class FilmstripLookAndFeel : public KnobLookAndFeel
{
public:

	FilmstripLookAndFeel() :
		imageToUse(Image()),
		isVertical(true),
		numStrips(0)
	{};

	void setFilmstripImage(const Image &imageToUse_, int numStrips_, bool isVertical_=true)
	{
		imageToUse = imageToUse_;
		numStrips = numStrips_;
		isVertical = isVertical_;

		if (numStrips == 0) return;

		if (isVertical)
		{
			heightOfEachStrip = imageToUse.getHeight() / numStrips;
			widthOfEachStrip = imageToUse.getWidth();
		}
		else
		{
			heightOfEachStrip = imageToUse.getHeight();
			widthOfEachStrip = imageToUse.getWidth() / numStrips;
		}

	};

	void drawToggleButton(Graphics &g, ToggleButton &b, bool isMouseOverButton, bool isButtonDown) override;

	void drawRotarySlider(Graphics &g, int /*x*/, int /*y*/, int width, int height, float /*sliderPosProportional*/, float /*rotaryStartAngle*/, float /*rotaryEndAngle*/, Slider &s) override;

private:

	int heightOfEachStrip;
	int widthOfEachStrip;

	
	bool isVertical;
	Image imageToUse;
    int numStrips;

};


class BalanceButtonLookAndFeel: public LookAndFeel_V3
{

public:

	BalanceButtonLookAndFeel()
	{
		cachedImage_balanceKnob_png = ImageCache::getFromMemory (balanceKnob_png, balanceKnob_pngSize);
	}

	void drawRotarySlider (Graphics &g, int /*x*/, int /*y*/, int /*width*/, int /*height*/, float /*sliderPosProportional*/, float /*rotaryStartAngle*/, float /*rotaryEndAngle*/, Slider &s) override
	{
		float alphaValue = 1.0f;

		if(!s.isEnabled()) alphaValue *= 0.4f;

		
		

		const double value = s.getValue();

		if(value == 0.0) alphaValue *= 0.66f;

        const double normalizedValue = (value - s.getMinimum()) / (s.getMaximum() - s.getMinimum());
		const double proportion = pow(normalizedValue, s.getSkewFactor());
		const int stripIndex = (int) (proportion * 63);

        const int offset = stripIndex * 28;
		Image clip = cachedImage_balanceKnob_png.getClippedImage(Rectangle<int>(0, offset, 28, 28));

        g.setColour (Colours::black.withAlpha(alphaValue));
        g.drawImage (clip, 0, 0, 24, 24, 0, 0, 28, 28); 

		

	}

private:
	Image cachedImage_balanceKnob_png;


	static const char* balanceKnob_png;
    static const int balanceKnob_pngSize;
};


class ChainBarButtonLookAndFeel: public LookAndFeel_V3
{
public:

	enum ColourIds
	{
		IconColour = Slider::ColourIds::thumbColourId,
		IconColourOff = Slider::ColourIds::trackColourId
	};

	Font getTextButtonFont (TextButton&, int /*buttonHeight*/) override
	{
		return GLOBAL_FONT();
	}

	void drawButtonText(Graphics& g, TextButton& button, bool /*isMouseOverButton*/, bool /*isButtonDown*/) override
	{		
		g.setFont(GLOBAL_FONT());
		g.setColour(button.findColour(button.getToggleState() ? TextButton::textColourOnId
			: TextButton::textColourOffId)
			.withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f));


		g.drawText(button.getButtonText(), 0, 0, button.getWidth(), button.getHeight(), Justification::centred, false);

	}

	void drawButtonBackground (Graphics &g, Button &b, const Colour &backgroundColour, bool isMouseOverButton, bool isButtonDown) override
	{


		LookAndFeel_V3::drawButtonBackground(g, b, backgroundColour, isMouseOverButton, isButtonDown);

		g.setColour(Colours::white.withAlpha(b.getToggleState() ? 0.8f : 0.3f));

		Path path;

		if(b.getButtonText() == "MIDI")
		{
			static const unsigned char pathData[] = { 110,109,0,0,226,66,92,46,226,67,98,0,0,226,66,112,2,227,67,79,80,223,66,92,174,227,67,0,0,220,66,92,174,227,67,98,177,175,216,66,92,174,227,67,0,0,214,66,112,2,227,67,0,0,214,66,92,46,226,67,98,0,0,214,66,72,90,225,67,177,175,216,66,92,174,224,67,0,0,
			220,66,92,174,224,67,98,79,80,223,66,92,174,224,67,0,0,226,66,72,90,225,67,0,0,226,66,92,46,226,67,99,109,0,128,218,66,92,110,222,67,98,0,128,218,66,112,66,223,67,79,208,215,66,92,238,223,67,0,128,212,66,92,238,223,67,98,177,47,209,66,92,238,223,67,0,
			128,206,66,112,66,223,67,0,128,206,66,92,110,222,67,98,0,128,206,66,72,154,221,67,177,47,209,66,92,238,220,67,0,128,212,66,92,238,220,67,98,79,208,215,66,92,238,220,67,0,128,218,66,72,154,221,67,0,128,218,66,92,110,222,67,99,109,0,128,203,66,92,142,220,
			67,98,0,128,203,66,112,98,221,67,79,208,200,66,92,14,222,67,0,128,197,66,92,14,222,67,98,177,47,194,66,92,14,222,67,0,128,191,66,112,98,221,67,0,128,191,66,92,142,220,67,98,0,128,191,66,72,186,219,67,177,47,194,66,92,14,219,67,0,128,197,66,92,14,219,
			67,98,79,208,200,66,92,14,219,67,0,128,203,66,72,186,219,67,0,128,203,66,92,142,220,67,99,109,0,128,188,66,92,110,222,67,98,0,128,188,66,112,66,223,67,79,208,185,66,92,238,223,67,0,128,182,66,92,238,223,67,98,177,47,179,66,92,238,223,67,0,128,176,66,
			112,66,223,67,0,128,176,66,92,110,222,67,98,0,128,176,66,72,154,221,67,177,47,179,66,92,238,220,67,0,128,182,66,92,238,220,67,98,79,208,185,66,92,238,220,67,0,128,188,66,72,154,221,67,0,128,188,66,92,110,222,67,99,109,0,0,181,66,92,46,226,67,98,0,0,181,
			66,112,2,227,67,79,80,178,66,92,174,227,67,0,0,175,66,92,174,227,67,98,177,175,171,66,92,174,227,67,0,0,169,66,112,2,227,67,0,0,169,66,92,46,226,67,98,0,0,169,66,72,90,225,67,177,175,171,66,92,174,224,67,0,0,175,66,92,174,224,67,98,79,80,178,66,92,174,
			224,67,0,0,181,66,72,90,225,67,0,0,181,66,92,46,226,67,99,109,0,128,197,66,151,79,215,67,98,243,139,173,66,151,79,215,67,0,0,154,66,148,50,220,67,0,0,154,66,151,47,226,67,98,0,0,154,66,154,44,232,67,243,139,173,66,151,15,237,67,0,128,197,66,151,15,237,
			67,98,12,116,221,66,151,15,237,67,0,0,241,66,154,44,232,67,0,0,241,66,151,47,226,67,98,0,0,241,66,148,50,220,67,13,116,221,66,151,79,215,67,0,128,197,66,151,79,215,67,99,109,0,128,197,66,151,79,218,67,98,209,247,214,66,151,79,218,67,0,0,229,66,163,209,
			221,67,0,0,229,66,151,47,226,67,98,0,0,229,66,139,141,230,67,210,247,214,66,151,15,234,67,0,128,197,66,151,15,234,67,98,47,8,180,66,151,15,234,67,0,0,166,66,139,141,230,67,0,0,166,66,151,47,226,67,98,0,0,166,66,163,209,221,67,47,8,180,66,151,79,218,67,
			0,128,197,66,151,79,218,67,99,101,0,0 };

			path.loadPathFromData (pathData, sizeof (pathData));


		}
		else if(b.getButtonText() == "Gain")
		{
			static const unsigned char pathData[] = { 110,109,0,0,47,67,92,174,218,67,108,0,0,57,67,92,174,218,67,108,0,0,57,67,92,174,238,67,108,0,0,47,67,92,174,238,67,99,109,0,0,32,67,92,174,223,67,108,0,0,42,67,92,174,223,67,108,0,0,42,67,92,174,233,67,108,0,0,32,67,92,174,233,67,99,109,0,0,17,67,92,
			46,226,67,108,0,0,27,67,92,46,226,67,108,0,0,27,67,92,46,231,67,108,0,0,17,67,92,46,231,67,99,101,0,0 };

			path.loadPathFromData (pathData, sizeof (pathData));

		}
		else if(b.getButtonText() == "Pitch")
		{
			static const unsigned char pathData[] = { 110,109,0,184,107,67,0,184,218,67,98,89,124,106,67,63,158,218,67,185,58,105,67,60,166,218,67,0,32,104,67,0,212,218,67,98,141,234,101,67,136,47,219,67,150,101,100,67,146,229,219,67,0,48,99,67,0,164,220,67,98,211,196,96,67,221,32,222,67,91,76,95,67,169,
			241,223,67,0,56,94,67,0,116,225,67,98,165,35,93,67,87,246,226,67,0,136,92,67,0,48,228,67,0,136,92,67,0,48,228,67,108,0,136,100,67,0,44,229,67,98,0,136,100,67,0,44,229,67,181,1,101,67,33,37,228,67,0,240,101,67,0,216,226,67,98,75,222,102,67,223,138,225,
			67,115,83,104,67,182,254,223,67,0,144,105,67,0,60,223,67,98,33,32,106,67,88,227,222,67,39,131,106,67,88,204,222,67,0,144,106,67,0,200,222,67,98,185,100,107,67,178,16,223,67,71,71,110,67,175,251,224,67,0,24,113,67,0,80,229,67,98,110,32,116,67,251,249,
			233,67,128,181,118,67,4,145,236,67,0,208,122,67,0,248,237,67,98,64,221,124,67,126,171,238,67,141,170,127,67,136,227,238,67,0,240,128,67,0,136,238,67,98,185,10,130,67,120,44,238,67,53,205,130,67,110,118,237,67,0,104,131,67,0,184,236,67,98,150,157,132,
			67,35,59,235,67,211,89,133,67,87,106,233,67,0,228,133,67,0,232,231,67,98,45,110,134,67,169,101,230,67,0,188,134,67,0,44,229,67,0,188,134,67,0,44,229,67,108,0,188,130,67,0,48,228,67,98,0,188,130,67,0,48,228,67,37,127,130,67,223,54,229,67,0,8,130,67,0,
			132,230,67,98,219,144,129,67,33,209,231,67,70,214,128,67,74,93,233,67,0,56,128,67,0,32,234,67,98,19,226,127,67,77,119,234,67,219,135,127,67,204,146,234,67,0,120,127,67,0,152,234,67,98,88,113,127,67,86,149,234,67,176,123,127,67,253,151,234,67,3,112,127,
			67,2,148,234,67,98,74,155,126,67,80,75,234,67,188,184,123,67,83,96,232,67,3,232,120,67,2,12,228,67,98,149,223,117,67,7,98,223,67,131,74,115,67,254,206,220,67,3,48,111,67,2,104,219,67,98,99,41,110,67,67,14,219,67,170,243,108,67,195,209,218,67,3,184,107,
			67,2,184,218,67,99,101,0,0 };

			path.loadPathFromData (pathData, sizeof (pathData));


		}
		else if (b.getButtonText() == "FX")
		{
			static const unsigned char pathData[] = { 110,109,0,0,160,67,92,174,238,67,108,0,0,165,67,92,174,228,67,108,0,0,160,67,92,174,218,67,108,0,0,165,67,92,174,218,67,108,0,128,167,67,92,174,223,67,108,0,0,170,67,92,174,218,67,108,0,0,175,67,92,174,218,67,108,0,0,170,67,92,174,228,67,108,0,0,175,
			67,92,174,238,67,108,0,0,170,67,92,174,238,67,108,0,128,167,67,92,174,233,67,108,0,0,165,67,92,174,238,67,99,109,0,0,150,67,92,46,226,67,108,0,128,157,67,92,46,226,67,108,0,128,157,67,92,46,231,67,108,0,0,150,67,92,46,231,67,99,109,0,128,147,67,92,174,
			218,67,108,0,0,160,67,92,174,218,67,108,0,0,160,67,92,174,223,67,108,0,128,147,67,92,174,223,67,99,109,0,128,147,67,92,174,218,67,108,0,128,152,67,92,174,218,67,108,0,128,152,67,92,174,238,67,108,0,128,147,67,92,174,238,67,99,101,0,0 };
			path.loadPathFromData (pathData, sizeof (pathData));

		}

		else if(b.getButtonText() == "Sample Start")
		{
			static const unsigned char pathData[] = { 110,109,0,0,200,67,92,174,238,67,108,0,0,200,67,92,174,218,67,108,0,0,215,67,92,174,228,67,99,109,0,128,192,67,92,174,238,67,108,0,128,192,67,92,174,218,67,108,0,128,197,67,92,174,218,67,108,0,128,197,67,92,174,238,67,99,101,0,0 };

			path.loadPathFromData (pathData, sizeof (pathData));

		}
		
		else if (b.getButtonText() == "Group Fade")
		{
			static const unsigned char pathData[] = { 110, 109, 0, 214, 35, 67, 192, 171, 170, 67, 108, 0, 214, 35, 67, 64, 118, 174, 67, 108, 128, 174, 44, 67, 64, 118, 174, 67, 108, 0, 17, 49, 67, 192, 56, 178, 67, 108, 128, 151, 44, 67, 0, 15, 182, 67, 108, 128, 191, 35, 67, 0, 15, 182, 67, 108, 128, 191, 35, 67, 192, 217, 185, 67, 108, 128, 43, 40, 67, 192, 217, 185, 67,
				108, 128, 3, 49, 67, 192, 217, 185, 67, 108, 0, 125, 53, 67, 128, 3, 182, 67, 108, 128, 242, 57, 67, 64, 214, 185, 67, 108, 0, 55, 71, 67, 64, 214, 185, 67, 108, 0, 55, 71, 67, 192, 11, 182, 67, 108, 128, 94, 62, 67, 192, 11, 182, 67, 108, 0, 233, 57, 67, 0, 57, 178, 67, 108, 0, 72, 62, 67, 192, 121, 174, 67, 108, 0, 32, 71,
				67, 192, 121, 174, 67, 108, 0, 32, 71, 67, 0, 175, 170, 67, 108, 0, 220, 57, 67, 0, 175, 170, 67, 108, 0, 125, 53, 67, 64, 110, 174, 67, 108, 128, 26, 49, 67, 192, 171, 170, 67, 108, 128, 66, 40, 67, 192, 171, 170, 67, 108, 0, 214, 35, 67, 192, 171, 170, 67, 99, 101, 0, 0 };

			path.loadPathFromData(pathData, sizeof(pathData));

		}

		path.scaleToFit(4.0f, 4.0f, (float)b.getHeight() - 8.0f, (float)b.getHeight() - 8.0f, true);

		
		g.setColour(b.findColour(b.getToggleState() ? IconColour : IconColourOff));
		

		g.fillPath(path);

	};
};


class MacroKnobLookAndFeel: public LookAndFeel_V3
{

public:

	MacroKnobLookAndFeel()
	{
		cachedImage_macroKnob_png = ImageProvider::getImage(ImageProvider::KnobEmpty);// ImageCache::getFromMemory(macroKnob_png, macroKnob_pngSize);
		cachedImage_macroKnob_ring_png = ImageProvider::getImage(ImageProvider::KnobUnmodulated);
	}

	void drawRotarySlider (Graphics &g, int /*x*/, int /*y*/, int /*width*/, int /*height*/, float /*sliderPosProportional*/, float /*rotaryStartAngle*/, float /*rotaryEndAngle*/, Slider &s) override;

	Label *createSliderTextBox (Slider &)
	{


		Label *label = new Label("Textbox");
		label->setFont (GLOBAL_FONT());
		label->setEditable (false, false, false);
		label->setColour (Label::textColourId, Colours::black);
		
		label->setJustificationType (Justification::centred);

		return label;
	}



private:
	Image cachedImage_macroKnob_png;

	Image cachedImage_macroKnob_ring_png;

	static const char* macroKnob_png;
    static const int macroKnob_pngSize;
};



#endif