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


FloatingFlexBoxWindow::FloatingFlexBoxWindow() :
	DocumentWindow("HISE Floating Window", Colour(0xFF333333), allButtons, true)
{
	setContentOwned(new FloatingTile(nullptr, var()), false);

	setResizable(true, true);
	setUsingNativeTitleBar(true);

	centreWithSize(1500, 1000);

	auto fsc = dynamic_cast<FloatingTile*>(getContentComponent());
	fsc->setLayoutModeEnabled(false);
	
	FloatingInterfaceBuilder ib(fsc);

	const int root = 0;

#if 0
	ib.setNewContentType<FloatingTabComponent>(root);

	const int firstVertical = ib.addChild<VerticalTile>(root);

	const int leftColumn = ib.addChild<HorizontalTile>(firstVertical);
	const int mainColumn = ib.addChild<HorizontalTile>(firstVertical);
	const int rightColumn = ib.addChild<HorizontalTile>(firstVertical);

	ib.setSizes(firstVertical, { -0.5, 900.0, -0.5 }, dontSendNotification);
	ib.setAbsoluteSize(firstVertical, { false, true, false }, dontSendNotification);
	ib.setLocked(firstVertical, { false, true, false}, sendNotification);
	ib.setDeletable(root, false, { false });
	ib.setDeletable(firstVertical, false, { false, false, false });


	const int mainArea = ib.addChild<EmptyComponent>(mainColumn);
	const int keyboard = ib.addChild<MidiKeyboardPanel>(mainColumn);

	ib.setSwappable(firstVertical, false, { false, false, false });
	ib.setSwappable(mainColumn, false, { false, false });

	ib.getPanel(firstVertical)->setDeletable(false);
	ib.getContainer(firstVertical)->setAllowInserting(false);
	ib.getPanel(mainColumn)->setReadOnly(true);
	ib.getPanel(root)->setDeletable(false);

	ib.setCustomName(firstVertical, "Main Workspace", { "Left Panel", "Mid Panel", "Right Panel" });
	

	ib.getPanel(mainColumn)->getParentContainer()->refreshLayout();

	ib.finalizeAndReturnRoot(true);
#endif

	//FloatingPanelTemplates::createMainPanel(fsc);
}

void FloatingFlexBoxWindow::closeButtonPressed()
{
	//ValueTree v = dynamic_cast<FloatingTile*>(getContentComponent())->getCurrentFloatingPanel()->exportAsValueTree();

	File f("D:\\testa.xml");

	//v.createXml()->writeToFile(f, "");

	delete this;
}

Identifier FloatingTileContent::getDefaultablePropertyId(int index) const
{
	RETURN_DEFAULT_PROPERTY_ID(index, PanelPropertyId::Type, "Type");
	RETURN_DEFAULT_PROPERTY_ID(index, PanelPropertyId::Title, "Title");
	RETURN_DEFAULT_PROPERTY_ID(index, PanelPropertyId::StyleData, "StyleData");
	RETURN_DEFAULT_PROPERTY_ID(index, PanelPropertyId::LayoutData, "LayoutData");

	jassertfalse;

	return Identifier();
}

var FloatingTileContent::getDefaultProperty(int id) const
{
	auto prop = (PanelPropertyId)id;

	switch (prop)
	{
	case FloatingTileContent::Type: return var(); // This property is not defaultable
		break;
	case FloatingTileContent::Title: return "";
	case FloatingTileContent::StyleData:
	{ 
		DynamicObject::Ptr newStyleData = new DynamicObject(); 
		return var(newStyleData);	
	}
	case FloatingTileContent::LayoutData: return var(); // this property is restored explicitely
	case FloatingTileContent::numPropertyIds:
	default:
		break;
	}

	return var();
}

BackendRootWindow* FloatingTileContent::getRootWindow()
{
	return getParentShell()->getRootWindow();
}

const BackendRootWindow* FloatingTileContent::getRootWindow() const
{
	return getParentShell()->getRootWindow();
}

var FloatingTileContent::toDynamicObject() const
{
	DynamicObject::Ptr o = new DynamicObject();

	var obj(o);

	storePropertyInObject(obj, FloatingTileContent::PanelPropertyId::Type, getIdentifierForBaseClass().toString());
	storePropertyInObject(obj, FloatingTileContent::PanelPropertyId::Title, getCustomTitle(), "");
	storePropertyInObject(obj, PanelPropertyId::StyleData, var(styleData));
	storePropertyInObject(obj, PanelPropertyId::LayoutData, var(getParentShell()->getLayoutData().getLayoutDataObject()));

	if (getFixedSizeForOrientation() != 0)
		o->removeProperty("Size");

	return obj;
}


