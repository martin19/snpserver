## project plan: "snp native client"

### ✨ MS1: connect to snp and stream dummy video to client (22.09.2024)  

GOAL: ✨ simplify server development via mock environment (create 30 and 60 fps dummy video streams)

* ✅ DONE: implement snpserver
  * ✅ DONE: make snpserver compile, link and run on windows
  * ✅ DONE: make openh264 dll load on windows environment  
    * ✅ DONE: implement class SnpSinkNetworkTCP
      * ✅ DONE: proof class for disconnect, cleanup  
    * ✅ DONE: implement class video/SnpSourceDummy  
      * ✅ DONE: extend protocol to support SnpSourceDummy 
  * ✅ DONE: build protobuf from source via cmake.
  * ✅ DONE: implement sample main function for snpserver
  * ✅ DONE: there is a cmake target to invoke protoc (didn't work, created custom command)
  
* ✅ DONE: implement snpclient
  * ✅ DONE: maybe some code/ideas can be shared between snpclient and snpserver
    --> share code in same cmake project.
  * ✅ DONE: review architecture of snpclient-web and bring the good parts to snpclient-native 🚀
  * ✅ DONE: snpclient: integrate openh264 decoder 
    * ✅ DONE: implement yuv2rgba (simple)
    * ✅ DONE: implement buffering of NALUs -> nalus should not be split/combined on protobuf layer
  * ✅ DONE: implement SnpSourceNetworkTcp
  * ✅ DONE: implement initial snappy protobuf protocol handler 
       --> implemented this directly in Tcp localComponents, think about if this is the right place
  * ✅ DONE: implement SnpSinkDisplay

### ✨ MS1.1: refine localComponents and pipes architecture (29.09.2024)
  * ✅ DONE: implement fps counter in SnpCanvas
  * ✅ DONE: mermaid diagram for pipe structure 
  * ✅ DONE: there is setEnabled, start and stop - do we need both, probably delete enabled flag
             and move all code to start, stop
  * ✅ DONE: what was streamId meant to do? -> each pipe has its own streamId, data is 
             fed into correct pipe according to streamId
  * ✅ DONE: implement simple option for configuration (ui/file/mock) -> QSettings, snp.ini
  * ✅ DONE: rename protocol snappyv1 -> snp
  * ✅ DONE: connect components in pipe and connect to source pipe (network, local file etc.)
    * ✅ DONE: verify component "compatibility" at runtime
  * ✅ DONE: implement setting up a pipe via client 
    * ✅ DONE: send remote config to server and setup pipe using createPipes  
  * ✅ DONE: probably start/stop pipe localComponents in reverse data flow direction   

### ✨ MS2: hw accelerated video codecs (12.10.2024)
* implement more codecs
  * ✅ DONE: implement AMF (advanced media framework) encoder/decoder (https://github.com/GPUOpen-LibrariesAndSDKs/AMF/wiki/Guide-for-Video-CODEC-Encoder-App-Developers)
  * ✅ DONE: optimize local experiment for stable 1920x1080@60fps  
    * ✅ DONE: find problem with openh264, amf decoder cannot handle stream created by openh264 encoder  
    * -> despite initializing with 1080 height openh264 encodes to 1088 height which amf decoder does not like.  
  * ✅ DONE: implement AMF decoder, **encoder** cpu usage is really low (while openh264 exeeds 50%)  
  * ✅ TODO: implement VAAPI (intel) encoder
  * 
  * 🟦 TODO: implement different VAAPI inputs (dx11, ram, ..?) 
  * 🟦 TODO: implement VAAPI (intel) decoder  
  

  * 🟦 TODO: consider using c++ smart pointers, to avoid memory leaks, e.g. pipe factory.
  * 🟦 TODO: compile and make run release mode with optimizations
  * 🟦 TODO: think about porttype (both, copy, mmap) - the idea is good, is the implementation
  * 🟦 TODO: fix temporarily deactivated websocket sink (marked usages with "TODO: websocket")
 
🔲✅❎❌🟩