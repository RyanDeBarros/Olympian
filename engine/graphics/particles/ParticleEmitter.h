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
		float _pad0[2] = { 0.0f, 0.0f };
	public:

		glm::vec2 position = {}; // TODO v6 use generator
		glm::vec2 velocity = { 10.0f, 0.0f }; // TODO v6 use generator

		float rotation = 0.0f; // TODO v6 use generator
	private:
		float _pad1[1] = { 0.0f };
	public:
		glm::vec2 size = { 10.0f, 10.0f }; // TODO v6 use generator

		glm::vec4 color = { 1.0f, 0.0f, 0.0f, 1.0f }; // TODO v6 use generator
	};

	class ParticleEmitter
	{
		graphics::VertexArray vao;

		struct DrawArraysIndirectCommand
		{
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
				graphics::LightweightSSBO<graphics::Mutability::IMMUTABLE>& in();
				const graphics::LightweightSSBO<graphics::Mutability::IMMUTABLE>& out() const;
				graphics::LightweightSSBO<graphics::Mutability::IMMUTABLE>& out();
			} particles;

			graphics::LightweightSSBO<graphics::Mutability::IMMUTABLE> emitter;
			graphics::LightweightSSBO<graphics::Mutability::IMMUTABLE> draw_command;

			BufferList(const ParticleEmitterParams& params);
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

		float time_elapsed = 0.0f;
		mutable float last_render_time = 0.0f;

		ParticleEmitterParams emitter_params;

	public:
		Camera2DRef camera = REF_DEFAULT;
		Transformer2D transformer;

		ParticleEmitter(const ParticleEmitterParams& params);
		ParticleEmitter(const ParticleEmitter&) = delete;
		ParticleEmitter(ParticleEmitter&&) = delete;

		void on_tick();
		void render() const;
		
	private:
		void spawn_particles(GLuint to_spawn) const;
		void update_particles(GLuint in_primitive_count, float delta_time) const;
		void draw_particles() const;

	public:
		float get_time_elapsed() const { return time_elapsed; }

		struct ParamsView
		{
		private:
			ParticleEmitter& emitter;

			friend class ParticleEmitter;
			ParamsView(ParticleEmitter& emitter) : emitter(emitter) {}

		public:
			GLuint get_max_particles() const { return emitter.emitter_params.max_particles; }
			float get_lifetime() const { return emitter.emitter_params.lifetime; }
			glm::vec2 get_position() const { return emitter.emitter_params.position; }
			glm::vec2 get_velocity() const { return emitter.emitter_params.velocity; }
			float get_rotation() const { return emitter.emitter_params.rotation; }
			glm::vec2 get_size() const { return emitter.emitter_params.size; }
			glm::vec4 get_color() const { return emitter.emitter_params.color; }

			void set_max_particles(GLuint max_particles);
			void set_lifetime(float lifetime);
			void set_position(glm::vec2 position);
			void set_velocity(glm::vec2 velocity);
			void set_rotation(float rotation);
			void set_size(glm::vec2 size);
			void set_color(glm::vec4 color);
		};

		ParamsView params() { return ParamsView(*this); }

		struct ConstParamsView
		{
		private:
			const ParticleEmitter& emitter;

			friend class ParticleEmitter;
			ConstParamsView(const ParticleEmitter& emitter) : emitter(emitter) {}

		public:
			GLuint get_max_particles() const { return emitter.emitter_params.max_particles; }
			float get_lifetime() const { return emitter.emitter_params.lifetime; }
			glm::vec2 get_position() const { return emitter.emitter_params.position; }
			glm::vec2 get_velocity() const { return emitter.emitter_params.velocity; }
			float get_rotation() const { return emitter.emitter_params.rotation; }
			glm::vec2 get_size() const { return emitter.emitter_params.size; }
			glm::vec4 get_color() const { return emitter.emitter_params.color; }
		};

		ConstParamsView params() const { return ConstParamsView(*this); }
	};
}
