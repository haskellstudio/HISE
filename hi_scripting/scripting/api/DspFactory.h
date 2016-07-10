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

#ifndef DSPFACTORY_H_INCLUDED
#define DSPFACTORY_H_INCLUDED

#pragma warning (push)
#pragma warning (disable: 4127)

/** A generic template factory class. 
*
*	Create a instance with the base class as template and add subclasses via registerType<SubClass>().
*	The subclasses must have a static method 
*	
*		static Identifier getName() { RETURN_STATIC_IDENTIFIER(name) }
*
*	so that the factory can decide which subtype to create.
*/
template <typename BaseClass>
class Factory
{
public:
	
	/** Register a subclass to this factory. The subclass must have a static method 'Identifier getName()'. */
	template <typename DerivedClass> void registerType()
	{
		if (std::is_base_of<BaseClass, DerivedClass>::value)
		{
			ids.add(DerivedClass::getName());
			functions.add(&createFunc<DerivedClass>);
		}
	}

	/** Creates a subclass instance with the registered Identifier and returns a base class pointer to this. You need to take care of the ownership of course. */
	BaseClass* createFromId(const Identifier &id) const
	{
		const int index = ids.indexOf(id);

		if (index != -1) return functions[index]();
		else			 return nullptr;
	}

	const Array<Identifier> &getIdList() const { return ids; }

private:

	/** @internal */
	template <typename DerivedClass> static BaseClass* createFunc() { return new DerivedClass(); }

	/** @internal */
	typedef BaseClass* (*PCreateFunc)();
	
	Array<Identifier> ids;
	Array <PCreateFunc> functions;;
};


#pragma warning (pop)


class DspInstance;

class DspFactory : public DynamicObject
{
public:

	class Handler
	{
	public:

		Handler();
		~Handler();

		DspInstance *createDspInstance(const String &factoryName, const String &moduleName);

		template <class T> static void registerStaticFactory(Handler *instance);

		/** Returns a factory with the given name.
		*
		*	It looks for static factories first. If no static library is found, it searches for opened dynamic factories.
		*	If no dynamic factory is found, it will open the dynamic library at the standard path and returns this instance.
		*/
		DspFactory *getFactory(const String &name);

	private:

		static void registerStaticFactories(Handler *instance);

		ReferenceCountedArray<DspFactory> staticFactories;
		ReferenceCountedArray<DspFactory> loadedPlugins;
	};

	class LibraryLoader : public DynamicObject
	{
	public:

		LibraryLoader()
		{
			ADD_DYNAMIC_METHOD(getLibrary);
		}

		var getLibrary(const String &name)
		{
			DspFactory *f = dynamic_cast<DspFactory*>(handler->getFactory(name));
			return var(f);
		}

	private:

		struct Wrapper
		{
			DYNAMIC_METHOD_WRAPPER_WITH_RETURN(LibraryLoader, getLibrary, ARG(0).toString());
		};

		SharedResourcePointer<DspFactory::Handler> handler;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LibraryLoader)
	};

	DspFactory():
		DynamicObject()
	{
		ADD_DYNAMIC_METHOD(createModule);
		ADD_DYNAMIC_METHOD(getModuleList);
	}

	/** If you write a static DspLibrary, you need to overwrite this method and return an unique ID. */
	virtual Identifier getId() const = 0;

	virtual var createModule(const String &module) const = 0;

	/** This method is called by the DspInstance object to create a instance of a DSP module. */
	virtual DspBaseObject *createDspBaseObject(const String &module) const = 0;

	/** This method is called by the DspInstance object's destructor to delete the module. */
	virtual void destroyDspBaseObject(DspBaseObject *object) const = 0;

	/** This method must return an array with all module names that can be created. */
	virtual var getModuleList() const = 0;

	struct Wrapper
	{
		DYNAMIC_METHOD_WRAPPER_WITH_RETURN(DspFactory, createModule, ARG(0).toString());
		DYNAMIC_METHOD_WRAPPER_WITH_RETURN(DspFactory, getModuleList);
	};

	typedef ReferenceCountedObjectPtr<DspFactory> Ptr;
	typedef ReferenceCountedObjectPtr<const DspFactory> ConstPtr;

};



/** This objects is a wrapper around the actual DSP module that is loaded from a plugin.
*
*	It contains the glue code for accessing it per Javascript and is reference counted to manage the lifetime of the external module.
*/
class DspInstance : public DynamicObject,
					public AssignableObject
{
public:

	/** Creates a new instance from the given Factory with the supplied name. */
	DspInstance(const DspFactory *f, const String &moduleName_);

	virtual ~DspInstance();

	/** Calls the setup method of the external module. */
	void prepareToPlay(double sampleRate, int samplesPerBlock);

	/** Calls the processMethod of the external module. */
	void processBlock(const var &data);

	void setParameter(int index, var newValue);
	var getParameter(int index) const;

	void assign(const int index, var newValue) override;
	var getAssignedValue(int index) const override;
	int getCachedIndex(const var &name) const override;

	

	/** Applies the module on the data.
	*
	*	The incoming data can be either a VariantBuffer or a array of VariantBuffers. */
	void operator >>(var &data);

	/** Copies the modules internal data into either the supplied multichannel array or the buffer. */
	void operator << (var &data) const;

	var getInfo() const;

private:

	struct Wrapper
	{
		DYNAMIC_METHOD_WRAPPER(DspInstance, processBlock, ARG(0));
		DYNAMIC_METHOD_WRAPPER(DspInstance, prepareToPlay, (double)ARG(0), (int)ARG(1));
		DYNAMIC_METHOD_WRAPPER(DspInstance, setParameter, (int)ARG(0), ARG(1));
		DYNAMIC_METHOD_WRAPPER_WITH_RETURN(DspInstance, getParameter, (int)ARG(0));
		DYNAMIC_METHOD_WRAPPER_WITH_RETURN(DspInstance, getInfo);
	};

	void throwError(const String &errorMessage)
	{
		throw String(errorMessage);
	}

	const String moduleName;

	DspBaseObject *object;

	DspFactory::Ptr factory;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DspInstance)
};


/** This class is used to create modules.
*
*	Unline the DynamicDspFactory, it needs to be embedded in the main application.
*/
class StaticDspFactory : public DspFactory
{
public:

	StaticDspFactory() {};
	virtual ~StaticDspFactory() {};

	var createModule(const String &name) const override;

	DspBaseObject *createDspBaseObject(const String &moduleName) const override;

	void destroyDspBaseObject(DspBaseObject* handle) const override;

	/** Overwrite this method and register every module you want to create with this factory using registerDspModule<Type>(). */
	virtual void registerModules() = 0;

	var getModuleList() const override;

protected:

	/** Use this helper method to register every module. */
	template <class DspModule>void registerDspModule() { factory.registerType<DspModule>(); }

private:

	Factory<DspBaseObject> factory;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StaticDspFactory)
};


class DynamicDspFactory : public DspFactory
{
public:

	DynamicDspFactory(const String &name_);;

	DspBaseObject *createDspBaseObject(const String &moduleName) const override;
	void destroyDspBaseObject(DspBaseObject *object) const override;

	void initialise();
	var createModule(const String &moduleName) const override;
	Identifier getId() const override { RETURN_STATIC_IDENTIFIER(name); }
	var getModuleList() const override;

private:

	const String name;
	ScopedPointer<DynamicLibrary> library;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DynamicDspFactory)
};




#endif  // DSPFACTORY_H_INCLUDED