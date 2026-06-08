#pragma once

#include "desc/Fields.h"

#include "definitions/enums/SignalBindingType.h"

namespace oly::editor
{
	struct SignalArrayDesc
	{


		void Reset(SignalArrayDesc& source);
		void Isolate();
	};

#define SIGNAL_ARRAY_GENERATOR(M)

	struct RouteArrayDesc
	{


		void Reset(RouteArrayDesc& source);
		void Isolate();
	};

#define ROUTE_ARRAY_GENERATOR(M)

	struct SignalFullDesc
	{
		SignalArrayDesc signals;
		static const detail::Key signals_key;
		RouteArrayDesc routes;
		static const detail::Key routes_key;

		void Reset(SignalFullDesc& source);
		void Isolate();
	};

#define SIGNAL_FULL_GENERATOR(M) \
	M(signals) \
	M(routes)
}
