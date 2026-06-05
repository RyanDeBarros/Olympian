#pragma once

#include "external/GL.h"
#include "core/base/Errors.h"

namespace oly::graphics
{
	class FenceSync
	{
		GLsync _sync;

	public:
		FenceSync();
		FenceSync(const FenceSync&) = delete;
		FenceSync(FenceSync&& other) noexcept;
		~FenceSync();
		FenceSync& operator=(FenceSync&& other) noexcept;

		bool wait(GLuint64 timeout_ns) const;
		bool signaled() const;
	};
}
