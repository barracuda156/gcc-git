%{
#include "logger.h"
#include "lexer.h"
#include "parser.h"
#include "slist.h"
#include "util.h"
#include "cursor.h"
#include "statement.h"
#include "label.h"
#include "type.h"
#include "symbol.h"
#include "boundspair.h"
#include "expression.h"
#include "desig-expr.h"
#include "for-elmt.h"
#include "a60_symtab.h"
#include "meta.h"
#include "visitor-impl.h"

#include <stdio.h>
#include <assert.h>

#include <stdlib.h>
#include <string.h>

static char const* private_parser_signature = "parser";

typedef struct struct_parser_rep_t
{
  visitable_t base;

  lexer_t * lexer;
  logger_t * log;
  int manage;
  statement_t * result;

  slist_t * blockstack;
  container_t * block;
  estring_t * tmp;
} parser_rep_t;

// call lexer
static int
yylex (YYSTYPE * arg, YYLTYPE * loc, parser_rep_t * parser)
{
  return lexer_tok (parser->lexer, arg, loc);
}

// create new cursor
static cursor_t *
cr_csr (parser_rep_t * parser, YYLTYPE * loc)
{
  return new_cursor (lexer_pp_filename (parser->lexer), loc->first_line);
}

// report error
static void
yyerror (YYLTYPE * loc, parser_rep_t * parser, char const * str)
{
  log_printfc (parser->log, ll_error, cr_csr (parser, loc),
	      "syntax error: %s", str);
}

static void private_dump_log_labels (parser_rep_t * parser, slist_t * slist);

/// Take all labels in `slist', make symbols from them, bind these
/// symbols with command `target', and add them to symbol table of
/// containing block `cont'.
static void private_add_labels_to_symtab (parser_rep_t * parser, container_t * cont,
					  slist_t * slist, statement_t * target);

/// Given a symbol `sym', assign it a type `qt`, and place it into
/// symbol table.  If there already is a symbol of such label, output
/// an error message and give up.
static void private_setup_and_add_symbol (parser_rep_t * parser, symbol_t * sym,
					  type_t * qt);

static void private_open_block (parser_rep_t * parser, container_t * cont);
static container_t * private_close_block (parser_rep_t * parser);
%}

%union {
  // lexer portion
  estring_t * slit;
  long int ilit;

  // parser portion
  int flag;
  slist_t * lst;
  statement_t * stmt;
  container_t * cont;
  label_t * lbl;
  symbol_t * sym;
  type_t * type;
  boundspair_t * bnds;
  expression_t * expr;
  desig_expr_t * dexpr;
  estring_t * estr;
  void * lelm;
}

%pure-parser
%error-verbose
 //%token-table
%locations
%parse-param {parser_rep_t * parser}
%lex-param {parser_rep_t * parser}

%token EOFTOK
%token KWTRUE
%token KWFALSE

%token AOPADD
%token AOPSUB
%token AOPMUL
%token AOPRDIV
%token AOPIDIV
%token AOPPOW

%token ROPLT
%token ROPLTE
%token ROPEQ
%token ROPGT
%token ROPGTE
%token ROPNEQ

%token LOPEQ
%token LOPIMP
%token LOPOR
%token LOPAND
%token LOPNOT

%token KWGOTO
%token KWIF
%token KWTHEN
%token KWELSE
%token KWFOR
%token KWDO

%token SEPCOMMA
%token SEPPERIOD
%token SEP10E
%token SEPCOLON
%token SEPSEMICOLON
%token SEPASSIGN
%token KWSTEP
%token KWUNTIL
%token KWWHILE
%token KWCOMMENT

%token SEPLPAREN
%token SEPRPAREN
%token SEPLBRACK
%token SEPRBRACK
%token SEPLQUOT
%token SEPRQUOT
%token KWBEGIN
%token KWEND

%token KWOWN
%token KWBOOLEAN
%token KWINTEGER
%token KWREAL
%token KWARRAY
%token KWSWITCH
%token KWPROCEDURE

%token KWSTRING
%token KWLABEL
%token KWVALUE

%token <slit> IDENTIFIER

%token <slit> LITREAL
%token <ilit> LITINTEGER
%token <slit> LITSTRING

%token BOGUS

%left LOPEQ
%left LOPIMP
%left LOPOR
%left LOPAND
%left LOPNOT
%left ROPGT ROPGTE ROPLT ROPLTE ROPEQ ROPNEQ
%left AOPADD AOPSUB
%left AOPMUL AOPRDIV AOPIDIV
%left AOPPOW
%right PREC_UNARY_MINUS

%type <cont> Program
%type <lst> LabelList
%type <lbl> Label
%type <lbl> LabelIdentifier
%type <lbl> Identifier
//%type <sym> DeclaredIdentifier
%type <stmt> Block
%type <type> Specifier
%type <lst> SpecificationPart
%type <lst> ValuePart
%type <lst> FormalParameterList
%type <lst> FormalParameterPart
//%type <> BlockDeclarationsList
//%type <> BlockDeclarations
%type <type> Type
%type <type> OptType
%type <flag> OptOwn
%type <type> OptIntrinsicType
%type <type> IntrinsicType
%type <lst> IdentifierList
%type <lst> SwitchList
%type <lst> OptBoundsPairList
%type <lst> BoundsPairList
%type <bnds> BoundsPair
%type <expr> Expression
%type <expr> SimpleExpression
%type <expr> FunctionDesignator
%type <expr> VariableAccess
%type <lst> SubscriptList
%type <lst> ActualParamList
%type <estr> ParameterDelimiter
//%type <> StatementList
%type <stmt> Statement
%type <stmt> UnconditionalStatement
%type <dexpr> DesignationalExpression
%type <dexpr> SimpleDesignationalExpression
%type <stmt> ConditionalStatement
%type <stmt> BasicStatement
%type <expr> IfClause
%type <stmt> OptElseClause
%type <lst> LeftPartList
%type <expr> LeftPart
%type <stmt> ForStatement
%type <lst> ForList
%type <lelm> ForListElmt
%%

