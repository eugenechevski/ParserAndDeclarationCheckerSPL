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
    scope_check_constDecls(prog.const_decls);
    scope_check_varDecls(prog.var_decls);
    scope_check_procDecls(prog.proc_decls);
    scope_check_stmts(prog.stmts);
    symtab_leave_scope();
}

// declare all procedure identifiers
void scope_check_procDecls(proc_decls_t pds)
{
    proc_decl_t *pdp = pds.proc_decls;
    while (pdp != NULL)
    {
        scope_check_procDecl(*pdp);
        pdp = pdp->next;
    }
}

void scope_check_procDecl(proc_decl_t pd)
{
    // check if the procedure name is already declared
    if (symtab_declared_in_current_scope(pd.name))
    {
        bail_with_prog_error(
            *(pd.file_loc),
            "procedure \"%s\" is already declared",
            pd.name);
        exit(EXIT_FAILURE);
    }
    else
    {
        // declare the name of the procedure
        int ofst_cnt = symtab_scope_loc_count();
        id_attrs *attrs = create_id_attrs(*(pd.file_loc), pd.type_tag, ofst_cnt);
        symtab_insert(pd.name, attrs);
    }
    
    // check the block of the procedure
    scope_check_program(*(pd.block));
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

// declare all constant identifiers
void scope_check_constDecls(const_decls_t cds)
{
    const_decl_t *cdp = cds.start;
    while (cdp != NULL)
    {
        scope_check_constDecl(*cdp);
        cdp = cdp->next;
    }
}

void scope_check_constDecl(const_decl_t cd)
{
    const_def_list_t cdl = cd.const_def_list;
    const_def_t *cdp = cdl.start;
    while (cdp != NULL)
    {
        scope_check_constDef(*cdp);
        cdp = cdp->next;
    }
}

void scope_check_constDef(const_def_t cd)
{
    scope_check_declare_ident(cd.ident, cd.type_tag, constant_idk);
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
    scope_check_idents(vd.ident_list, vd.type_tag, variable_idk);
}

// Add declarations for the names in ids
// to current scope as type vt
// reporting any duplicate declarations
void scope_check_idents(ident_list_t ids, AST_type vt, id_kind kind)
{
    ident_t *idp = ids.start;
    while (idp != NULL)
    {
        scope_check_declare_ident(*idp, vt, kind);
        idp = idp->next;
    }
}

// Add declaration for id
// to current scope as type vt
// reporting if it's a duplicate declaration
void scope_check_declare_ident(ident_t id, AST_type vt, id_kind kind)
{
    if (symtab_declared_in_current_scope(id.name))
    {
        // Get existing identifier's kind
        id_kind existing_kind = symtab_lookup(id.name)->attrs->kind;
        
        const char *existing_kind_str = (existing_kind == 6) ? "variable" :
                                        (existing_kind == 4) ? "constant" :
                                        "procedure";

        const char *new_kind_str = (kind == variable_idk) ? "variable" :
                                   (kind == constant_idk) ? "constant" : "procedure";

        bail_with_prog_error(
            *(id.file_loc),
            "%s \"%s\" is already declared as a %s",
            new_kind_str,
            id.name,
            existing_kind_str);
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
        scope_check_assignStmt(stmt.data.assign_stmt); //
        break;
    case call_stmt:
        scope_check_callStmt(stmt.data.call_stmt); //
        break;
    case while_stmt:
        scope_check_whileStmt(stmt.data.while_stmt); //
        break;
    case if_stmt:
        scope_check_ifStmt(stmt.data.if_stmt); //
        break;
    case read_stmt:
        scope_check_readStmt(stmt.data.read_stmt); //
        break;
    case print_stmt:
        scope_check_printStmt(stmt.data.print_stmt); //
        break;
    case block_stmt:
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
    scope_check_stmts(*stmt.then_stmts);

    // Check the else part S2 if it exists
    if (stmt.else_stmts != NULL)
    {
        scope_check_stmts(*stmt.else_stmts);
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
    // assert(id_use_get_attrs(ret) != NULL);
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
