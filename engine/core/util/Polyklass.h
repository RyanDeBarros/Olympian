#pragma once

#include <imp/polymorphic.hpp>

#define _OLY_POLYKLASS_IF_CASE(name)\
		if (klass == #name)\
		{\
			if (!_polyklass_obj.castable<name>())\
				_polyklass_obj = imp::make_poly<name>();\
		}

#define _OLY_POLYKLASS_ELSE_IF_CASE(name)\
		else _OLY_POLYKLASS_IF_CASE(name)

#define _OLY_POLYKLASS_CASES_BEGIN(obj) \
    auto& _polyklass_obj = obj; \
    do\
	{

#define _OLY_POLYKLASS_CASES_END \
    } while(false);
