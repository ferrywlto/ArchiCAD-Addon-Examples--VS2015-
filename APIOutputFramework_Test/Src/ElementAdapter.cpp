#include "ElementAdapter.hpp"
#include "IAPIOutputManager.hpp"
#include "IAPIOutputAdapterFactory.hpp"


ElementAdapter::ElementAdapter (GSAPI::IAPIOutputManager* manager, GSAPI::IAPIOutputAdapterFactory* factory) :
	GSAPI::IAPIOutputAdapter<API_Element> (manager, factory)
{
}

ElementAdapter::ElementAdapter (const ElementAdapter& adapter) :
	GSAPI::IAPIOutputAdapter<API_Element> (adapter)
{
}

ElementAdapter::~ElementAdapter ()
{
}

ElementAdapter& ElementAdapter::operator= (const ElementAdapter& adapter)
{
	static_cast<GSAPI::IAPIOutputAdapter<API_Element>&> (*this) = adapter;
	return (*this);
}

void ElementAdapter::Output (GS::OChannel& oChannel)
{
	if ((DBERROR (outputManager == NULL)) || (DBERROR (outputAdapterFactory == NULL)) || (DBERROR (element == NULL)) || (DBERROR (dbInfo == NULL))) {
		return;
	}

	switch (element->header.typeID) {
        case API_WallID:
			{
				GSAPI::IAPIOutputAdapter<API_WallType>* wallAdapter = outputAdapterFactory->GetAdapter (element->wall, *dbInfo, attributeProxy, attributeOutputType);
				*wallAdapter >> oChannel;
			}
			break;

		case API_WindowID:
		case API_DoorID:
			{
				GSAPI::IAPIOutputAdapter<API_WindowType>* windowAdapter = outputAdapterFactory->GetAdapter (element->window, *dbInfo, attributeProxy, attributeOutputType);
				*windowAdapter >> oChannel;
			}
			break;

        default:                    break;
	}
}

void ElementAdapter::operator>> (GS::OChannel& oChannel)
{
	Output (oChannel);
}
