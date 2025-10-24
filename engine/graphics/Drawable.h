#pragma once

#include "core/types/SmartReference.h"
#include "core/containers/TreeNode.h"

#include <map>

namespace oly::rendering
{
	struct IDrawable : public TreeNode<IDrawable>
	{
		// DOC
		// For z-order to come into effect, the drawable must be part of a CanvasLayerStack tree.
		float z_order = 0.0f;

		virtual ~IDrawable() = default;

		virtual void on_draw() const {}

		void draw() const
		{
			draw_tree();
		}

	protected:
		virtual void draw_tree() const
		{
			on_draw();
			for (const IDrawable& d : *this)
				d.draw_tree();
		}
	};

	class CanvasLayerStack : public IDrawable
	{
		typedef std::map<float, std::vector<const IDrawable*>> LayerMap;
		mutable LayerMap layers;

	protected:
		void draw_tree() const override;

	private:
		void gen_layer_map() const;
		void append_layers(const IDrawable& drawable) const;
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
