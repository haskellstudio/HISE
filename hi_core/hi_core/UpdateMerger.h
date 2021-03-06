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

#ifndef UPDATEMERGER_H_INCLUDED
#define UPDATEMERGER_H_INCLUDED

/** A counter which can be used to limit the frequency for eg. GUI updates
*	@ingroup utility
*
*	If set up correctly using either limitFromSampleRateToFrameRate() or limitFromBlockSizeToFrameRate(), it has an internal counter
*	that is incremented each time update() is called and returns true, if a new change message is due.
*	
*/
class UpdateMerger
{
public:
	/** creates a new UpdateMerger and registeres the given listener.*/
	UpdateMerger():
		frameRate(20),
        countLimit(0),
        updateCounter(0)
        
	{};

	/** A handy method to limit updates from buffer block to frame rate level.
	*	
	*	Use this if you intend to call update() every buffer block.
	*/
	void limitFromBlockSizeToFrameRate(double sampleRate, int blockSize) noexcept
	{
		limitFromSampleRateToFrameRate(sampleRate / blockSize);
	}

	/** sets a manual skip number. Use this if you don't need the fancy block -> frame conversion. */
	void setManualCountLimit(int skipAmount)
	{
		SpinLock::ScopedLockType sl(processLock);

		countLimit = skipAmount;

		updateCounter = 0;
	};

	/** Call this method whenever something changes and the UpdateMerger class will check if a update is necessary.
	*
	*	@return \c true if a update should be made or \c false if not.
	*/
	inline bool shouldUpdate() noexcept
	{
		// the limit rate has not been set!
		jassert(countLimit > 0);
		if(++updateCounter == countLimit)
		{
			SpinLock::ScopedLockType sl(processLock);
			updateCounter = 0;
			return true;
		};
		return false;		
	};

	/** Call this method whenever something changes and the UpdateMerger class will check if a update is necessary.
	*
	*	You can pass a step amount if you want to merge some steps. If the count limit is reached, the overshoot will be
	*	retained.
	*/
	inline bool shouldUpdate(int stepsToSkip)
	{
		jassert(countLimit > 0);
		
		updateCounter += stepsToSkip;

		if(updateCounter >= countLimit)
		{
			SpinLock::ScopedLockType sl(processLock);

			updateCounter = updateCounter % countLimit;
			return true;
		}

		return false;

	}

private:

	SpinLock processLock;

	

	/** handy method to limit updates from sample rate level to frame rate level.
	*	
	*	Use this if you intend to call update() every sample.
	*/
	void limitFromSampleRateToFrameRate(double sampleRate) noexcept
	{
		countLimit = (int)(sampleRate) / frameRate;
		updateCounter = 0;
	};

	int frameRate;
	
	int countLimit;
	volatile int updateCounter;
};

#if JUCE_DEBUG
#define GUI_UPDATER_FRAME_RATE 150
#else
#define GUI_UPDATER_FRAME_RATE 30
#endif


/** Utility class that reduces the update rate to a common framerate (~30 fps)
*
*	Unlike the UpdateMerger class, this class checks the time between calls to shouldUpdate() and returns true, if 30ms have passed since the last succesfull call to shouldUpdate().
*
*/
class GUIUpdater
{
public:

	GUIUpdater():
		timeOfLastCall(Time::currentTimeMillis()),
		timeOfDebugCall(Time::currentTimeMillis())
	{}

	/** Call this to check if the last update was longer than 30 ms ago. 
	*
	*	If debugInterval is true, then the interval between calls will be printed in debug mode.
	*/
	bool shouldUpdate(bool debugInterval=false)
	{
		IGNORE_UNUSED_IN_RELEASE(debugInterval);

		const int64 currentTime = Time::currentTimeMillis();

#ifdef JUCE_DEBUG

		if (debugInterval)
		{
			timeOfDebugCall = currentTime;
		}

#endif

		if ((currentTime - timeOfLastCall) > GUI_UPDATER_FRAME_RATE)
		{
			timeOfLastCall = currentTime;
			return true;
		}

		return false;
	}

private:

	int64 timeOfLastCall;

	int64 timeOfDebugCall;
};