void FloatingTileContent::fromDynamicObject(const var& object)
{
	setCustomTitle(getPropertyWithDefault(object, PanelPropertyId::Title));

	styleData = getPropertyWithDefault(object, PanelPropertyId::StyleData);

	getParentShell()->setLayoutDataObject(getPropertyWithDefault(object, PanelPropertyId::LayoutData));
}

FloatingTileContent* FloatingTileContent::createPanel(const var& data, FloatingTile* parent)
{
	if (auto obj = data.getDynamicObject())
	{
		auto panelId = Identifier(obj->getProperty("Type"));

		auto p = parent->getPanelFactory()->createFromId(panelId, parent);

		jassert(p != nullptr);

		if (p != nullptr)
		{
			//p->fromDynamicObject(data);
		}
			
		return p;
	}
	else
	{
		jassertfalse;

		return new EmptyComponent(parent);
	}
}


FloatingTileContent* FloatingTileContent::createNewPanel(const Identifier& id, FloatingTile* parent)
{
	return parent->getPanelFactory()->createFromId(id, parent);
}

void FloatingTileContent::setDynamicTitle(const String& newDynamicTitle)
{
	dynamicTitle = newDynamicTitle;
	getParentShell()->repaint();
}

const BackendProcessorEditor* FloatingTileContent::getMainPanel() const
{
#if USE_BACKEND && DONT_INCLUDE_FLOATING_LAYOUT_IN_FRONTEND
	return getParentShell()->findParentComponentOfClass<BackendRootWindow>()->getMainPanel();
#else
	return nullptr;
#endif
}

BackendProcessorEditor* FloatingTileContent::getMainPanel()
{
#if USE_BACKEND && DONT_INCLUDE_FLOATING_LAYOUT_IN_FRONTEND
	return getParentShell()->findParentComponentOfClass<BackendRootWindow>()->getMainPanel();
#else
	return nullptr;
#endif
}

int FloatingTileContent::getFixedSizeForOrientation() const
{
	auto pType = getParentShell()->getParentType();

	if (pType == FloatingTile::ParentType::Horizontal)
		return getFixedWidth();
	else if (pType == FloatingTile::ParentType::Vertical)
		return getFixedHeight();
	else
		return 0;
}

#if 0

FloatingPanel* FloatingPanel::createPanel(ValueTree& state, FloatingShellComponent* parent)
{
	auto pt = getPanelTypeForId(Identifier(state.getType()));

	FloatingPanel* p = createNewPanel(pt, parent);

	if (p != nullptr)
		p->restoreFromValueTree(state);

	return p;
}

FloatingPanel* FloatingPanel::Factory::createNewPanel(PanelType pt, FloatingShellComponent* parent)
{
	FloatingPanel* p = nullptr;

	switch (pt)
	{
	case PanelType::PTHorizontalTile: p = new HorizontalTile(parent); break;
	case PanelType::PTVerticalTile:	p = new VerticalTile(parent); break;
	case PanelType::PTTabs:			p = new FloatingTabComponent(parent); break;
	case PanelType::PTNote:			p = new Note(parent); break;
	}

	if (p == nullptr)
		return new EmptyComponent(parent);

	return p;
}
#endif

struct FloatingPanelTemplates::Helpers
{
	static void addNewShellTo(FloatingTileContainer* parent)
	{
		parent->addFloatingTile(new FloatingTile(parent));
	}

	static FloatingTileContainer* getChildContainer(FloatingTile* parent)
	{
		return dynamic_cast<FloatingTileContainer*>(parent->getCurrentFloatingPanel());
	}

	template <class PanelType> static void setContent(FloatingTile* parent)
	{
		parent->setNewContent(PanelType::getPanelId());
	};

	static FloatingTile* getChildShell(FloatingTile* parent, int index)
	{
		auto c = dynamic_cast<FloatingTileContainer*>(parent->getCurrentFloatingPanel());

		jassert(c != nullptr);

		return c->getComponent(index);
	}

