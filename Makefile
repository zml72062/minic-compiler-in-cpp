CXX = g++
CXXFLAGS = -std=c++11 -O2 -g

all:

test_lex: test/lex_main.o utils.o lex
	${CXX} -o test/main lex/*.o test/*.o utils.o

test_parse: test/parse_main.o utils.o lex parse
	${CXX} -o test/main parse/*.o test/*.o utils.o lex/*.o

test_intermediate: test/intermediate_main.o utils.o lex parse intermediate
	${CXX} -o test/main parse/*.o test/*.o utils.o lex/*.o intermediate/*.o

test_ra_opt: test/ra_opt_main.o utils.o lex parse intermediate ra_opt
	${CXX} -o test/main parse/*.o test/*.o utils.o lex/*.o intermediate/*.o ra_opt/*.o

lex: lex/lexeme_impl.o lex/lexeme_val_impl.o lex/lexer_impl.o

parse: parse/symbols_impl.o parse/symtab_entry_impl.o parse/symtab_impl.o \
parse/type_impl.o parse/parser_exp_impl.o parse/parser_impl.o \
parse/parser_const_decl_impl.o parse/parser_var_decl_impl.o \
parse/parser_decl_impl.o parse/parser_block_impl.o parse/parser_funcdef_impl.o \
parse/parser_compunit_impl.o

intermediate: intermediate/intermediate_code_gen_impl.o \
intermediate/intermediate_code_impl.o intermediate/intermediate_code_gen_exp_impl.o \
intermediate/intermediate_code_gen_block_stmt_impl.o \
intermediate/intermediate_code_gen_global_impl.o

ra_opt: ra_opt/procedure_impl.o ra_opt/basicblock_impl.o ra_opt/liveness_analysis_impl.o \
ra_opt/procedure_utils_impl.o ra_opt/register_alloc_impl.o

clean:
	rm -rf *.o lex/*.o test/main test/*.o parse/*.o intermediate/*.o ra_opt/*.o