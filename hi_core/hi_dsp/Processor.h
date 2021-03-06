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
*   along with HISE.  If not, see <http://www.gnu.org/licenses/>.
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

#ifndef PROCESSOR_H_INCLUDED
#define PROCESSOR_H_INCLUDED

class Chain;

class ProcessorEditor;
class ProcessorEditorBody;

class FactoryType;

class BaseConstrainer
{

};

#define loadAttribute(name, nameAsString) (setAttribute(name, (float)v.getProperty(nameAsString, false), sendNotification))
#define saveAttribute(name, nameAsString) (v.setProperty(nameAsString, getAttribute(name), nullptr))

#define loadAttributeWithDefault(parameterId) setAttribute(parameterId, v.getProperty(getIdentifierForParameterIndex(parameterId), getDefaultValue(parameterId)), dontSendNotification);

// Handy macro to set the name of the processor (type = Identifier, name = Displayed processor name)
#define SET_PROCESSOR_NAME(type, name) static String getClassName() {return name;}; \
						  const String getName() const override {	return getClassName();}; \
						  static Identifier getClassType() {return Identifier(type);} \
						  const Identifier getType() const override {return getClassType();}



/** The base class for all modules.
*
*	Every object within HISE that processes audio or MIDI is derived from this base class to share the following features:
*
*	- handling of child processors (and special treatment for child processors which are Chains)
*	- bypassing & parameter management (using the 'float' type for compatibility with audio processing & plugin parameters): setAttribute() / getAttribute()
*	- set / restore view properties (folded, body shown etc.) using a NamedValueSet (setEditorState(), getEditorState())
*	- set input / output values for metering and stuff (setInputValue(), setOutputValue())
*	- access to the global MainController object (see ControlledObject)
*	- import / export via ValueTree (see RestorableObject)
*	- methods for identification (getId(), getType(), getName(), getSymbol(), getColour())
*	- access to the console
*	- a specially designed component (ProcessorEditor) which acts as interface for the Processor.
*
*	The general architecture of a HISE patch is a tree of Processors, with a main processor (which can be obtained using getMainController()->getMainSynthChain()).
*	There is a small helper class ProcessorHelpers, which contains some static methods for searching & checking the type of the Processor.
*
*	Normally, you would not derive from this class directly, but use some of its less generic subclasses (MidiProcessor, Modulator, EffectProcessor or ModulatorSynth). 
*	There are also some additional interface classes to extend the Processor class with specific features: \ref processor_interfaces
*/
class Processor: public SafeChangeBroadcaster,
				 public RestorableObject,
				 public ControlledObject
{
public:

	/**	Creates a new Processor with the given Identifier. */
	Processor(MainController *m, const String &id_):
		ControlledObject(m),
		id(id_),
		consoleEnabled(false),
		bypassed(false),
		visible(true),
		samplerate(-1.0),
		blockSize(-1),
		inputValue(0.0f),
		outputValue(0.0f),
		editorState(0),
		symbol(Path())
	{
		editorStateIdentifiers.add("Folded");
		editorStateIdentifiers.add("BodyShown");
		editorStateIdentifiers.add("Visible");
		editorStateIdentifiers.add("Solo");
		
		setEditorState(Processor::BodyShown, true, dontSendNotification);
		setEditorState(Processor::Visible, true, dontSendNotification);
		setEditorState(Processor::Solo, false, dontSendNotification);

		if (Identifier::isValidIdentifier(id))
		{
			idAsIdentifier = Identifier(id);
		}
	};

	/** Overwrite this if you need custom destruction behaviour. */
	virtual ~Processor()
	{
		getMainController()->getMacroManager().removeMacroControlsFor(this);
		masterReference.clear();
		removeAllChangeListeners();	
	};

	/** Overwrite this enum and add new parameters. This is used by the set- / getAttribute methods. */
	enum SpecialParameters {};

	/** Overwrite this enum and list all internal chains. This can be used to access them by index. */
	enum InternalChains {};

	enum EditorState
	{
		Folded = 0,
		BodyShown,
		Visible,
		Solo,
		numEditorStates
	};

	/** Creates a ProcessorEditor for this Processor and returns the pointer.

		If you subclass this, just allocate a ProcessorEditor of the desired type on the heap and return it.
		Remember to pass the Processor as parameter to allow asynchronous GUI notification.
		The concept between Processor and ProcessorEditor is the same as AudioProcessor and AudioProcessorEditor.
	*/
	virtual ProcessorEditorBody *createEditor(ProcessorEditor* parentEditor) = 0;

	/** This saves the Processor.
	*
	*	It saves the ID, the bypassed state and the fold state. It also saves all child processors.
	*	You can overwrite this function and add more properties and their child processors but make sure you call the base class method.
	*
	*	For primitive values, you can use the macro saveAttribute(name, nameAsString):
	*	
	*	You don't need to save the editor states, as the first 32 bits of EditorState is saved.
	*
		@code
		ValueTree exportAsValueTree() const override
		{
			// must be named 'v' for the macros
			ValueTree v = BaseClass::exportAsValueTree(); 

			saveAttribute(attributeName, "AttributeName");

			// ...

			if(useTable) saveTable(tableVariableName, "TableVariableNameData");

			return v;
		};
		@endcode
	*
	*
	*	@see restoreFromValueTree()
	*
	*/
	virtual ValueTree exportAsValueTree() const override
	{
#if USE_OLD_FILE_FORMAT
		ValueTree v(getType());
#else
		ValueTree v("Processor");
		v.setProperty("Type", getType().toString(), nullptr);
#endif

		v.setProperty("ID", getId(), nullptr);
		v.setProperty("Bypassed", isBypassed(), nullptr);

		ScopedPointer<XmlElement> editorValueSet = new XmlElement("EditorStates");
		editorStateValueSet.copyToXmlAttributes(*editorValueSet);		

#if USE_OLD_FILE_FORMAT
		v.setProperty("EditorState", editorValueSet->createDocument(""), nullptr);

		for(int i = 0; i < getNumChildProcessors(); i++)
		{
			v.addChild(getChildProcessor(i)->exportAsValueTree(), i, nullptr);
		};

#else
		ValueTree editorStateValueTree = ValueTree::fromXml(*editorValueSet);
		v.addChild(editorStateValueTree, -1, nullptr);

		ValueTree childProcessors("ChildProcessors");

		for(int i = 0; i < getNumChildProcessors(); i++)
		{
			childProcessors.addChild(getChildProcessor(i)->exportAsValueTree(), i, nullptr);
		};

		v.addChild(childProcessors, -1, nullptr);
#endif
		return v;

	};
	
	/** Restores a previously saved ValueTree. 
	*
	*	The value tree must be created with exportAsValueTree or it will be unpredictable.
	*	The child processors are created automatically (both for chains and processors with fixed internal chains,
	*	but you should overwrite this method if your Processor uses parameters.
	*
	*	There is a handy macro saveAttribute(name, nameAsString) for this purpose.
	*
	*	@code
		restoreFromValueTree(const ValueTree &v) override // parameter must be named 'v' for the macros
		{
			// replace BaseClass with the class name of the immediate base class
			BaseClass::restoreFromValueTree(v); 
			
			loadAttribute(attributeName, "AttributeName");
			// ...
			
			// If your Processor uses tables: 
			if(useTable) loadTable(tableVariableName, "TableVariableData");
		}
		@endcode
	*
	*	@see exportAsValueTree() 
	*/
	virtual void restoreFromValueTree(const ValueTree &previouslyExportedProcessorState) override;
	
	/** Overwrite this method to specify the name. 
    *
    *   Use non-whitespace Strings (best practice is CamelCase). In most cases, this will be enough:
	*
    *       const String getType() const { return "ProcessorName";}; 
    */
	virtual const Identifier getType() const = 0;

	

	/** Returns the symbol of the Processor. 
	*
	*	It either checks if a special symbol is set for this particular Processor, or returns the default one defined in getSpecialSymbol()
	*/
	const Path getSymbol() const
	{
		if(symbol.isEmpty())
		{
			return getSpecialSymbol();
		}
		else return symbol;
	}

	/** Sets a special symbol for the Processor.
	*
	*	If this method is used, the getSymbol() method will return this Path instead of the default one defined in getSpecialSymbol();
	*/
	void setSymbol(Path newSymbol) {symbol = newSymbol;	};

	/** Changes a Processor parameter. 
	*
	*   This can be used in the audio thread eg. by other Processors. Overwrite the method setInternalAttribute() to store the
    *   parameters in private member variables.
	*   If the Processor is polyphonic, the attributes must stay the same for different voices. In this
    *   case store a multiplicator for the attribute in a ProcessorState.
    *
	*   \param parameterIndex the parameter index (use a enum from the derived class)
	*   \param newValue the new value between 0.0 and 1.0
	*	\param notifyEditor if sendNotification, then a asynchronous message is sent.
	*/
	void setAttribute(int parameterIndex, float newValue, juce::NotificationType notifyEditor )
					 
	{
		setInternalAttribute(parameterIndex, newValue);
		if(notifyEditor == sendNotification) sendChangeMessage();
	}

	/** returns the attribute with the specified index (use a enum in the derived class). */
	virtual float getAttribute(int parameterIndex) const = 0;

	/** Overwrite this and return the default value. */
	virtual float getDefaultValue(int /*parameterIndex*/) const { return 1.0f; }

	/** This must be overriden by every Processor and return the Chain with the Chain index.
	*
	*	You can either:
	*	- return nullptr, if the Processor uses no internal chains
	*	- one of the internal chains (it's recommended to use the InternalChains enum for this case)
	*	- return the Processor at the index if the Processor is a Chain. You can use it's handler's getProcessor() method.
	*/
	virtual Processor *getChildProcessor(int processorIndex) = 0;

	virtual const Processor *getChildProcessor(int processorIndex) const = 0;

	/** This must return the number of Child processors.
	*
	*	If this Processor is a Chain, you can use it's getHandler()->getNumProcessor() method.
	*/
	virtual int getNumChildProcessors() const = 0;

	/** If your processor uses internal chains, you can return the number here. 
	*
	*	This is used by the ProcessorEditor to make the chain button bar. 
	*/
	virtual int getNumInternalChains() const { return 0;};

	void setConstrainerForAllInternalChains(BaseConstrainer *constrainer);

	/** Enables the Processor to output messages to the Console.
    *
	*   This method itself does nothing, but you can use this to make a button or something else.
	*/
	void enableConsoleOutput(bool shouldBeEnabled) {consoleEnabled = shouldBeEnabled;};

	/** Overwrite this method if you want a special colour. 
	*
	*	This colour will be used in the debug console and in the editor.
	*/
	virtual Colour getColour() const
	{
		return Colours::grey;
	};

	/** Returns the unique id of the Processor instance (!= the Processor name). 
	*
	*	It must be a valid Identifier (so no whitespace and weird characters).
	*/
	const String & getId() const {return id;};

	/** Overwrite this and return a pretty name. If you don't overwrite it, the getType() method is used. */
	virtual const String getName() const 
	{
		return getType().toString();
	}

	void setId(const String &newId, NotificationType notifyChangeHandler=dontSendNotification)
	{
		id = newId;

		if (Identifier::isValidIdentifier(id))
		{
			idAsIdentifier = Identifier(id);
		}
		else
		{
			idAsIdentifier = Identifier();
		}

		sendChangeMessage();

		if (notifyChangeHandler)
			getMainController()->getProcessorChangeHandler().sendProcessorChangeMessage(this, 
				MainController::ProcessorChangeHandler::EventType::ProcessorRenamed);
	};

	Identifier getIDAsIdentifier() const
	{
		return idAsIdentifier;
	}

	/** This bypasses the processor. You don't have to check in the processors logic itself, normally the chain should do that for you. */
	virtual void setBypassed(bool shouldBeBypassed, NotificationType notifyChangeHandler=dontSendNotification) noexcept 
	{ 
		bypassed = shouldBeBypassed; 
		currentValues.clear();

		if (notifyChangeHandler)
			getMainController()->getProcessorChangeHandler().sendProcessorChangeMessage(this, MainController::ProcessorChangeHandler::EventType::ProcessorBypassed, false);
	};

	/** Returns true if the processor is bypassed. */
	bool isBypassed() const noexcept { return bypassed; };

	/** Sets the sample rate and the block size. */
	virtual void prepareToPlay(double sampleRate_, int samplesPerBlock_)
	{
		samplerate = sampleRate_;
		blockSize = samplesPerBlock_;
	}

	/** Returns the sample rate. */
	double getSampleRate() const { return samplerate; };

	/** Returns the block size. */
	int getBlockSize() const
    {   
        return blockSize;
    };

	
#if USE_BACKEND
	/** Prints a message to the console.
    *
	*	Use this internally to debug important information if the console output is enabled for the Processor.
	*	Never use this method directly, but use the macro debugMod() so the String creation can be skipped if compiling with the flag HI_DEBUG_MODULATORS set to 0
	*	
	*/
	void debugProcessor(const String &t);
#endif

	/** This can be used to display the Processors output value. */
	float getOutputValue() const {	return currentValues.outL;	};

	/** This can be used to display the Processors input value. */
	float getInputValue() const
	{
		return inputValue;
	};

	/** Saves the state of the Processor's editor. It must be saved within the Processor, because the Editor can be deleted. 
	*
	*	You can add more states in your subclass (they should be expressable as bool). Best use a enum:
	*
	*		enum EditorState
	*		{
	*			newState = Processor::numEditorStates,
	*		};
	*/
	void setEditorState(int state, bool isOn, NotificationType notifyView=sendNotification)
	{
		const Identifier stateId = getEditorStateForIndex(state);

		editorStateValueSet.set(stateId, isOn);

        if(notifyView)
		{
			getMainController()->setCurrentViewChanged();
		}
	};

	const var getEditorState(Identifier editorStateId) const
	{
		return editorStateValueSet[editorStateId];
	}

	void setEditorState(Identifier state, var stateValue, NotificationType notifyView=sendNotification)
	{
		jassert(state.isValid());

		editorStateValueSet.set(state, stateValue);

		if(notifyView)
		{
			getMainController()->setCurrentViewChanged();
		}
	}

	Identifier getEditorStateForIndex(int index) const
	{
		// Did you forget to add the identifier to the id list in your subtype constructor?
		jassert(index < editorStateIdentifiers.size());

		return editorStateIdentifiers[index];
	}

	void toggleEditorState(int index, NotificationType notifyEditor)
	{
		bool on = getEditorState(getEditorStateForIndex(index));

		setEditorState(getEditorStateForIndex(index), !on, notifyEditor);
	}

	/** Restores the state of the Processor's editor. It must be saved within the Processor, because the Editor can be deleted. */
	bool getEditorState(int state) const 
	{
		return editorStateValueSet[getEditorStateForIndex(state)];

		//return editorState[state];
	};

	XmlElement *getCompleteEditorState() const
	{
		XmlElement *state = new XmlElement("EditorState");

		editorStateValueSet.copyToXmlAttributes(*state);

		return state;
	};

	/** Restores the EditorState from a BigInteger that was retrieved using getCompleteEditorState. */
	void restoreCompleteEditorState(const XmlElement *storedState)
	{
		if(storedState != nullptr)
		{
			editorStateValueSet.setFromXmlAttributes(*storedState);
		}
	};

	struct DisplayValues
	{
		DisplayValues():
			inL(0.0f),
			inR(0.0f),
			outL(0.0f),
			outR(0.0f)
		{};

		void clear()
		{
			inL = 0.0f;
			inR = 0.0f;
			outL = 0.0f;
			outR = 0.0f;
		};

		float inL;
		float outL;
		float inR;
		float outR;
	};

	DisplayValues getDisplayValues() const { return currentValues;};

	/** A iterator over all child processors. 
	*
	*	You don't have to use a inherited class of Processor for the template argument, it works with all classes.
	*/
	template<class SubTypeProcessor=Processor>
	class Iterator
	{
	public:

		/** Creates a new iterator. Simply pass in the Processor which children you want to iterate.
		*
		*	It creates a list of all child processors (children before siblings).
		*	Call getNextProcessor() to get the next child processor. 
		*/
		Iterator(Processor *root_, bool useHierarchy=false):
			hierarchyUsed(useHierarchy),
			index(0)
		{
			if(useHierarchy)
			{
				internalHierarchyLevel = 0;
				addProcessorWithHierarchy(root_);

			}
			else
			{
				addProcessor(root_);
			}
		};

		Iterator(const Processor *root_, bool useHierarchy = false) :
			hierarchyUsed(useHierarchy),
			index(0)
		{
			if (useHierarchy)
			{
				internalHierarchyLevel = 0;
				addProcessorWithHierarchy(const_cast<Processor*>(root_));

			}
			else
			{
				addProcessor(const_cast<Processor*>(root_));
			}
		};

		/** returns the next processor. 
		*
		*	If a Processor gets deleted between creating the Iterator and calling this method, it will safely return nullptr.
		*	(in this case the remaining processors are not iterated, but this is acceptable for the edge case.
		*/
		SubTypeProcessor *getNextProcessor()
		{
			if(index == allProcessors.size()) return nullptr;

			return dynamic_cast<SubTypeProcessor*>(allProcessors[index++].get());
		};

		/** returns a const pointer to the next processor. 
		*
		*	If a Processor gets deleted between creating the Iterator and calling this method, it will safely return nullptr.
		*	(in this case the remaining processors are not iterated, but this is acceptable for the edge case.
		*/
		const SubTypeProcessor *getNextProcessor() const
		{
			if(index == allProcessors.size()) return nullptr;

			return dynamic_cast<const SubTypeProcessor*>(allProcessors[index++].get());
		}

		int getHierarchyForCurrentProcessor() const
		{

			// You must use the other method!
			jassert(hierarchyData.size() > index-1);

			return hierarchyData[index-1];

		}


		int getNumProcessors() const
		{
			return allProcessors.size();
		}

		SubTypeProcessor *getProcessor(int index)
		{
			return allProcessors[index];
		}

		const SubTypeProcessor* getProcessor(int index) const
		{
			return allProcessors[index];
		}

	private:


		void addProcessor(Processor *p)
		{
			jassert(p != nullptr);

			if(dynamic_cast<SubTypeProcessor*>(p) != nullptr)
			{
				allProcessors.add(p);
			}

			if (p == nullptr) return;

			for(int i = 0; i < p->getNumChildProcessors(); i++)
			{
				addProcessor(p->getChildProcessor(i));
			}
		};

		void addProcessorWithHierarchy(Processor *p)
		{
			jassert(p != nullptr);

			if (p == nullptr) return;

			const int thisHierarchy = internalHierarchyLevel;

			if(dynamic_cast<SubTypeProcessor*>(p) != nullptr)
			{
				allProcessors.add(p);
				hierarchyData.add(thisHierarchy);
			}

			internalHierarchyLevel++;

			for(int i = 0; i < p->getNumChildProcessors(); i++)
			{
				

				addProcessorWithHierarchy(p->getChildProcessor(i));

				internalHierarchyLevel = thisHierarchy + 1;

			}
		};

		const bool hierarchyUsed;

		int internalHierarchyLevel;
		
		mutable int index;

		Array<int> hierarchyData;

		Array<WeakReference<Processor>> allProcessors;
		
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Iterator<SubTypeProcessor>)
	};

	/** This returns a Identifier with the name of the parameter.
	*
	*	If you want to use this feature (this lets you access Parameters with the script, you should add the parameter name
	*	for each parameter in your subtype constructor. */
	const Identifier getIdentifierForParameterIndex(int parameterIndex) const
	{
		if(parameterIndex > parameterNames.size()) return Identifier();

		return parameterNames[parameterIndex];
	};

	/** This returns the number of (named) parameters. */
	int getNumParameters() const {return parameterNames.size();}; 

	/** Call this method after inserting the processor in the signal chain.
	*
	*	If you call prepareToPlay before calling this method, it won't lock which makes inserting new processors nicer.
	*/
	void setIsOnAir(bool isBeingProcessedInAudioThread);

	bool isOnAir() const noexcept{ return onAir; }

	/** Call this method to get either the given lock or a dummy lock when the processor isn't on air yet. */
	const CriticalSection& getDummyLockWhenNotOnAir() const
	{
		return dummyLock;
	}

	struct DeleteListener
	{
		virtual ~DeleteListener()
		{
			masterReference.clear();
		}

		virtual void processorDeleted(Processor* deletedProcessor) = 0;

		virtual void updateChildEditorList(bool forceUpdate) = 0;

	private:

		friend class WeakReference<DeleteListener>;
		WeakReference<DeleteListener>::Master masterReference;
	};

	void addDeleteListener(DeleteListener* listener)
	{
		deleteListeners.addIfNotAlreadyThere(listener);
	}

	void removeDeleteListener(DeleteListener* listener)
	{
		deleteListeners.removeAllInstancesOf(listener);
	}

	void sendDeleteMessage()
	{
		int numListeners = deleteListeners.size();

		for (int i = numListeners-1; i >= 0; --i)
		{
			if (deleteListeners[i].get() != nullptr)
			{
				deleteListeners[i]->processorDeleted(this);
			}
		}
	}

	void sendRebuildMessage(bool forceUpdate=false)
	{
		int numListeners = deleteListeners.size();

		for (int i = 0; i < numListeners; i++)
		{
			if (deleteListeners[i].get() != nullptr)
			{
				deleteListeners[i]->updateChildEditorList(forceUpdate);
			}
		}
	}

