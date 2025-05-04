#pragma once

#include "external/GL.h"
#include "core/base/Errors.h"

namespace oly
{
	namespace rendering
	{
		class FenceSync
		{
			GLsync sync;

		public:
			FenceSync() { sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0); }
			FenceSync(const FenceSync&) = delete;
			FenceSync(FenceSync&& other) noexcept : sync(other.sync) { other.sync = nullptr; }
			~FenceSync() { glDeleteSync(sync); }
			FenceSync& operator=(FenceSync&& other) noexcept
			{
				if (this != &other)
				{
					glDeleteSync(sync);
					sync = other.sync;
					other.sync = nullptr;
				}
				return *this;
			}

			bool wait(GLuint64 timeout_ns) const
			{
				GLenum result = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, timeout_ns);
				if (result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED)
					return true;
				else if (result == GL_TIMEOUT_EXPIRED)
					return false;
				else
					throw Error(ErrorCode::WAIT_FAILED);
			}

			bool signaled() const
			{
				GLint result;
				glGetSynciv(sync, GL_SYNC_STATUS, sizeof(GLint), nullptr, &result);
				return result == GL_SIGNALED;
			}
		};
	}
}
