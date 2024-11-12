#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scope_check.h"
#include "id_attrs.h"
#include "file_location.h"
#include "ast.h"
#include "utilities.h"
#include "symtab.h"
#include "scope_check.h"
#include "id_use.h"
#include <assert.h>

// Build the symbol table for prog
// and check for duplicate declarations
// or uses of undeclared identifiers
void scope_check_program(block_t prog)
{
    symtab_enter_scope();
    scope_check_varDecls(prog.var_decls);
    scope_check_stmts(prog.stmts);
    symtab_leave_scope();
}

// check that all identifiers used in exp
// have been declared
// (if not, then produce an error)

void scope_check_binary_op_expr(binary_op_expr_t exp)
{
    scope_check_expr(*(exp.expr1));
    // (note: no identifiers can occur in the operator)
    scope_check_expr(*(exp.expr2));
}

// build the symbol table
// and check the declarations in vds
void scope_check_varDecls(var_decls_t vds)
{
    var_decl_t *vdp = vds.var_decls;
    while (vdp != NULL)
    {
        scope_check_varDecl(*vdp);
        vdp = vdp->next;
    }
}

// Add declarations for the names in vd,
// reporting duplicate declarations
void scope_check_varDecl(var_decl_t vd)
{
    scope_check_idents(vd.ident_list, vd.type_tag);
}

// Add declarations for the names in ids
// to current scope as type vt
// reporting any duplicate declarations
void scope_check_idents(ident_list_t ids, AST_type vt)
{
    ident_t *idp = ids.start;
    while (idp != NULL)
    {
        scope_check_declare_ident(*idp, vt);
        idp = idp->next;
    }
}

// Add declaration for id
// to current scope as type vt
// reporting if it's a duplicate declaration
void scope_check_declare_ident(ident_t id, AST_type vt)
{
    if (symtab_declared_in_current_scope(id.name))
    {
        // only variables in FLOAT
        bail_with_prog_error(
            *(id.file_loc),
            "variable \"%s\" is already declared as a variable",
            // Assuming `file_loc` has `line` attribute
            id.name);
        exit(EXIT_FAILURE);
    }
    else
    {
        int ofst_cnt = symtab_scope_loc_count();
        id_attrs *attrs = create_id_attrs(*(id.file_loc), vt, ofst_cnt);
        symtab_insert(id.name, attrs);
    }
}

// This is between assignStmt and beginStmt
//  check the statements to make sure that
//  all idenfifiers referenced in them have been declared
//  (if not, then produce an error)
//  Return the modified AST with id_use pointers

void scope_check_stmts(stmts_t stmts)
{
    stmt_t *sp = stmts.stmt_list.start;

    while (sp != NULL)
    {

        scope_check_stmt(*sp);

        sp = sp->next;
    }
}

// check the statement to make sure that
// all idenfifiers used have been declared
// (if not, then produce an error)

void scope_check_stmt(stmt_t stmt)
{
    switch (stmt.stmt_kind)
    {
    case assign_stmt:
        // fprintf(stderr, "I am here 1");

        scope_check_assignStmt(stmt.data.assign_stmt); //
        break;
    case call_stmt:
        // fprintf(stderr, "I am here 2");
        scope_check_callStmt(stmt.data.call_stmt); //
        break;
    case while_stmt:
        fprintf(stderr, "I am here 3");
        scope_check_whileStmt(stmt.data.while_stmt); //
        break;
    case if_stmt:
        fprintf(stderr, "I am here 4");
        scope_check_ifStmt(stmt.data.if_stmt); //
        break;
    case read_stmt:
        fprintf(stderr, "I am here 5");
        scope_check_readStmt(stmt.data.read_stmt); //
        break;
    case print_stmt:
        // fprintf(stderr, "I am here 6");
        scope_check_printStmt(stmt.data.print_stmt); //
        break;
    case block_stmt:
        fprintf(stderr, "I am here 7");
        scope_check_blockStmt(stmt.data.block_stmt);
        break;
    default:
        bail_with_error("Call to scope_check_stmt with an AST that is not a statement!");
        break;
    }
}

// check the statement for
// undeclared identifiers
// Return the modified AST with id_use pointers
void scope_check_assignStmt(assign_stmt_t stmt)
{

    const char *name = stmt.name;
    scope_check_ident_declared(*(stmt.file_loc), name);
    // assert(stmt.idu != NULL);  // since would bail if not declared or use bail_with_prog_error
    scope_check_expr(*(stmt.expr));
}

