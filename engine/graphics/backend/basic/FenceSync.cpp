#include "FenceSync.h"

#include "core/base/Errors.h"

namespace oly::graphics
{
    FenceSync::FenceSync()
    {
        sync_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }

    FenceSync::FenceSync(FenceSync&& o) noexcept
        : sync_(o.sync_)
    {
        o.sync_ = nullptr;
    }

    FenceSync::~FenceSync()
    {
        glDeleteSync(sync_);
    }

    FenceSync& FenceSync::operator=(FenceSync&& o) noexcept
    {
        if (this != &o)
        {
            glDeleteSync(sync_);
            sync_ = o.sync_;
            o.sync_ = nullptr;
        }
        return *this;
    }

    bool FenceSync::wait(GLuint64 timeout_ns) const
    {
        GLenum result = glClientWaitSync(sync_, GL_SYNC_FLUSH_COMMANDS_BIT, timeout_ns);
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
        glGetSynciv(sync_, GL_SYNC_STATUS, sizeof(GLint), nullptr, &result);
        return result == GL_SIGNALED;
    }
}