Program:
  {
    a60_symtab_t * symtab;

    // dummy container, so that toplevel references to ->parent end up
    // somewhere sensible
    symtab = NULL;
    container_t * dummy = new_stmt_toplev (NULL, symtab);
    private_open_block (parser, dummy);

    // actual toplevel container, contains internal declarations
    symtab = a60_new_symtab (a60_stk_ordinary);
    a60_symtab_toplev_define_internals (symtab);
    container_t * c = new_stmt_toplev (NULL, symtab);
    container_set_parent (c, dummy);
    private_open_block (parser, c);

    // This block contains actual program, and is fallback container
    // for all labels that don't end up in some more enclosing scope.
    symtab = a60_new_symtab (a60_stk_block);
    container_t * d = new_stmt_block (NULL, symtab);
    container_set_parent (d, c);
    private_open_block (parser, d);
  }
  LabelList Block SEPSEMICOLON EOFTOK
    {
      log_printf (parser->log, ll_debug, "Program -> LabelList Block SEPSEMICOLON EOFTOK");
      container_add_stmt (parser->block, $3);
      if (!slist_empty ($2))
	log_printfc (parser->log, ll_error, cr_csr (parser, &@3),
		     "labels not allowed at program block.");
      container_t * d = private_close_block (parser); // dummy label scope
      container_add_stmt (parser->block, (statement_t *)d);
      parser->result = a60_as_statement (private_close_block (parser)); // toplev
      private_close_block (parser); // dummy
      YYACCEPT;
    }

LabelList:
  /*epsilon*/
    {
      log_printf (parser->log, ll_debug, "LabelList -> <eps>");
      $$ = new_slist ();
    }
  |
  LabelList Label
    {
      log_printf (parser->log, ll_debug, "LabelList -> Label LabelList");
      slist_pushback ($1, $2);
      $$ = $1;
    }

Label:
  LabelIdentifier SEPCOLON
    {
      log_printf (parser->log, ll_debug, "Label -> LabelIdentifier SEPCOLON");
      $$ = $1;
    }

LabelIdentifier:
  Identifier
  |
  LITINTEGER
    {
      log_printf (parser->log, ll_debug, "LabelIdentifier -> LITINTEGER");
      $$ = new_label (new_estring_fmt ("%ld", $1));
      label_set_cursor ($$, cr_csr (parser, &@1));
    }

Identifier:
  IDENTIFIER
    {
      log_printf (parser->log, ll_debug, "Identifier -> IDENTIFIER");
      $$ = new_label ($1);
      label_set_cursor ($$, cr_csr (parser, &@1));
      @$ = @1;
    }


Block:
  KWBEGIN
  {
    container_t * c = new_stmt_block (cr_csr (parser, &@1), a60_new_symtab (a60_stk_ordinary));
    container_set_parent (c, parser->block);
    private_open_block (parser, c);
  }
  BlockDeclarationsList StatementList
  KWEND
    {
      log_printf (parser->log, ll_debug, "Block -> KWBEGIN BlockDeclarationList StatementList KWEND");
      $$ = a60_as_statement (private_close_block (parser));
    }

BlockDeclarationsList:
  /*epsilon*/
    {
      log_printf (parser->log, ll_debug, "BlockDeclarationList -> <eps>");
    }
  |
  BlockDeclarations SEPSEMICOLON BlockDeclarationsList
    {
      log_printf (parser->log, ll_debug, "BlockDeclarationList -> BlockDeclaration BlockDeclarationList");
    }

