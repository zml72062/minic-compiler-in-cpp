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

ArrayInitialization::ArrayInitialization(const std::vector<std::size_t>& _sizes): Symbol()
{
    this->symbol_idx = SYMBOL_ARR_INIT;
    this->sizes = _sizes;
}

void ArrayInitialization::add_subvalue(Symbol* _symbol)
{
    this->children.push_back(_symbol);
    _symbol->parent = this;
}

std::vector<int> ArrayInitialization::to_vector()
{
    std::size_t dim = sizes.size();
    std::vector<std::size_t> mul_sizes(dim, 1);
    for (auto i = dim - 1; i > 0; i--)
    {
        mul_sizes[i - 1] = mul_sizes[i] * sizes[i];
    }
    std::size_t full_size = mul_sizes[0] * sizes[0], cur_idx = 0;
    std::vector<int> res_array(full_size);

    for (auto& child : children)
    {
        if (child->symbol_idx == SYMBOL_ARR_INIT)
        {
            size_t i = 0;
            for (; i < dim; i++)
                if (cur_idx % mul_sizes[i] == 0)
                    break;
            ArrayInitialization* as_array = (ArrayInitialization*)child;
            std::vector<std::size_t> used_sizes(sizes.begin() + (i + 1),
                                                sizes.end());
            if (used_sizes.size() == 0)
            {
                if (cur_idx == full_size)
                    return res_array;
                res_array[cur_idx++] = as_array->to_vector()[0];
            }
            else
            {
                as_array->sizes = used_sizes;
                auto subarray = as_array->to_vector();
                auto subarray_size = subarray.size();
                for (auto i = 0; i < subarray_size; i++)
                {
                    if (cur_idx == full_size)
                        return res_array;
                    res_array[cur_idx++] = subarray[i];
                }
            }
        }
        else
        {
            if (cur_idx == full_size)
                return res_array;
            res_array[cur_idx++] = ((Number*)child)->value[0];
        }
    }
    for (; cur_idx < full_size; cur_idx++)
    {
        res_array[cur_idx] = 0;
    }
    return res_array;
}

ExpArrayInitialization::ExpArrayInitialization(const std::vector<std::size_t>& _sizes): Symbol()
{
    this->symbol_idx = SYMBOL_EXP_ARR_INIT;
    this->sizes = _sizes;
}

void ExpArrayInitialization::add_subvalue(Symbol* _symbol)
{
    this->children.push_back(_symbol);
    _symbol->parent = this;
}

std::vector<Symbol*> ExpArrayInitialization::to_vector()
{
    std::size_t dim = sizes.size();
    std::vector<std::size_t> mul_sizes(dim, 1);
    for (auto i = dim - 1; i > 0; i--)
    {
        mul_sizes[i - 1] = mul_sizes[i] * sizes[i];
    }
    std::size_t full_size = mul_sizes[0] * sizes[0], cur_idx = 0;
    std::vector<Symbol*> res_array(full_size);

    for (auto& child : children)
    {
        if (child->symbol_idx == SYMBOL_EXP_ARR_INIT)
        {
            size_t i = 0;
            for (; i < dim; i++)
                if (cur_idx % mul_sizes[i] == 0)
                    break;
            ExpArrayInitialization* as_array = (ExpArrayInitialization*)child;
            std::vector<std::size_t> used_sizes(sizes.begin() + (i + 1),
                                                sizes.end());
            if (used_sizes.size() == 0)
            {
                if (cur_idx == full_size)
                    return res_array;
                res_array[cur_idx++] = as_array->to_vector()[0];
            }
            else
            {
                as_array->sizes = used_sizes;
                auto subarray = as_array->to_vector();
                auto subarray_size = subarray.size();
                for (auto i = 0; i < subarray_size; i++)
                {
                    if (cur_idx == full_size)
                        return res_array;
                    res_array[cur_idx++] = subarray[i];
                }
            }
        }
        else
        {
            if (cur_idx == full_size)
                return res_array;
            res_array[cur_idx++] = child;
        }
    }
    for (; cur_idx < full_size; cur_idx++)
    {
        res_array[cur_idx] = new Number(std::vector<std::size_t>(), std::vector<int>({0}));
    }
    return res_array;
}

ExpArray::ExpArray(const std::vector<std::size_t>& _sizes, const std::vector<Symbol*>& _value): Symbol()
{
    this->symbol_idx = SYMBOL_EXP_ARR;
    this->sizes = _sizes;
    this->value = _value;
}

