#pragma once

#include <memory>

namespace oly
{
	namespace internal
	{
		template<typename Derived>
		class Issuer : public std::enable_shared_from_this<Issuer<Derived>>
		{
			using Super = std::enable_shared_from_this<Issuer<Derived>>;

		protected:
			Issuer() = default;
			Issuer(const Issuer&) = default;
			Issuer(Issuer&&) noexcept = default;
			~Issuer() = default;
			Issuer& operator=(const Issuer&) = default;
			Issuer& operator=(Issuer&&) noexcept = default;

		public:
			template<typename... Args>
			static std::shared_ptr<Derived> instantiate(Args&&... args)
			{
				return std::make_shared<Derived>(std::forward<Args>(args)...);
			}

		public:
			class Handle
			{
				friend class Issuer<Derived>;
				std::weak_ptr<Issuer<Derived>> issuer;

			protected:
				Handle(std::weak_ptr<Issuer<Derived>>&& issuer)
					: issuer(std::move(issuer))
				{
				}

			public:
				Handle() = default;

				bool is_valid() const
				{
					return !issuer.expired();
				}

			protected:
				class Accessor
				{
					friend class Issuer<Derived>::Handle;
					std::shared_ptr<Issuer<Derived>> issuer;

					Accessor(std::shared_ptr<Issuer>&& issuer)
						: issuer(std::move(issuer))
					{
					}

				public:
					const Derived* get() const
					{
						if (issuer)
							return static_cast<const Derived*>(issuer.get());
						else
							return nullptr;
					}

					Derived* get()
					{
						if (issuer)
							return static_cast<Derived*>(issuer.get());
						else
							return nullptr;
					}

					const Derived& operator*() const
					{
						if (issuer)
							return static_cast<const Derived&>(*issuer);
						else
							throw Error(ErrorCode::NULL_POINTER);
					}

					Derived& operator*()
					{
						if (issuer)
							return static_cast<Derived&>(*issuer);
						else
							throw Error(ErrorCode::NULL_POINTER);
					}

					const Derived* operator->() const
					{
						return get();
					}

					Derived* operator->()
					{
						return get();
					}

					operator bool() const
					{
						return issuer && issuer.get();
					}

					void reset()
					{
						issuer.reset();
					}
				};

				Accessor lock() const
				{
					return Accessor(issuer.lock());
				}

				void reset()
				{
					issuer.reset();
				}

				void reset(Issuer<Derived>& issuer)
				{
					this->issuer = issuer.weak_from_this();
				}
			};

		protected:
			Handle issue()
			{
				return Handle(Super::shared_from_this());
			}
		};
	}

	template<typename Derived> requires std::derived_from<Derived, internal::Issuer<Derived>>
	class PublicIssuer
	{
		std::shared_ptr<Derived> issuer;
		Derived& cache;

	public:
		template<typename... Args>
		PublicIssuer(Args&&... args)
			: issuer(internal::Issuer<Derived>::instantiate(std::forward<Args>(args)...)), cache(*issuer)
		{
		}

		PublicIssuer(const PublicIssuer<Derived>& other)
			: issuer(internal::Issuer<Derived>::instantiate(*other.issuer)), cache(*issuer)
		{
		}

		PublicIssuer(PublicIssuer<Derived>&& other) noexcept
			: issuer(internal::Issuer<Derived>::instantiate(std::move(*other.issuer))), cache(*issuer)
		{
		}

		PublicIssuer<Derived>& operator=(const PublicIssuer<Derived>& other)
		{
			if (this != &other)
				*issuer = *other.issuer;
			return *this;
		}

		PublicIssuer<Derived>& operator=(PublicIssuer<Derived>&& other) noexcept
		{
			if (this != &other)
				*issuer = std::move(*other.issuer);
			return *this;
		}

		const Derived& operator*() const
		{
			return cache;
		}

		Derived& operator*()
		{
			return cache;
		}

		const Derived* operator->() const
		{
			return &cache;
		}

		Derived* operator->()
		{
			return &cache;
		}

		const Derived* address() const
		{
			return &cache;
		}

		Derived* address()
		{
			return &cache;
		}
	};

	template<typename Derived> requires std::derived_from<Derived, internal::Issuer<Derived>>
	using PublicIssuerHandle = internal::Issuer<Derived>::Handle;
}