BlockDeclarations:
  Type IdentifierList
    {
      // Classic algol only allows in this context:
      // ['own'] {'integer'|'real'|'Boolean'} ['array']
      type_t * rt = type_get_root ($1);
      if (rt != type_int ()
	  && rt != type_real ()
	  && rt != type_bool ())
	log_printfc (parser->log, ll_error, cr_csr (parser, &@1),
		    "type %s is invalid in this context.",
		    estr_cstr (type_to_str ($1, parser->tmp)));

      slist_it_t * it;
      for (it = slist_iter ($2); slist_it_has (it); slist_it_next (it))
	{
	  label_t * lbl = a60_as_label (slist_it_get (it));
	  type_t * qt = rt;

	  // If it was array, see if identifier has dimensions and
	  // mangle `rt' to reflect number of dimensions.
	  //
	  // Initially, bounds pair lists are stored at the label node by a
	  // parser.  During the translation process the list of bound pairs
	  // is transformed to tree structure of arrays, each array level with
	  // its own bounds.  E.g. this:
	  //  (symbol type=(array host=(int))
	  //          label=(id name="x" bounds=(1:5 1:10)))
	  // becomes this:
	  //  (symbol type=(array bounds=(1:5)
	  //                      host=(array bounds=(1:10)
	  //                                  host=(int)))
	  //          label=(id name="x"))
	  //
	  if (types_match ($1, type_array_any ())) // 'own' doesn't influence matching
	    {
	      if (label_boundspairs (lbl) == NULL)
		log_printfc (parser->log, ll_error, label_cursor (lbl),
			    "identifier `%s' needs array bounds.",
			    estr_cstr (label_to_str (lbl, parser->tmp)));
	      else
		{
		  assert (!slist_empty (label_boundspairs (lbl)));
		  slist_it_t * jt = slist_iter (label_boundspairs (lbl));
		  while (slist_it_has (jt))
		    {
		      qt = new_t_array (qt);
		      t_array_set_bounds (qt, slist_it_get (jt));
		      slist_it_next (jt);
		    }
		  delete_slist_it (jt);
		  label_remove_boundspairs (lbl);
		}
	    }

	  // If original type is `own', make also this type `own'.
	  if (type_is_own ($1))
	    qt = new_t_own (qt);

	  private_setup_and_add_symbol (parser, new_symbol_var (lbl), qt);
	}
      delete_slist_it (it);
    }
  |
  KWSWITCH Identifier SEPASSIGN SwitchList
    {
      log_printf (parser->log, ll_debug, "BlockDeclarations -> KWSWITCH Identifier SEPASSIGN SwitchList");
      type_t * t = new_t_switch ($4);
      private_setup_and_add_symbol (parser, new_symbol_var ($2), t);
    }
  |
  OptType KWPROCEDURE Identifier FormalParameterPart SEPSEMICOLON ValuePart SpecificationPart
  {
    // Before parsing a statement, we have to open two new blocks.
    //
    // First is the scope of function itself, and holds formal
    // parameters (and perhaps implied parameters, if this gets
    // implemented).  This block is parent-less, because function's
    // enclosing scope isn't at the definition site, but at the call
    // site.
    //
    // Second is the body of function.  Its symtab has a kind of
    // a60_stk_block, because all labels in function has to be stored
    // at most there, even if the block ends up being compound
    // statement actually.

    container_t * c1 = new_stmt_toplev (NULL, a60_new_symtab (a60_stk_ordinary));
    container_t * c2 = new_stmt_toplev (NULL, a60_new_symtab (a60_stk_block));
    container_set_parent (c2, c1);

    private_open_block (parser, c1);
    private_open_block (parser, c2);
  }
  Statement
    {
      log_printf (parser->log, ll_debug,
		  "BlockDeclarations -> OptType KWPROCEDURE Identifier "
		  "FormalParameterPart SEPSEMICOLON ValuePart "
		  "SpecificationPart Statement");

      // Add statement to body block.
      container_add_stmt (parser->block, $9);

      slist_t * formal_ids = $4; // list of all formal parameters
      slist_t * value_ids = $6;  // list of formals passed by value
      slist_t * id_types = $7;   // list [type, [id, ...], type, [id, ...], ...]

      // RRA60: """In the heading a specification part, giving
      // information about the kinds and types of the formal
      // parameters by means of an obvious notation, may be included.
      // In this part no formal parameter may occur more than once.
      // Specification of formal parameters called by value [...]
      // must be supplied and specifications of formal parameters
      // called by name [...] may be omitted."""

      // This list collects type for each declared parameter.
      // `type_any()' is inserted for by-name symbols without type
      // specifier.
      slist_t * types = new_slist_typed (adapt_test, a60_as_type);

      // This list collects formal parameters of the function.
      slist_t * formparms = new_slist_typed (adapt_test, a60_as_symbol);

      slist_it_t * ht = slist_iter (formal_ids);
      slist_it_t * it = new_slist_it (); // iterate over id_types and value_ids
      slist_it_t * jt = new_slist_it (); // iterate over lists inside id_types

      // Build a list of formal parameters (formparam symbols).
      for (; slist_it_has (ht); slist_it_next (ht))
	{
	  label_t * id0 = a60_as_label (slist_it_get (ht));
	  type_t * type0 = NULL;

	  // Iterate over all id_types specifiers, and find if type
	  // is specified at most once.
	  for (slist_it_reset (it, id_types); slist_it_has (it); )
	    {
	      slist_t * bunch = a60_as_slist (slist_it_get (it));
	      slist_it_next (it);

	      type_t * type1 = a60_as_type (slist_it_get (it));
	      slist_it_next (it);

	      for (slist_it_reset (jt, bunch); slist_it_has (jt); slist_it_next (jt))
		{
		  label_t * id1 = a60_as_label (slist_it_get (jt));
		  if (label_eq (id0, id1))
		    {
		      if (type0 != NULL)
			{
			  log_printfc (parser->log, ll_error, label_cursor (id1),
				       "duplicate specifier: `%s' already declared as `%s'.",
				       estr_cstr (label_id (id1)),
				       estr_cstr (type_to_str (type0, parser->tmp)));
			}
		      else
		        type0 = type1;
		    }
		}
	    }

	  parmconv_t convention = pc_byname;
	  for (slist_it_reset (it, value_ids);
	       slist_it_has (it); slist_it_next (it))
	    if (label_eq (id0, a60_as_label (slist_it_get (it))))
	      {
		convention = pc_byvalue;
		break;
	      }

	  // Having the type unspecified is only allowed for by-name
	  // parameters.  Unpecified type implies type_any.
	  if (type0 == NULL && convention == pc_byvalue)
	    {
	      log_printfc (parser->log, ll_error, label_cursor (id0),
			   "parameter `%s' is declared by-value, but has no type specifier.",
			   estr_cstr (label_id (id0)));
	      type0 = type_int ();
	     }
	  else if (type0 == NULL)
	    {
	      assert (convention == pc_byname);
	      type0 = type_any ();
	    }

	  symbol_t * formparm = new_symbol_formparm (id0, type0, convention);
	  slist_pushback (formparms, formparm);
	  slist_pushfront (types, type0);
	}

      // Check that all parameters in ValuePart are really formal parameters,
      // and that each parameter is specified at most once in ValuePart.
      for (slist_it_reset (it, value_ids); slist_it_has (it); slist_it_next (it))
	{
	  label_t * id1 = a60_as_label (slist_it_get (it));
	  int found = 0;

    	  for (slist_it_reset (ht, formal_ids);
	       slist_it_has (ht); slist_it_next (ht))
	    if (label_eq (id1, a60_as_label (slist_it_get (ht))))
	      {
		found = 1;
		break;
	      }

	  if (!found)
	    log_printfc (parser->log, ll_error, label_cursor (id1),
			 "invalid identifier in value part: `%s' is not a formal parameter.",
			 estr_cstr (label_id (id1)));

	  for (slist_it_reset_it (ht, it), slist_it_next (ht);
	       slist_it_has (ht); slist_it_next (ht))
	    if (label_eq (id1, a60_as_label (slist_it_get (ht))))
	      {
		log_printfc (parser->log, ll_error, label_cursor (id1),
			     "duplicate identifier in value part: `%s' was already specified.",
			     estr_cstr (label_id (id1)));
		break;
	      }
	}

      // Check that all parameters in SpecificationPart are really formal parameters.
      for (slist_it_reset (it, id_types); slist_it_has (it); )
	{
	  slist_t * bunch = a60_as_slist (slist_it_get (it));
	  slist_it_next (it);
	  slist_it_next (it);

	  for (slist_it_reset (jt, bunch); slist_it_has (jt); slist_it_next (jt))
	    {
	      label_t * id1 = a60_as_label (slist_it_get (jt));
	      int found = 0;

	      for (slist_it_reset (ht, formal_ids);
		   slist_it_has (ht); slist_it_next (ht))
		if (label_eq (id1, a60_as_label (slist_it_get (ht))))
		  {
		    found = 1;
		    break;
		  }

	      if (!found)
		{
		  log_printfc (parser->log, ll_error, label_cursor (id1),
			       "invalid specifier: `%s' is not a formal parameter.",
			       estr_cstr (label_id (id1)));
		}
	    }
	}

      delete_slist_it (jt);
      delete_slist_it (it);
      delete_slist_it (ht);


      // BODY is a function body.
      container_t * body = private_close_block (parser);
      container_add_stmt (parser->block, (statement_t *)body);

      // Add parameters to function symbol table.
      for (slist_it_reset (ht, formparms); slist_it_has (ht); slist_it_next (ht))
	{
	  symbol_t * parameter = a60_as_symbol (slist_it_get (ht));
	  type_t * type = symbol_type (parameter);
	  private_setup_and_add_symbol (parser, parameter, type);
	}

      // Build function symbol and type.  Close function parameter
      // block, leave parentless, re-parent later.
      type_t * func_type = new_t_proc ($1 ? $1 : type_void (), types);
      symbol_t * func_sym = new_symbol_func ($3);
      container_t * parblock = private_close_block (parser);
      symbol_set_stmt (func_sym, (statement_t*)parblock);
      symbol_set_type (func_sym, func_type);

      // Accomodate function symbol.
      private_setup_and_add_symbol (parser, func_sym, func_type);
    }

