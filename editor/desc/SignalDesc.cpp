#include "SignalDesc.h"

#include "definitions/Keys.h"

namespace oly::editor
{
	void SignalArrayDesc::Reset(SignalArrayDesc& source)
	{
		RESET_FIELDS(SIGNAL_ARRAY_GENERATOR);
	}

	void SignalArrayDesc::Isolate()
	{
		ISOLATE_FIELDS(SIGNAL_ARRAY_GENERATOR);
	}

	void RouteArrayDesc::Reset(RouteArrayDesc& source)
	{
		RESET_FIELDS(SIGNAL_ARRAY_GENERATOR);
	}

	void RouteArrayDesc::Isolate()
	{
		ISOLATE_FIELDS(SIGNAL_ARRAY_GENERATOR);
	}

	const detail::Key SignalFullDesc::signals_key = detail::Key::SignalArray;
	const detail::Key SignalFullDesc::routes_key = detail::Key::RoutingArray;

	void SignalFullDesc::Reset(SignalFullDesc& source)
	{
		RESET_FIELDS(SIGNAL_ARRAY_GENERATOR);
	}

	void SignalFullDesc::Isolate()
	{
		ISOLATE_FIELDS(SIGNAL_ARRAY_GENERATOR);
	}
}
