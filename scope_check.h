#ifndef _SCOPE_CHECK_H
#define _SCOPE_CHECK_H
#include "ast.h"
#include "id_use.h"
#include "type_exp.h"
#include "id_attrs.h"


// varType ::= float | bool
typedef enum {float_type, bool_type
} var_type_e;

// Build the symbol table for the given program AST
// and check the given program AST for duplicate declarations
// or uses of identifiers that were not declared
extern void scope_check_program(block_t prog);

// build the symbol table and check the declarations in vds
extern void scope_check_varDecls(var_decls_t vds);

// Add declarations to the current scope's symbol table for vd,
// producing errors for duplicate declarations
extern void scope_check_varDecl(var_decl_t vd);

// Add declarations for the names in ids to the current scope's symbol table,
// for variables of the type t,
// producing errors for any duplicate declarations
extern void scope_check_idents(ident_list_t ids, AST_type t, id_kind kind);

// check the statements to make sure that
// all idenfifiers referenced in them have been declared
// (if not, then produce an error)
// Return the modified AST with id_use pointers
extern void scope_check_stmts(stmts_t stmts);

// check the statement to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
// Return the modified AST with id_use pointers
extern void scope_check_stmt(stmt_t stmt);

extern void scope_check_binary_op_expr(binary_op_expr_t exp);

// Add a declaration of the name id.name with the type t
// to the current scope's symbol table,
// producing an error if this would be a duplicate declaration
extern void scope_check_declare_ident(ident_t id, AST_type vt, id_kind kind);

// check the statement to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
// Return the modified AST with id_use pointers
extern void scope_check_assignStmt(assign_stmt_t stmt);

// check the statement to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
// Return the modified AST with id_use pointers
extern void scope_check_callStmt(call_stmt_t stmt);

extern void scope_check_whileStmt(while_stmt_t stmt);



// check the statement to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
// Return the modified AST with id_use pointers
extern void scope_check_ifStmt(if_stmt_t stmt);

// check the statement to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
// Return the modified AST with id_use pointers
extern void scope_check_readStmt(read_stmt_t stmt);

// check the statement to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
// Return the modified AST with id_use pointers


extern void scope_check_printStmt(print_stmt_t stmt);


extern void scope_check_blockStmt(block_stmt_t stmt);


// check the expresion to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
// Return the modified AST with id_use pointers
extern void scope_check_expr(expr_t exp);

// check the expression (exp) to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
// Return the modified AST with id_use pointers
extern void scope_check_binary_op_expr(binary_op_expr_t exp);

// check the identifier (id) to make sure that
// all it has been declared (if not, then produce an error)
// Return the modified AST with id_use pointers
extern void scope_check_ident_expr(ident_t id);

// check that the given name has been declared,
// if so, then return an id_use (containing the attributes) for that name,
// otherwise, produce an error using the file_location (floc) given.
// Return the modified AST with id_use pointers
extern id_use *scope_check_ident_declared(file_location floc, const char *name);

#endif