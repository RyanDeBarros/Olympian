#include "Attribute.h"

#include "graphics/particles/implementations/AttributeOperations.h"
#include "graphics/particles/implementations/AttributeOperationEnum.h"
#include "core/util/Logger.h"

#include "detail/definitions/Keys.h"

namespace oly::particles
{
	namespace internal
	{
		size_t parse_attribute_value_key()
		{
			return static_cast<size_t>(detail::Key::Value);
		}
	}

	Polymorphic<IAttributeOperation> IAttributeOperation::load(TOMLNode node)
	{
		if (auto op = assets::Parser(node).optional<particles::AttributeOperationEnum>(detail::Key::Operation)())
		{
			switch (*op)
			{
			case AttributeOperationEnum::Sequence:
				return ops::Sequence<0>::load_fixed(node);
			case AttributeOperationEnum::Selector:
				return ops::Selector::load(node);
			case AttributeOperationEnum::SineWave1D:
				return ops::SineWave1D::load(node);
			case AttributeOperationEnum::Polarization2D:
				return ops::Polarization2D::load(node);
			}
		}

		return nullptr;
	}

	namespace ops
	{
		Polymorphic<Sequence<0>> Sequence<0>::load(TOMLNode node)
		{
			auto arr = assets::Parser(node).required<TOMLArray>(detail::Key::OperationArray)();

			std::vector<Polymorphic<IAttributeOperation>> ops;

			for (size_t i = 0; i < arr->size(); ++i)
			{
				if (auto subnode = arr->get(i))
					ops.push_back(IAttributeOperation::load(TOMLNode(*subnode)));
				else
					_OLY_ENGINE_LOG_WARNING("ASSETS") << "Failed to load oly::particles::Sequence sub-operation at index (" << i << ")" << LOG.nl;
			}

			return make_polymorphic<Sequence<0>>(std::move(ops));
		}

		template<size_t N>
		static Polymorphic<IAttributeOperation> load_sequence(std::vector<Polymorphic<IAttributeOperation>>&& ops)
		{
			std::array<Polymorphic<IAttributeOperation>, N> arr;
			for (size_t i = 0; i < N; ++i)
				arr[i] = std::move(ops[i]);

			return make_polymorphic<Sequence<N>>(std::move(arr));
		}

		Polymorphic<IAttributeOperation> Sequence<0>::load_fixed(TOMLNode node)
		{
			auto arr = assets::Parser(node).required<TOMLArray>(detail::Key::OperationArray)();

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
				return make_polymorphic<Sequence<0>>(std::move(ops));
		}

		Polymorphic<Selector> Selector::load(TOMLNode node)
		{
			try
			{
				assets::Parser parser(node);
				return make_polymorphic<Selector>(IAttributeOperation::load(parser.field(detail::Key::InnerOperation)),
					parser.defaulted<particles::SubSelector>(detail::Key::Selector)());
			}
			catch (const Error& e)
			{
				_OLY_ENGINE_LOG_WARNING("ASSETS") << "Failed to load oly::particles::operations::Selector: " << (int)e.code << LOG.nl;
				return nullptr;
			}
		}
	}
}