	static void setShellSize(FloatingTileContainer* parent, int index, double size, bool isRelative)
	{
		if (isRelative)
			size *= -1.0;

		parent->getComponent(index)->getLayoutData().setCurrentSize(size);
	}
};



void FloatingPanelTemplates::create2x2Matrix(FloatingTile* parent)
{
	Helpers::setContent<HorizontalTile>(parent);
	Helpers::addNewShellTo(Helpers::getChildContainer(parent));

	Helpers::setContent<VerticalTile>(Helpers::getChildShell(parent, 0));
	Helpers::setContent<VerticalTile>(Helpers::getChildShell(parent, 1));

	auto v1 = Helpers::getChildContainer(Helpers::getChildShell(parent, 0));
	auto v2 = Helpers::getChildContainer(Helpers::getChildShell(parent, 1));

	Helpers::addNewShellTo(v1);
	Helpers::addNewShellTo(v2);
}

void FloatingPanelTemplates::create3Columns(FloatingTile* parent)
{
	Helpers::setContent<VerticalTile>(parent);

	auto c = Helpers::getChildContainer(parent);

	Helpers::addNewShellTo(c);
	Helpers::addNewShellTo(c);
}

void FloatingPanelTemplates::create3Rows(FloatingTile* parent)
{
	Helpers::setContent<HorizontalTile>(parent);

	auto c = Helpers::getChildContainer(parent);

	Helpers::addNewShellTo(c);
	Helpers::addNewShellTo(c);
}



Component* FloatingPanelTemplates::createHiseLayout(FloatingTile* rootTile)
{
#if USE_BACKEND
	rootTile->setLayoutModeEnabled(false);

	FloatingInterfaceBuilder ib(rootTile);

	const int root = 0;

	ib.setNewContentType<HorizontalTile>(root);

	const int topBar = ib.addChild<MainTopBar>(root);

	ib.getContainer(root)->setIsDynamic(false);

	const int personaContainer = ib.addChild<VerticalTile>(root);
	ib.getContainer(personaContainer)->setIsDynamic(true);



	ib.setFoldable(root, false, { true, false });
	ib.setId(personaContainer, "PersonaContainer");

	ib.getContent(root)->setStyleColour(ResizableFloatingTileContainer::ColourIds::backgroundColourId,
		HiseColourScheme::getColour(HiseColourScheme::ColourIds::EditorBackgroundColourIdBright));

	auto mainPanel = createMainPanel(ib.getPanel(personaContainer));

	ib.getContainer(personaContainer)->setIsDynamic(false);




	File scriptJSON = File(PresetHandler::getDataFolder()).getChildFile("Workspaces/ScriptingWorkspace.json");
	File sampleJSON = File(PresetHandler::getDataFolder()).getChildFile("Workspaces/SamplerWorkspace.json");

	var scriptData = JSON::parse(scriptJSON.loadFileAsString());
	var sampleData = JSON::parse(sampleJSON.loadFileAsString());



	createScriptingWorkspace(ib.getPanel(personaContainer));

	//ib.getPanel(scriptPanel)->setContent(scriptData);

	//jassert(!ib.getPanel(scriptPanel)->isEmpty());



	//auto samplePanel = ib.addChild < EmptyComponent>(personaContainer);

	createSamplerWorkspace(ib.getPanel(personaContainer));

	//ib.getPanel(samplePanel)->setContent(sampleData);
	//ib.getPanel(samplePanel)->getLayoutData().setId("SamplerWorkspace");



	const int customPanel = ib.addChild<HorizontalTile>(personaContainer);
	ib.getContent(customPanel)->setCustomTitle("Custom Workspace");
	ib.getPanel(customPanel)->getLayoutData().setId("CustomWorkspace");
	ib.getContainer(customPanel)->setIsDynamic(true);
	ib.addChild<EmptyComponent>(customPanel);

	return mainPanel;
#else
	return rootTile;
#endif
}



