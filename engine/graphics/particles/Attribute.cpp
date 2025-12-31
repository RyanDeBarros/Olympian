#include "Attribute.h"

#include "graphics/particles/distributions/AttributeOperations.h"
#include "core/algorithms/STLUtils.h"
#include "core/util/Logger.h"

namespace oly::particles
{
	Polymorphic<IAttributeOperation> IAttributeOperation::load(TOMLNode node)
	{
		std::string op = algo::to_lower(node["op"].value_or<std::string>(""));

		if (op == "sequence")
			return operations::Sequence<0>::load_fixed(node);
		else if (op == "selector")
			return operations::Selector::load(node);
		else if (op == "sinewave1d")
			return operations::SineWave1D::load(node);
		else if (op == "polarization2d")
			return operations::Polarization2D::load(node);

		_OLY_ENGINE_LOG_WARNING("ASSETS") << "Failed to load oly::particles::IAttributeOperation: missing or unrecognized 'op' field \"" << op << "\"" << LOG.nl;
		return nullptr;
	}

	namespace operations
	{
		Polymorphic<Sequence<0>> Sequence<0>::load(TOMLNode node)
		{
			if (auto arr = node["ops"].as_array())
			{
				std::vector<Polymorphic<IAttributeOperation>> ops;

				for (size_t i = 0; i < arr->size(); ++i)
				{
					if (auto subnode = arr->get(i))
						ops.push_back(IAttributeOperation::load(TOMLNode(*subnode)));
					else
						_OLY_ENGINE_LOG_WARNING("ASSETS") << "Failed to load oly::particles::Sequence sub-operation at index (" << i << ")" << LOG.nl;
				}

				return make_polymorphic<operations::Sequence<0>>(std::move(ops));
			}
			_OLY_ENGINE_LOG_WARNING("ASSETS") << "Failed to load oly::particles::operations::Sequence: missing 'ops' field." << LOG.nl;
			return nullptr;
		}

		template<size_t N>
		static Polymorphic<IAttributeOperation> load_sequence(std::vector<Polymorphic<IAttributeOperation>>&& ops)
		{
			std::array<Polymorphic<IAttributeOperation>, N> arr;
			for (size_t i = 0; i < N; ++i)
				arr[i] = std::move(ops[i]);

			return make_polymorphic<operations::Sequence<N>>(std::move(arr));
		}

		Polymorphic<IAttributeOperation> Sequence<0>::load_fixed(TOMLNode node)
		{
			if (auto arr = node["ops"].as_array())
			{
				std::vector<Polymorphic<IAttributeOperation>> ops;

				for (size_t i = 0; i < arr->size(); ++i)
					if (auto subnode = arr->get(i))
						ops.push_back(IAttributeOperation::load(TOMLNode(*subnode)));

				if (ops.size() == 1)
					return load_sequence<1>(std::move(ops));
				else if (ops.size() == 2)
					return load_sequence<2>(std::move(ops));
				else if (ops.size() == 3)
					return load_sequence<3>(std::move(ops));
				else if (ops.size() == 4)
					return load_sequence<4>(std::move(ops));
				else if (ops.size() == 5)
					return load_sequence<5>(std::move(ops));
				else if (ops.size() == 6)
					return load_sequence<6>(std::move(ops));
				else if (ops.size() == 7)
					return load_sequence<7>(std::move(ops));
				else if (ops.size() == 8)
					return load_sequence<8>(std::move(ops));
				else
					return make_polymorphic<operations::Sequence<0>>(std::move(ops));
			}
			_OLY_ENGINE_LOG_WARNING("ASSETS") << "Failed to load oly::particles::operations::Sequence: missing 'ops' field." << LOG.nl;
			return nullptr;
		}

		Polymorphic<Selector> Selector::load(TOMLNode node)
		{
			try
			{
				return make_polymorphic<operations::Selector>(IAttributeOperation::load(node["inner_op"]), SubSelector::load(node["selector"]));
			}
			catch (Error e)
			{
				_OLY_ENGINE_LOG_WARNING("ASSETS") << "Failed to load oly::particles::operations::Selector: " << (int)e.code << LOG.nl;
				return nullptr;
			}
		}
	}
}
