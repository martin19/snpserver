#include "SnpProperty.h"

SnpProperty::SnpProperty(std::string name, PropertyType type) {
    this->name = name;
    this->type = type;
    value_bool = false;
    value_double = .0;
    value_string = "";
    value_uint32 = 0;
}

void SnpProperty::setValue(std::string value) {
    value_string = value;
}

void SnpProperty::setValue(bool value) {
    value_bool = value;
}

void SnpProperty::setValue(uint32_t value) {
    value_uint32 = value;
}

void SnpProperty::setValue(double value) {
    value_double = value;
}

const std::string &SnpProperty::getName() const {
    return name;
}

PropertyType SnpProperty::getType() const {
    return type;
}

const std::string &SnpProperty::getValueString() const {
    return value_string;
}

bool SnpProperty::getValueBool() const {
    return value_bool;
}

uint32_t SnpProperty::getValueUint32() const {
    return value_uint32;
}

double SnpProperty::getValueDouble() const {
    return value_double;
}