Component* FloatingPanelTemplates::createSamplerWorkspace(FloatingTile* rootTile)
{
#if USE_BACKEND
	MainController* mc = rootTile->findParentComponentOfClass<BackendRootWindow>()->getBackendProcessor();

	jassert(mc != nullptr);

	FloatingInterfaceBuilder ib(rootTile);

	const int personaContainer = 0;

	const int samplePanel = ib.addChild <VerticalTile>(personaContainer);
	ib.setDynamic(samplePanel, false);
	ib.setId(samplePanel, "SamplerWorkspace");

	const int toggleBar = ib.addChild<VisibilityToggleBar>(samplePanel);
	const int fileBrowser = ib.addChild<GenericPanel<FileBrowser>>(samplePanel);
	const int samplePoolTable = ib.addChild<GenericPanel<SamplePoolTable>>(samplePanel);
    
    ib.getPanel(samplePoolTable)->getLayoutData().setVisible(false);
    
	const int sampleHorizontal = ib.addChild<HorizontalTile>(samplePanel);
	ib.setDynamic(sampleHorizontal, false);
	ib.addChild<GlobalConnectorPanel<ModulatorSampler>>(sampleHorizontal);
	const int sampleEditor = ib.addChild<SampleEditorPanel>(sampleHorizontal);
	const int sampleVertical = ib.addChild<VerticalTile>(sampleHorizontal);
	ib.setDynamic(sampleVertical, false);
    
    
    
	const int sampleMapEditor = ib.addChild<SampleMapEditorPanel>(sampleVertical);
	const int samplerTable = ib.addChild<SamplerTablePanel>(sampleVertical);

    ib.setSizes(sampleVertical, {-0.7, -0.3});
    
	ib.setSizes(samplePanel, { 32.0, 280.0, 280.0, -0.5 });
	ib.getPanel(sampleHorizontal)->setCustomIcon((int)FloatingTileContent::Factory::PopupMenuOptions::SampleEditor);

	auto tb = ib.getContent<VisibilityToggleBar>(toggleBar);

	ib.setId(fileBrowser, "SampleFileBrowser");
	ib.setId(samplePoolTable, "SamplePoolTable");
	ib.setId(sampleEditor, "MainSampleEditor");
	ib.setId(sampleMapEditor, "MainSampleMapEditor");
	ib.setId(samplerTable, "MainSamplerTable");
	
	ib.setCustomPanels(toggleBar, { fileBrowser, samplePoolTable, sampleEditor, sampleMapEditor, samplerTable });
	ib.getPanel(toggleBar)->setCanBeFolded(false);

	ib.getContent<FloatingTileContent>(sampleEditor)->setStyleProperty("showConnectionBar", false);
	ib.getContent<FloatingTileContent>(sampleMapEditor)->setStyleProperty("showConnectionBar", false);
	ib.getContent<FloatingTileContent>(samplerTable)->setStyleProperty("showConnectionBar", false);
#endif

	return nullptr;
}

