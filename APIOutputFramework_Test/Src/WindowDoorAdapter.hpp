#ifndef __WINDOWDOORADAPTER_HPP__
#define __WINDOWDOORADAPTER_HPP__

#include "IAPIOutputAdapter.hpp"

class WindowDoorAdapter : public GSAPI::IAPIOutputAdapter<API_WindowType> {
public:
			WindowDoorAdapter (GSAPI::IAPIOutputManager* manager, GSAPI::IAPIOutputAdapterFactory* factory);
			WindowDoorAdapter (const WindowDoorAdapter& adapter);
	virtual ~WindowDoorAdapter ();

	WindowDoorAdapter& operator= (const WindowDoorAdapter& adapter);

	virtual void Output (GS::OChannel& oChannel) override;
	virtual void operator>> (GS::OChannel& oChannel) override;
};

#endif // __WINDOWDOORADAPTER_HPP__
