/*
  ==============================================================================

    ExternalFilePool.h
    Created: 11 Jun 2015 11:10:41pm
    Author:  Christoph

  ==============================================================================
*/

#ifndef EXTERNALFILEPOOL_H_INCLUDED
#define EXTERNALFILEPOOL_H_INCLUDED



/** A base class for all objects that can be saved as value tree. 
*	@ingroup core
*/
class RestorableObject
{
public:
    
    virtual ~RestorableObject() {};

	/** Overwrite this method and return a representation of the object as ValueTree.
	*
	*	It's best practice to only store variables that are not internal (eg. states ...)
	*/
	virtual ValueTree exportAsValueTree() const = 0;

	/** Overwrite this method and restore the properties of this object using the referenced ValueTree.
	*/
	virtual void restoreFromValueTree(const ValueTree &previouslyExportedState) = 0;
};


/** A template class which handles the logic of a pool of external file references.
*
*	Features:
*	- load and stores data from files
*	- caches them so that they are referenced multiple times
*	- reference counting and clearing of unused data (but it must be handled manually)
*	- export / import the whole pool as ValueTree.
*/
template <class FileType> class Pool: public SafeChangeBroadcaster,
								public RestorableObject
{
public:

	virtual ~Pool() {};

	void restoreFromValueTree(const ValueTree &v) override;;

	ValueTree exportAsValueTree() const override;

	/** Returns a non-whitespace version of the file as id (check for collisions!) */
	Identifier getIdForFileName(const String &absoluteFileName) const
	{
		return Identifier(File(absoluteFileName).getFileNameWithoutExtension().removeCharacters(" \n\t"));
	}

	const String getFileNameForId(Identifier identifier);;

	Identifier getIdForIndex(int index);

	int getIndexForId(Identifier id) const;

	int getNumLoadedFiles() const {return data.size();};
	
	StringArray getFileNameList() const;

	StringArray getTextDataForId(int i);


	/** Clears the data. */
	void clearData();

	/** Call this whenever you don't need the buffer anymore. */
	void releasePoolData(const FileType *b);

	/** This loads the file into the pool. If it is already in the pool, it will be referenced. */
	const FileType *loadFileIntoPool(const String &fileName, bool forceReload=false);;

	/** Overwrite this method and return a file type identifier that will be used for the ValueTree id and stuff. */
	virtual Identifier getFileTypeName() const = 0;

protected:

	/** Set a property for the data. */
	void setPropertyForData(const Identifier &id, const Identifier &propertyName, var propertyValue) const;

	/** returns the property for the data */
	var getPropertyForData(const Identifier &id, const Identifier &propertyName) const;

	/** Overwrite this method and return a data object which will be owned by the pool from then. 
	*
	*	You also must delete the stream object when the job is finished (a bit ugly because The AudioFormatReader class does this automatically)
	*/
	virtual FileType *loadDataFromStream(InputStream *inputStream) const = 0;

	/** Overwrite this method and reload the data from the file. The object must not be deleted, because there may be references to it,
	*	so make sure it will replace the data internally.
	*
	*	You can use loadDataFromStream to load the data and then copy the internal data to the existing data. */
	virtual void reloadData(FileType &dataToBeOverwritten, const String &fileName) = 0;

	/** Overwrite this method and return its file size. */
	virtual size_t getFileSize(const FileType *data) const = 0;

private:

	void addData(Identifier id, const String &fileName, InputStream *inputStream);;

	const FileType *getDataForId(Identifier &id) const;

	struct PoolData
	{
		Identifier id;
		ScopedPointer<FileType> data;
		String fileName;
		NamedValueSet properties;
		int refCount;
	};

	OwnedArray<PoolData> data;
};


/** A pool for audio samples
*
*	This is used to embed impulse responses into the binary file and load it from there instead of having the impulse file as seperate audio file.
*	In order to use it, create a object, call loadExternalFilesFromValueTree() and the convolution effect checks automatically if it should load the file from disk or from the pool.
*/
class AudioSampleBufferPool: public Pool<AudioSampleBuffer>
{
public:

	AudioSampleBufferPool():
	sampleRateIdentifier("SampleRate")
	{ cache = new AudioThumbnailCache(128);	};

	Identifier getFileTypeName() const override {return Identifier("AudioFile");};

	virtual size_t getFileSize(const AudioSampleBuffer *data) const override
	{
		return (size_t)data->getNumSamples() * (size_t)data->getNumChannels() * sizeof(float);
	}

	AudioThumbnailCache *getCache() { return cache; }

	double getSampleRateForFile(const Identifier &id)
	{
		return (double)getPropertyForData(id, sampleRateIdentifier);
	}

protected:

	AudioSampleBuffer *loadDataFromStream(InputStream *inputStream) const override;

	void reloadData(AudioSampleBuffer &dataToBeOverwritten, const String &fileName) override;;	


private:

	Identifier sampleRateIdentifier;

	ScopedPointer<AudioThumbnailCache> cache;
	
};


/** A pool for images. */
class ImagePool: public Pool<Image>
{
public:

	Identifier getFileTypeName() const override {return Identifier("Image");};

	virtual size_t getFileSize(const Image *data) const override
	{
		return (size_t)data->getWidth() * (size_t)data->getHeight() * sizeof(uint32);
	}

	static Image getEmptyImage(int width, int height);

protected:


	void reloadData(Image &image, const String &fileName) override;;	

	Image *loadDataFromStream(InputStream *inputStream) const override;;
};

#endif  // EXTERNALFILEPOOL_H_INCLUDED
