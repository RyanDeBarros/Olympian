#pragma once

#include "graphics/backend/specialized/LightweightBuffers.h"
#include "graphics/backend/basic/VertexArrays.h"
#include "core/base/Transforms.h"
#include "graphics/Camera.h"

namespace oly::rendering
{
	struct Particle {
		float timeElapsed;
		float lifetime;
		glm::vec2 position;
		glm::vec2 velocity;
		float rotation;
		glm::vec2 size;
		glm::vec4 color;
	};

	// TODO v6 allow for editing of these parameters, instead of only passing in constructor? For max particles, will need to make some buffers mutable since they'd be resized.
	struct alignas(16) ParticleEmitterParams
	{
		GLuint max_particles = 2000;
		float lifetime = 3.0f; // TODO v6 use generator
	private:
		float _pad0[2];
	public:

		glm::vec2 position = {}; // TODO v6 use generator
		glm::vec2 velocity = { 10.0f, 0.0f }; // TODO v6 use generator

		float rotation = 0.0f; // TODO v6 use generator
	private:
		float _pad1[1];
	public:
		glm::vec2 size = { 10.0f, 10.0f }; // TODO v6 use generator

		glm::vec4 color = { 1.0f, 0.0f, 0.0f, 1.0f }; // TODO v6 use generator
	};

	class ParticleEmitter
	{
		graphics::VertexArray vao;

		struct DrawElementsIndirectCommand {
			GLuint count;
			GLuint primCount;
			GLuint first;
			GLuint baseVertex;
		};

		struct BufferList
		{
			class ParticleDoubleBuffer
			{
				graphics::LightweightSSBO<graphics::Mutability::IMMUTABLE> a;
				graphics::LightweightSSBO<graphics::Mutability::IMMUTABLE> b;
				mutable bool state = true;
				
			public:
				ParticleDoubleBuffer(GLuint max_particles);

				void swap() const;
				const graphics::LightweightSSBO<graphics::Mutability::IMMUTABLE>& in() const;
				const graphics::LightweightSSBO<graphics::Mutability::IMMUTABLE>& out() const;
			} particles;

			graphics::LightweightSSBO<graphics::Mutability::IMMUTABLE> emitter;
			graphics::LightweightSSBO<graphics::Mutability::IMMUTABLE> draw_command;

			BufferList(const ParticleEmitterParams& params);

			GLuint primitive_count() const;
			void reset_draw_command() const;
		} buffers;

		struct
		{
			GLuint compute_spawn;
			GLuint compute_update;
			GLuint renderer;
		} shaders;

		struct
		{
			struct
			{
				GLint time;
				GLint spawn_count;
			} compute_spawn;

			struct
			{
				GLint delta_time;
				GLint in_prim_count;
			} compute_update;

			struct
			{
				GLint transform;
			} renderer;
		} shader_locations;

		mutable float time_elapsed = 0.0f;
		mutable float last_render_time = 0.0f;

	public:
		Camera2DRef camera = REF_DEFAULT;
		Transformer2D transformer;

		ParticleEmitter(const ParticleEmitterParams& params);
		ParticleEmitter(const ParticleEmitter&) = delete;
		ParticleEmitter(ParticleEmitter&&) = delete;

		void on_tick() const;
		void render() const;
		
	private:
		void spawn_particles(GLuint to_spawn) const;
		void update_particles(GLuint in_primitive_count, float delta_time) const;
		void draw_particles() const;
	};
}
