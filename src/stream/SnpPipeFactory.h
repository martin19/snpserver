#ifndef SNPSERVER_SNPPIPEFACTORY_H
#define SNPSERVER_SNPPIPEFACTORY_H

#include <network/snappyv1.pb.h>
#include "SnpComponent.h"

class SnpPipe;
class SnpClientWebsocket;

class SnpPipeFactory {
public:
    static SnpPipe *createPipe(uint32_t streamId,
                               SnpComponent *source,
                               SnpComponent *sink,
                               snappyv1::StreamMedium medium,
                               snappyv1::StreamDirection direction,
                               snappyv1::StreamEndpoint endpoint,
                               snappyv1::StreamEncoding encoding);
private:
    static SnpPipe *createVideoPipe(uint32_t streamId,
                                    SnpComponent *source,
                                    SnpComponent *sink,
                                    snappyv1::StreamDirection direction,
                                    snappyv1::StreamEndpoint endpoint,
                                    snappyv1::StreamEncoding encoding);
    static SnpPipe *createAudioPipe(uint32_t streamId,
                                    SnpComponent *source,
                                    SnpComponent *sink,
                                    snappyv1::StreamDirection direction,
                                    snappyv1::StreamEndpoint endpoint,
                                    snappyv1::StreamEncoding encoding);
    static SnpPipe *createPeripheralPipe(uint32_t streamId,
                                          SnpComponent *source,
                                          SnpComponent *sink,
                                          snappyv1::StreamDirection direction,
                                          snappyv1::StreamEndpoint endpoint,
                                          snappyv1::StreamEncoding encoding);
    static SnpPipe *createVideoInputPipe(uint32_t streamId,
                                         SnpComponent *source,
                                         snappyv1::StreamEndpoint endpoint,
                                         snappyv1::StreamEncoding encoding);
    static SnpPipe *createVideoOutputPipe(uint32_t streamId,
                                          SnpComponent *sink,
                                          snappyv1::StreamEndpoint endpoint,
                                          snappyv1::StreamEncoding encoding);
    static SnpPipe *createAudioInputPipe(uint32_t streamId,
                                         SnpComponent *source,
                                         snappyv1::StreamEndpoint endpoint,
                                         snappyv1::StreamEncoding encoding);
    static SnpPipe *createAudioOutputPipe(uint32_t streamId,
                                          SnpComponent *sink,
                                          snappyv1::StreamEndpoint endpoint,
                                          snappyv1::StreamEncoding encoding);
    static SnpPipe *createPeripheralInputPipe(uint32_t streamId,
                                              SnpComponent *source,
                                              snappyv1::StreamEndpoint endpoint,
                                              snappyv1::StreamEncoding encoding);
    static SnpPipe *createPeripheralOutputPipe(uint32_t streamId,
                                               SnpComponent *sink,
                                               snappyv1::StreamEndpoint endpoint,
                                               snappyv1::StreamEncoding encoding);
};


#endif //SNPSERVER_SNPPIPEFACTORY_H
