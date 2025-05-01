#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <variant>

namespace oly
{
	constexpr unsigned int unsigned_mod(int pos, int mod)
	{
		pos = pos % mod;
		return pos >= 0 ? pos : pos + mod;
	}
	
	constexpr int roundi(float v)
	{
		return (v > 0.0f) ? static_cast<int>(v + 0.5f) : static_cast<int>(v - 0.5f);
	}

	template<typename T>
	constexpr T max(const T& first, const T& second)
	{
		return first >= second ? first : second;
	}

	template<typename T, typename... Rest>
	inline T max(const T& first, const Rest&... rest)
	{
		return max(first, max(rest...));
	}

	template<typename T>
	inline T dupl(const T& obj)
	{
		return obj;
	}

	template<typename Struct, typename Member>
	constexpr std::size_t member_offset(Member Struct::* member)
	{
		return reinterpret_cast<std::size_t>(&(reinterpret_cast<Struct*>(0)->*member));
	}

	template<typename ToVariant, typename FromVariant>
	inline ToVariant convert_variant(const FromVariant& from)
	{
		return std::visit([](const auto& f) { return ToVariant{ f }; }, from);
	}

	template<typename T>
	struct Range
	{
		T initial = T();
		T length = T();

		constexpr T end() const { return initial + length; }
		constexpr bool contains(T in) const { return in >= initial && in < end(); }
		constexpr bool contains(Range<T> sub) const { return contains(sub.initial) && sub.end() <= end(); }
		constexpr bool overlaps(Range<T> other) const { return contains(other.initial) || other.contains(initial); }
		constexpr bool adjacent(Range<T> other) const { return end() == other.initial || other.end() == initial; }
		void merge(const Range<T>& other)
		{
			T new_end = std::max(end(), other.end());
			initial = std::min(initial, other.initial);
			length = new_end - initial;
		}
	};

	template<typename T>
	struct RangeComparator
	{
		constexpr bool operator()(const Range<T>& lhs, const Range<T>& rhs) const
		{
			return lhs.initial < rhs.initial || (lhs.initial == rhs.initial && lhs.length < rhs.length);
		}
	};

	template<typename T>
	struct Interval
	{
		T left = T(), right = T();

		T length() const { return right - left; }

		enum class Mode
		{
			CLOSED,
			OPEN,
			RIGHT_OPEN,
			LEFT_OPEN
		};
		template<Mode mode = Mode::CLOSED>
		bool contains(T pt) const
		{
			if constexpr (mode == Mode::CLOSED)
				return left <= pt && pt <= right;
			else if constexpr (mode == Mode::OPEN)
				return left < pt && pt < right;
			else if constexpr (mode == Mode::RIGHT_OPEN)
				return left <= pt && pt < right;
			else
				return left < pt && pt <= right;
		}
	};

	template<typename T>
	inline void swap_adjacent_subbuffers(T* buf, size_t offset1, size_t count1, size_t offset2, size_t count2)
	{
		if (count1 < count2)
		{
			std::swap(count1, count2);
			std::swap(offset1, offset2);
		}

		T* temp = new T[count1];
		memcpy(temp, buf + offset1, count1 * sizeof(T));
		if (offset1 < offset2)
		{
			memcpy(buf + offset1, buf + offset2, count2 * sizeof(T));
			memcpy(buf + offset2 + count2 - count1, temp, count1 * sizeof(T));
		}
		else
		{
			memcpy(buf + offset1 + count1 - count2, buf + offset2, count2 * sizeof(T));
			memcpy(buf + offset2, temp, count1 * sizeof(T));
		}
		delete[] temp;
	}

	template<typename T>
	inline void vector_erase(std::vector<T>& vec, const T& el)
	{
		vec.erase(std::find(vec.begin(), vec.end(), el));
	}

	template<typename T>
	concept numeric = std::integral<T> || std::floating_point<T>;
	class TimeImpl
	{
		double _now;
		double _delta;

	public:
		template<numeric T>
		T now() const { return (T)_now; }
		template<numeric T>
		T delta() const { return (T)_delta; }

	private:
		friend class Context;
		void init() { _now = glfwGetTime(); _delta = 1.0f / 60.0f; }
		void sync() { double n = glfwGetTime(); _delta = n - _now; _now = n; }
	};
	inline TimeImpl TIME;

	template<typename T>
	inline std::shared_ptr<T> move_shared(T&& obj) { return std::make_shared<T>(std::move(obj)); }
	template<typename T>
	inline std::unique_ptr<T> move_unique(T&& obj) { return std::make_unique<T>(std::move(obj)); }
}
