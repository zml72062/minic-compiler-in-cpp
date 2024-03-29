#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <vector>
#include "../lex/lex.h"
#include "symtab.h"
#include <string>

#define SYMBOL_VARIABLE 0
#define SYMBOL_FUNCTION 1
#define SYMBOL_NUMBER 2
#define SYMBOL_UNARY 3
#define SYMBOL_MUL 4
#define SYMBOL_ADD 5
#define SYMBOL_REL 6
#define SYMBOL_EQ 7
#define SYMBOL_AND 8
#define SYMBOL_OR 9
#define SYMBOL_INDEX 10
#define SYMBOL_ARR_INIT 11
#define SYMBOL_EXP_ARR_INIT 12
#define SYMBOL_EXP_ARR 13
#define SYMBOL_BLOCK 14
#define SYMBOL_EMPTY_STMT 15
#define SYMBOL_IF_STMT 16
#define SYMBOL_IF_ELSE_STMT 17
#define SYMBOL_RETURN_NONE_STMT 18
#define SYMBOL_RETURN_VAL_STMT 19
#define SYMBOL_CONTINUE_STMT 20
#define SYMBOL_BREAK_STMT 21
#define SYMBOL_WHILE_STMT 22
#define SYMBOL_EXPR_STMT 23
#define SYMBOL_ASSIGN_STMT 24
#define SYMBOL_FUNC_DEF 25
#define SYMBOL_LOCAL_VAR_DECL 26

struct Symbol
{
    int symbol_idx;
    Symbol* parent;
    std::vector<Symbol*> children;
    Symbol();
    virtual ~Symbol();
    virtual Symbol* copy();
    virtual std::string to_str();
    virtual bool is_lval();
    virtual Type type();
};

struct LexemePacker: public Symbol
{
    Lexeme lexeme;
    LexemePacker(const Lexeme& _lexeme);
    virtual LexemePacker* copy();
    virtual std::string to_str();
    virtual bool is_lval();
    virtual Type type();
};

struct Variable: public Symbol
{
    SymbolTableEntry* entry;
    Variable(SymbolTableEntry* _entry);
    virtual Variable* copy();
    virtual std::string to_str();
    virtual bool is_lval();
    virtual Type type();
};

struct Function: public Symbol
{
    SymbolTableEntry* entry;
    Function(SymbolTableEntry* _entry);
    void add_argument(Symbol* _symbol);
    virtual Function* copy();
    virtual std::string to_str();
    virtual Type type();
};

struct Number: public Symbol /* Including compile-time constants. */
{
    std::vector<std::size_t> sizes;
    std::vector<int> value;
    Number(const std::vector<std::size_t>& _sizes,
           const std::vector<int>& _value);
    virtual Number* copy();
    virtual std::string to_str();
    virtual Type type();
};

#define UNARY_EXP_OPERATOR_PLUS 0
#define UNARY_EXP_OPERATOR_MINUS 1
#define UNARY_EXP_OPERATOR_NOT 2

struct UnaryExpression: public Symbol
{
    int operation;
    UnaryExpression(int _operation, Symbol* _operand);
    virtual UnaryExpression* copy();
    virtual std::string to_str();
    virtual Type type();
};

#define MUL_EXP_OPERATOR_TIMES 0
#define MUL_EXP_OPERATOR_DIVIDE 1
#define MUL_EXP_OPERATOR_MOD 2

struct MulExpression: public Symbol
{
    int operation;
    MulExpression(int _operation, Symbol* _loperand, Symbol* _roperand);
    virtual MulExpression* copy();
    virtual std::string to_str();
    virtual Type type();
};

#define ADD_EXP_OPERATOR_PLUS 0
#define ADD_EXP_OPERATOR_MINUS 1

struct AddExpression: public Symbol
{
    int operation;
    AddExpression(int _operation, Symbol* _loperand, Symbol* _roperand);
    virtual AddExpression* copy();
    virtual std::string to_str();
    virtual Type type();
};

#define REL_EXP_OPERATOR_G 0
#define REL_EXP_OPERATOR_GEQ 1
#define REL_EXP_OPERATOR_L 2
#define REL_EXP_OPERATOR_LEQ 3

struct RelExpression: public Symbol
{
    int operation;
    RelExpression(int _operation, Symbol* _loperand, Symbol* _roperand);
    virtual RelExpression* copy();
    virtual std::string to_str();
    virtual Type type();
};

#define EQ_EXP_OPERATOR_EQ 0
#define EQ_EXP_OPERATOR_NEQ 1

