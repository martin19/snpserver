#ifndef SNPSERVER_SNPPROPERTY_H
#define SNPSERVER_SNPPROPERTY_H

#include <cstdint>
#include <string>

enum PropertyType {
    PROPERTY_TYPE_STRING = 0,
    PROPERTY_TYPE_BOOL = 1,
    PROPERTY_TYPE_UINT32 = 2,
    PROPERTY_TYPE_DOUBLE = 3,
};

class SnpProperty {
public:
    SnpProperty(std::string name, PropertyType type);
    SnpProperty(std::string name, std::string value);
    SnpProperty(std::string name, uint32_t value);
    SnpProperty(std::string name, bool value);
    SnpProperty(std::string name, double value);

    void setValue(std::string value);
    void setValue(bool value);
    void setValue(uint32_t value);
    void setValue(double value);
    [[nodiscard]] const std::string &getName() const;
    [[nodiscard]] PropertyType getType() const;
private:
    PropertyType type;
    std::string name;
public:
    [[nodiscard]] const std::string &getValueString() const;
    [[nodiscard]] bool getValueBool() const;
    [[nodiscard]] uint32_t getValueUint32() const;
    [[nodiscard]] double getValueDouble() const;

private:
    std::string value_string;
    bool value_bool;
    uint32_t value_uint32;
    double value_double;



};

#endif //SNPSERVER_SNPPROPERTY_H