void scope_check_callStmt(call_stmt_t stmt)
{
    // Check that the function name used in the call statement is declared
    scope_check_ident_declared(*(stmt.file_loc), stmt.name);
}

void scope_check_printStmt(print_stmt_t stmt)
{
    scope_check_ident_expr(stmt.expr.data.ident);
    scope_check_expr(stmt.expr);
    /*
    const char *name = stmt.name;
    assert(stmt.idu != NULL);  // since would bail if not declared or use bail_with_prog_error
    *stmt.expr = scope_check_expr(*(stmt.expr));
   */
}

void scope_check_blockStmt(block_stmt_t stmt)
{
    /*
    const char *name = stmt.name;
    scope_check_ident_declared(*(stmt.file_loc),name);
    assert(stmt.idu != NULL);  // since would bail if not declared or use bail_with_prog_error
    *stmt.expr = scope_check_expr(*(stmt.expr));
   */
    // Enter a new scope for the block
    symtab_enter_scope();
    // Check all the statements within the block
    scope_check_varDecls(stmt.block->var_decls);
    scope_check_stmts((stmt.block->stmts));
    // Leave the scope after checking the block
    symtab_leave_scope();
}

// check the statement to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
// Return the modified AST with id_use pointers

void scope_check_ifStmt(if_stmt_t stmt)
{
    // Check if condition
    // Check the condition expression C for undeclared identifiers
    if (stmt.condition.cond_kind == ck_db)
    {
        scope_check_expr(stmt.condition.data.db_cond.dividend);
        scope_check_expr(stmt.condition.data.db_cond.divisor);
    }
    else
    {
        scope_check_expr(stmt.condition.data.rel_op_cond.expr1);
        scope_check_expr(stmt.condition.data.rel_op_cond.expr2);
    }

    // Check the statement list S1 for identifiers
    scope_Check_stmts(stmt.then_stmts);

    // Check the else part S2 if it exists
    if (stmt.else_stmts != NULL)
    {
        scope_Check_stmts(stmt.else_stmts);
    }
}

// check the statement to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
// Return the modified AST with id_use pointers

void scope_check_readStmt(read_stmt_t stmt)
{
    /*
    stmt.idu = scope_check_ident_declared(*(stmt.file_loc),stmt.name);
  */
    const char *name = stmt.name;
    // Check if the identifier is declared
    scope_check_ident_declared(*(stmt.file_loc), name);
}

// check the statement to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
// Return the modified AST with id_use pointers

void scope_check_whileStmt(while_stmt_t stmt)
{
    // Check the condition
    if (stmt.condition.cond_kind == ck_db)
    {
        scope_check_expr(stmt.condition.data.db_cond.dividend);
        scope_check_expr(stmt.condition.data.db_cond.divisor);
    }
    else
    {
        scope_check_expr(stmt.condition.data.rel_op_cond.expr1);
        scope_check_expr(stmt.condition.data.rel_op_cond.expr2);
    }

    // Check the statement list
    scope_check_stmts(*(stmt.body));
}

// check id to make sure that
// all it has been declared
// (if not, then produce an error)

void scope_check_ident_expr(ident_t id)
{

    scope_check_ident_declared(*(id.file_loc), id.name);
}

id_use *scope_check_ident_declared(file_location floc, const char *name)
{
    id_use *ret = symtab_lookup(name);
    if (ret == NULL)
    {
        bail_with_prog_error(floc,
                             "identifier \"%s\" is not declared!",
                             name);
    }
    assert(id_use_get_attrs(ret) != NULL);
    return ret;
}

// check the expresion to make sure that
// all idenfifiers used have been declared
// (if not, then produce an error)

void scope_check_expr(expr_t exp)
{
    switch (exp.expr_kind)
    {
    case expr_bin:
        scope_check_binary_op_expr(exp.data.binary);
        break;
    case expr_ident:
        scope_check_ident_expr(exp.data.ident);
        break;
    case expr_number:
        // no identifiers in numbers
        break;
    case expr_negated:
        scope_check_expr(*exp.data.negated.expr);
        break;
    default:
        bail_with_error("Unknown expression kind encountered during scope checking!");
        break;
    }
}
