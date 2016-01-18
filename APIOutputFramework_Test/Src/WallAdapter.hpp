#ifndef __WALLADAPTER_HPP__
#define __WALLADAPTER_HPP__

#include "IAPIOutputAdapter.hpp"

class WallAdapter : public GSAPI::IAPIOutputAdapter<API_WallType> {
public:
			WallAdapter (GSAPI::IAPIOutputManager* manager, GSAPI::IAPIOutputAdapterFactory* factory);
			WallAdapter (const WallAdapter& adapter);
	virtual ~WallAdapter ();

	WallAdapter& operator= (const WallAdapter& adapter);

	virtual void Output (GS::OChannel& oChannel) override;
	virtual void operator>> (GS::OChannel& oChannel) override;
};

#endif // __WALLADAPTER_HPP__
