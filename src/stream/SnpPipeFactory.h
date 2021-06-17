#ifndef SNPSERVER_SNPPIPEFACTORY_H
#define SNPSERVER_SNPPIPEFACTORY_H

#include <network/snappyv1.pb.h>

class SnpPipe;
class SnpClient;

class SnpPipeFactory {
public:
    static SnpPipe *createPipe(uint32_t streamId, SnpClient *client, snappyv1::StreamMedium medium,
                               snappyv1::StreamDirection direction, snappyv1::StreamEndpoint endpoint,
                               snappyv1::StreamEncoding encoding);
private:
    static SnpPipe *createVideoPipe(uint32_t streamId, SnpClient *client, snappyv1::StreamDirection direction,
                                    snappyv1::StreamEndpoint endpoint, snappyv1::StreamEncoding encoding);
    static SnpPipe *createAudioPipe(uint32_t streamId, SnpClient *client, snappyv1::StreamDirection direction,
                                    snappyv1::StreamEndpoint endpoint, snappyv1::StreamEncoding encoding);
    static SnpPipe *createPeripherialPipe(uint32_t streamId, SnpClient *client, snappyv1::StreamDirection direction,
                                          snappyv1::StreamEndpoint endpoint,snappyv1::StreamEncoding encoding);
    static SnpPipe *createVideoInputPipe(uint32_t streamId, SnpClient *client, snappyv1::StreamEndpoint endpoint,
                                         snappyv1::StreamEncoding encoding);
    static SnpPipe *createVideoOutputPipe(uint32_t streamId, SnpClient *client, snappyv1::StreamEndpoint endpoint,
                                          snappyv1::StreamEncoding encoding);
    static SnpPipe *createAudioInputPipe(uint32_t streamId, SnpClient *client, snappyv1::StreamEndpoint endpoint,
                                         snappyv1::StreamEncoding encoding);
    static SnpPipe *createAudioOutputPipe(uint32_t streamId, SnpClient *client, snappyv1::StreamEndpoint endpoint,
                                          snappyv1::StreamEncoding encoding);
    static SnpPipe *createPeripherialInputPipe(uint32_t streamId, SnpClient *client, snappyv1::StreamEndpoint endpoint,
                                               snappyv1::StreamEncoding encoding);
    static SnpPipe *createPeripherialOutputPipe(uint32_t streamId, SnpClient *client, snappyv1::StreamEndpoint endpoint,
                                                snappyv1::StreamEncoding encoding);
};


#endif //SNPSERVER_SNPPIPEFACTORY_H