FormalParameterPart:
  /*epsilon*/ { $$ = new_slist (); }
  |
  SEPLPAREN FormalParameterList SEPRPAREN { $$ = $2; }

FormalParameterList:
  Identifier
    {
      $$ = new_slist ();
      slist_pushfront ($$, $1);
      @$ = @1;
    }
  |
  FormalParameterList ParameterDelimiter Identifier
    {
      slist_pushfront ($$, $3);
      @$ = @1;
    }

ValuePart:
  /*epsilon*/ { $$ = new_slist (); }
  |
  KWVALUE IdentifierList SEPSEMICOLON { $$ = $2; }

SpecificationPart:
  /*epsilon*/
    { $$ = new_slist (); }
  |
  SpecificationPart Specifier IdentifierList SEPSEMICOLON
    {
      slist_pushback ($$, $3);
      slist_pushback ($$, $2);
      @$ = @1;
    }

Specifier:
  IntrinsicType
  |
  IntrinsicType KWARRAY { $$ = new_t_array ($1); }
  |
  KWARRAY { $$ = type_array_real (); }
  |
  KWLABEL
    {
      log_printf (parser->log, ll_debug, "Specifier -> KWLABEL");
      $$ = type_label ();
      @$ = @1;
    }
  |
  KWSWITCH
    {
      log_printf (parser->log, ll_debug, "Specifier -> KWSWITCH");
      $$ = type_switch_any ();
      @$ = @1;
    }
  |
  OptIntrinsicType KWPROCEDURE
    {
      log_printf (parser->log, ll_debug, "Specifier -> OptType KWPROCEDURE");
      $$ = new_t_proc_stub ($1 ? $1 : type_any ());
      @$ = @1;
    }

Type:
  OptOwn IntrinsicType
    {
      log_printf (parser->log, ll_debug, "Type -> OptOwn IntrinsicType");
      if ($1)
	{
	  $$ = new_t_own ($2);
	  @$ = @1;
	}
      else
        {
	  $$ = $2;
	  @$ = @2;
	}
    }
  |
  OptOwn OptIntrinsicType KWARRAY
    {
      log_printf (parser->log, ll_debug, "Type -> OptOwn IntrinsicType");
      // if no type declarator is given the type 'real' is understood
      type_t * t = new_t_array (($2 != NULL) ? $2 : type_real ());
      if ($1)
        {
	  t = new_t_own (t);
	  @$ = @1;
	}
      else if ($2)
        @$ = @2;
      else
        @$ = @3;
      $$ = t;
    }

OptType:
  /*epsilon*/ { $$ = NULL; }
  |
  Type

OptOwn:
  /*epsilon*/ { $$ = 0; }
  |
  KWOWN   { $$ = 1; @$ = @1; }

OptIntrinsicType:
  /*epsilon*/
    {
      log_printf (parser->log, ll_debug, "OptIntrinsicType -> epsilon");
      $$ = NULL;
    }
  |
  IntrinsicType
    {
      log_printf (parser->log, ll_debug, "OptIntrinsicType -> IntrinsicType");
      $$ = $1;
      @$ = @1;
    }

IntrinsicType:
  KWBOOLEAN
    {
      log_printf (parser->log, ll_debug, "IntrinsicType -> KWBOOLEAN");
      $$ = type_bool ();
      @$ = @1;
    }
  |
  KWINTEGER
    {
      log_printf (parser->log, ll_debug, "IntrinsicType -> KWINTEGER");
      $$ = type_int ();
      @$ = @1;
    }
  |
  KWREAL
    {
      log_printf (parser->log, ll_debug, "IntrinsicType -> KWREAL");
      $$ = type_real ();
      @$ = @1;
    }
  |
  KWSTRING
    {
      log_printf (parser->log, ll_debug, "IntrinsicType -> KWSTRING");
      $$ = type_string ();
      @$ = @1;
    }

IdentifierList:
  Identifier OptBoundsPairList
    {
      log_printf (parser->log, ll_debug, "IdentifierList -> Identifier OptBoundsPairList");

      if ($2)
	{
	  label_add_boundspairs_with ($1, $2);
	  delete_slist ($2);
	}

      $$ = new_slist ();
      slist_pushfront ($$, $1);
      @$ = @1;
    }
  |
  IdentifierList SEPCOMMA Identifier OptBoundsPairList
    {
      log_printf (parser->log, ll_debug,
		  "IdentifierList -> IdentifierList SEPCOMMA Identifier OptBoundsPairList");

      if ($4)
	{
	  label_add_boundspairs_with ($3, $4);
	  delete_slist ($4);
	}

      slist_pushfront ($1, $3);
      $$ = $1;
      @$ = @1;
    }

