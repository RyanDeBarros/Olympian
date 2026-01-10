#pragma once

#include "graphics/particles/ParticleEmitter.h"
#include "graphics/backend/specialized/LightweightBuffers.h"
#include "graphics/backend/basic/VertexArrays.h"
#include "graphics/backend/basic/Shader.h"
#include "core/base/Transforms.h"
#include "graphics/Camera.h"

#include <unordered_set>

namespace oly::rendering
{
	class ParticleSystem;

	namespace internal
	{
		class ParticleSystemManager final : public Singleton<ParticleSystemManager>
		{
			friend class Singleton<ParticleSystemManager>;

			friend class ParticleSystem;
			std::unordered_set<ParticleSystem*> systems;

		public:
			void clear()
			{
				systems.clear();
			}

			void on_tick();
		};
	}

	class ParticleSystem
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
			graphics::LightweightSSBO<graphics::Mutability::IMMUTABLE> ps_data;

			BufferList(GLuint particle_capacity);
		} buffers;

		struct
		{
			SmartReference<graphics::Shader> compute_spawn_ref = REF_NULL;
			GLuint compute_spawn = 0;

			SmartReference<graphics::Shader> compute_update_ref = REF_NULL;
			GLuint compute_update = 0;

			GLuint renderer = 0;
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

		std::vector<particles::ParticleEmitter> emitters;
		GLuint particle_capacity;
		GLushort compute_threads;

	public:
		bool camera_invariant = false;
		bool auto_tick = true;

	private:
		float time_elapsed = 0.0f;
		mutable float last_tick_time = 0.0f;
		mutable float last_render_time = 0.0f;

	public:
		enum class AgeSort
		{
			YOUNG_ON_OLD,
			OLD_ON_YOUNG
		} age_sort = AgeSort::YOUNG_ON_OLD;

		rendering::Camera2DRef camera = REF_DEFAULT;
		Transformer2D transformer;

		ParticleSystem(particles::ParticleEmitter&& emitter = {}, GLuint particle_capacity = 2000, GLushort compute_threads = 64);
		ParticleSystem(std::vector<particles::ParticleEmitter>&& emitters, GLuint particle_capacity = 2000, GLushort compute_threads = 64);
		ParticleSystem(size_t emitter_count, GLuint particle_capacity = 2000, GLushort compute_threads = 64);
		ParticleSystem(const ParticleSystem&);
		ParticleSystem(ParticleSystem&&) noexcept;
		~ParticleSystem();
		ParticleSystem& operator=(const ParticleSystem&) = delete;
		ParticleSystem& operator=(ParticleSystem&&) = delete;

	private:
		void init();

	public:
		void on_tick();
		void render() const;

	private:
		void spawn_particles(const particles::ParticleEmitter& emitter) const;
		void update_particles(GLuint in_primitive_count, float delta_time) const;
		void draw_particles() const;

	public:
		float get_time_elapsed() const { return time_elapsed; }

		const particles::ParticleEmitter& emitter(size_t i = 0) const { return emitters[i]; }
		particles::ParticleEmitter& emitter(size_t i = 0) { return emitters[i]; }
		void add_emitter(particles::ParticleEmitter&& emitter = {});
		void remove_emitter(size_t i);

		GLuint get_particle_capacity() const { return particle_capacity; }
		void set_particle_capacity(GLuint capacity);
	};
}
