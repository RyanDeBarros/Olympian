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
	};

	struct EllipseColorGradient {
		glm::vec4 fill_inner = glm::vec4(1.0f);
		glm::vec4 fill_outer = glm::vec4(1.0f);
		glm::vec4 border_inner = glm::vec4(1.0f);
		glm::vec4 border_outer = glm::vec4(1.0f);
	};

	class EllipseReference;

	namespace internal
	{
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

			GLuint projection_location;

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

	class EllipseReference : public PublicIssuerHandle<internal::EllipseBatch>
	{
		using Super = PublicIssuerHandle<internal::EllipseBatch>;
		GLuint id = internal::EllipseBatch::NULL_ID;

	public:
		EllipseReference(Unbatched = UNBATCHED);
		EllipseReference(EllipseBatch& batch);
		EllipseReference(const EllipseReference&);
		EllipseReference(EllipseReference&&) noexcept;
		EllipseReference& operator=(const EllipseReference&);
		EllipseReference& operator=(EllipseReference&&) noexcept;
		~EllipseReference();

		auto get_batch() const { return lock(); }
		void set_batch(Unbatched);
		void set_batch(EllipseBatch& batch);

		EllipseDimension get_dimension() const;
		EllipseDimension& set_dimension();
		const EllipseColorGradient& get_color() const;
		EllipseColorGradient& set_color();
		const glm::mat3& get_transform() const;
		glm::mat3& set_transform();

		void draw() const;
	};

	struct Ellipse
	{
		EllipseReference ellipse;
		Transformer2D transformer;

		Ellipse(Unbatched = UNBATCHED) : ellipse(UNBATCHED) {}
		Ellipse(EllipseBatch& batch) : ellipse(batch) {}
		Ellipse(EllipseBatch& batch, float r, glm::vec4 color = glm::vec4(1.0f));
		Ellipse(EllipseBatch& batch, float rx, float ry, glm::vec4 color = glm::vec4(1.0f));
		Ellipse(const Ellipse&) = default;
		Ellipse(Ellipse&&) noexcept = default;
		Ellipse& operator=(const Ellipse&) = default;
		Ellipse& operator=(Ellipse&&) noexcept = default;

		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		void draw() const;

		void set_color(glm::vec4 color);
	};

	typedef SmartReference<Ellipse> EllipseRef;
}
