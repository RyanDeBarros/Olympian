#pragma once

#include "core/base/Constants.h"
#include "core/containers/IDGenerator.h"
#include "core/types/Issuer.h"

#include "graphics/backend/basic/VertexArrays.h"
#include "graphics/backend/specialized/ElementBuffers.h"
#include "graphics/Tags.h"
#include "graphics/Camera.h"

namespace oly::rendering
{
	struct EllipseDimension
	{
		float rx = 0.0f, ry = 0.0f, border = 0.0f;
		float fill_exp = 0.0f, border_exp = 0.0f;
		GLuint camera_invariant = 0;
	};

	struct EllipseColorGradient {
		glm::vec4 fill_inner = glm::vec4(1.0f);
		glm::vec4 fill_outer = glm::vec4(1.0f);
		glm::vec4 border_inner = glm::vec4(1.0f);
		glm::vec4 border_outer = glm::vec4(1.0f);

		bool is_uniform() const
		{
			return fill_inner == fill_outer && fill_inner == border_inner && fill_inner == border_outer;
		}

		void set_uniform(glm::vec4 color)
		{
			fill_inner = color;
			fill_outer = color;
			border_inner = color;
			border_outer = color;
		}
	};

	namespace internal
	{
		class EllipseReference;

		class EllipseBatch : public oly::internal::Issuer<EllipseBatch>
		{
			friend class EllipseReference;

			graphics::VertexArray vao;
			graphics::PersistentEBO<6> ebo;
			
			enum
			{
				DIMENSION,
				COLOR,
				TRANSFORM
			};
			graphics::LazyPersistentGPUBufferBlock<EllipseDimension, EllipseColorGradient, glm::mat3> ssbo_block;

			struct
			{
				GLuint projection, invariant_projection;
			} shader_locations;

		public:
			Camera2DRef camera = REF_DEFAULT;

			EllipseBatch();
			EllipseBatch(const EllipseBatch&) = delete;
			EllipseBatch(EllipseBatch&&) = delete;

			void render() const;

		private:
			SoftIDGenerator<GLuint> id_generator;
			static const GLuint NULL_ID = GLuint(-1);
			static void assert_valid_id(GLuint id);
			GLuint generate_id();
			void erase_id(GLuint id);
		};
	}

	using EllipseBatch = PublicIssuer<internal::EllipseBatch>;

	namespace internal
	{
		class EllipseReference : public PublicIssuerHandle<EllipseBatch>
		{
			using Super = PublicIssuerHandle<EllipseBatch>;
			GLuint id = EllipseBatch::NULL_ID;

		public:
			EllipseReference(Unbatched = UNBATCHED);
			EllipseReference(rendering::EllipseBatch& batch);
			EllipseReference(const EllipseReference&);
			EllipseReference(EllipseReference&&) noexcept;
			EllipseReference& operator=(const EllipseReference&);
			EllipseReference& operator=(EllipseReference&&) noexcept;
			~EllipseReference();

			auto get_batch() const { return lock(); }
			void set_batch(Unbatched);
			void set_batch(rendering::EllipseBatch& batch);

			EllipseDimension get_dimension() const;
			EllipseDimension& set_dimension() const;
			const EllipseColorGradient& get_color() const;
			EllipseColorGradient& set_color() const;
			void set_color(glm::vec4 color) const;
			const glm::mat3& get_transform() const;
			glm::mat3& set_transform() const;

			void draw() const;
		};
	}

	class StaticEllipse
	{
		internal::EllipseReference ref;
		EllipseDimension dimension;
		EllipseColorGradient color;
		glm::mat3 transform = 1.0f;
		
	public:
		StaticEllipse(Unbatched = UNBATCHED) : ref(UNBATCHED) {}
		StaticEllipse(EllipseBatch& batch) : ref(batch) {}
		StaticEllipse(EllipseBatch& batch, float r, glm::vec4 color = glm::vec4(1.0f));
		StaticEllipse(EllipseBatch& batch, float rx, float ry, glm::vec4 color = glm::vec4(1.0f));
		StaticEllipse(const StaticEllipse&) = default;
		StaticEllipse(StaticEllipse&&) noexcept = default;
		StaticEllipse& operator=(const StaticEllipse&) = default;
		StaticEllipse& operator=(StaticEllipse&&) noexcept = default;

		void draw() const { ref.draw(); }

		auto get_batch() const { return ref.get_batch(); }
		void set_batch(Unbatched) { ref.set_batch(Unbatched{}); }
		void set_batch(EllipseBatch& batch);

		EllipseDimension get_dimension() const { return dimension; }
		void set_dimension(EllipseDimension dimension);
		const EllipseColorGradient& get_color() const { return color; }
		void set_color(const EllipseColorGradient& color);
		void set_color(glm::vec4 color);
		const glm::mat3& get_transform() const { return transform; }
		void set_transform(const glm::mat3& local);

		math::Rect2D bounds() const;
		math::RotatedRect2D rotated_bounds() const;
	};

	class Ellipse
	{
		internal::EllipseReference ref;
		Transformer2D transformer;

	public:
		Ellipse(Unbatched = UNBATCHED) : ref(UNBATCHED) {}
		Ellipse(EllipseBatch& batch) : ref(batch) {}
		Ellipse(EllipseBatch& batch, float r, glm::vec4 color = glm::vec4(1.0f));
		Ellipse(EllipseBatch& batch, float rx, float ry, glm::vec4 color = glm::vec4(1.0f));
		Ellipse(const Ellipse&) = default;
		Ellipse(Ellipse&&) noexcept = default;
		Ellipse& operator=(const Ellipse&) = default;
		Ellipse& operator=(Ellipse&&) noexcept = default;

		EllipseDimension get_dimension() const { return ref.get_dimension(); }
		EllipseDimension& set_dimension() { return ref.set_dimension(); }
		const EllipseColorGradient& get_color() const { return ref.get_color(); }
		EllipseColorGradient& set_color() { return ref.set_color(); }
		void set_color(glm::vec4 color) { ref.set_color(color); }

		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		const Transformer2D& get_transformer() const { return transformer; }
		Transformer2D& set_transformer() { return transformer; }

		void draw() const;

		static Ellipse load(TOMLNode node, const char* source = nullptr);
	};

	typedef SmartReference<Ellipse> EllipseRef;
}
