// *****************************************************************************
// Header file for the BuildingMaterial Dialog in DG Test Add-On
// API Development Kit 19; Mac/Win
//
// Namespaces:        Contact person:
//     -None-						LT
//
// [SG compatible] - Yes
// *****************************************************************************

#if !defined (BUILDINGMATERIALDIALOG_H)
#define BUILDINGMATERIALDIALOG_H

#pragma once

#include	"DG.h"
#include	"UC.h"
#include	"DGModule.hpp"
#include	"UCModule.hpp"
#include	"Array.hpp"


#define BUILDING_MATERIAL_DIALOG_RESID			32580


// --- BuildingMaterialDialog -------------------------------------------------

class BuildingMaterialDialog: public DG::ModalDialog
{
friend class	BuildingMaterialObserver;

private:

	enum {
		BuildingMatDrawTab		= 1,
		BuildingMatNameTab		= 2,
		BuildingMatControlTab	= 3,
		PenIndexTab				= 4,
		PenControlTab			= 5,
		NTabFields				= PenControlTab
	};

	enum {
		OkButtonId				= 1,
		BuildingMatControlId	= 2,
		PenControlId			= 3,
		ListBoxId				= 4
	};

	DG::SingleSelListBox		buildMatList;
	DG::Button					okButton;
	UC::UC257					buildingMatControl;
	UC::UC257					penControl;

	GS::Array<short>			buildMatIndexList;
	GS::Array<short>			penIndexList;

public:

	BuildingMaterialDialog	(GSResModule dialResModule, short resId);
	~BuildingMaterialDialog	();
};

// --- BuildingMaterialObserver -----------------------------------------------

class BuildingMaterialObserver:	public		DG::PanelObserver,
								public		DG::ListBoxObserver,
								public		DG::ButtonItemObserver,
								public		DG::UserControlObserver,
								public		DG::CompoundItemObserver
{
private:
	BuildingMaterialDialog*		dialog;

protected:
	virtual void	PanelOpened (const DG::PanelOpenEvent& ev) override;
	virtual void	PanelResized (const DG::PanelResizeEvent& ev) override;

	virtual void	ListBoxSelectionChanged (const DG::ListBoxSelectionEvent& ev) override;
	virtual void	ButtonClicked (const DG::ButtonClickEvent& ev) override;
	virtual void	UserControlChanged (const DG::UserControlChangeEvent& ev) override;

	virtual void	ListBoxTabFieldUpdate (const DG::ListBoxTabItemUpdateEvent& ev) override;

	void UpdateBuildingMatDrawTab (const DG::ListBoxTabItemUpdateEvent& ev);
	void UpdatePenControlTab (const DG::ListBoxTabItemUpdateEvent& ev);

public:
	explicit		BuildingMaterialObserver (BuildingMaterialDialog* testDialog);
					~BuildingMaterialObserver ();
};

#endif // BUILDINGMATERIALDIALOG_H
