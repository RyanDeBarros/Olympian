#pragma once

#include "core/types/SmartReference.h"
#include "core/containers/TreeNode.h"

namespace oly::rendering
{
	struct IDrawable : public TreeNode<IDrawable>
	{
		virtual ~IDrawable() = default;

		virtual void on_draw() const {}

		void draw() const
		{
			// TODO v5 Add z_index for customized draw ordering.
			on_draw();
			for (auto it = begin(); it != end(); ++it)
				it->on_draw();
		}
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