Block::Block(): Symbol()
{
    this->symbol_idx = SYMBOL_BLOCK;
}

void Block::add_statement(Symbol* _stmt)
{
    this->children.push_back(_stmt);
    _stmt->parent = this;
}

EmptyStatement::EmptyStatement(): Symbol()
{
    this->symbol_idx = SYMBOL_EMPTY_STMT;
}

IfStatement::IfStatement(Symbol* _expr, Symbol* _stmt): Symbol()
{
    this->symbol_idx = SYMBOL_IF_STMT;
    this->children.push_back(_expr);
    _expr->parent = this;
    this->children.push_back(_stmt);
    _stmt->parent = this;
}

IfElseStatement::IfElseStatement(Symbol* _expr, Symbol* _stmt_if, Symbol* _stmt_else): Symbol()
{
    this->symbol_idx = SYMBOL_IF_ELSE_STMT;
    this->children.push_back(_expr);
    _expr->parent = this;
    this->children.push_back(_stmt_if);
    _stmt_if->parent = this;
    this->children.push_back(_stmt_else);
    _stmt_else->parent = this;
}

WhileStatement::WhileStatement(Symbol* _expr, Symbol* _stmt): Symbol()
{
    this->symbol_idx = SYMBOL_WHILE_STMT;
    this->children.push_back(_expr);
    _expr->parent = this;
    this->children.push_back(_stmt);
    _stmt->parent = this;
}

AssignStatement::AssignStatement(Symbol* _lval, Symbol* _rval): Symbol()
{
    this->symbol_idx = SYMBOL_ASSIGN_STMT;
    this->children.push_back(_lval);
    _lval->parent = this;
    this->children.push_back(_rval);
    _rval->parent = this;
}

ReturnNoneStatement::ReturnNoneStatement(): Symbol()
{
    this->symbol_idx = SYMBOL_RETURN_NONE_STMT;
}

BreakStatement::BreakStatement(): Symbol()
{
    this->symbol_idx = SYMBOL_BREAK_STMT;
}

ContinueStatement::ContinueStatement(): Symbol()
{
    this->symbol_idx = SYMBOL_CONTINUE_STMT;
}

ReturnValueStatement::ReturnValueStatement(Symbol* _retval): Symbol()
{
    this->symbol_idx = SYMBOL_RETURN_VAL_STMT;
    this->children.push_back(_retval);
    _retval->parent = this;
}

