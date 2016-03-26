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

#ifndef CONSTANTMODULATOR_H_INCLUDED
#define CONSTANTMODULATOR_H_INCLUDED

/** This modulator simply returns a constant value that can be used to change the gain or something else.
*
*	@ingroup modulatorTypes
*/
class ConstantModulator: public VoiceStartModulator
{
public:

	SET_PROCESSOR_NAME("Constant", "Constant")	

	/// Additional Parameters for the constant modulator
	enum SpecialParameters
	{
		numTotalParameters
	};

	ConstantModulator(MainController *mc, const String &id, int numVoices, Modulation::Mode m):
		VoiceStartModulator(mc, id, numVoices, m),
		Modulation(m)
	{ };

	ProcessorEditorBody *createEditor(BetterProcessorEditor *parentEditor)  override;

	/// sets the constant value. The only valid parameter_index is Intensity
	void setInternalAttribute(int, float ) override
	{
	};

	/// returns the constant value
	float getAttribute(int ) const override
	{
		return 0.0f;
	};

	

	/** Returns the 0.0f and let the intensity do it's job. */
	float calculateVoiceStartValue(const MidiMessage &) override
	{
		return (getMode() == GainMode) ? 0.0f : 1.0f;
	};

private:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConstantModulator)
};



#endif  // CONSTANTMODULATOR_H_INCLUDED