Component* FloatingPanelTemplates::createScriptingWorkspace(FloatingTile* rootTile)
{
#if USE_BACKEND
	MainController* mc = rootTile->findParentComponentOfClass<BackendRootWindow>()->getBackendProcessor();

	jassert(mc != nullptr);


	FloatingInterfaceBuilder ib(rootTile);

	const int personaContainer = 0;

	const int scriptPanel = ib.addChild <HorizontalTile>(personaContainer);
	ib.setDynamic(scriptPanel, false);

	ib.getPanel(scriptPanel)->getLayoutData().setId("ScriptingWorkspace");

	const int globalConnector = ib.addChild<GlobalConnectorPanel<JavascriptProcessor>>(scriptPanel);

	const int mainVertical = ib.addChild<VerticalTile>(scriptPanel);
	ib.setDynamic(mainVertical, false);

	const int toggleBar = ib.addChild < VisibilityToggleBar>(mainVertical);
	const int apiCollection = ib.addChild<GenericPanel<ApiCollection>>(mainVertical);
	const int fileBrowser = ib.addChild<GenericPanel<FileBrowser>>(mainVertical);
	const int codeEditor = ib.addChild<HorizontalTile>(mainVertical);
	ib.setDynamic(codeEditor, false);


	const int codeVertical = ib.addChild<VerticalTile>(codeEditor);
	ib.setDynamic(codeVertical, false);
	const int codeTabs = ib.addChild<FloatingTabComponent>(codeVertical);
	const int firstEditor = ib.addChild<CodeEditorPanel>(codeTabs);
	const int variableWatch = ib.addChild<ScriptWatchTablePanel>(codeVertical);

	const int console = ib.addChild<ConsolePanel>(codeEditor);
	ib.setCustomName(codeEditor, "Code Editor");
	ib.setSizes(codeEditor, { -0.75, -0.25 });
	ib.setSizes(codeVertical, { -0.8, -0.2 });

	const int interfaceDesigner = ib.addChild <VerticalTile>(mainVertical);
	ib.setDynamic(interfaceDesigner, false);
	const int interfaceHorizontal = ib.addChild<HorizontalTile>(interfaceDesigner);
	ib.setDynamic(interfaceHorizontal, false);
	const int interfacePanel = ib.addChild<ScriptContentPanel>(interfaceHorizontal);
	const int onInitPanel = ib.addChild<CodeEditorPanel>(interfaceHorizontal);
	const int keyboard = ib.addChild<MidiKeyboardPanel>(interfaceHorizontal);
	ib.getPanel(keyboard)->getLayoutData().setForceFoldButton(true);

	ib.addChild<GenericPanel<ScriptComponentEditPanel>>(interfaceDesigner);

	ib.setSizes(interfaceHorizontal, { -0.7, -0.3, 72.0 });
	ib.setCustomName(interfaceHorizontal, "", { "Interface", "onInit Callback", "" });

	ib.setCustomName(interfaceDesigner, "Interface Designer");
    
    ib.setId(toggleBar, "ScriptingWorkspaceToggleBar");
    
    ib.setId(codeEditor, "ScriptingWorkspaceCodeEditor");
    ib.setId(interfaceDesigner, "ScriptingWorkspaceInterfaceDesigner");
    
	ib.setSizes(interfaceDesigner, { -0.8, -0.2 });

	ib.setFoldable(mainVertical, false, { false, true, true, true, true });
	ib.setFoldable(interfaceHorizontal, false, { false, true, true });

	ib.setSizes(mainVertical, { 32.0, 300.0, 300.0, -0.5, -0.5 });

    ib.getPanel(interfaceDesigner)->getLayoutData().setVisible(false);
    ib.getPanel(apiCollection)->getLayoutData().setVisible(false);
    ib.getPanel(fileBrowser)->getLayoutData().setVisible(false);
    
	ib.getContent<VisibilityToggleBar>(toggleBar)->refreshButtons();

	ib.getContent<FloatingTileContent>(onInitPanel)->setStyleProperty("showConnectionBar", false);
	ib.getContent<FloatingTileContent>(interfacePanel)->setStyleProperty("showConnectionBar", false);
	ib.getContent<FloatingTileContent>(variableWatch)->setStyleProperty("showConnectionBar", false);
	//ib.getContent<FloatingTileContent>(onInitPanel)->setStyleProperty("showConnectionBar", false);

	return ib.getPanel(scriptPanel);
#else
	return nullptr;
#endif
}





