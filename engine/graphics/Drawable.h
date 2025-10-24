#pragma once

#include "core/types/SmartReference.h"
#include "core/containers/TreeNode.h"

#include <map>

namespace oly::rendering
{
	class IDrawable : public TreeNode<IDrawable>
	{
		typedef std::map<float, std::vector<const IDrawable*>> LayerMap;

	public:
		float z_order = 0.0f;

		virtual ~IDrawable() = default;

		virtual void on_draw() const {}

		void draw() const;

	private:
		LayerMap gen_layer_map() const;
		void append_layers(LayerMap& layers) const;
	};

	template<typename T>
	struct Drawable : public IDrawable
	{
		SmartReference<T> ref;

		void on_draw() const override { ref->draw(); }

		const T& operator*() const
		{
			return *ref;
		}

		T& operator*()
		{
			return *ref;
		}

		const T* operator->() const
		{
			return ref.operator->();
		}

		T* operator->()
		{
			return ref.operator->();
		}
	};
}
