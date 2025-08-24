#include "SnpProperty.h"

#include <utility>


SnpProperty::SnpProperty(std::string name, PropertyType type) {
    this->name = std::move(name);
    this->type = type;
    value_bool = false;
    value_double = .0;
    value_string = "";
    value_uint32 = 0;
}

SnpProperty::SnpProperty(std::string name, std::string value): SnpProperty(std::move(name), PROPERTY_TYPE_STRING) {
    value_string = std::move(value);
}

SnpProperty::SnpProperty(std::string name, uint32_t value): SnpProperty(std::move(name), PROPERTY_TYPE_UINT32) {
    value_uint32 = value;
}

SnpProperty::SnpProperty(std::string name, bool value): SnpProperty(std::move(name), PROPERTY_TYPE_BOOL) {
    value_bool = value;
}

SnpProperty::SnpProperty(std::string name, double value): SnpProperty(std::move(name), PROPERTY_TYPE_BOOL)  {
    value_double = value;
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