protected:

	/** Overwrite this method if you want to supply a custom symbol for the Processor. 
	*
	*	By default, it creates an empty Path, so you either have to set a custom Symbol using setSymbol(), or overwrite the getSpecialSymbol() method in a subclass.
	*/
	virtual Path getSpecialSymbol() const { return Path(); }

	DisplayValues currentValues;

	/** Call this from the baseclass whenever you want its editor to display a value change. */
	void setOutputValue(float newValue)
	{
		currentValues.outL = newValue;

		//outputValue = (double)newValue;
		//sendChangeMessage();
	};

	/** Call this from the baseclass whenever you want its editor to display a input value change. 
	*
	*	For most stuff a 0.0 ... 1.0 range is assumed.
	*/
	void setInputValue(float newValue, NotificationType notify=sendNotification)
	{
		inputValue = newValue;

		if(notify == sendNotification)
		{
			sendChangeMessage();
		}
	};

	

	/** Changes a Processor parameter. 
	*
	*   Overwrite this method to do your handling. Call the overloaded method with the notification type parameter for external changes.
    *
	*   \param parameterIndex the parameter index (use a enum from the derived class)
	*   \param newValue the new value between 0.0 and 1.0
	*/
	virtual void setInternalAttribute(int parameterIndex, float newValue) = 0;
	
	bool consoleEnabled;

	Array<Identifier> parameterNames;
	Array<Identifier> editorStateIdentifiers;

	

