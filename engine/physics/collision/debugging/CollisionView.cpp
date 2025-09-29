#include "CollisionView.h"

namespace oly::debug
{
	CollisionObject::CollisionObject(CollisionLayer& layer, Variant&& v)
		: layer(layer)
	{
		init((Type)v.index());
		*this->v = std::move(v);
	}

	CollisionObject::CollisionObject(const CollisionObject& other)
		: layer(other.layer), v(std::make_unique<Variant>(*other.v))
	{
	}

	CollisionObject::CollisionObject(CollisionObject&& other) noexcept
		: layer(other.layer), v(std::move(other.v))
	{
	}

	CollisionObject& CollisionObject::operator=(const CollisionObject& other)
	{
		if (this != &other)
		{
			if (v->index() != other.v->index() && &layer != &other.layer)
				init((Type)other.v->index());
			*v = *other.v;
		}
		return *this;
	}

	CollisionObject& CollisionObject::operator=(CollisionObject&& other) noexcept
	{
		if (this != &other)
		{
			if (&layer == &other.layer)
				v = std::move(other.v);
			else
			{
				if (v->index() != other.v->index())
					init((Type)other.v->index());
				*v = std::move(*other.v);
			}
		}
		return *this;
	}

	void CollisionObject::init(CollisionObject::Type type)
	{
		switch (type)
		{
		case Type::ELLIPSE:
			v = std::make_unique<Variant>(layer.create_ellipse());
			break;
		case Type::POLYGON:
			v = std::make_unique<Variant>(layer.create_polygon());
			break;
		case Type::ARROW:
			v = std::make_unique<Variant>(layer.create_arrow());
			break;
		}
	}

	void CollisionObject::draw(rendering::GeometryPainter::PaintSupport& ps) const
	{
		std::visit([&ps](const auto& v) {
			if constexpr (visiting_class_is<decltype(v), rendering::EllipseReference>)
			{
				ps.pre_ellipse_draw();
				v.draw();
			}
			else if constexpr (visiting_class_is<decltype(v), rendering::StaticPolygon, rendering::StaticArrowExtension>)
			{
				ps.pre_polygon_draw();
				v.draw();
			}
			}, *v);
	}

	CollisionView::CollisionView(CollisionLayer& layer)
		: layer(&layer), obj(std::make_unique<CollisionObjectView>(EmptyCollision{}))
	{
		layer.assign(this);
	}

	CollisionView::CollisionView(CollisionLayer& layer, rendering::EllipseReference&& obj)
		: layer(&layer), obj(std::make_unique<CollisionObjectView>(CollisionObject(layer, std::move(obj))))
	{
		layer.assign(this);
	}

	CollisionView::CollisionView(CollisionLayer& layer, rendering::StaticPolygon&& obj)
		: layer(&layer), obj(std::make_unique<CollisionObjectView>(CollisionObject(layer, std::move(obj))))
	{
		layer.assign(this);
	}

	CollisionView::CollisionView(CollisionLayer& layer, rendering::StaticArrowExtension&& obj)
		: layer(&layer), obj(std::make_unique<CollisionObjectView>(CollisionObject(layer, std::move(obj))))
	{
		layer.assign(this);
	}

	CollisionView::CollisionView(const CollisionView& other)
		: layer(other.layer), obj(other.valid() ? std::make_unique<CollisionObjectView>(*other.obj) : nullptr)
	{
		if (valid())
			layer->assign(this);
	}

	CollisionView::CollisionView(CollisionView&& other) noexcept
		: layer(other.layer), obj(std::move(other.obj))
	{
		if (valid())
		{
			layer->collision_views.erase(&other);
			layer->collision_views.insert(this);
		}
	}

	CollisionView::~CollisionView()
	{
		if (valid())
			layer->unassign(this);
	}

	CollisionView& CollisionView::operator=(const CollisionView& other)
	{
		if (this != &other)
		{
			obj = other.valid() ? std::make_unique<CollisionObjectView>(*other.obj) : nullptr;
			view_changed();
		}
		return *this;
	}

