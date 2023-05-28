#include "symbols.h"
#include "../utils.h"

Symbol::Symbol()
{
    this->parent = nullptr;
}

Symbol::~Symbol()
{

}

LexemePacker::LexemePacker(const Lexeme& _lexeme): Symbol(), lexeme(_lexeme)
{

}

Variable::Variable(SymbolTableEntry* _entry): Symbol()
{
    this->symbol_idx = SYMBOL_VARIABLE;
    this->entry = _entry;
}

Function::Function(SymbolTableEntry* _entry): Symbol()
{
    this->symbol_idx = SYMBOL_FUNCTION;
    this->entry = _entry;
}

void Function::add_argument(Symbol* _symbol)
{
    this->children.push_back(_symbol);
    _symbol->parent = this;
}

Number::Number(const std::vector<std::size_t>& _sizes, 
               const std::vector<int>& _value): Symbol()
{
    this->symbol_idx = SYMBOL_NUMBER;
    this->sizes = _sizes;
    this->value = _value;
}

UnaryExpression::UnaryExpression(int _operation, Symbol* _operand): Symbol()
{
    this->symbol_idx = SYMBOL_UNARY;
    this->operation = _operation;
    this->children.push_back(_operand);
    _operand->parent = this;
}

MulExpression::MulExpression(int _operation, Symbol* _loperand, Symbol* _roperand): Symbol()
{
    this->symbol_idx = SYMBOL_MUL;
    this->operation = _operation;
    this->children.push_back(_loperand);
    _loperand->parent = this;
    this->children.push_back(_roperand);
    _roperand->parent = this;
}

AddExpression::AddExpression(int _operation, Symbol* _loperand, Symbol* _roperand): Symbol()
{
    this->symbol_idx = SYMBOL_ADD;
    this->operation = _operation;
    this->children.push_back(_loperand);
    _loperand->parent = this;
    this->children.push_back(_roperand);
    _roperand->parent = this;
}

RelExpression::RelExpression(int _operation, Symbol* _loperand, Symbol* _roperand): Symbol()
{
    this->symbol_idx = SYMBOL_REL;
    this->operation = _operation;
    this->children.push_back(_loperand);
    _loperand->parent = this;
    this->children.push_back(_roperand);
    _roperand->parent = this;
}

EqExpression::EqExpression(int _operation, Symbol* _loperand, Symbol* _roperand): Symbol()
{
    this->symbol_idx = SYMBOL_EQ;
    this->operation = _operation;
    this->children.push_back(_loperand);
    _loperand->parent = this;
    this->children.push_back(_roperand);
    _roperand->parent = this;
}

AndExpression::AndExpression(Symbol* _loperand, Symbol* _roperand): Symbol()
{
    this->symbol_idx = SYMBOL_AND;
    this->children.push_back(_loperand);
    _loperand->parent = this;
    this->children.push_back(_roperand);
    _roperand->parent = this;
}

OrExpression::OrExpression(Symbol* _loperand, Symbol* _roperand): Symbol()
{
    this->symbol_idx = SYMBOL_OR;
    this->children.push_back(_loperand);
    _loperand->parent = this;
    this->children.push_back(_roperand);
    _roperand->parent = this;
}

IndexExpression::IndexExpression(Symbol* _loperand, Symbol* _roperand): Symbol()
{
    this->symbol_idx = SYMBOL_INDEX;
    this->children.push_back(_loperand);
    _loperand->parent = this;
    this->children.push_back(_roperand);
    _roperand->parent = this;
}

LexemePacker* LexemePacker::copy()
{
    return new LexemePacker(*this);
}

Symbol* Symbol::copy()
{
    return new Symbol(*this);
}

Variable* Variable::copy()
{
    return new Variable(*this);
}

Function* Function::copy()
{
    return new Function(*this);
}

Number* Number::copy()
{
    return new Number(*this);
}

UnaryExpression* UnaryExpression::copy()
{
    return new UnaryExpression(*this);
}

MulExpression* MulExpression::copy()
{
    return new MulExpression(*this);
}

AddExpression* AddExpression::copy()
{
    return new AddExpression(*this);
}

RelExpression* RelExpression::copy()
{
    return new RelExpression(*this);
}

EqExpression* EqExpression::copy()
{
    return new EqExpression(*this);
}