/** A lowpass filter that can be used to smooth parameter changes.
*	@ingroup utility
*/
class Smoother
{
public:

	/** Creates a new smoother. If you use this manually, you have to call prepareToPlay() and setSmoothingTime() before using it. */
	Smoother():
		active(false),
		sampleRate(-1.0f),
        smoothTime(0.0f)
	{ 
		a0 = b0 = x = currentValue = prevValue = 0.0f;
		
	};

	/** smooth the next sample. */
	float smooth(float newValue)
	{
		SpinLock::ScopedLockType sl(spinLock);

		if(! active) return newValue;
		jassert(sampleRate > 0.0f);

		currentValue = a0 * newValue - b0 * prevValue;

		jassert(currentValue > -1000.0f);
		jassert(currentValue < 1000.0f);

		prevValue = currentValue;

		return currentValue;
	};

	void smoothBuffer(float* data, int numSamples)
	{
		if (!active) return;

		jassert(sampleRate > 0.0);

		for (int i = 0; i < numSamples; i++)
		{
			currentValue = a0 * data[i] - b0 * prevValue;
			prevValue = currentValue;
		}
	}

	/** Returns the smoothing time in seconds. */
	float getSmoothingTime() const
	{
		return smoothTime;
	};

	/** Sets the smoothing time in seconds. 
	*
	*	If you pass 0.0 as parameter, the smoother gets deactivated and saves precious CPU cycles.
	*/
	void setSmoothingTime(float newSmoothTime)
	{
		SpinLock::ScopedLockType sl(spinLock);

		active = (newSmoothTime != 0.0f);

		smoothTime = newSmoothTime;

		if (sampleRate > 0.0)
		{
			const float freq = 1000.0f / newSmoothTime;

			x = expf(-2.0f * float_Pi * freq / sampleRate);
			a0 = 1.0f - x;
			b0 = -x;
		}
	};

	/** Sets the internal sample rate. Call this method before setting the smooth time. */
	void prepareToPlay(double sampleRate_)
	{
		sampleRate = (float)sampleRate_;

		setSmoothingTime(smoothTime);
	};

	/** Sets the internal value to the given number. Use this to prevent clicks for the first smoothing operation (default is 0.0) */
	void setDefaultValue(float value)
	{
		prevValue = value;
	}

private:

	SpinLock spinLock;

	JUCE_LEAK_DETECTOR(Smoother)

	bool active;

	float sampleRate;

	float smoothTime;

	float a0, b0, x;

	float currentValue;
	float prevValue;
};


/** A Ramper applies linear ramping to a value.
*	@ingroup utility
*
*/
class Ramper
{
public:

	Ramper():
		targetValue(0.0f),
		stepDelta(0.0f),
		stepAmount(-1)
	{};

	/** Sets the step amount that the ramper will use. You can overwrite this value by supplying a step number in setTarget. */
	void setStepAmount(int newStepAmount) { stepAmount = newStepAmount; };

	/** sets the new target and recalculates the step size using either the supplied step number or the step amount previously set by setStepAmount(). */
	void setTarget(float currentValue, float newTarget, int numberOfSteps=-1)
	{
		if(numberOfSteps != -1) stepDelta = (newTarget - currentValue) / numberOfSteps;
		else if (stepAmount != -1) stepDelta = (newTarget - currentValue) / stepAmount;
		else jassertfalse; // Either the step amount should be set, or a new step amount should be supplied

		targetValue = newTarget;
		busy = true;
	};

	/** Sets the ramper value and the target to the new value and stops ramping. */
	void setValue(float newValue)
	{
		targetValue = newValue;
		stepDelta = 0.0f;
		busy = false;
	};

	/** ramps the supplied value and returns true if the targetValue is reached. */
	inline bool ramp(float &valueToChange) 
	{
		valueToChange += stepDelta;
		busy = fabs(targetValue - valueToChange) > 0.001f;
		return busy;
	};

	bool isBusy() const { return busy; }
private:

	bool busy = false;
	float targetValue, stepDelta;
	int stepAmount;

};


#endif  // UPDATEMERGER_H_INCLUDED
