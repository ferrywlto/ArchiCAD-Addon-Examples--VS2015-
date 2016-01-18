#include "WallAdapter.hpp"
#include "IAttributeProxy.hpp"


WallAdapter::WallAdapter (GSAPI::IAPIOutputManager* manager, GSAPI::IAPIOutputAdapterFactory* factory) :
	GSAPI::IAPIOutputAdapter<API_WallType> (manager, factory)
{
}

WallAdapter::WallAdapter (const WallAdapter& adapter) :
	GSAPI::IAPIOutputAdapter<API_WallType> (adapter)
{
}

WallAdapter::~WallAdapter ()
{
}

WallAdapter& WallAdapter::operator= (const WallAdapter& adapter)
{
	static_cast<GSAPI::IAPIOutputAdapter<API_WallType>&> (*this) = adapter;
	return (*this);
}

void WallAdapter::Output (GS::OChannel& oChannel)
{
	if ((DBERROR (outputManager == NULL)) || (DBERROR (outputAdapterFactory == NULL)) || (DBERROR (element == NULL)) || (DBERROR (dbInfo == NULL))) {
		return;
	}

	const API_PenType& pen = attributeProxy->GetProxiedAttribute (*dbInfo, API_PenID, element->contPen).pen;
	oChannel << "Wall (guid:" << APIGuid2GSGuid (element->head.guid).ToUniString ().ToCStr () <<
				" begC:(" << element->begC.x << ", " << element->begC.y << ")" <<
				" endC:(" << element->endC.x << ", " << element->endC.y << ")" <<
				" color:(R" << static_cast<int> (pen.rgb.f_red * 255.0) <<
				", G" << static_cast<int> (pen.rgb.f_green * 255.0) <<
				", B" << static_cast<int> (pen.rgb.f_blue * 255.0) << "))\n";
}

void WallAdapter::operator>> (GS::OChannel& oChannel)
{
	Output (oChannel);
}
