#pragma once

#include "core/base/SmartReference.h"

#include <imp/tree_node.hpp>

#include <map>

namespace oly::rendering
{
	// DOC for all these classes

	struct IDrawable
        : public imp::tree_node<IDrawable>
	{
		virtual ~IDrawable() = default;

    public:
		virtual void on_draw() const {}

        void draw() const;

	protected:
        virtual void draw_tree() const;
	};

	class ZLayeredDrawable
        : public IDrawable
	{
		// For z-layer to come into effect, the z-layered drawable must be a descendant of a CanvasLayerStack.
		float z_layer_ = 0.0f;

	public:
        float get_z_layer() const;
        void set_z_layer(float z);

	protected:
        virtual void on_attach(IDrawable* old_parent, IDrawable* new_parent) override;

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

        private:
			mutable LayerMap layers_;
			mutable bool dirty_ = true;

        public:
            void mark_dirty();

		protected:
			void draw_tree(const IDrawable& stack) const;

		private:
			void draw_subtree(const IDrawable& drawable) const;
			void append_layers(const IDrawable& drawable) const;

		};
	}

	template<bool ZLayer = true>
	class CanvasLayerStack
        : public DrawableBase<ZLayer>
        , public internal::CanvasLayerStackBase
	{
	protected:
		void draw_tree() const override
        {
            internal::CanvasLayerStackBase::draw_tree(*this);
        }
	};

	template<typename T, bool ZLayer = false>
	struct Drawable
        : public DrawableBase<ZLayer>
	{
		SmartReference<T> ref;

		void on_draw() const override
        {
            ref->draw();
        }

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
