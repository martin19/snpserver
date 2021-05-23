#include "SnpProtocol.h"
#include "snappyv1.pb.h"

using namespace snappyv1;

void SnpProtocol::sendServerInfo(SnpClient &c) {
    auto serverInfo = new ServerInfo();
    serverInfo->set_platform(PLATFORM_RASPBERRY);

    //add one source
    Source *source = serverInfo->add_available_sources();
    source->set_type(SOURCE_TYPE_VIDEO);
    source->set_sub_type(SOURCE_SUB_TYPE_X11);

    //parameter width
    {
        Parameter *p = source->add_parameters();
        p->set_param_type(PARAMETER_TYPE_UINT32);
        p->set_param_key("width");
        auto *value = new Parameter_ValueUint32();
        value->set_value(1920);
        p->set_allocated_value_uint32(value);
    }

    //parameter height
    {
        Parameter *p = source->add_parameters();
        p->set_param_type(PARAMETER_TYPE_UINT32);
        p->set_param_key("height");
        auto *value = new Parameter_ValueUint32();
        value->set_value(1080);
        p->set_allocated_value_uint32(value);
    }

    Encoder *encoder = serverInfo->add_available_encoders();
    encoder->set_type(ENCODER_TYPE_H264_HARDWARE);

    //parameter qp
    {
        Parameter *p = encoder->add_parameters();
        p->set_param_type(PARAMETER_TYPE_UINT32);
        p->set_param_key("qp");
        auto *value = new Parameter_ValueUint32();
        value->set_value(42);
        value->set_min(10);
        value->set_max(50);
        p->set_allocated_value_uint32(value);
    }

    //parameter string
    {
        Parameter *p = encoder->add_parameters();
        p->set_param_type(PARAMETER_TYPE_STRING);
        p->set_param_key("fookey");
        auto *value = new Parameter_ValueString();
        value->set_value("barvalue");
        p->set_allocated_value_string(value);
    }

    //wrap server info message in envelope
    Message msgEnvelope = Message();
    msgEnvelope.set_type(MESSAGE_TYPE_SERVER_INFO);
    msgEnvelope.set_allocated_server_info(serverInfo);
    std::string message = msgEnvelope.SerializeAsString();
    c.send(message);
}