private:

	Array<WeakReference<DeleteListener>> deleteListeners;

	CriticalSection dummyLock;

	bool onAir = false;

	Path symbol;

	WeakReference<Processor>::Master masterReference;
    friend class WeakReference<Processor>;

	Array<bool> editorStateAsBoolList;

	BigInteger editorState;

	NamedValueSet editorStateValueSet;
	

	float inputValue;
	double outputValue;

	bool bypassed;
	bool visible;

	double samplerate;
	int blockSize;

	OwnedArray<Chain> chains;

	/// the unique id of the Processor
	String id;

	Identifier idAsIdentifier;
};




/** Some handy helper functions that are using mainly the Iterator. */
class ProcessorHelpers
{
public:

	/** Small helper function that returns the parent processor of the given child processor. 
	*
	*	@param childProcessor the processor which parent should be found. It must be within the normal tree structure.
	*	@param getParentSynth if true, then the synth where the processor resides will be looked for. 
	*	If false, it will return the chain where the Processor resides (either ModulatorChain, MidiProcessorChain or EffectChain)
	*/
	static const Processor *findParentProcessor(const Processor *childProcessor, bool getParentSynth);

	static Processor *findParentProcessor(Processor *childProcessor, bool getParentSynth);

	/** Returns the first Processor with the given name (It skips all InternalChains). If there are multiple Processors with the same name, it will always return the first one.
	*
	*	To avoid this, use PresetHandler::findProcessorsWithDuplicateId...
	*/
	static Processor *getFirstProcessorWithName(const Processor *rootProcessor, const String &name);

