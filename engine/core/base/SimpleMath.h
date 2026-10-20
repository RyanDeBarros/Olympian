#pragma once

#include "core/base/Constants.h"

#include <imp/hash.hpp>

namespace oly
{
	constexpr bool empty_str(const char* s)
	{
		return s == nullptr || *s == '\0';
	}

	constexpr unsigned int unsigned_mod(int pos, int mod)
	{
		pos = pos % mod;
		return pos >= 0 ? pos : pos + mod;
	}

	constexpr unsigned int unsigned_mod(int pos, size_t mod)
	{
		return unsigned_mod(pos, (int)mod);
	}

	constexpr float unsigned_fmod(float x, float mod)
	{
		x = x - static_cast<float>(static_cast<int>(x / mod)) * mod;
		return x >= 0.0f ? x : x + mod;
	}

	constexpr int roundi(float v_)
	{
		return (v_ >= 0.0f) ? static_cast<int>(v_ + 0.5f) : static_cast<int>(v_ - 0.5f);
	}

	class BigSize
	{
		size_t order_ = 0;
		size_t v_;

	public:
        constexpr BigSize(size_t v = 0)
            : v_(v)
        {
        }
		
		constexpr BigSize& operator++()
        {
            if (v_ == SIZE_MAX)
            {
                ++order_;
                v_ = 0;
            }
            else
                ++v_;

            return *this;
        }
		
        constexpr BigSize operator++(int)
        {
            BigSize s = *this;
            ++*this;
            return s;
        }
		
        constexpr bool operator==(const BigSize&) const = default;
        constexpr bool operator!=(const BigSize&) const = default;
		
        constexpr bool operator<(const BigSize& rhs) const
        {
            return order_ < rhs.order_ || (order_ == rhs.order_ && v_ < rhs.order_);
        }

        constexpr bool operator<=(const BigSize& rhs) const
        {
            return order_ < rhs.order_ || (order_ == rhs.order_ && v_ <= rhs.order_);
        }

        constexpr bool operator>(const BigSize& rhs) const
        {
            return order_ >= rhs.order_ && (order_ != rhs.order_ || v_ > rhs.order_);
        }

        constexpr bool operator>=(const BigSize& rhs) const
        {
            return order_ >= rhs.order_ && (order_ != rhs.order_ || v_ <= rhs.order_);
        }

        constexpr size_t get_order() const
        {
            return order_;
        }
        
        constexpr size_t get_remainder() const
        {
            return v_;
        }
        
        constexpr size_t hash() const
        {
            return imp::hasher()
                .with(v_)
                .with(order_);
        }
	};

	using fpair = std::pair<float, float>;
}

template<>
struct std::hash<oly::BigSize>
{
	size_t operator()(const oly::BigSize& bgz) const
    {
        return bgz.hash();
    }
};
