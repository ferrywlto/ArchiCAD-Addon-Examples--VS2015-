// *****************************************************************************
// Header file for the Panel Test Add-On
// API Development Kit 19; Mac/Win
//
// Namespaces:			Contact person:
//	PanelTest
//
// [SG compatible] - Yes
// *****************************************************************************

#if !defined PANELS_HPP
#define PANELS_HPP

#pragma once


// --- Includes ----------------------------------------------------------------

#include "DGModule.hpp"
#include "IAPIAttrUIData.hpp"
#include "APIAttrUIPanel.hpp"
#include "IAPIToolUIData.hpp"
#include "APIToolUIPanel.hpp"

// --- Class declarations ------------------------------------------------------

namespace PanelTest {

// --- RegData structure -------------------------------------------------------

struct WallUserData {
	double	uValue;
	char	name[32];
};


// --- TestSettingsPage --------------------------------------------------------

class TestSettingsPage: public DG::TabPage {

friend class	TestSettingsPageObserver;

private:

	enum {
		NameTextEditId			=		2,
		UValueRealEditId		=		4,
		OldHeightLengthEditId	=		6,
		NewHeightLengthEditId	=		8
	};

	DG::TextEdit		nameTextEdit;
	DG::RealEdit		uValueRealEdit;
	DG::LengthEdit		oldHeightLengthEdit;
	DG::LengthEdit		newHeightLengthEdit;

	TBUI::IAPIToolUIData* 			uiData;
public:

	 TestSettingsPage (const DG::TabControl& tabControl, TBUI::IAPIToolUIData* puiData);
	~TestSettingsPage (void);
};


// --- TestSettingsPageObserver ------------------------------------------------

class TestSettingsPageObserver:	public  DG::PanelObserver,
								public	DG::TextEditBaseObserver,
								public	DG::RealEditObserver,
								public  DG::CompoundItemObserver,
								public  TBUI::IAPIToolUIDataObserver
{
private:
	TestSettingsPage*	tabPage;

	API_Element			previousElem;

	void			GetUserDataFromDefault (WallUserData* data);
	void			SetUserDataToDefault (WallUserData* sdata);
protected:

	virtual void	PanelOpened (const DG::PanelOpenEvent& ev) override;
	virtual void	PanelResized (const DG::PanelResizeEvent& ev) override;

	virtual	void	APIElementChanged	(const TBUI::APIElemDefaultFieldMask& fieldMask) override;
	virtual void	TextEditChanged		(const DG::TextEditChangeEvent& ev) override;
	virtual void	RealEditChanged		(const DG::RealEditChangeEvent& ev) override;

public:

	explicit TestSettingsPageObserver (TestSettingsPage* testPage);
	~TestSettingsPageObserver (void);

};

// --- Wrapper class -----------------------------------------------------------

class TestSettingsPanel	:	public TBUI::APIToolUIPanel
{
private:
	TestSettingsPageObserver*	observer;

public:
				 TestSettingsPanel (Int32 refCon);
	virtual		~TestSettingsPanel ();

	virtual		bool	CreatePage (const DG::TabControl& tabControl, TBUI::IAPIToolUIData* data, DG::TabPage** tabPage) override;
	virtual		void	DestroyPage (void) override;
};


// --- TestInfoBoxPage ---------------------------------------------------------

class TestInfoBoxPage: public DG::TabPage {

friend class	TestInfoBoxPageObserver;

private:

	enum {
		NameTextEditId		=		2,
		UValueRealEditId	=		4
	};

	DG::TextEdit		nameTextEdit;
	DG::RealEdit		uValueRealEdit;

	TBUI::IAPIToolUIData* 			uiData;
public:

	 TestInfoBoxPage (const DG::TabControl& tabControl, TBUI::IAPIToolUIData* puiData);
	~TestInfoBoxPage (void);
};


// --- TestInfoBoxPageObserver -------------------------------------------------

class TestInfoBoxPageObserver:	public	DG::TextEditBaseObserver,
								public	DG::RealEditObserver,
								public  DG::CompoundItemObserver,
								public  TBUI::IAPIToolUIDataObserver
{
private:
	TestInfoBoxPage*	tabPage;

	void			GetUserDataFromDefault (WallUserData* data);
	void			SetUserDataToDefault (WallUserData* sdata);
protected:
	virtual	void	APIElementChanged	(const TBUI::APIElemDefaultFieldMask& fieldMask) override;
	virtual void	TextEditChanged		(const DG::TextEditChangeEvent& ev) override;
	virtual void	RealEditChanged		(const DG::RealEditChangeEvent& ev) override;

public:

	explicit TestInfoBoxPageObserver (TestInfoBoxPage* testPage);
	~TestInfoBoxPageObserver (void);

};

// --- Wrapper class --------------------------------------------------------

class TestInfoBoxPanel	:	public TBUI::APIToolUIPanel
{
private:
	TestInfoBoxPageObserver*	observer;

public:
				 TestInfoBoxPanel (Int32 refCon);
	virtual		~TestInfoBoxPanel ();

	virtual		bool	CreatePage (const DG::TabControl& tabControl, TBUI::IAPIToolUIData* data, DG::TabPage** tabPage) override;
	virtual		void	DestroyPage (void) override;
};

// --- RegData structure -------------------------------------------------------

struct MaterialUserData {
	double	lambdaValue;
	char	name[32];
};


// --- TestAttributePage -------------------------------------------------------

class TestAttributePage: public DG::TabPage {

friend class	TestAttributePageObserver;

private:

	enum {
		NameTextEditId			=		2,
		LambdaValueRealEditId	=		4
	};

	DG::TextEdit		nameTextEdit;
	DG::RealEdit		lambdaValueRealEdit;

	VBAD::IAPIAttrUIData* 			uiData;
public:

	 TestAttributePage (const DG::TabControl& tabControl, VBAD::IAPIAttrUIData* puiData);
	~TestAttributePage (void);
};


// --- TestAttributePageObserver -----------------------------------------------

class TestAttributePageObserver:	public  DG::PanelObserver,
									public	DG::TextEditBaseObserver,
									public	DG::RealEditObserver,
									public  DG::CompoundItemObserver,
									public  VBAD::IAPIAttrUIDataObserver
{
private:
	TestAttributePage*	tabPage;

	API_Attribute			previousAttr;

	void			GetUserDataFromAttribute (MaterialUserData* data);
	void			SetUserDataToAttribute (MaterialUserData* sdata);
protected:

	virtual void	PanelOpened (const DG::PanelOpenEvent& ev) override;
	virtual void	PanelResized (const DG::PanelResizeEvent& ev) override;

	virtual	void	APIAttributeChanged	(const VBAD::APIAttributeFieldMask& fieldMask) override;
	virtual void	TextEditChanged		(const DG::TextEditChangeEvent& ev) override;
	virtual void	RealEditChanged		(const DG::RealEditChangeEvent& ev) override;

public:

	explicit TestAttributePageObserver (TestAttributePage* testPage);
	~TestAttributePageObserver (void);

};

// --- Wrapper class -----------------------------------------------------------

class TestAttributePanel	:	public VBAD::APIAttrUIPanel
{
private:
	TestAttributePageObserver*	observer;

public:
				 TestAttributePanel (Int32 refCon);
	virtual		~TestAttributePanel ();

	virtual		bool	CreatePage (const DG::TabControl& tabControl, VBAD::IAPIAttrUIData* data, DG::TabPage** tabPage) override;
	virtual		void	DestroyPage (void) override;
};


} // namespace PanelTest


#endif
