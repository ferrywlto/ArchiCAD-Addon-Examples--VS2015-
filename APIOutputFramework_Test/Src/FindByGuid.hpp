#ifndef __FINDBYGUID_HPP__
#define __FINDBYGUID_HPP__

/**
 * @file Implements the "Find element by unique ID" command.
 */

#include "DGDialog.hpp"
#include "DGButton.hpp"
#include "DGPopUp.hpp"
#include "DGStaticItem.hpp"
#include "DGEditControl.hpp"
#include "APIdefs_Database.h"
#include "APIdefs_Elements.h"

// predeclarations
namespace GS {
	template <class Type>
	class Array;
}

/**
* Find dialog
* Switches to the database, selects the element, and zooms to it.
*/

class FindDialog : public DG::ModalDialog,
				   public DG::ButtonItemObserver,
				   public DG::PanelObserver
{
private:
	enum {
		OkButtonId		 = 1,
		CancelButtonId	 = 2,
		DatabasePopupId	 = 3,
		GuidStaticTextId = 4,
		GuidEditId		 = 5,
		SeparatorId		 = 6
	};

private:
	static const short					dialogResId = 32501;

	DG::Button							okButton;
	DG::Button							cancelButton;
	DG::PopUp							databasePopup;
	DG::LeftText						guidStaticText;
	DG::TextEdit						guidEdit;
	DG::Separator						separator;

	const GS::Array<API_DatabaseInfo>&	databaseList;
	ULong&								databaseIndex;
	API_Guid&							requestedGuid;

private:
	FindDialog& operator= (const FindDialog&);	//disabled

	void FillDatabasePopup ();

protected:
	virtual void	ButtonClicked (const DG::ButtonClickEvent& ev) override;
	virtual void	PanelClosed (const DG::PanelCloseEvent& ev) override;

public:
	FindDialog (const GS::Array<API_DatabaseInfo>& inDbList, ULong& inDbIndex, API_Guid& inRequestedGuid);
	~FindDialog ();

	static GSErrCode  FindElemByGuid (void);
    static GSErrCode  SelectAndZoom  (API_DatabaseInfo homeDB, const API_Guid& requestedGuid);
};

#endif // __FINDBYGUID_HPP__