SwitchList:
  DesignationalExpression
    {
      log_printf (parser->log, ll_debug, "SwitchList -> DesignationalExpression");
      $$ = new_slist_from (1, $1);
      @$ = @1;
    }
  |
  SwitchList SEPCOMMA DesignationalExpression
    {
      log_printf (parser->log, ll_debug, "SwitchList -> SwitchList SEPCOMMA DesignationalExpression");
      slist_pushback ($1, $3);
      $$ = $1;
      @$ = @1;
    }

OptBoundsPairList:
  /*epsilon*/
    {
      log_printf (parser->log, ll_debug, "OptBoundsPairList -> epsilon");
      $$ = NULL;
    }
  |
  SEPLBRACK BoundsPairList SEPRBRACK
    {
      log_printf (parser->log, ll_debug, "OptBoundsPairList -> SEPRBRACK BoundsPairList SEPRBRACK");
      $$ = $2;
      @$ = @1;
    }

BoundsPairList:
  BoundsPair
    {
      log_printf (parser->log, ll_debug, "BoundsPairList -> BoundsPair");
      $$ = new_slist ();
      slist_pushback ($$, $1);
      @$ = @1;
    }
  |
  BoundsPairList SEPCOMMA BoundsPair
    {
      log_printf (parser->log, ll_debug, "BoundsPairList -> BoundsPairList SEPCOMMA BoundsPair");
      slist_pushback ($1, $3);
      $$ = $1;
      @$ = @1;
    }

BoundsPair:
  Expression SEPCOLON Expression
    {
      log_printf (parser->log, ll_debug, "BoundsPair -> Expression SEPCOLON Expression");

      expression_t * lb = $1;
      expression_t * hb = $3;

      /* @@@TODO: if it's possible to evaluate expression in place, do
	 it, and check that the boundaries make sense: A:B, A<B.  Also
	 it will be necessary to generate runtime tests for that. */
      $$ = new_boundspair (lb, hb);
      @$ = @1;
    }

Expression:
  SimpleExpression
    {
      log_printf (parser->log, ll_debug, "Expression -> SimpleExpression");
      $$ = $1;
      @$ = @1;
    }
  |
  KWIF Expression KWTHEN SimpleExpression KWELSE Expression
    {
      log_printf (parser->log, ll_debug, "ArithmeticExpression -> KWIF Expression"
		  " KWTHEN SimpleArithmeticExpression ELSE SimpleArithmeticExpression");
      $$ = new_expr_if (cr_csr (parser, &@1), $2, $4, $6);
      @$ = @1;
    }

