#pragma once

#include <memory>

namespace oly::internal
{
	// Issuer must be allocated on heap
	template<typename Derived>
	class Issuer : public std::enable_shared_from_this<Issuer<Derived>>
	{
		using Super = std::enable_shared_from_this<Issuer<Derived>>;

	protected:
		class Handle
		{
			friend class Issuer<Derived>;
			std::weak_ptr<Issuer<Derived>> issuer;

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
			};

			Accessor lock() const
			{
				return Accessor(issuer.lock());
			}

			void reset()
			{
				issuer.reset();
			}
		};

		Handle issue()
		{
			return Handle(Super::shared_from_this());
		}
	};
}
