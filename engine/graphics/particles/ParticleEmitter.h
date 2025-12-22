#pragma once

#include "graphics/backend/specialized/LightweightBuffers.h"
#include "graphics/backend/basic/VertexArrays.h"
#include "graphics/backend/basic/Shader.h"
#include "core/base/Transforms.h"
#include "graphics/Camera.h"

// TODO v6 separate into different files
namespace oly::particles
{
	// TODO v6 don't expose Sampler/Domain/Generator raw types - use polymorphism which will then convert to raw data. Same for EmitterParams and put EmitterParams in .cpp file?

	namespace internal
	{
		struct alignas(16) Sampler
		{
			enum Type : GLuint
			{
				UNIFORM = 0
			} type = Type::UNIFORM;

		private:
			float _pad[3] = { 0.0f };

		public:
			float params[8] = { 0.0f };
		};

		struct alignas(16) Domain
		{
			enum Type : GLuint
			{
				CONSTANT = 0,
				LINE = 1,
				BILINE = 2
			} type = Type::CONSTANT;

		private:
			float _pad[3] = { 0.0f };

		public:
			float params[8] = { 0.0 };
		};

		struct alignas(16) Generator
		{
			Sampler sampler = {};
			Domain domain = {};
		};

		struct alignas(16) EmitterParams
		{
			GLuint max_particles = 2000;
			GLuint attached = false;

		private:
			float _pad0[2] = { 0.0f };

		public:
			Generator lifetime = {};

			glm::vec2 position = {}; // TODO v6 use generator
			float rotation = 0.0f; // TODO v6 use generator

		private:
			float _pad1[1] = { 0.0f };

		public:
			glm::vec2 size = { 10.0f, 10.0f }; // TODO v6 use generator
			glm::vec2 velocity = { 10.0f, 0.0f }; // TODO v6 use generator

			glm::vec4 color = { 1.0f, 0.0f, 0.0f, 1.0f }; // TODO v6 use generator
		};
	}

	struct ISampler
	{
		virtual internal::Sampler raw() const = 0;

		OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(ISampler);
	};

	struct UniformSampler : public ISampler
	{
		internal::Sampler raw() const override
		{
			internal::Sampler sampler;
			sampler.type = internal::Sampler::UNIFORM;
			return sampler;
		}

		OLY_POLYMORPHIC_CLONE_OVERRIDE(UniformSampler);
	};

	struct IDomain
	{
		virtual internal::Domain raw() const = 0;

		OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(IDomain);
	};

	struct ConstantDomain : public IDomain
	{
		float c;

		ConstantDomain(float c = 0.0f) : c(c) {}

		internal::Domain raw() const override
		{
			internal::Domain domain;
			domain.type = internal::Domain::CONSTANT;
			domain.params[0] = c;
			return domain;
		}

		OLY_POLYMORPHIC_CLONE_OVERRIDE(ConstantDomain);
	};

	struct LineDomain : public IDomain
	{
		float a;
		float b;

		LineDomain(float a = 0.0f, float b = 0.0f) : a(a), b(b) {}

		internal::Domain raw() const override
		{
			internal::Domain domain;
			domain.type = internal::Domain::LINE;
			domain.params[0] = a;
			domain.params[1] = b;
			return domain;
		}

		OLY_POLYMORPHIC_CLONE_OVERRIDE(LineDomain);
	};

	struct BiLineDomain : public IDomain
	{
		float a;
		float b;
		float c;

		BiLineDomain(float a = 0.0f, float b = 0.0f, float c = 0.0f) : a(a), b(b), c(c) {}

		internal::Domain raw() const override
		{
			internal::Domain domain;
			domain.type = internal::Domain::BILINE;
			domain.params[0] = a;
			domain.params[1] = b;
			domain.params[2] = c;
			return domain;
		}

		OLY_POLYMORPHIC_CLONE_OVERRIDE(BiLineDomain);
	};

	struct AttributeGenerator
	{
		Polymorphic<ISampler> sampler;
		Polymorphic<IDomain> domain;

		explicit operator internal::Generator() const
		{
			return { .sampler = sampler->raw(), .domain = domain->raw() };
		}
	};

	class ParticleSystem;

	struct ParticleEmitter
	{
		GLuint max_particles;
		GLuint attached;

		AttributeGenerator lifetime;

		glm::vec2 position; // TODO v6 use generator
		float rotation; // TODO v6 use generator

		glm::vec2 size; // TODO v6 use generator
		glm::vec2 velocity; // TODO v6 use generator

		glm::vec4 color; // TODO v6 use generator

	private:
		mutable float _spawn_debt = 0.0f;

	public:
		ParticleEmitter()
			: max_particles(2000),
			attached(false),
			lifetime({ .sampler = make_polymorphic<UniformSampler>(), .domain = make_polymorphic<ConstantDomain>(3.0f) }),
			position({}),
			rotation(0.0f),
			size({ 10.0f, 10.0f }),
			velocity({ 10.0f, 0.0f }),
			color({ 1.0f, 0.0f, 0.0f, 1.0f })
		{
		}

		explicit operator internal::EmitterParams() const
		{
			internal::EmitterParams params;
			params.max_particles = max_particles;
			params.attached = attached;
			params.lifetime = (internal::Generator)lifetime;
			params.position = position;
			params.rotation = rotation;
			params.size = size;
			params.velocity = velocity;
			params.color = color;
			return params;
		}

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
		rendering::Camera2DRef camera = REF_DEFAULT;
		bool camera_invariant = false;
		enum class AgeSort
		{
			YOUNG_ON_OLD,
			OLD_ON_YOUNG
		} age_sort = AgeSort::YOUNG_ON_OLD;

		Transformer2D transformer;

		ParticleSystem(ParticleEmitter&& emitter = {}, GLuint particle_capacity = 2000, GLushort compute_threads = 64);
		ParticleSystem(std::vector<ParticleEmitter>&& emitters, GLuint particle_capacity = 2000, GLushort compute_threads = 64);
		ParticleSystem(size_t emitter_count, GLuint particle_capacity = 2000, GLushort compute_threads = 64);
		ParticleSystem(const ParticleSystem&) = delete;
		ParticleSystem(ParticleSystem&&) = delete;

	private:
		void init();

	public:
		void on_tick();
		void render() const;

	private:
		void spawn_particles(const ParticleEmitter& emitter) const;
		void update_particles(GLuint in_primitive_count, float delta_time) const;
		void draw_particles() const;

	public:
		float get_time_elapsed() const { return time_elapsed; }

		const ParticleEmitter& emitter(size_t i = 0) const { return emitters[i]; }
		ParticleEmitter& emitter(size_t i = 0) { return emitters[i]; }
		void add_emitter(ParticleEmitter&& emitter = {}) { emitters.push_back(std::move(emitter)); }
		void remove_emitter(size_t i) { emitters.erase(emitters.begin() + i); }

		GLuint get_particle_capacity() const { return particle_capacity; }
		void set_particle_capacity(GLuint capacity);
	};
}
