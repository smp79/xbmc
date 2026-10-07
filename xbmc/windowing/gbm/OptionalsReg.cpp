/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "OptionalsReg.h"

//-----------------------------------------------------------------------------
// VAAPI
//-----------------------------------------------------------------------------
#if defined(HAVE_LIBVA)
#include "cores/VideoPlayer/DVDCodecs/Video/VAAPI.h"

#include <cstdlib>
#include <map>
#include <mutex>

#include <fcntl.h>
#include <unistd.h>
#include <va/va_drm.h>
#include <xf86drm.h>
#if defined(HAS_GL)
#include "cores/VideoPlayer/VideoRenderers/HwDecRender/RendererVAAPIGL.h"
#endif
#if defined(HAS_GLES)
#include "cores/VideoPlayer/VideoRenderers/HwDecRender/RendererVAAPIGLES.h"
#endif

namespace KODI
{
namespace WINDOWING
{
namespace GBM
{

class CVaapiProxy : public VAAPI::IVaapiWinSystem
{
public:
  CVaapiProxy(int fd) : m_fd(fd){};
  virtual ~CVaapiProxy() = default;
  VADisplay GetVADisplay() override;
  void ReleaseVADisplay(VADisplay display) override;
  VADisplay GetSharedVADisplay() { return vaGetDisplayDRM(m_fd); }
  void* GetEGLDisplay() override { return eglDisplay; };

  VADisplay vaDpy;
  void* eglDisplay;

private:
  int m_fd{-1};
  std::mutex m_mutex;
  std::map<VADisplay, int> m_privateFds;
};

VADisplay CVaapiProxy::GetVADisplay()
{
  char* node = drmGetRenderDeviceNameFromFd(m_fd);
  if (node)
  {
    const int fd = open(node, O_RDWR | O_CLOEXEC);
    free(node);
    if (fd >= 0)
    {
      VADisplay display = vaGetDisplayDRM(fd);
      if (display)
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_privateFds[display] = fd;
        return display;
      }
      close(fd);
    }
  }
  return GetSharedVADisplay();
}

void CVaapiProxy::ReleaseVADisplay(VADisplay display)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  auto it = m_privateFds.find(display);
  if (it != m_privateFds.end())
  {
    close(it->second);
    m_privateFds.erase(it);
  }
}

CVaapiProxy* VaapiProxyCreate(int fd)
{
  return new CVaapiProxy(fd);
}

void VaapiProxyDelete(CVaapiProxy* proxy)
{
  delete proxy;
}

void VaapiProxyConfig(CVaapiProxy* proxy, void* eglDpy)
{
  proxy->vaDpy = proxy->GetSharedVADisplay();
  proxy->eglDisplay = eglDpy;
}

void VAAPIRegister(CVaapiProxy* winSystem, bool deepColor)
{
  VAAPI::CDecoder::Register(winSystem, deepColor);
}

#if defined(HAS_GL)
void VAAPIRegisterRenderGL(CVaapiProxy* winSystem, bool& general, bool& deepColor)
{
  CRendererVAAPIGL::Register(winSystem, winSystem->vaDpy, winSystem->eglDisplay, general,
                             deepColor);
}
#endif

#if defined(HAS_GLES)
void VAAPIRegisterRenderGLES(CVaapiProxy* winSystem, bool& general, bool& deepColor)
{
  CRendererVAAPIGLES::Register(winSystem, winSystem->vaDpy, winSystem->eglDisplay, general,
                               deepColor);
}
#endif
} // namespace GBM
} // namespace WINDOWING
} // namespace KODI

#else

namespace KODI
{
namespace WINDOWING
{
namespace GBM
{

class CVaapiProxy
{
};

CVaapiProxy* VaapiProxyCreate(int fd)
{
  return nullptr;
}

void VaapiProxyDelete(CVaapiProxy* proxy)
{
}

void VaapiProxyConfig(CVaapiProxy* proxy, void* eglDpy)
{
}

void VAAPIRegister(CVaapiProxy* winSystem, bool deepColor)
{
}

#if defined(HAS_GL)
void VAAPIRegisterRenderGL(CVaapiProxy* winSystem, bool& general, bool& deepColor)
{
}
#endif

#if defined(HAS_GLES)
void VAAPIRegisterRenderGLES(CVaapiProxy* winSystem, bool& general, bool& deepColor)
{
}
#endif
} // namespace GBM
} // namespace WINDOWING
} // namespace KODI

#endif
