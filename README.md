# drm-gl screen capture method

The `SnpSourceGL` module implements a screen capture method which uses `libdrm` and `egl`.
There are some online resources for programming `libdrm` from
a user space perspective, however most documentation seems to go into very much detail
from a kernel perspective.

To understand the capture method, one needs to understand a little bit of `drm`, `egl`
and some `dma-buf`. I'll explain the interesting parts in the sections below.

## DRM/KMS

DRM/KMS (direct rendering manager, kernel mode setting) is an abstraction for specific
hardware resources of different vendors which provides an api through `ioctl` calls.
As ioctls are quite cumbersome to program, a wrapper library called `libdrm` has been
created which should make programming a little bit less painful. With it directly
access of GPU resources from user space is possible. What are these resources?

* framebuffers
* planes
* crtcs
* encoders
* connectors

#### Connector
Represents an actual connector on the GPU (such as HDMI-1, or Composite out..)
#### Encoder
Represents some signal transformation module to transform a signal from CRTC for  
one or multiple encoders.
#### CRTC
Literally "Cathode ray tube controller", abstracts away display timings.
#### Planes
An image on the framebuffer can be made up of several planes.
#### Framebuffer
A framebuffers is essentially the chunks of gpu memory which contains the pixel information
in a specific format (e.g, 1920x1080x32 - RGBA or some vendor specific pixel format) for example.
There is a *primary* framebuffer which contains the actual pixel information displayed
on the screen. There can be other framebuffers, e.g. a smaller one for the cursor is common.

The following illustration has been taken from kernel org shows the relationship
between these resource objects.

![](./docs/kms.png)

### Framebuffer Discovery
It turns out, to access a framebuffer it must be discovered first. Taking a look at the diagram
suggests the method to do so: tracing the path from the bottommost object (e.g. connector HDMI-1)
to the framebuffer at the top. Then using the method `drmModeGetFB` a handle to the framebuffer
object can be acquired.

### Framebuffer Mapping
The framebuffer can be mapped directly to user-space memory which is useful to read back the
pixels quickly or pass the to a compression algorithm, such as h.264.

### Framebuffer Format

...


