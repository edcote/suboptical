#include "include/system_context.h"

#include <dos.h>
#include <dpmi.h>
#include <go32.h>
#include <sys/nearptr.h>

#include "include/logger.h"

namespace subdemo {

SystemContext::SystemContext()
    : original_video_mode_(0x03), timers_installed_(false) {}

std::unique_ptr<SystemContext> SystemContext::Create() {
  auto context = std::unique_ptr<SystemContext>(new SystemContext());
  if (!context->Init()) {
    return nullptr;
  }
  return context;
}

SystemContext::~SystemContext() {
  StopTimers();
  SetVideoMode(original_video_mode_);
  __djgpp_nearptr_disable();
}

bool SystemContext::Init() {
  // Save original mode so we can restore it on exit.
  original_video_mode_ = GetVideoMode();

  if (__djgpp_nearptr_enable() == 0) {
    LogError("Failed to enable near pointer access.");
    return false;
  }

  video_ = std::make_unique<Video>();
  if (!video_->InitModeX()) {
    LogError("Failed to initialize VGA Mode X.");
    return false;
  }

  resource_manager_ = std::make_unique<ResourceManager>();

  return true;
}

}  // namespace subdemo