	CollisionView& CollisionView::operator=(CollisionView&& other) noexcept
	{
		if (this != &other)
		{
			obj = std::move(other.obj);
			view_changed();
		}
		return *this;
	}

	void CollisionView::init_on_layer(CollisionLayer& layer)
	{
		if (valid())
		{
			if (this->layer == &layer)
				return;

			this->layer->unassign(this);
			this->layer = &layer;
			this->layer->assign(this);
			if (obj->view.index() == CollisionObjectView::Type::SINGLE)
			{
				CollisionObject old_obj = std::move(std::get<CollisionObjectView::Type::SINGLE>(obj->view));
				obj = std::make_unique<CollisionObjectView>(CollisionObject(layer, std::move(*old_obj.v)));
			}
			else if (obj->view.index() == CollisionObjectView::Type::GROUP)
			{
				std::vector<CollisionObject> old_objs = std::move(std::get<CollisionObjectView::Type::GROUP>(obj->view));
				std::vector<CollisionObject> new_objs = CollisionObjectGroup(old_objs.size(), layer.default_collision_object());
				for (size_t i = 0; i < new_objs.size(); ++i)
					new_objs[i] = std::move(old_objs[i]);
				obj = std::make_unique<CollisionObjectView>(std::move(new_objs));
			}
		}
		else
		{
			this->layer = &layer;
			this->layer->assign(this);
			obj = std::make_unique<CollisionObjectView>(EmptyCollision{});
		}
	}

