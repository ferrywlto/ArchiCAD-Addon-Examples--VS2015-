#ifndef __ELEMENTADAPTER_HPP__
#define __ELEMENTADAPTER_HPP__

#include "IAPIOutputAdapter.hpp"

class ElementAdapter : public GSAPI::IAPIOutputAdapter<API_Element>
{
public:
			ElementAdapter (GSAPI::IAPIOutputManager* manager, GSAPI::IAPIOutputAdapterFactory* factory);
			ElementAdapter (const ElementAdapter& adapter);
	virtual ~ElementAdapter ();

	ElementAdapter& operator= (const ElementAdapter& adapter);

	virtual void Output (GS::OChannel& oChannel) override;
	virtual void operator>> (GS::OChannel& oChannel) override;
};

#endif	// __ELEMENTADAPTER_HPP__