Component* FloatingPanelTemplates::createMainPanel(FloatingTile* rootTile)
{
#if USE_BACKEND
	MainController* mc = rootTile->findParentComponentOfClass<BackendRootWindow>()->getBackendProcessor();

	jassert(mc != nullptr);


	FloatingInterfaceBuilder ib(rootTile);

	const int personaContainer = 0;

	const int firstVertical = ib.addChild<VerticalTile>(personaContainer);

	ib.getContainer(firstVertical)->setIsDynamic(false);
	ib.getPanel(firstVertical)->setVital(true);
	ib.setId(firstVertical, "MainWorkspace");

	ib.getContent(firstVertical)->setStyleColour(ResizableFloatingTileContainer::ColourIds::backgroundColourId,
		HiseColourScheme::getColour(HiseColourScheme::ColourIds::EditorBackgroundColourIdBright));

	const int leftColumn = ib.addChild<HorizontalTile>(firstVertical);
	const int mainColumn = ib.addChild<HorizontalTile>(firstVertical);
	const int rightColumn = ib.addChild<HorizontalTile>(firstVertical);

	ib.getContainer(leftColumn)->setIsDynamic(true);
	ib.getContainer(mainColumn)->setIsDynamic(false);
	ib.getContainer(rightColumn)->setIsDynamic(false);

	ib.getContent(leftColumn)->setStyleColour(ResizableFloatingTileContainer::ColourIds::backgroundColourId, Colour(0xFF222222));
	ib.getPanel(leftColumn)->getLayoutData().setMinSize(150);
	ib.getPanel(leftColumn)->setCanBeFolded(true);
	ib.setId(leftColumn, "MainLeftColumn");

	ib.getContent(mainColumn)->setStyleColour(ResizableFloatingTileContainer::ColourIds::backgroundColourId,
		HiseColourScheme::getColour(HiseColourScheme::ColourIds::EditorBackgroundColourIdBright));


	ib.getContent(rightColumn)->setStyleColour(ResizableFloatingTileContainer::ColourIds::backgroundColourId, Colour(0xFF222222));
	ib.getPanel(rightColumn)->getLayoutData().setMinSize(150);
	ib.getPanel(rightColumn)->setCanBeFolded(true);
	ib.setId(rightColumn, "MainRightColumn");

	const int rightToolBar = ib.addChild<VisibilityToggleBar>(rightColumn);
	ib.getPanel(rightToolBar)->getLayoutData().setCurrentSize(28);
	ib.getContent(rightToolBar)->setStyleColour(VisibilityToggleBar::ColourIds::backgroundColour, Colour(0xFF222222));

	const int macroTable = ib.addChild<GenericPanel<MacroParameterTable>>(rightColumn);
	ib.getPanel(macroTable)->setCloseTogglesVisibility(true);
	ib.setId(macroTable, "MainMacroTable");

	const int scriptWatchTable = ib.addChild<ScriptWatchTablePanel>(rightColumn);
	ib.getPanel(scriptWatchTable)->setCloseTogglesVisibility(true);

	const int editPanel = ib.addChild<GenericPanel<ScriptComponentEditPanel>>(rightColumn);
	ib.getPanel(editPanel)->setCloseTogglesVisibility(true);

	const int plotter = ib.addChild<PlotterPanel>(rightColumn);
	ib.getPanel(plotter)->getLayoutData().setCurrentSize(300.0);
	ib.getPanel(plotter)->setCloseTogglesVisibility(true);

	const int console = ib.addChild<ConsolePanel>(rightColumn);
	
	ib.setId(console, "MainConsole");


	ib.getPanel(console)->setCloseTogglesVisibility(true);

	ib.setVisibility(rightColumn, true, { true, false, false, false, false, false });

	ib.setSizes(firstVertical, { -0.5, 900.0, -0.5 }, dontSendNotification);


	const int mainArea = ib.addChild<BackendProcessorEditor>(mainColumn);

	

	const int keyboard = ib.addChild<MidiKeyboardPanel>(mainColumn);


	ib.getContent(keyboard)->setStyleColour(ResizableFloatingTileContainer::ColourIds::backgroundColourId,
		HiseColourScheme::getColour(HiseColourScheme::ColourIds::EditorBackgroundColourIdBright));

	ib.setFoldable(mainColumn, true, { false, true });
	ib.getPanel(mainColumn)->getLayoutData().setForceFoldButton(true);
	ib.getPanel(keyboard)->getLayoutData().setForceFoldButton(true);
	ib.getPanel(keyboard)->resized();

	ib.setCustomName(firstVertical, "", { "Left Panel", "", "Right Panel" });

	
	ib.getPanel(mainArea)->getLayoutData().setId("MainColumn");


	ib.getContent<VisibilityToggleBar>(rightToolBar)->refreshButtons();

	ib.finalizeAndReturnRoot();

	jassert(BackendPanelHelpers::getMainTabComponent(rootTile) != nullptr);
	jassert(BackendPanelHelpers::getMainLeftColumn(rootTile) != nullptr);
	jassert(BackendPanelHelpers::getMainRightColumn(rootTile) != nullptr);


	return dynamic_cast<Component*>(ib.getPanel(mainArea)->getCurrentFloatingPanel());
#else
	return nullptr;
#endif
}



void JSONEditor::replace()
{
	if (editedComponent.getComponent() != nullptr)
	{
		var newData;

		auto result = JSON::parse(doc->getAllContent(), newData);

		if (result.wasOk())
		{
			dynamic_cast<ObjectWithDefaultProperties*>(editedComponent.getComponent())->fromDynamicObject(newData);

			auto parent = dynamic_cast<FloatingTileContent*>(editedComponent.getComponent())->getParentShell();

			jassert(parent != nullptr);

			parent->refreshRootLayout();
			parent->refreshPinButton();
			parent->refreshFoldButton();
			parent->refreshMouseClickTarget();

			editedComponent->repaint();
		}
		else
		{
			PresetHandler::showMessageWindow("JSON Parser Error", result.getErrorMessage(), PresetHandler::IconType::Error);
		}
	}
}
