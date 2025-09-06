#include "Time.h"

namespace oly::internal
{
	void TimeImpl::process()
	{
		_processed_now = _raw_now * time_scale;
		_inv_processed_now = 1.0 / _processed_now;

		_processed_delta = std::min(_raw_delta, frame_length_clip) * time_scale;
		_inv_processed_delta = 1.0 / _processed_delta;
	}

	void TimeImpl::init()
	{
		_raw_delta = 1.0 / 60.0;
		_raw_now = glfwGetTime();
		process();
	}

	void TimeImpl::sync()
	{
		double n = glfwGetTime();
		_raw_delta = n - _raw_now;
		_raw_now = n;
		process();
	}
}
