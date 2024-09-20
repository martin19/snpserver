## project plan: "snp native client"

### ✨ MS1: connect to snp and stream dummy video to client   

GOAL: ✨ simplify server development via mock environment (create 30 and 60 fps dummy video streams)

* 🟦 TODO: implement snpserver
  * ✅ DONE: make snpserver compile, link and run on windows
  * ✅ DONE: make openh264 dll load on windows environment  
    * ✅ DONE: implement class SnpSinkNetworkTCP
      * ✅ DONE: proof class for disconnect, cleanup  
    * ✅ DONE: implement class video/SnpSourceDummy  
      * ✅ DONE: extend protocol to support SnpSourceDummy 
  * ✅ DONE: build protobuf from source via cmake.
  * ✅ DONE: implement sample main function for snpserver
   
  * 🟦 TODO: there is a cmake target to invoke protoc 
   
  * 🟦 TODO: what was streamId meant to do? -> define it
  * 🟦 TODO: there is setEnabled, start and stop - do we need both
    * 🟦 TODO: probably start/stop pipe components in reverse data flow direction
  * 🟦 TODO: verify component "compatibility" at runtime
  * 🟦 TODO: think about porttype (both, copy, mmap) - the idea is good, is the implementation 
  
* 🟦 TODO: implement snpclient
  * ✅ DONE: maybe some code/ideas can be shared between snpclient and snpserver
    --> share code in same cmake project.
  * ✅ DONE: review architecture of snpclient-web and bring the good parts to snpclient-native 🚀
  * ✅ DONE: snpclient: integrate openh264 decoder 
    * ✅ DONE: implement yuv2rgba (simple)
    * ✅ DONE: implement buffering of NALUs -> nalus should not be split/combined on protobuf layer
  * ✅ DONE: implement SnpSourceNetworkTcp
  * ✅ TODO: implement initial snappy protobuf protocol handler 
       --> implemented this directly in Tcp components, think about if this is the right place
   
  * 🟦 TODO: mermaid diagram for pipe structure
  * 🟦 TODO: fix temporarily deactivated websocket sink (marked usages with "TODO: websocket")
  * 🟦 TODO: implement simple option for configuration (ui/file/mock)  

### ✨ MS2:

🔲✅❎❌🟩