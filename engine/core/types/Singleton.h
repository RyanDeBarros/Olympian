#pragma once

namespace oly
{
	template<typename T>
	class Singleton
	{
	protected:
		Singleton() = default;

	private:
		Singleton(const Singleton&) = delete;
		Singleton(Singleton&&) = delete;
		Singleton& operator=(const Singleton&) = delete;
		Singleton& operator=(Singleton&&) = delete;

	public:
		~Singleton() = default;

		static T& instance()
		{
			static T inst;
			return inst;
		}
	};
}
