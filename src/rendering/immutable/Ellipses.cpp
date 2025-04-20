#include "Ellipses.h"

#include <glm/gtc/type_ptr.hpp>

#include <algorithm>

#include "../Resources.h"
#include "math/Transforms.h"

namespace oly
{
	namespace immut
	{
		EllipseBatch::EllipseBatch(Capacity capacity, const glm::vec4& projection_bounds)
			: capacity(capacity), ebo(capacity.ellipses), dimension_ssbo(capacity.ellipses), color_ssbo(capacity.ellipses), transform_ssbo(capacity.ellipses), z_order(capacity.ellipses)
		{
			projection_location = shaders::location(shaders::ellipse_batch, "uProjection");

			glBindVertexArray(vao);
			rendering::pre_init(ebo);
			ebo.bind();
			ebo.init();
			glBindVertexArray(0);

			set_projection(projection_bounds);
			draw_specs.push_back({ 0, capacity.ellipses });
		}

		void EllipseBatch::draw(size_t draw_spec)
		{
			glUseProgram(shaders::ellipse_batch);
			glBindVertexArray(vao);

			dimension_ssbo.bind_base(0);
			color_ssbo.bind_base(1);
			transform_ssbo.bind_base(2);
			ebo.set_draw_spec(draw_specs[draw_spec].initial, draw_specs[draw_spec].length);
			ebo.draw(GL_TRIANGLES, GL_UNSIGNED_SHORT);
		}

		void EllipseBatch::set_projection(const glm::vec4& projection_bounds) const
		{
			glm::mat3 proj = glm::ortho<float>(projection_bounds[0], projection_bounds[1], projection_bounds[2], projection_bounds[3]);
			glUseProgram(shaders::ellipse_batch);
			glUniformMatrix3fv(projection_location, 1, GL_FALSE, glm::value_ptr(proj));
		}

		EllipseBatch::EllipseReference::EllipseReference(EllipseBatch* batch)
			: _batch(batch)
		{
			pos = _batch->pos_generator.generate();
			_dimension = &_batch->dimension_ssbo.vector()[pos.get()];
			_color = &_batch->color_ssbo.vector()[pos.get()];
			_transform = &_batch->transform_ssbo.vector()[pos.get()];
			_batch->ellipse_refs.push_back(this);
			_batch->dirty_z = true;
		}
		
		EllipseBatch::EllipseReference::EllipseReference(EllipseReference&& other) noexcept
			: _batch(other._batch), pos(std::move(other.pos)), _dimension(other._dimension), _color(other._color), _transform(other._transform)
		{
			other.active = false;
			auto it = std::find(_batch->ellipse_refs.begin(), _batch->ellipse_refs.end(), &other);
			if (it != _batch->ellipse_refs.end())
				*it = this;
		}
		
		EllipseBatch::EllipseReference::~EllipseReference()
		{
			if (active)
				vector_erase(_batch->ellipse_refs, this);
		}
		
		EllipseBatch::EllipseReference& EllipseBatch::EllipseReference::operator=(EllipseReference&& other) noexcept
		{
			if (this != &other)
			{
				if (active)
					vector_erase(_batch->ellipse_refs, this);
				_batch = other._batch;
				other.active = false;
				auto it = std::find(_batch->ellipse_refs.begin(), _batch->ellipse_refs.end(), &other);
				if (it != _batch->ellipse_refs.end())
					*it = this;
				pos = std::move(other.pos);
				_dimension = other._dimension;
				_color = other._color;
				_transform = other._transform;
			}
			return *this;
		}

		void EllipseBatch::EllipseReference::send_dimension() const
		{
			_batch->dimension_ssbo.lazy_send(pos.get());
		}

		void EllipseBatch::EllipseReference::send_color() const
		{
			_batch->color_ssbo.lazy_send(pos.get());
		}

		void EllipseBatch::EllipseReference::send_transform() const
		{
			_batch->transform_ssbo.lazy_send(pos.get());
		}

		void EllipseBatch::EllipseReference::send_data() const
		{
			_batch->dimension_ssbo.lazy_send(pos.get());
			_batch->color_ssbo.lazy_send(pos.get());
			_batch->transform_ssbo.lazy_send(pos.get());
		}

		void EllipseBatch::swap_ellipse_order(EllipsePos pos1, EllipsePos pos2)
		{
			if (pos1 != pos2)
			{
				std::swap(ebo.vector()[pos1], ebo.vector()[pos2]);
				z_order.swap_range(pos1, pos2);
				ebo.lazy_send(pos1);
				ebo.lazy_send(pos2);
			}
		}

		void EllipseBatch::move_ellipse_order(EllipsePos from, EllipsePos to)
		{
			to = std::clamp(to, (EllipsePos)0, EllipsePos(capacity.ellipses - 1));
			from = std::clamp(from, (EllipsePos)0, EllipsePos(capacity.ellipses - 1));

			if (from < to)
			{
				for (EllipsePos i = from; i < to; ++i)
					swap_ellipse_order(i, i + 1);
			}
			else if (from > to)
			{
				for (EllipsePos i = from; i > to; --i)
					swap_ellipse_order(i, i - 1);
			}
		}

		void EllipseBatch::flush()
		{
			for (Ellipse* ellipse : ellipses)
				ellipse->flush();
			dimension_ssbo.flush();
			color_ssbo.flush();
			transform_ssbo.flush();
			flush_z_values();
			ebo.flush();
		}

		void EllipseBatch::flush_z_values()
		{
			if (!dirty_z)
				return;
			dirty_z = false;
			for (EllipsePos i = 0; i < ellipse_refs.size() - 1; ++i)
			{
				bool swapped = false;
				for (EllipsePos j = 0; j < ellipse_refs.size() - i - 1; ++j)
				{
					if (ellipse_refs[j]->z_value > ellipse_refs[j + 1]->z_value)
					{
						std::swap(ellipse_refs[j], ellipse_refs[j + 1]);
						swap_ellipse_order(ellipse_refs[j]->index_pos(), ellipse_refs[j + 1]->index_pos());
						swapped = true;
					}
				}
				if (!swapped)
					break;
			}
		}

		Ellipse::Ellipse(EllipseBatch* ellipse_batch)
			: ellipse(ellipse_batch)
		{
			batch().ellipses.insert(this);
		}

		Ellipse::Ellipse(Ellipse&& other) noexcept
			: ellipse(std::move(other.ellipse)), transformer(std::move(other.transformer))
		{
			batch().ellipses.erase(&other);
		}

		Ellipse::~Ellipse()
		{
			batch().ellipses.erase(this);
		}

		Ellipse& Ellipse::operator=(Ellipse&& other) noexcept
		{
			if (this != &other)
			{
				bool different_batch = (&batch() != &other.batch());
				if (different_batch)
					batch().ellipses.erase(this);
				ellipse = std::move(other.ellipse);
				transformer = std::move(other.transformer);
				batch().ellipses.erase(&other);
				if (different_batch)
					batch().ellipses.insert(this);
			}
			return *this;
		}

		void Ellipse::post_set()
		{
			transformer.post_set();
		}

		void Ellipse::pre_get() const
		{
			transformer.pre_get();
		}

		void Ellipse::flush()
		{
			if (transformer.flush())
			{
				transformer.pre_get();
				ellipse.transform() = transformer.global();
				ellipse.send_transform();
			}
		}
	}
}
