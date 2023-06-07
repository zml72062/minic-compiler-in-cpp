#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <cstddef>
#include <vector>
#include <string>
#include <stack>
#include <set>


void read_file(const char* filename, char* code, std::size_t num);
std::string array_type_to_str(const std::vector<std::size_t>& sizes);
void print_stack(const std::stack<int>& stack);
void print_symbol_stack(const std::stack<void*>& stack);
void print_set(const std::set<std::size_t>& set);

#endif
