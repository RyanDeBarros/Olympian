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
		// For z-layer to come into effect, the z-layered drawable must be a descendant of a CanvasLayerStack.
		float z_layer = 0.0f;

	public:
		float get_z_layer() const { return z_layer; }
		void set_z_layer(float z) { z_layer = z; flag_parent_canvas(get_parent()); }

	protected:
		virtual void on_attach(IDrawable* old_parent, IDrawable* new_parent) override { flag_parent_canvas(old_parent); flag_parent_canvas(new_parent); }

	private:
		void flag_parent_canvas(IDrawable* parent);
	};

	template<bool ZLayer>
	using DrawableBase = std::conditional_t<ZLayer, ZLayeredDrawable, IDrawable>;

	namespace internal
	{
		class CanvasLayerStackBase
		{
			typedef std::map<float, std::vector<const ZLayeredDrawable*>> LayerMap;
			mutable LayerMap layers;
			mutable bool dirty = true;

		protected:
			void draw_tree(const IDrawable& stack) const;

		private:
			void draw_subtree(const IDrawable& drawable) const;
			void append_layers(const IDrawable& drawable) const;

		public:
			void mark_dirty() { dirty = true; }
		};
	}

	template<bool ZLayer = true>
	class CanvasLayerStack : public DrawableBase<ZLayer>, public internal::CanvasLayerStackBase
	{
	protected:
		void draw_tree() const override { internal::CanvasLayerStackBase::draw_tree(*this); }
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