	const CollisionLayer& CollisionView::get_layer() const
	{
		if (valid())
			return *layer;
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	CollisionLayer& CollisionView::get_layer()
	{
		if (valid())
			return *layer;
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void CollisionView::draw(rendering::GeometryPainter::PaintSupport& ps) const
	{
		// TODO v5 specialized shader for collision view, especially since it's rendered on a separate framebuffer.
		if (valid())
		{
			std::visit([&ps](const auto& view) {
				if constexpr (visiting_class_is<decltype(view), CollisionObject>)
					view.draw(ps);
				else if constexpr (visiting_class_is<decltype(view), CollisionObjectGroup>)
					for (const auto& v : view)
						v.draw(ps);
				}, obj->view);
		}
	}

	void CollisionView::clear_view()
	{
		if (valid())
		{
			obj->view = EmptyCollision{};
			view_changed();
		}
	}

	void CollisionView::set_view(CollisionObjectView&& obj)
	{
		if (valid())
		{
			*this->obj = std::move(obj);
			view_changed();
		}
	}

	void CollisionView::set_view(rendering::EllipseReference&& obj)
	{
		if (valid())
			set_view(CollisionObjectView(CollisionObject(*layer, std::move(obj))));
	}

	void CollisionView::set_view(rendering::StaticPolygon&& obj)
	{
		if (valid())
			set_view(CollisionObjectView(CollisionObject(*layer, std::move(obj))));
	}

	void CollisionView::set_view(rendering::StaticArrowExtension&& obj)
	{
		if (valid())
			set_view(CollisionObjectView(CollisionObject(*layer, std::move(obj))));
	}

	size_t CollisionView::view_size() const
	{
		if (valid())
		{
			if (obj->view.index() == CollisionObjectView::Type::EMPTY)
				return 0;
			else if (obj->view.index() == CollisionObjectView::Type::SINGLE)
				return 1;
			else
				return std::get<CollisionObjectView::Type::GROUP>(obj->view).size();
		}
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void CollisionView::resize_view(size_t size)
	{
		if (!valid())
			throw Error(ErrorCode::NULL_POINTER);

		if (size == 0)
			clear_view();
		else if (obj->view.index() == CollisionObjectView::Type::EMPTY)
		{
			if (size == 1)
			{
				obj->view = layer->default_collision_object();
				view_changed();
			}
			else if (size > 1)
			{
				obj->view = CollisionObjectGroup(size, layer->default_collision_object());
				view_changed();
			}
		}
		else if (obj->view.index() == CollisionObjectView::Type::SINGLE)
		{
			if (size > 1)
			{
				CollisionObject old_obj = std::move(std::get<CollisionObjectView::Type::SINGLE>(obj->view));
				obj->view = CollisionObjectGroup(size, layer->default_collision_object());
				std::get<CollisionObjectView::Type::GROUP>(obj->view)[0] = std::move(old_obj);
				view_changed();
			}
		}
		else if (obj->view.index() == CollisionObjectView::Type::GROUP)
		{
			if (size == 1)
			{
				CollisionObject old_obj = std::move(std::get<CollisionObjectView::Type::GROUP>(obj->view)[0]);
				obj->view = std::move(old_obj);
				view_changed();
			}
			else
			{
				std::get<CollisionObjectView::Type::GROUP>(obj->view).resize(size, layer->default_collision_object());
				view_changed();
			}
		}
	}

	void CollisionView::set_view(size_t i, CollisionObject&& obj)
	{
		if (!valid())
			throw Error(ErrorCode::NULL_POINTER);

		if (this->obj->view.index() == CollisionObjectView::Type::EMPTY || this->obj->view.index() == CollisionObjectView::Type::SINGLE)
		{
			if (i == 0)
			{
				this->obj->view = std::move(obj);
				view_changed();
			}
			else
				throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
		}
		else
		{
			std::get<CollisionObjectView::Type::GROUP>(this->obj->view)[i] = std::move(obj);
			view_changed();
		}
	}

	void CollisionView::merge(CollisionView&& other)
	{
		if (!valid() || !other.valid())
			return;

		if (other.obj->view.index() == CollisionObjectView::Type::EMPTY)
			return;

		if (obj->view.index() == CollisionObjectView::Type::EMPTY)
		{
			set_view(std::move(*other.obj));
			return;
		}

		if (obj->view.index() == CollisionObjectView::Type::SINGLE)
		{
			CollisionObject old_view = std::move(std::get<CollisionObjectView::Type::SINGLE>(obj->view));
			obj->view = CollisionObjectGroup();
			std::get<CollisionObjectView::Type::GROUP>(obj->view).push_back(std::move(old_view));
		}

		std::visit([&view = std::get<CollisionObjectView::Type::GROUP>(obj->view)](auto&& other_view) {
			if constexpr (visiting_class_is<decltype(other_view), CollisionObject>)
				view.push_back(std::move(other_view));
			else if constexpr (visiting_class_is<decltype(other_view), CollisionObjectGroup>)
				view.insert(view.end(), std::make_move_iterator(other_view.begin()), std::make_move_iterator(other_view.end()));
			}, std::move(other.obj->view));

		view_changed();
	}

	const CollisionObject& CollisionView::get_view(size_t i) const
	{
		if (!valid())
			throw Error(ErrorCode::NULL_POINTER);

		if (obj->view.index() == CollisionObjectView::Type::EMPTY)
			throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
		else if (obj->view.index() == CollisionObjectView::Type::SINGLE)
		{
			if (i == 0)
				return std::get<CollisionObjectView::Type::SINGLE>(obj->view);
			else
				throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
		}
		else
			return std::get<CollisionObjectView::Type::GROUP>(obj->view)[i];
	}

	CollisionObject& CollisionView::get_view(size_t i)
	{
		if (!valid())
			throw Error(ErrorCode::NULL_POINTER);

		if (obj->view.index() == CollisionObjectView::Type::EMPTY)
			throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
		else if (obj->view.index() == CollisionObjectView::Type::SINGLE)
		{
			if (i == 0)
				return std::get<CollisionObjectView::Type::SINGLE>(obj->view);
			else
				throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
		}
		else
			return std::get<CollisionObjectView::Type::GROUP>(obj->view)[i];
	}

	void CollisionView::view_changed() const
	{
		if (valid())
			layer->painter.flag_dirty();
	}

	void CollisionView::update_color(glm::vec4 color)
	{
		if (!valid())
			throw Error(ErrorCode::NULL_POINTER);

		static const auto update_color = [](CollisionObject& obj, glm::vec4 color) {
			std::visit([color](auto&& obj) {
				if constexpr (visiting_class_is<decltype(obj), rendering::EllipseReference>)
					obj.set_color().fill_outer = color;
				else if constexpr (visiting_class_is<decltype(obj), rendering::StaticPolygon>)
					obj.set_colors() = { color };
				else if constexpr (visiting_class_is<decltype(obj), rendering::StaticArrowExtension>)
					obj.set_color(color);
				}, *obj);
			};

		bool updated = std::visit([color](auto&& obj) -> bool {
			if constexpr (visiting_class_is<decltype(obj), CollisionObject>)
			{
				update_color(obj, color);
				return true;
			}
			else if constexpr (visiting_class_is<decltype(obj), CollisionObjectGroup>)
			{
				for (CollisionObject& subobj : obj)
					update_color(subobj, color);
				return !obj.empty();
			}
			else
				return false;
			}, obj->view);

		if (updated)
			view_changed();
	}

	CollisionLayer::CollisionLayer()
		: painter(paint_fn())
	{
		painter.paint_fn = paint_fn();
	}

	CollisionLayer::CollisionLayer(rendering::Unbatched)
		: painter(paint_fn(), rendering::UNBATCHED)
	{
		painter.paint_fn = paint_fn();
	}

	CollisionLayer::CollisionLayer(rendering::SpriteBatch& batch)
		: painter(paint_fn(), batch)
	{
		painter.paint_fn = paint_fn();
	}

	CollisionLayer::CollisionLayer(const CollisionLayer& other)
		: painter(other.painter)
	{
		painter.paint_fn = paint_fn();
	}

	CollisionLayer::CollisionLayer(CollisionLayer&& other) noexcept
		: painter(std::move(other.painter)), collision_views(std::move(other.collision_views))
	{
		painter.paint_fn = paint_fn();
	}

	CollisionLayer::~CollisionLayer()
	{
		for (CollisionView* view : collision_views)
			view->invalidate_layer();
	}

	CollisionLayer& CollisionLayer::operator=(const CollisionLayer& other)
	{
		if (this != &other)
		{
			for (CollisionView* view : collision_views)
				view->invalidate_layer();
			collision_views.clear();
			painter = other.painter;
			painter.paint_fn = paint_fn();
		}
		return *this;
	}

	CollisionLayer& CollisionLayer::operator=(CollisionLayer&& other) noexcept
	{
		if (this != &other)
		{
			for (CollisionView* view : collision_views)
				view->invalidate_layer();
			collision_views.clear();
			painter = std::move(other.painter);
			painter.paint_fn = paint_fn();
		}
		return *this;
	}

	rendering::GeometryPainter::PaintFunction CollisionLayer::paint_fn() const
	{
		return [this](rendering::GeometryPainter::PaintSupport& ps) {
			for (const auto& collision_view : collision_views)
				collision_view->draw(ps);
			};
	}

	void CollisionLayer::assign(CollisionView* view)
	{
		if (collision_views.insert(view).second)
			painter.flag_dirty();
	}

	void CollisionLayer::unassign(CollisionView* view)
	{
		if (collision_views.erase(view))
			painter.flag_dirty();
	}

	void CollisionLayer::draw() const
	{
		painter.draw();
	}

	CollisionObject CollisionLayer::default_collision_object()
	{
		return CollisionObject(*this, rendering::EllipseReference());
	}

	rendering::EllipseReference CollisionLayer::create_ellipse()
	{
		return rendering::EllipseReference(&painter.get_ellipse_batch());
	}

	rendering::StaticPolygon CollisionLayer::create_polygon()
	{
		return rendering::StaticPolygon(&painter.get_polygon_batch());
	}
	
	rendering::StaticArrowExtension CollisionLayer::create_arrow()
	{
		return rendering::StaticArrowExtension(&painter.get_polygon_batch());
	}
}
