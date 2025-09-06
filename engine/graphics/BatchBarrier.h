#pragma once

namespace oly
{
	struct BatchBarrier
	{
		bool barrier;

		constexpr explicit BatchBarrier(bool barrier) : barrier(barrier) {}
		constexpr operator bool() const { return barrier; }
	};

	namespace batch
	{
		constexpr BatchBarrier BARRIER = BatchBarrier(true);
		constexpr BatchBarrier PARALLEL = BatchBarrier(false);
	}
}