struct EqExpression: public Symbol
{
    int operation;
    EqExpression(int _operation, Symbol* _loperand, Symbol* _roperand);
    virtual EqExpression* copy();
    virtual std::string to_str();
    virtual Type type();
};

struct AndExpression: public Symbol
{
    AndExpression(Symbol* _loperand, Symbol* _roperand);
    virtual AndExpression* copy();
    virtual std::string to_str();
    virtual Type type();
};

struct OrExpression: public Symbol
{
    OrExpression(Symbol* _loperand, Symbol* _roperand);
    virtual OrExpression* copy();
    virtual std::string to_str();
    virtual Type type();
};

struct IndexExpression: public Symbol
{
    IndexExpression(Symbol* _loperand, Symbol* _roperand);
    virtual IndexExpression* copy();
    virtual std::string to_str();
    virtual bool is_lval();
    virtual Type type();
};

struct ArrayInitialization: public Symbol
{
    std::vector<std::size_t> sizes;
    ArrayInitialization(const std::vector<std::size_t>& _sizes);
    void add_subvalue(Symbol* _symbol);
    std::vector<int> to_vector();
    virtual ArrayInitialization* copy();
    virtual std::string to_str();
};

struct ExpArrayInitialization: public Symbol
{
    std::vector<std::size_t> sizes;
    ExpArrayInitialization(const std::vector<std::size_t>& _sizes);
    void add_subvalue(Symbol* _symbol);
    std::vector<Symbol*> to_vector();
    virtual ExpArrayInitialization* copy();
    virtual std::string to_str();
};

struct ExpArray: public Symbol
{
    std::vector<std::size_t> sizes;
    std::vector<Symbol*> value;
    ExpArray(const std::vector<std::size_t>& _sizes,
             const std::vector<Symbol*>& _value);
    virtual ExpArray* copy();
    virtual std::string to_str();
    virtual Type type();
};

struct Block: public Symbol
{
    Block();
    void add_statement(Symbol* _stmt);
    virtual Block* copy();
    virtual std::string to_str();
};

struct EmptyStatement: public Symbol
{
    EmptyStatement();
    virtual EmptyStatement* copy();
    virtual std::string to_str();
};

struct IfStatement: public Symbol
{
    IfStatement(Symbol* _expr, Symbol* _stmt);
    virtual IfStatement* copy();
    virtual std::string to_str();
};

struct IfElseStatement: public Symbol
{
    IfElseStatement(Symbol* _expr, Symbol* _stmt_if, Symbol* _stmt_else);
    virtual IfElseStatement* copy();
    virtual std::string to_str();
};

struct WhileStatement: public Symbol
{
    WhileStatement(Symbol* _expr, Symbol* _stmt);
    virtual WhileStatement* copy();
    virtual std::string to_str();
};

struct ReturnNoneStatement: public Symbol
{
    ReturnNoneStatement();
    virtual ReturnNoneStatement* copy();
    virtual std::string to_str();
};

struct ReturnValueStatement: public Symbol
{
    ReturnValueStatement(Symbol* _retval);
    virtual ReturnValueStatement* copy();
    virtual std::string to_str();
};

struct BreakStatement: public Symbol
{
    BreakStatement();
    virtual BreakStatement* copy();
    virtual std::string to_str();
};

struct ContinueStatement: public Symbol
{
    ContinueStatement();
    virtual ContinueStatement* copy();
    virtual std::string to_str();
};

struct AssignStatement: public Symbol
{
    AssignStatement(Symbol* _lval, Symbol* _rval);
    virtual AssignStatement* copy();
    virtual std::string to_str();
};

struct ExpressionStatement: public Symbol
{
    ExpressionStatement(Symbol* _expr);
    virtual ExpressionStatement* copy();
    virtual std::string to_str();
};

struct FunctionDef: public Symbol
{
    /* This is the place that "symbol_table" must point at 
       when translating function calls. */
    SymbolTable* args_scope;
    FunctionDef(SymbolTable* _args_scope);
    void add_definition(Block* _def);
    virtual FunctionDef* copy();
    virtual std::string to_str();
};

struct LocalVarDeclaration: public Symbol
{
    SymbolTableEntry* entry;
    LocalVarDeclaration(SymbolTableEntry* _entry);
    virtual LocalVarDeclaration* copy();
    virtual std::string to_str();
};

void clear(Symbol* symbol);
void clear_exp_arr_init(ExpArrayInitialization* symbol);

#endif