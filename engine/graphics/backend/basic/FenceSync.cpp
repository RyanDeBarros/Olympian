#include "FenceSync.h"

namespace oly::graphics
{
	FenceSync::FenceSync()
	{
		_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	}

	FenceSync::FenceSync(FenceSync&& other) noexcept
		: _sync(other._sync)
	{
		other._sync = nullptr;
	}

	FenceSync::~FenceSync()
	{
		glDeleteSync(_sync);
	}

	FenceSync& FenceSync::operator=(FenceSync&& other) noexcept
	{
		if (this != &other)
		{
			glDeleteSync(_sync);
			_sync = other._sync;
			other._sync = nullptr;
		}
		return *this;
	}

	bool FenceSync::wait(GLuint64 timeout_ns) const
	{
		GLenum result = glClientWaitSync(_sync, GL_SYNC_FLUSH_COMMANDS_BIT, timeout_ns);
		if (result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED)
			return true;
		else if (result == GL_TIMEOUT_EXPIRED)
			return false;
		else
			throw Error(ErrorCode::WaitFailed);
	}

	bool FenceSync::signaled() const
	{
		GLint result;
		glGetSynciv(_sync, GL_SYNC_STATUS, sizeof(GLint), nullptr, &result);
		return result == GL_SIGNALED;
	}
}
