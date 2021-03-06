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
#ifndef PRESETCOMPONENTS_H_INCLUDED
#define PRESETCOMPONENTS_H_INCLUDED

class PresetBrowserWindow : public Component,
							public QuasiModalComponent,
							public ButtonListener,
							public UserPresetData::Listener
{
public:

	// ================================================================================================================

	PresetBrowserWindow(const UserPresetData *data);
	~PresetBrowserWindow();

	void resized() override;
	void paint(Graphics &g);

	// ================================================================================================================

	void buttonClicked(Button *b) override;
	void presetLoaded(int categoryIndex_, int presetIndex_, const String &presetName) override;
	bool keyPressed(const KeyPress &k) override;

	// ================================================================================================================

private:

	void setCategory(int newCategoryIndex);
	void setPreset(int newPresetIndex);

	class CategoryList;
	class PresetList;

	int categoryIndex = -1;
	int presetIndex = -1;
	int selectedCategoryIndex = -1;

	ScopedPointer<CategoryList> categoryListModel;
	ScopedPointer<PresetList> presetListModel;

	ScopedPointer<ListBox> categoryList;
	ScopedPointer<ListBox> presetList;
	ScopedPointer<ShapeButton> closeButton;

	const UserPresetData* data;

	// ================================================================================================================

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBrowserWindow);
};


class PresetBox : public Component,
				  public SettableTooltipClient,
				  public ButtonListener,
				  public UserPresetData::Listener
{
public:
	// ================================================================================================================

	PresetBox(MainController* mc);
	~PresetBox();

	void resized() override;
	void mouseDown(const MouseEvent& event) override;

	
	void presetLoaded(int categoryIndex, int presetIndex, const String &presetName);
	void buttonClicked(Button *b) override;

private:

	// ================================================================================================================

	MainController* mc;

	const UserPresetData* data;
	
	ScopedPointer<Label> presetNameLabel;
	ScopedPointer<ShapeButton> previousButton;
	ScopedPointer<ShapeButton> nextButton;
	ScopedPointer<ShapeButton> presetSaveButton;

	// ================================================================================================================

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBox);
};




#endif  // PRESETCOMPONENTS_H_INCLUDED