	template <class ProcessorType> static int getAmountOf(const Processor *rootProcessor, const Processor *upTochildProcessor = nullptr);

	template <class ProcessorType> static StringArray getAllIdsForType(const Processor *rootProcessor)
	{
		StringArray sa;

		Processor::Iterator<const Processor> iter(rootProcessor);

		const Processor *p;

		while ((p = iter.getNextProcessor()) != nullptr)
		{
			if (dynamic_cast<const ProcessorType*>(p) != nullptr) sa.add(p->getId());
		}

		return sa;
	}

	/** Small helper function that checks if the given processor is of the supplied type. */
	template <class ProcessorType> static bool is(const Processor *p)
	{
		return dynamic_cast<const ProcessorType*>(p) != nullptr;
	}

	/** Small helper function that checks if the given processor is of the supplied type. */
	template <class ProcessorType> static bool is(Processor *p)
	{
		return dynamic_cast<ProcessorType*>(p) != nullptr;
	}

	/** Checks if the Processor can be hidden. This returns true for all processors that show up in the popup list. */
	static bool isHiddableProcessor(const Processor *p);

	/** Returns a string that declares a variable to be copied into a script.
	*
	*	For a given Processor of type "Type" and id "name" it will return:
	*		
	*		id = Synth.getType("id");
	*
	*	Currently supported types (including all subclasses):
	*	- MidiProcessors
	*	- Modulators
	*	- ModulatorSynths
	*/
	static String getScriptVariableDeclaration(const Processor *p, bool copyToClipboard=true);

	static String getBase64String(const Processor* p, bool copyToClipboard=true);

	static void restoreFromBase64String(Processor* p, const String& base64String);

	static void deleteProcessor(Processor* p);

	static void increaseBufferIfNeeded(AudioSampleBuffer& b, int numSamplesNeeded)
	{
		// The channel amount must be set correctly in the constructor
		jassert(b.getNumChannels() > 0);

		if (b.getNumSamples() < numSamplesNeeded)
		{
			b.setSize(b.getNumChannels(), numSamplesNeeded, true, true, true);
			b.clear();
		}
	}

	struct ValueTreeHelpers
	{
		static String getBase64StringFromValueTree(const ValueTree& v);;

		static ValueTree getValueTreeFromBase64String(const String& base64State);
	};

};


#endif  // PROCESSOR_H_INCLUDED