SimpleExpression:
  LITINTEGER
    {
      log_printf (parser->log, ll_debug, "SimpleExpression -> LITINTEGER");
      $$ = new_expr_int (cr_csr (parser, &@1), $1);
      @$ = @1;
    }
  |
  LITREAL
    {
      log_printf (parser->log, ll_debug, "SimpleExpression -> LITREAL");
      $$ = new_expr_real (cr_csr (parser, &@1), $1);
      @$ = @1;
    }
  |
  LITSTRING
    {
      log_printf (parser->log, ll_debug, "SimpleExpression -> LITSTRING");
      $$ = new_expr_string (cr_csr (parser, &@1), $1);
      @$ = @1;
    }
  |
  KWFALSE
    {
      log_printf (parser->log, ll_debug, "SimpleExpression -> KWFALSE");
      $$ = new_expr_bool (cr_csr (parser, &@1), 0);
      @$ = @1;
    }
  |
  KWTRUE
    {
      log_printf (parser->log, ll_debug, "SimpleExpression -> KWTRUE");
      $$ = new_expr_bool (cr_csr (parser, &@1), 1);
      @$ = @1;
    }
  |
  FunctionDesignator
    {
      log_printf (parser->log, ll_debug, "SimpleExpression -> FunctionDesignator");
      $$ = $1;
      @$ = @1;
    }
  |
  SEPLPAREN Expression SEPRPAREN
    {
      log_printf (parser->log, ll_debug, "SimpleExpression -> SEPLPAREN Expression SEPRPAREN");
      $$ = $2;
      @$ = @2;
    }
  |
  AOPSUB SimpleExpression %prec PREC_UNARY_MINUS
    {
      log_printf (parser->log, ll_debug, "SimpleExpression -> AOPSUB SimpleExpression");
      $$ = new_expr_unary (cr_csr (parser, &@2), euk_uminus, $2);
      @$ = @2;
    }
  |
  SimpleExpression AOPADD SimpleExpression
    {
      log_printf (parser->log, ll_debug, "SimpleExpression -> SimpleExpression AOPADD SimpleExpression");
      $$ = new_expr_binary (cr_csr (parser, &@1), ebk_aadd, $1, $3);
      @$ = @1;
    }
  |
  SimpleExpression AOPSUB SimpleExpression
    {
      log_printf (parser->log, ll_debug, "SimpleExpression -> SimpleExpression AOPSUB SimpleExpression");
      $$ = new_expr_binary (cr_csr (parser, &@1), ebk_asub, $1, $3);
      @$ = @1;
    }
  |
  SimpleExpression AOPMUL SimpleExpression
    {
      log_printf (parser->log, ll_debug, "SimpleExpression -> SimpleExpression AOPMUL SimpleExpression");
      $$ = new_expr_binary (cr_csr (parser, &@1), ebk_amul, $1, $3);
      @$ = @1;
    }
  |
  SimpleExpression AOPIDIV SimpleExpression
    {
      log_printf (parser->log, ll_debug, "SimpleExpression -> SimpleExpression AOPIDIV SimpleExpression");
      $$ = new_expr_binary (cr_csr (parser, &@1), ebk_aidiv, $1, $3);
      @$ = @1;
    }
  |
  SimpleExpression AOPRDIV SimpleExpression
    {
      log_printf (parser->log, ll_debug, "SimpleExpression -> SimpleExpression AOPRDIV SimpleExpression");
      $$ = new_expr_binary (cr_csr (parser, &@1), ebk_ardiv, $1, $3);
      @$ = @1;
    }
  |
  SimpleExpression AOPPOW SimpleExpression
    {
      log_printf (parser->log, ll_debug, "SimpleExpression -> SimpleExpression AOPPOW SimpleExpression");
      $$ = new_expr_binary (cr_csr (parser, &@1), ebk_apow, $1, $3);
      @$ = @1;
    }
  |
  SimpleExpression ROPNEQ SimpleExpression
    {
      log_printf (parser->log, ll_debug, "SimpleExpression -> SimpleExpression ROPNEQ SimpleExpression");
      $$ = new_expr_binary (cr_csr (parser, &@1), ebk_rneq, $1, $3);
      @$ = @1;
    }
  |
  SimpleExpression ROPEQ SimpleExpression
    {
      log_printf (parser->log, ll_debug, "SimpleExpression -> SimpleExpression ROPEQ SimpleExpression");
      $$ = new_expr_binary (cr_csr (parser, &@1), ebk_req, $1, $3);
      @$ = @1;
    }
  |
  SimpleExpression ROPLTE SimpleExpression
    {
      log_printf (parser->log, ll_debug, "SimpleExpression -> SimpleExpression ROPLTE SimpleExpression");
      $$ = new_expr_binary (cr_csr (parser, &@1), ebk_rlte, $1, $3);
      @$ = @1;
    }
  |
  SimpleExpression ROPLT SimpleExpression
    {
      log_printf (parser->log, ll_debug, "SimpleExpression -> SimpleExpression ROPLT SimpleExpression");
      $$ = new_expr_binary (cr_csr (parser, &@1), ebk_rlt, $1, $3);
      @$ = @1;
    }
  |
  SimpleExpression ROPGTE SimpleExpression
    {
      log_printf (parser->log, ll_debug, "SimpleExpression -> SimpleExpression ROPGTE SimpleExpression");
      $$ = new_expr_binary (cr_csr (parser, &@1), ebk_rgte, $1, $3);
      @$ = @1;
    }
  |
  SimpleExpression ROPGT SimpleExpression
    {
      log_printf (parser->log, ll_debug, "SimpleExpression -> SimpleExpression ROPGT SimpleExpression");
      $$ = new_expr_binary (cr_csr (parser, &@1), ebk_rgt, $1, $3);
      @$ = @1;
    }
  |
  SimpleExpression LOPEQ SimpleExpression
    {
      log_printf (parser->log, ll_debug, "SimpleExpression -> SimpleExpression LOPEQ SimpleExpression");
      $$ = new_expr_binary (cr_csr (parser, &@1), ebk_leq, $1, $3);
      @$ = @1;
    }
  |
  SimpleExpression LOPIMP SimpleExpression
    {
      log_printf (parser->log, ll_debug, "SimpleExpression -> SimpleExpression LOPIMP SimpleExpression");
      $$ = new_expr_binary (cr_csr (parser, &@1), ebk_limp, $1, $3);
      @$ = @1;
    }
  |
  SimpleExpression LOPAND SimpleExpression
    {
      log_printf (parser->log, ll_debug, "SimpleExpression -> SimpleExpression LOPAND SimpleExpression");
      $$ = new_expr_binary (cr_csr (parser, &@1), ebk_land, $1, $3);
      @$ = @1;
    }
  |
  SimpleExpression LOPOR SimpleExpression
    {
      log_printf (parser->log, ll_debug, "SimpleExpression -> SimpleExpression LOPOR SimpleExpression");
      $$ = new_expr_binary (cr_csr (parser, &@1), ebk_lor, $1, $3);
      @$ = @1;
    }
  |
  LOPNOT SimpleExpression
    {
      log_printf (parser->log, ll_debug, "SimpleExpression -> LOPNOT SimpleExpression");
      $$ = new_expr_unary (cr_csr (parser, &@1), euk_not, $2);
      @$ = @1;
    }

FunctionDesignator:
  Identifier SEPLPAREN ActualParamList SEPRPAREN
    {
      $$ = new_expr_call (cr_csr (parser, &@1), $1, $3);
      @$ = @1;
    }
  |
  VariableAccess
    {
      $$ = $1;
      @$ = @1;
    }

VariableAccess:
  Identifier SEPLBRACK SubscriptList SEPRBRACK
    {
      log_printf (parser->log, ll_debug, "Identifier SEPLBRACK Expression SEPRBRACK");
      $$ = new_expr_subscript (cr_csr (parser, &@1), $1, $3);
      @$ = @1;
    }
  |
  Identifier
    {
      // note: identifier node may denote funcall without parameters.  The
      // translation from expr_idref to expr_call is done during semantic
      // analysis, where we can decide what type given id is.
      $$ = new_expr_idref (cr_csr (parser, &@1), $1);
      @$ = @1;
    }

ActualParamList:
  Expression
    {
      $$ = new_slist ();
      slist_pushback ($$, $1);
    }
  |
  ActualParamList ParameterDelimiter Expression
    {
      if ($2 != NULL)
	{
	  // @@@TODO: delimiters are ignored as per RRA60, but we want
	  // to check match with formals
	}

      $$ = $1;
      slist_pushback ($$, $3);
    }

SubscriptList:
  Expression
    {
      $$ = new_slist ();
      slist_pushback ($$, $1);
    }
  |
  SubscriptList SEPCOMMA Expression
    {
      $$ = $1;
      slist_pushback ($$, $3);
    }

ParameterDelimiter:
  SEPCOMMA
    {
      $$ = NULL;
    }
  |
  SEPRPAREN IDENTIFIER SEPCOLON SEPLPAREN
    {
      $$ = $2;
    }

