#include "Time.h"

namespace oly::internal
{
	void TimeImpl::init()
	{
		_delta = 1.0 / 60.0;
		_lagged = 0.0;
		
		_now = glfwGetTime();
		_inv_now = 1.0 / _now;
		_inv_delta = 1.0 / _delta;
	}

	void TimeImpl::sync()
	{
		double n = glfwGetTime();
		_delta = n - _now;
		_lagged += std::max(_delta - frame_length_clip, 0.0);
		_delta = std::min(_delta, frame_length_clip);

		_now = n;
		_inv_now = 1.0 / _now;
		_inv_delta = 1.0 / _delta;
	}
}
