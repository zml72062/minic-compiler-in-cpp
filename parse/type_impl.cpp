#include "symtab.h"

Type::Type()
{

}

Type::Type(int _basic_type)
{
    this->basic_type = _basic_type;
}

Type::~Type()
{

}

void Type::create_array_type(std::size_t len)
{
    this->array_lengths.push_back(len);
}

std::string Type::to_str()
{
    std::string repr;
    for (auto& arr_len : this->array_lengths)
    {
        repr += ("array of " + std::to_string(arr_len) + " ");
    }
    const char* _basic_types[] = {"int", "const int", "func"};
    repr += (_basic_types[this->basic_type] + std::string("s"));
    return "<" + repr + ">";
}