// *****************************************************************************
// File:			ExecuteQueryDialog.hpp
//
// Description:		Executing an sql query to the database
//
// Project:			APITools/SQL
//
// Namespace:		SQL
//
// Contact person:	BLA
//
// SG compatible
// *****************************************************************************

#if !defined SQL_EXECUTEQUERYDIALOG_HPP_
#define SQL_EXECUTEQUERYDIALOG_HPP_

#pragma once

// --- Includes ----------------------------------------------------------------

#include "DGModule.hpp"

// --- CExecuteQueryDialog class -----------------------------------------------

namespace SQL {

class CExecuteQueryDialog : public DG::ModalDialog,
							public DG::ButtonItemObserver
{
private:
	DG::LeftText				m_static;
	DG::MultiLineEdit			m_queryEdit;
	DG::Button					m_executeButton;
	DG::Button					m_closeButton;

public:
	CExecuteQueryDialog ();
private:
	CExecuteQueryDialog (const CExecuteQueryDialog&); // Disabled
public:
	~CExecuteQueryDialog ();
// Implementation:
protected:
	void						ButtonClicked (const DG::ButtonClickEvent& ev) override;
private:
	const CExecuteQueryDialog&	operator = (const CExecuteQueryDialog&); // Disabled
};

}	// namespace SQL

#endif // !defined SQL_EXECUTEQUERYDIALOG_HPP_