StatementList:
  StatementList SEPSEMICOLON Statement
    {
      log_printf (parser->log, ll_debug, "StatementList -> StatementList SEPSEMICOLON Statement");
      container_add_stmt (parser->block, $3);
    }
  |
  Statement
    {
      log_printf (parser->log, ll_debug, "StatementList -> Statement");
      container_add_stmt (parser->block, $1);
    }

Statement:
  UnconditionalStatement
    {
      log_printf (parser->log, ll_debug, "Statement -> UnconditionalStatement");
      $$ = $1;
    }
  |
  ConditionalStatement
    {
      log_printf (parser->log, ll_debug, "Statement -> ConditionalStatement");
      $$ = $1;
    }
  |
  ForStatement
    {
      log_printf (parser->log, ll_debug, "Statement -> ForStatement");
      $$ = $1;
    }

UnconditionalStatement:
  LabelList Block
    {
      log_printf (parser->log, ll_debug, "UnconditionalStatement -> LabelList Block");
      private_dump_log_labels (parser, $1);
      private_add_labels_to_symtab (parser, parser->block, $1, $2);
      $$ = $2;
    }
  |
  LabelList BasicStatement
    {
      log_printf (parser->log, ll_debug, "UnconditionalStatement -> BasicStatement");
      private_add_labels_to_symtab (parser, parser->block, $1, $2);
      $$ = $2;
    }

BasicStatement:
  /*epsilon*/
    {
      log_printf (parser->log, ll_debug, "BasicStatement -> LabelList");
      $$ = new_stmt_dummy (NULL);
    }
  |
  LeftPartList Expression
    {
      log_printf (parser->log, ll_debug, "BasicStatement -> LabelList LeftPartList Expression");
      // $1 can happen to be empty if all targets were non-lvalues.
      // In such a case, substitute dummy stmt instead.
      if (slist_empty ($1))
	{
	  $$ = new_stmt_dummy (cr_csr (parser, &@1));
	  delete_slist ($1);
	}
      else
	$$ = new_stmt_assign (cr_csr (parser, &@1), $1, $2);
    }
  |
  FunctionDesignator
    {
      $$ = new_stmt_call (cr_csr (parser, &@1), $1);
    }
  |
  KWGOTO DesignationalExpression
    {
      $$ = new_stmt_goto (cr_csr (parser, &@1), $2);
    }

LeftPartList:
  LeftPart
    {
      $$ = new_slist ();
      if (expr_is_lvalue ($1))
	slist_pushback ($$, $1);
      else
	log_printfc (parser->log, ll_error, cr_csr (parser, &@1),
		     "lvalue required as an assignment target");
    }
  |
  LeftPartList LeftPart
    {
      if (expr_is_lvalue ($2))
	slist_pushback ($1, $2);
      else
	log_printfc (parser->log, ll_error, cr_csr (parser, &@1),
		     "lvalue required as an assignment target");
      $$ = $1;
    }

LeftPart:
  VariableAccess SEPASSIGN
    {
      $$ = $1;
    }

DesignationalExpression:
  SimpleDesignationalExpression
    {
      log_printf (parser->log, ll_debug, "DesignationalExpression -> SimpleDesignationalExpression");
      $$ = $1;
    }
  |
  IfClause SimpleDesignationalExpression KWELSE SimpleDesignationalExpression
    {
      log_printf (parser->log, ll_debug, "DesignationalExpression -> IfClause SimpleDesignationalExpression KWELSE SimpleDesignationalExpression");
      $$ = new_desig_expr_if (cr_csr (parser, &@1), $1, $2, $4);
    }

SimpleDesignationalExpression:
  LabelIdentifier
    {
      log_printf (parser->log, ll_debug, "SimpleDesignationalExpression -> LabelIdentifier");
      $$ = new_desig_expr_label (cr_csr (parser, &@1), $1);
    }
  |
  Identifier SEPLBRACK Expression SEPRBRACK
    {
      log_printf (parser->log, ll_debug, "SimpleDesignationalExpression -> Identifier SEPLBRACK Expression SEPRBRACK");
      $$ = new_desig_expr_switch (cr_csr (parser, &@1), $1, $3);
    }
  |
  SEPLPAREN DesignationalExpression SEPRPAREN
    {
      log_printf (parser->log, ll_debug, "SimpleDesignationalExpression -> SEPLPAREN DesignationalExpression SEPRPAREN");
      $$ = $2;
    }

ConditionalStatement:
  LabelList IfClause UnconditionalStatement OptElseClause
    {
      log_printf (parser->log, ll_debug, "ConditionalStatement -> LabelList IfClause UnconditionalStatement OptElseClause");
      private_dump_log_labels (parser, $1);
      $$ = new_stmt_cond (cr_csr (parser, &@2), $2, $3, $4);
      @$ = @2;
      private_add_labels_to_symtab (parser, parser->block, $1, $$);
    }

IfClause:
  KWIF Expression KWTHEN
    {
      log_printf (parser->log, ll_debug, "IfClause -> KWIF Expression KWTHEN");
      $$ = $2;
      @$ = @1;
    }

OptElseClause:
  /*epsilon*/
    {
      $$ = NULL;
    }
  |
  KWELSE Statement
    {
      $$ = $2;
      @$ = @2;
    }

ForStatement:
  LabelList KWFOR Identifier SEPASSIGN ForList KWDO Statement
    {
      log_printf (parser->log, ll_debug, "ForStatement -> LabelList KWFOR Identifier SEPASSIGN ForList KWDO Statement");
      private_dump_log_labels (parser, $1);
      $$ = new_stmt_for (cr_csr (parser, &@2),
			 new_expr_idref (cr_csr (parser, &@3), $3),
			 $5, $7);
      @$ = @2;
      private_add_labels_to_symtab (parser, parser->block, $1, $$);
    }

ForList:
  ForListElmt
    {
      log_printf (parser->log, ll_debug, "ForList -> ForListElmt");
      $$ = new_slist_typed_from (adapt_test, a60_as_for_elmt, 1, $1);
      @$ = @1;
    }
  |
  ForList SEPCOMMA ForListElmt
    {
      log_printf (parser->log, ll_debug, "ForList -> ForList SEPCOMMA ForListElmt");
      slist_pushback ($1, $3);
      $$ = $1;
      @$ = @1;
    }

