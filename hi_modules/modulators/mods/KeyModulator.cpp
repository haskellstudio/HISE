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

KeyModulator::KeyModulator(MainController *mc, const String &id, int numVoices, Modulation::Mode m):
		VoiceStartModulator(mc, id, numVoices, m),
		Modulation(m),
		mode(KeyMode),
		keyTable(new DiscreteTable()),
		midiTable(new MidiTable())
{
	parameterNames.add("KeyMode");
	parameterNames.add("NumberMode");
};

ProcessorEditorBody *KeyModulator::createEditor(BetterProcessorEditor *parentEditor)
{ 
#if USE_BACKEND

	return new KeyEditor(parentEditor); 
	
#else

	jassertfalse;

	return nullptr;

#endif
};