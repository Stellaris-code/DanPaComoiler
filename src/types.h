/*
types.h

Copyright (c) 25 Yann BOUCHER (yann)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
#ifndef TYPES_H
#define TYPES_H

#include "dynarray.h"

#define POD_SIZE 1 // 1 word = 32-bits

typedef struct function_t function_t;
typedef struct program_t program_t;

typedef struct expression_t expression_t;
typedef struct primary_expression_t primary_expression_t;
typedef struct token_t token_t;

typedef enum base_type_t
{
    INVALID_TYPE = -1,
    INT = 0,
    REAL,
    STR,
    VOID,

    // special types, e.g. used by builtins for argument matching, or for null
    SPEC_ARRAY, POD_TYPES_END = SPEC_ARRAY,
    SPEC_POINTER,
    SPEC_NULL,
    SPEC_ANY,

    DEFAULT_TYPES_END
} base_type_t;

typedef struct type_t type_t;

typedef struct pointer_type_t
{
    type_t* pointed_type;
} pointer_type_t;
typedef struct optional_type_t
{
    type_t* opt_type;
} optional_type_t;
typedef struct array_type_t
{
    type_t* array_type;
    expression_t* initial_size;
    int is_empty;
} array_type_t;
typedef struct function_type_t
{
    struct function_signature_t* signature;
} function_type_t;
typedef struct type_t
{
    enum
    {
        BASIC = 0,
        POINTER = 1,
        ARRAY = 2,
        FUNCTION = 3,
        OPTIONAL = 4 // not implemented yet
    } kind;
    union
    {
        base_type_t base_type;
        pointer_type_t pointer;
        optional_type_t opt;
        array_type_t array;
        function_type_t function;
    };
    token_t* token;
} type_t;

typedef struct function_signature_t
{
    type_t ret_type;
    DYNARRAY(type_t) parameter_types;
} function_signature_t;

typedef struct structure_field_t
{
    unsigned offset;
    size_t byte_size;
    token_t* name;
    type_t type;
} structure_field_t;

typedef struct structure_t
{
    int incomplete;
    size_t byte_size;
    token_t* name;
    DYNARRAY(structure_field_t) fields;
} structure_t;

void types_init();

type_t mk_type(base_type_t base);

const char* type_to_str(const type_t* type);
type_t get_type(const char* type_str);
int is_lvalue(const primary_expression_t* prim_expr);
static inline int is_struct(const type_t* type)
{
    if (type->kind != BASIC)
        return 0;
    return type->base_type >= DEFAULT_TYPES_END;
}
static inline int is_indirect_type(const type_t* type)
{
    if (type->kind == BASIC)
        return type->base_type != INT && type->base_type != REAL;
    else
        return 1;
}
const structure_t* get_struct(const type_t* type);
int cmp_types(const type_t* lhs, const type_t* rhs);
size_t sizeof_type(const type_t* type);
void add_typedef(const char* alias, type_t real_type);

// returns type id
type_t forward_declare_structure(const char* name);
void define_structure(type_t* type, const structure_t* structure);

type_t get_prim_expr_type(const primary_expression_t* prim_expr);
type_t get_expression_type(const expression_t* expr);
int can_implicit_cast(const type_t* lhs, const type_t* to);
int can_explicit_cast(const type_t* lhs, const type_t* to);

#endif // TYPES_H
