#include "WWOOutputAdapterFactory.hpp"
#include "ElementAdapter.hpp"
#include "WallAdapter.hpp"
#include "WindowDoorAdapter.hpp"

WWOOutputAdapterFactory::WWOOutputAdapterFactory (GSAPI::IAPIOutputManager& manager) :
	NullTypeOutputAdapterFactory (manager)
{
}

WWOOutputAdapterFactory::WWOOutputAdapterFactory (const WWOOutputAdapterFactory& factory) :
	NullTypeOutputAdapterFactory (factory)
{
}

WWOOutputAdapterFactory::~WWOOutputAdapterFactory ()
{
}

WWOOutputAdapterFactory& WWOOutputAdapterFactory::operator= (const WWOOutputAdapterFactory& factory)
{
	NullTypeOutputAdapterFactory::operator= (factory); return (*this);
}


GSAPI::IAPIOutputAdapter<API_Element>* WWOOutputAdapterFactory::GetAdapter (const API_Element&				forType,
																			const API_DatabaseInfo&			dbInfo,
																			GSAPI::IAttributeProxy*			attributeProxy	    /*= NULL*/,
																			GSAPI::OutputUsedAttributesType	attributeOutputType /*= GSAPI::DoNotOutput*/,
																			const API_ElementMemo*			/*withMemo            = NULL*/)
{
	ElementAdapter* adapter = new ElementAdapter (&outputManager, this);
	adapter->SetParameters (&forType, &dbInfo, attributeProxy, attributeOutputType);
	return adapter;
}

GSAPI::IAPIOutputAdapter<API_WallType>* WWOOutputAdapterFactory::GetAdapter (const API_WallType&				forType,
																			 const API_DatabaseInfo&			dbInfo,
																			 GSAPI::IAttributeProxy*			attributeProxy	    /*= NULL*/,
																			 GSAPI::OutputUsedAttributesType	attributeOutputType /*= GSAPI::DoNotOutput*/)
{
	WallAdapter* adapter = new WallAdapter (&outputManager, this);
	adapter->SetParameters (&forType, &dbInfo, attributeProxy, attributeOutputType);
	return adapter;
}

GSAPI::IAPIOutputAdapter<API_WindowType>* WWOOutputAdapterFactory::GetAdapter (const API_WindowType&			forType,
																			   const API_DatabaseInfo&			dbInfo,
																			   GSAPI::IAttributeProxy*			attributeProxy	    /*= NULL*/,
																			   GSAPI::OutputUsedAttributesType	attributeOutputType /*= GSAPI::DoNotOutput*/)
{
	WindowDoorAdapter* adapter = new WindowDoorAdapter (&outputManager, this);
	adapter->SetParameters (&forType, &dbInfo, attributeProxy, attributeOutputType);
	return adapter;
}
