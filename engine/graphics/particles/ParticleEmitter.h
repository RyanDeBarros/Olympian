#pragma once

#include "graphics/backend/specialized/LightweightBuffers.h"
#include "graphics/backend/basic/VertexArrays.h"
#include "graphics/backend/basic/Shader.h"
#include "core/base/Transforms.h"
#include "graphics/Camera.h"

namespace oly::rendering
{
	struct alignas(16) EmitterParams
	{
		GLuint max_particles = 2000;
		float lifetime = 3.0f; // TODO v6 use generator
		GLuint attached = false;

	private:
		float _pad0[1] = { 0.0f };

	public:
		glm::vec2 position = {}; // TODO v6 use generator
		float rotation = 0.0f; // TODO v6 use generator

	private:
		float _pad1[1] = { 0.0f };

	public:
		glm::vec2 size = { 10.0f, 10.0f }; // TODO v6 use generator
		
	private:
		float _pad2[2] = { 0.0f, 0.0f };

	public:
		glm::vec4 color = { 1.0f, 0.0f, 0.0f, 1.0f }; // TODO v6 use generator

		glm::vec2 velocity = { 10.0f, 0.0f }; // TODO v6 use generator
	};

	class ParticleSystem;

	struct ParticleEmitter
	{
		EmitterParams params;

	private:
		mutable float _spawn_debt = 0.0f;

	public:
		ParticleEmitter(const EmitterParams& params = {}) : params(params) {}

	private:
		friend class ParticleSystem;
		void on_tick(float delta_time) const;
		GLuint spawn_debt() const;
	};

	class ParticleSystem
	{
		friend class ParticleEmitter;

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
			graphics::LightweightSSBO<graphics::Mutability::IMMUTABLE> ps_data;

			BufferList(GLuint particle_capacity);
		} buffers;

		struct
		{
			SmartReference<graphics::Shader> compute_spawn_ref = REF_NULL;
			GLuint compute_spawn;

			SmartReference<graphics::Shader> compute_update_ref = REF_NULL;
			GLuint compute_update;

			GLuint renderer;
		} shaders;

		struct
		{
			struct
			{
				GLint time;
				GLint spawn_count;
				GLint transform;
			} compute_spawn;

			struct
			{
				GLint delta_time;
				GLint in_prim_count;
			} compute_update;

			struct
			{
				GLint projection;
				GLint transform;
				GLint reverse_draw_order;
			} renderer;
		} shader_locations;

		std::vector<ParticleEmitter> emitters;
		GLuint particle_capacity;
		GLushort compute_threads;

		float time_elapsed = 0.0f;
		mutable float last_tick_time = 0.0f;
		mutable float last_render_time = 0.0f;

	public:
		Camera2DRef camera = REF_DEFAULT;
		bool camera_invariant = false;
		enum class AgeSort
		{
			YOUNG_ON_OLD,
			OLD_ON_YOUNG
		} age_sort = AgeSort::YOUNG_ON_OLD;

		Transformer2D transformer;

		ParticleSystem(const EmitterParams& emitter = {}, GLuint particle_capacity = 2000, GLushort compute_threads = 64);
		ParticleSystem(const std::vector<EmitterParams>& emitters, GLuint particle_capacity = 2000, GLushort compute_threads = 64);
		ParticleSystem(size_t emitter_count, GLuint particle_capacity = 2000, GLushort compute_threads = 64);
		ParticleSystem(const ParticleSystem&) = delete;
		ParticleSystem(ParticleSystem&&) = delete;

	private:
		void init();

	public:
		void on_tick();
		void render() const;

	private:
		void spawn_particles(const EmitterParams& emitter, GLuint to_spawn) const;
		void update_particles(GLuint in_primitive_count, float delta_time) const;
		void draw_particles() const;

	public:
		float get_time_elapsed() const { return time_elapsed; }

		const ParticleEmitter& emitter(size_t i = 0) const { return emitters[i]; }
		ParticleEmitter& emitter(size_t i = 0) { return emitters[i]; }
		void add_emitter(const EmitterParams& params = {}) { emitters.emplace_back(params); }
		void remove_emitter(size_t i) { emitters.erase(emitters.begin() + i); }

		GLuint get_particle_capacity() const { return particle_capacity; }
		void set_particle_capacity(GLuint capacity);
	};
}