ExpressionStatement::ExpressionStatement(Symbol* _expr): Symbol()
{
    this->symbol_idx = SYMBOL_EXPR_STMT;
    this->children.push_back(_expr);
    _expr->parent = this;
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

ArrayInitialization* ArrayInitialization::copy()
{
    return new ArrayInitialization(*this);
}

ExpArrayInitialization* ExpArrayInitialization::copy()
{
    return new ExpArrayInitialization(*this);
}

ExpArray* ExpArray::copy()
{
    return new ExpArray(*this);
}

Block* Block::copy()
{
    return new Block(*this);
}

EmptyStatement* EmptyStatement::copy()
{
    return new EmptyStatement(*this);
}
IfStatement* IfStatement::copy()
{
    return new IfStatement(*this);
}
IfElseStatement* IfElseStatement::copy()
{
    return new IfElseStatement(*this);
}
WhileStatement* WhileStatement::copy()
{
    return new WhileStatement(*this);
}
ReturnNoneStatement* ReturnNoneStatement::copy()
{
    return new ReturnNoneStatement(*this);
}
ReturnValueStatement* ReturnValueStatement::copy()
{
    return new ReturnValueStatement(*this);
}
ExpressionStatement* ExpressionStatement::copy()
{
    return new ExpressionStatement(*this);
}
AssignStatement* AssignStatement::copy()
{
    return new AssignStatement(*this);
}
BreakStatement* BreakStatement::copy()
{
    return new BreakStatement(*this);
}
ContinueStatement* ContinueStatement::copy()
{
    return new ContinueStatement(*this);
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
        if (this->value.size() == 0)
            return std::string();
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

std::string ArrayInitialization::to_str()
{
    std::string val_str = "{";
    std::vector<int> value = this->to_vector();
    for (auto& val: value)
    {
        val_str += (std::to_string(val) + ", ");
    }
    return array_type_to_str(this->sizes) + val_str + "}";
}

std::string ExpArrayInitialization::to_str()
{
    std::string val_str = "{";
    std::vector<Symbol*> value = this->to_vector();
    for (auto& val: value)
    {
        val_str += (val->to_str() + ", ");
    }
    return array_type_to_str(this->sizes) + val_str + "}";
}

std::string ExpArray::to_str()
{
    if (this->sizes.size() == 0)
    {
        if (this->value.size() == 0)
            return std::string();
        return this->value[0]->to_str();
    }
    std::string val_str = "{";
    for (auto& val: this->value)
    {
        val_str += (val->to_str() + ", ");
    }
    return array_type_to_str(this->sizes) + val_str + "}";
}

std::string Block::to_str()
{
    std::string val_str = "Block containing {\n";
    for (auto& val: this->children)
    {
        val_str += ("    " + val->to_str() + "\n");
    }
    return val_str + "}\n";
}

std::string EmptyStatement::to_str()
{
    return std::string("Statement()");
}
std::string IfStatement::to_str()
{
    return "if (" + this->children[0]->to_str() + ") {" +
                    this->children[1]->to_str() + "}";
}
std::string IfElseStatement::to_str()
{
    return "if (" + this->children[0]->to_str() + ") {" +
                    this->children[1]->to_str() + "} else {" + 
                    this->children[2]->to_str() + "}";
}
std::string ReturnNoneStatement::to_str()
{
    return std::string("return()");
}
std::string ReturnValueStatement::to_str()
{
    return "return(" + this->children[0]->to_str() + ")";
}
std::string AssignStatement::to_str()
{
    return "assign " + this->children[1]->to_str() + " to "
                     + this->children[0]->to_str();
}
std::string ExpressionStatement::to_str()
{
    return "Expression(" + this->children[0]->to_str() + ")";
}
std::string WhileStatement::to_str()
{
    return "while (" + this->children[0]->to_str() + ") {" +
                       this->children[1]->to_str() + "}";
}
std::string BreakStatement::to_str()
{
    return std::string("break");
}
std::string ContinueStatement::to_str()
{
    return std::string("continue");
}

void clear(Symbol* symbol)
{
    for (auto& child: symbol->children)
    {
        clear(child);
    }
    delete symbol;
}

void clear_exp_arr_init(ExpArrayInitialization* symbol)
{
    for (auto& child: symbol->children)
    {
        if (child->symbol_idx == SYMBOL_EXP_ARR_INIT)
            clear_exp_arr_init((ExpArrayInitialization*)child);
    }
    delete symbol;
}

bool LexemePacker::is_lval()
{
    if (this->lexeme.lex_type == IDENTIFIERS)
        return true;
    return false;
}

bool Symbol::is_lval()
{
    return false;
}

bool Variable::is_lval()
{
    return true;
}

bool Function::is_lval()
{
    return false;
}

bool Number::is_lval()
{
    return false;
}

bool UnaryExpression::is_lval()
{
    return false;
}

bool MulExpression::is_lval()
{
    return false;
}

bool AddExpression::is_lval()
{
    return false;
}

bool RelExpression::is_lval()
{
    return false;
}

bool EqExpression::is_lval()
{
    return false;
}

bool AndExpression::is_lval()
{
    return false;
}

bool OrExpression::is_lval()
{
    return false;
}

bool IndexExpression::is_lval()
{
    return this->children[0]->is_lval();
}

bool ArrayInitialization::is_lval()
{
    return false;
}

bool ExpArrayInitialization::is_lval()
{
    return false;
}

bool ExpArray::is_lval()
{
    return false;
}

bool Block::is_lval()
{
    return false;
}
bool EmptyStatement::is_lval()
{
    return false;
}
bool IfStatement::is_lval()
{
    return false;
}
bool IfElseStatement::is_lval()
{
    return false;
}
bool ReturnNoneStatement::is_lval()
{
    return false;
}
bool ReturnValueStatement::is_lval()
{
    return false;
}
bool AssignStatement::is_lval()
{
    return false;
}
bool ExpressionStatement::is_lval()
{
    return false;
}
bool WhileStatement::is_lval()
{
    return false;
}
bool BreakStatement::is_lval()
{
    return false;
}
bool ContinueStatement::is_lval()
{
    return false;
}
