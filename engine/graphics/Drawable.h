#pragma once

#include "core/types/SmartReference.h"
#include "core/containers/TreeNode.h"

#include <map>

namespace oly::rendering
{
	// DOC for all these classes

	struct IDrawable : public TreeNode<IDrawable>
	{
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

	class ZLayeredDrawable : public IDrawable
	{
		// For z-layer to come into effect, the drawable must be part of a CanvasLayerStack tree.
		float z_layer = 0.0f;

	public:
		float get_z_layer() const { return z_layer; }
		void set_z_layer(float z) { z_layer = z; } // TODO v5 flag canvases - also in copy/move semantics
	};

	template<bool ZLayer>
	using DrawableBase = std::conditional_t<ZLayer, ZLayeredDrawable, IDrawable>;

	class CanvasLayerStack : public IDrawable
	{
		typedef std::map<float, std::vector<const ZLayeredDrawable*>> LayerMap; // TODO v5 proper memory management with connecting/disconnecting ZLayeredDrawables
		mutable LayerMap layers;
		mutable bool dirty = true;

	protected:
		void draw_tree() const override;

	public:
		void re_calculate_z_layers() const { dirty = true; }

	private:
		void draw_subtree(const IDrawable& drawable) const;
		void gen_layer_map() const;
		void append_layers(const IDrawable& drawable) const;
	};

	template<typename T, bool ZLayer = false>
	struct Drawable : public DrawableBase<ZLayer>
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
