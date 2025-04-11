#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>

namespace oly
{
	constexpr unsigned int unsigned_mod(int pos, int mod)
	{
		pos = pos % mod;
		return pos >= 0 ? pos : pos + mod;
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

	template<typename T>
	struct Range
	{
		T initial = T();
		T length = T();

		constexpr T end() const { return initial + length; }
		constexpr bool contains(T in) const { return in >= initial && in < end(); }
		constexpr bool contains(Range<T> sub) const { return contains(sub.initial) && sub.end() <= end(); }
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

		// LATER make these private and make Universe a friend so it can call these.
		void init() { _now = glfwGetTime(); _delta = 1.0f / 60.0f; }
		void sync() { double n = glfwGetTime(); _delta = n - _now; _now = n; }
	};
	inline TimeImpl TIME;

	inline float rng() { return (float)rand() / RAND_MAX; }
}
