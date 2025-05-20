#pragma once

#include <functional>
#include <memory>

namespace oly
{
	template<typename T>
	struct Functor;

	template<typename R, typename... Args>
	struct Functor<R(Args...)>
	{
		virtual ~Functor() = default;
		virtual R operator()(Args...) = 0;
	};

	template<typename F, typename R, typename... Args>
	struct Callable : public Functor<R(Args...)>
	{
		explicit Callable(const F& func) : f(func) {}
		explicit Callable(F&& func) : f(std::move(func)) {}

		R operator()(Args... args) override {
			return f(std::forward<Args>(args)...);
		}

	private:
		F f;
	};

	template<typename R, typename... Args, typename F>
	std::shared_ptr<Functor<R(Args...)>> make_functor(F&& f) {
		return std::make_shared<Callable<std::decay_t<F>, R, Args...>>(std::forward<F>(f));
	}

	template<typename R, typename... Args>
	std::shared_ptr<Functor<R(Args...)>> make_functor(const std::function<R(Args...)>& f) {
		return std::make_shared<Callable<std::decay_t<decltype(f)>, R, Args...>>(f);
	}

	template<typename R, typename... Args>
	std::shared_ptr<Functor<R(Args...)>> make_functor(std::function<R(Args...)>&& f) {
		return std::make_shared<Callable<std::decay_t<decltype(f)>, R, Args...>>(std::move(f));
	}
}