ForListElmt:
  Expression
    {
      $$ = new_for_elmt_expr (cr_csr (parser, &@1), $1);
      @$ = @1;
    }
  |
  Expression KWSTEP Expression KWUNTIL Expression
    {
      $$ = new_for_elmt_until (cr_csr (parser, &@1), $1, $3, $5);
      @$ = @1;
    }
  |
  Expression KWWHILE Expression
    {
      $$ = new_for_elmt_while (cr_csr (parser, &@1), $1, $3);
      @$ = @1;
    }

%%

parser_t *
new_parser (lexer_t * lexer, int manage)
{
  assert (lexer != NULL);

  parser_rep_t * ret = calloc (1, sizeof (parser_rep_t));


  if (ret == NULL)
    return NULL;

  jmp_buf buf;
  if (setjmp (buf) == 0)
    {
#ifndef NDEBUG
      ret->base.signature = private_parser_signature;
#endif
      ret->lexer = lexer;
      ret->manage = manage;
      guard_ptr (buf, 1, ret->log = new_logger ("parser"));
      log_set_filter (ret->log, ll_debug);
      guard_ptr (buf, 1, ret->blockstack = new_slist ());
      guard_ptr (buf, 1, ret->tmp = new_estring ());
      return (void*)ret;
    }
  else
    {
      delete_parser ((parser_t*)ret);
      return NULL;
    }
}

void
delete_parser (parser_t * _parser)
{
  if (_parser != NULL)
    {
      parser_rep_t * parser = (void*)_parser;
      delete_logger (parser->log);
      delete_slist (parser->blockstack);
      delete_estring (parser->tmp);
      free (parser);
    }
}

parser_t *
a60_as_parser (void * obj)
{
#ifndef NDEBUG
  a60_check_access (obj, private_parser_signature);
#endif
  return (parser_t *)obj;
}

statement_t *
parser_parse (parser_t * _parser)
{
  assert (_parser != NULL);
  parser_rep_t * parser = (void*)_parser;
  log_printf (parser->log, ll_debug, "parser_parse: let's rock");

  int ret = yyparse (parser);
  if (ret == 0)
    return parser->result;
  else
    return NULL;
}

void
parser_set_logging (parser_t * _parser, debug_level_t filter)
{
  assert (_parser != NULL);
  parser_rep_t * parser = (void*)_parser;
  log_set_filter (parser->log, filter);
}

logger_t const*
parser_log (parser_t * _parser)
{
  assert (_parser != NULL);
  parser_rep_t * parser = (void*)_parser;
  return parser->log;
}

static void
private_dump_log_labels (parser_rep_t * parser, slist_t * slist)
{
  slist_it_t * it = slist_iter (slist);
  estring_t * lstr = new_estring ();
  if (slist_it_has (it))
    log_printf (parser->log, ll_debug, " this block has following labels:");
  for (; slist_it_has (it); slist_it_next (it))
    {
      label_to_str (slist_it_get (it), lstr);
      log_printf (parser->log, ll_debug, "  %s", estr_cstr (lstr));
    }
  delete_estring (lstr);
  delete_slist_it (it);
}

static void
private_add_labels_to_symtab (parser_rep_t * parser,
			      container_t * context,
			      slist_t * slist, statement_t * target)
{
  if (slist_empty (slist))
    return;

  // Algol has an interesting quirk: labels are local to the nearest
  // enclosing *block* (as opposed to mere compound statement).  So we
  // must actually look up nearest block with declarations.
  //
  // Strictly speaking, when there are no declarations at program
  // block, then it's actually program compound statement, and no
  // labels may be defined at that level.  However we will put all
  // labels to the program block, if no other block is closer.

  a60_symtab_t * symtab = container_symtab (context);
  while (a60_symtab_kind (symtab) != a60_stk_block
	 && a60_symtab_empty (container_symtab (context)))
    symtab = a60_symtab_parent (symtab);

  slist_it_t * it = slist_iter (slist);
  for (; slist_it_has (it); slist_it_next (it))
    {
      label_t * lbl = slist_it_get (it);
      symbol_t * sym = new_symbol_var (lbl);
      if (a60_symtab_add_symbol (symtab, sym, sek_ordinary) != 0)
	{
	  cursor_t * csr = stmt_cursor (target);
	  log_printfc (parser->log, ll_error, csr,
		       "duplicate label `%s'.", estr_cstr (label_id (lbl)));
	  delete_symbol (sym);
	}
      else
	{
	  symbol_set_stmt (sym, target);
	  symbol_set_type (sym, type_label ());
	  stmt_add_label (target, sym);
	}
    }
  delete_slist_it (it);
}

static void
private_setup_and_add_symbol (parser_rep_t * parser, symbol_t * sym, type_t * qt)
{
  // Setup symbol and add to table.
  label_t const * lbl = symbol_label (sym);
  symbol_set_type (sym, qt);
  a60_symtab_t * symtab = container_symtab (parser->block);
  int conflict = a60_symtab_add_symbol (symtab, sym, sek_ordinary);
  if (conflict)
    {
      parser->tmp = label_to_str (lbl, parser->tmp);
      log_printfc (parser->log, ll_error, label_cursor (lbl),
		  "duplicate identifier `%s'.", estr_cstr (parser->tmp));
    }
}

static void
private_open_block (parser_rep_t * parser, container_t * cont)
{
  assert (cont != NULL);
  slist_pushfront (parser->blockstack, parser->block);
  log_printf (parser->log, ll_debug, "open %p", cont);
  parser->block = cont;
}

static container_t *
private_close_block (parser_rep_t * parser)
{
  assert (!slist_empty (parser->blockstack));
  container_t * cont = parser->block;
  parser->block = slist_popfront (parser->blockstack);
  log_printf (parser->log, ll_debug, "close %p", cont);
  return cont;
}


/*
 * Local Variables:
 * mode: c
 * c-syntactic-indentation: nil
 * End:
 */
