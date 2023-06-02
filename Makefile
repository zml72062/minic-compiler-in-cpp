CXX = g++
CXXFLAGS = -std=c++11 -O2

all:

test_lex: test/lex_main.o lex/lexeme_impl.o lex/lexeme_val_impl.o \
lex/lexer_impl.o utils.o
	${CXX} -o test/main lex/*.o test/*.o utils.o

test_parse: parse/symbols_impl.o parse/symtab_entry_impl.o parse/symtab_impl.o \
parse/type_impl.o parse/parser_exp_impl.o parse/parser_impl.o \
test/parse_main.o utils.o lex/lexeme_impl.o lex/lexeme_val_impl.o \
lex/lexer_impl.o parse/parser_const_decl_impl.o parse/parser_var_decl_impl.o \
parse/parser_decl_impl.o parse/parser_block_impl.o parse/parser_funcdef_impl.o \
parse/parser_compunit_impl.o
	${CXX} -o test/main parse/*.o test/*.o utils.o lex/*.o

test_intermediate: parse/symbols_impl.o parse/symtab_entry_impl.o parse/symtab_impl.o \
parse/type_impl.o parse/parser_exp_impl.o parse/parser_impl.o \
test/intermediate_main.o utils.o lex/lexeme_impl.o lex/lexeme_val_impl.o \
lex/lexer_impl.o parse/parser_const_decl_impl.o parse/parser_var_decl_impl.o \
parse/parser_decl_impl.o parse/parser_block_impl.o parse/parser_funcdef_impl.o \
parse/parser_compunit_impl.o intermediate/intermediate_code_gen_impl.o \
intermediate/intermediate_code_impl.o intermediate/intermediate_code_gen_exp_impl.o \
intermediate/intermediate_code_gen_block_stmt_impl.o
	${CXX} -o test/main parse/*.o test/*.o utils.o lex/*.o intermediate/*.o

lex: lex/lexeme_impl.o lex/lexeme_val_impl.o lex/lexer_impl.o

parse: parse/symbols_impl.o parse/symtab_entry_impl.o parse/symtab_impl.o \
parse/type_impl.o parse/parser_exp_impl.o parse/parser_impl.o \
parse/parser_const_decl_impl.o parse/parser_var_decl_impl.o \
parse/parser_decl_impl.o parse/parser_block_impl.o parse/parser_funcdef_impl.o \
parse/parser_compunit_impl.o

intermediate: intermediate/intermediate_code_gen_impl.o \
intermediate/intermediate_code_impl.o intermediate/intermediate_code_gen_exp_impl.o \
intermediate/intermediate_code_gen_block_stmt_impl.o

clean:
	rm -rf *.o lex/*.o test/main test/*.o parse/*.o intermediate/*.o