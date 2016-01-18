#ifndef __WWOOUTPUTADAPTERFACTORY_HPP__
#define __WWOOUTPUTADAPTERFACTORY_HPP__

#include "NullTypeOutputAdapterFactory.hpp"

class WWOOutputAdapterFactory : public GSAPI::NullTypeOutputAdapterFactory
{
public:
			WWOOutputAdapterFactory (GSAPI::IAPIOutputManager& manager);
			WWOOutputAdapterFactory (const WWOOutputAdapterFactory& factory);
	virtual ~WWOOutputAdapterFactory ();

    WWOOutputAdapterFactory& operator= (const WWOOutputAdapterFactory& factory);

	virtual GSAPI::IAPIOutputAdapter<API_Element>*		GetAdapter (const API_Element&				forType,
																	const API_DatabaseInfo&			dbInfo,
																	GSAPI::IAttributeProxy*			attributeProxy	    = NULL,
																	GSAPI::OutputUsedAttributesType	attributeOutputType = GSAPI::DoNotOutput,
																	const API_ElementMemo*			withMemo            = NULL) override;

	virtual GSAPI::IAPIOutputAdapter<API_WallType>*		GetAdapter (const API_WallType&				forType,
																	const API_DatabaseInfo&			dbInfo,
																	GSAPI::IAttributeProxy*			attributeProxy	    = NULL,
																	GSAPI::OutputUsedAttributesType	attributeOutputType = GSAPI::DoNotOutput) override;

	virtual GSAPI::IAPIOutputAdapter<API_WindowType>*	GetAdapter (const API_WindowType&			forType,
																	const API_DatabaseInfo&			dbInfo,
																	GSAPI::IAttributeProxy*			attributeProxy	    = NULL,
																	GSAPI::OutputUsedAttributesType	attributeOutputType = GSAPI::DoNotOutput) override;
};

#endif	// __WWOOUTPUTADAPTERFACTORY_HPP__