AndExpression* AndExpression::copy()
{
    return new AndExpression(*this);
}

OrExpression* OrExpression::copy()
{
    return new OrExpression(*this);
}

IndexExpression* IndexExpression::copy()
{
    return new IndexExpression(*this);
}

std::string Symbol::to_str()
{
    return std::string();
}

std::string LexemePacker::to_str()
{
    return std::string();
}

std::string Variable::to_str()
{
    return 
    // this->entry->type.to_str() + " " + 
    "var(" + this->entry->name + ")";
}

std::string Function::to_str()
{
    std::string name = "func(" + this->entry->name + ").call(";
    for (auto& arg: this->children)
    {
        name += (arg->to_str() + ", ");
    }
    return name + ")";
}

std::string Number::to_str()
{
    if (this->sizes.size() == 0)
    {
        return std::to_string(this->value[0]);
    }
    std::string val_str = "{";
    for (auto& val: this->value)
    {
        val_str += (std::to_string(val) + ", ");
    }
    return array_type_to_str(this->sizes) + val_str + "}";
}

// std::string UnaryExpression::to_str()
// {
//     const char* names[] = {"uplus", "uminus", "not"};
//     return names[this->operation] + ("(" + this->children[0]->to_str() + ")");
// }

// std::string MulExpression::to_str()
// {
//     const char* names[] = {"times", "divide", "mod"};
//     return names[this->operation] + ("(" + this->children[0]->to_str() + ", ")
//                                   + (this->children[1]->to_str() + ")");
// }

// std::string AddExpression::to_str()
// {
//     const char* names[] = {"plus", "minus"};
//     return names[this->operation] + ("(" + this->children[0]->to_str() + ", ")
//                                   + (this->children[1]->to_str() + ")");
// }

// std::string RelExpression::to_str()
// {
//     const char* names[] = {"gt", "geq", "lt", "leq"};
//     return names[this->operation] + ("(" + this->children[0]->to_str() + ", ")
//                                   + (this->children[1]->to_str() + ")");
// }

// std::string EqExpression::to_str()
// {
//     const char* names[] = {"eq", "neq"};
//     return names[this->operation] + ("(" + this->children[0]->to_str() + ", ")
//                                   + (this->children[1]->to_str() + ")");
// }

// std::string AndExpression::to_str()
// {
//     return "and" + ("(" + this->children[0]->to_str() + ", ")
//                  + (this->children[1]->to_str() + ")");
// }

// std::string OrExpression::to_str()
// {
//     return "or" + ("(" + this->children[0]->to_str() + ", ")
//                 + (this->children[1]->to_str() + ")");
// }

// std::string IndexExpression::to_str()
// {
//     return "index" + ("(" + this->children[0]->to_str() + ", ")
//                    + (this->children[1]->to_str() + ")");
// }

std::string UnaryExpression::to_str()
{
    const char* names[] = {"+", "-", "!"};
    return names[this->operation] + this->children[0]->to_str();
}

std::string MulExpression::to_str()
{
    const char* names[] = {"*", "/", "%"};
    return this->children[0]->to_str() + " " + names[this->operation] 
            + " " + this->children[1]->to_str();
}

std::string AddExpression::to_str()
{
    const char* names[] = {"+", "-"};
    return this->children[0]->to_str() + " " + names[this->operation] 
            + " " + this->children[1]->to_str();
}

std::string RelExpression::to_str()
{
    const char* names[] = {">", ">=", "<", "<="};
    return this->children[0]->to_str() + " " + names[this->operation] 
            + " " + this->children[1]->to_str();
}

std::string EqExpression::to_str()
{
    const char* names[] = {"==", "!="};
    return this->children[0]->to_str() + " " + names[this->operation] 
            + " " + this->children[1]->to_str();
}

std::string AndExpression::to_str()
{
    return this->children[0]->to_str() + " && " + this->children[1]->to_str();
}

std::string OrExpression::to_str()
{
    return this->children[0]->to_str() + " || " + this->children[1]->to_str();

}

std::string IndexExpression::to_str()
{
    return this->children[0]->to_str() + "[" + this->children[1]->to_str() + "]";

}


void clear(Symbol* symbol)
{
    for (auto& child: symbol->children)
    {
        clear(child);
    }
    delete symbol;
}

