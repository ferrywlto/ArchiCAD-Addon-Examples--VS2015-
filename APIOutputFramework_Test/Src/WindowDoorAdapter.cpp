#include "WindowDoorAdapter.hpp"
#include "IAttributeProxy.hpp"


WindowDoorAdapter::WindowDoorAdapter (GSAPI::IAPIOutputManager* manager, GSAPI::IAPIOutputAdapterFactory* factory) :
	GSAPI::IAPIOutputAdapter<API_WindowType> (manager, factory)
{
}

WindowDoorAdapter::WindowDoorAdapter (const WindowDoorAdapter& adapter) :
	GSAPI::IAPIOutputAdapter<API_WindowType> (adapter)
{
}

WindowDoorAdapter::~WindowDoorAdapter ()
{
}

WindowDoorAdapter& WindowDoorAdapter::operator= (const WindowDoorAdapter& adapter)
{
	static_cast<GSAPI::IAPIOutputAdapter<API_WindowType>&> (*this) = adapter;
	return (*this);
}

void WindowDoorAdapter::Output (GS::OChannel& oChannel)
{
	if ((DBERROR (outputManager == NULL)) || (DBERROR (outputAdapterFactory == NULL)) || (DBERROR (element == NULL)) || (DBERROR (dbInfo == NULL))) {
		return;
	}

	const API_PenType& pen = attributeProxy->GetProxiedAttribute (*dbInfo, API_PenID, element->openingBase.sectContPen).pen;
	oChannel << "\t";
	if (element->head.typeID == API_WindowID){
		oChannel << "Window (";
	} else {
		oChannel << "Door (";
	}
	oChannel << "guid:" << APIGuid2GSGuid (element->head.guid).ToUniString ().ToCStr () <<
				" location:" << element->objLoc <<
				" width:" << element->openingBase.width <<
				" height:" << element->openingBase.height <<
				" color:(R" << static_cast<int> (pen.rgb.f_red * 255.0) <<
				", G" << static_cast<int> (pen.rgb.f_green * 255.0) <<
				", B" << static_cast<int> (pen.rgb.f_blue * 255.0) << "))\n";
}

void WindowDoorAdapter::operator>> (GS::OChannel& oChannel)
{
	Output (oChannel);
}
