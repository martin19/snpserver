//
// Created by marti on 17/06/2021.
//

#include <stream/SnpProperty.h>
#include "PropertyUtil.h"
#include "network/snappyv1.pb.h"

void PropertyUtil::copySnpPropertyToProtocolProperty(snappyv1::Property& dst, SnpProperty& src) {
    dst.set_key(src.getName());
    switch(src.getType()) {
        case PROPERTY_TYPE_BOOL: {
            dst.set_type(snappyv1::PROPERTY_TYPE_BOOL);
            auto value = new snappyv1::Property_ValueBool();
            value->set_value(src.getValueBool());
            dst.set_allocated_value_bool(value);
        } break;
        case PROPERTY_TYPE_UINT32: {
            dst.set_type(snappyv1::PROPERTY_TYPE_UINT32);
            auto value = new snappyv1::Property_ValueUint32();
            value->set_value(src.getValueUint32());
            dst.set_allocated_value_uint32(value);
        } break;
        case PROPERTY_TYPE_DOUBLE: {
            dst.set_type(snappyv1::PROPERTY_TYPE_DOUBLE);
            auto value = new snappyv1::Property_ValueDouble();
            value->set_value(src.getValueDouble());
            dst.set_allocated_value_double(value);
        } break;
        case PROPERTY_TYPE_STRING: {
            dst.set_type(snappyv1::PROPERTY_TYPE_STRING);
            auto value = new snappyv1::Property_ValueString();
            value->set_value(src.getValueString());
            dst.set_allocated_value_string(value);
        } break;
    }
}

//void SnpProperty::fromProperty(snappyv1::Property *parameter) {
//    //TODO:
//}