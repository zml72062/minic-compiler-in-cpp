#include "symtab.h"

Type::Type()
{

}

Type::Type(int _basic_type)
{
    this->basic_type = _basic_type;
}

Type::Type(int _basic_type, const std::vector<std::size_t>& _arr_lengths)
{
    this->basic_type = _basic_type;
    this->array_lengths = _arr_lengths;
}


Type::~Type()
{

}

void Type::create_array_type(std::size_t len)
{
    this->array_lengths.push_back(len);
}

void Type::add_ret_or_arg_type(const Type& _type)
{
    this->ret_and_arg_types.push_back(Type(_type));
}

std::string Type::to_str()
{
    std::string repr;
    for (auto& arr_len : this->array_lengths)
    {
        repr += ("array of " + std::to_string(arr_len) + " ");
    }
    if (this->basic_type == BASIC_TYPE_INT)
    {
        repr += "int";
    }
    else if (this->basic_type == BASIC_TYPE_CONST_INT)
    {
        repr += "const int";
    }
    else if (this->basic_type == BASIC_TYPE_FUNC)
    {
        auto args_size = ret_and_arg_types.size() - 1;
        repr += (ret_and_arg_types[0].to_str() + "(");
        for (auto i = 0; i < args_size; i++)
        {
            repr += (ret_and_arg_types[i + 1].to_str() + ",");
        }
        repr += ")";
    }
    else if (this->basic_type == BASIC_TYPE_NONE)
    {
        repr += "void";
    }
    return "<" + repr + ">";
}