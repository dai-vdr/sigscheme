/*===========================================================================
 *  FileName : operations-siod.c
 *  About    : SIOD compatible procedures
 *
 *  Copyright (C) 2005      by Kazuki Ohta (mover@hct.zaq.ne.jp)
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of authors nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
===========================================================================*/
/*=======================================
  System Include
=======================================*/
#include "sigscheme.h"
#include "sigschemeinternal.h"

/*=======================================
  Local Include
=======================================*/

/*=======================================
  File Local Struct Declarations
=======================================*/

/*=======================================
  File Local Macro Declarations
=======================================*/
/*
 * SIOD's verbose-level compatible debug message printing control:
 * Search 'siod_verbose_level' in slib.c to know further detail.
 *
 * Don't change the verbose level 2 for SCM_DBG_BACKTRACE. This is used to
 * suppress backtrace when run by the testing framework of uim.
 *   -- YamaKen 2005-11-05
 *
 * Extra control:
 *   v0: suppress all printing even if normal 'write' or 'display'
 *   v1: print each result of repl
 *   v2: print the "> " prompt
 */
#define SCM_DBG_SIOD_V0 SCM_DBG_NONE
#define SCM_DBG_SIOD_V1 SCM_DBG_ERRMSG
#define SCM_DBG_SIOD_V2 (SCM_DBG_SIOD_V1 | SCM_DBG_BACKTRACE)
#define SCM_DBG_SIOD_V3 (SCM_DBG_SIOD_V2 | SCM_DBG_FILE)
#define SCM_DBG_SIOD_V4 (SCM_DBG_SIOD_V3 | SCM_DBG_GC)
#define SCM_DBG_SIOD_V5 (SCM_DBG_SIOD_V4 | SCM_DBG_READ)

/*=======================================
  Variable Declarations
=======================================*/
static const int sscm_debug_mask_tbl[] = {
    SCM_DBG_SIOD_V0,
    SCM_DBG_SIOD_V1,
    SCM_DBG_SIOD_V2,
    SCM_DBG_SIOD_V3,
    SCM_DBG_SIOD_V4,
    SCM_DBG_SIOD_V5
};
static long sscm_verbose_level = -1;

static ScmObj saved_output_port  = NULL;
static ScmObj saved_error_port  = NULL;

/*=======================================
  File Local Function Declarations
=======================================*/

/*=======================================
  Function Implementations
=======================================*/
/*=======================================
  SIOD compatible procedures

  TODO : remove these functions!
=======================================*/
void SigScm_Initialize_SIOD(void)
{
    ScmExp_use(Scm_Intern("srfi-60"), SCM_INTERACTION_ENV);
    Scm_DefineAlias("bit-and"               , "logand");
    Scm_DefineAlias("bit-or"                , "logior");
    Scm_DefineAlias("bit-xor"               , "logxor");
    Scm_DefineAlias("bit-not"               , "lognot");

#if SCM_USE_REGISTER_TABLE
    REGISTER_FUNC_TABLE(siod_func_info_table);
#else /* SCM_USE_REGISTER_TABLE */
    Scm_RegisterProcedureFixed1("symbol-value"         , ScmOp_symbol_value);
    Scm_RegisterProcedureFixed2("set-symbol-value!"    , ScmOp_set_symbol_valued);
#if SCM_COMPAT_SIOD_BUGS
    Scm_RegisterProcedureFixed2("="                    , ScmOp_siod_eql);
#endif
    Scm_RegisterProcedureFixedTailRec0("the-environment" , ScmOp_the_environment);
    Scm_RegisterProcedureFixed1("%%closure-code"       , ScmOp_sscm_closure_code);
    Scm_RegisterProcedureVariadic0("verbose" , ScmOp_verbose);
    Scm_RegisterProcedureFixed0("eof-val" , ScmOp_eof_val);
    Scm_RegisterSyntaxFixed1("undefine" , ScmExp_undefine);
#endif /* SCM_USE_REGISTER_TABLE */

    saved_output_port = SCM_FALSE;
    saved_error_port  = SCM_FALSE;
    SigScm_GC_Protect(&saved_output_port);
    SigScm_GC_Protect(&saved_error_port);

    SigScm_SetVerboseLevel(2);
}

/*
 * TODO:
 * - replace with a portable proc such as (eval 'sym (interaction-environment))
 * - make the portable proc interface similar to a de facto standard of other
 *   Scheme implementations if existing
 */
ScmObj ScmOp_symbol_value(ScmObj var)
{
    DECLARE_FUNCTION("symbol-value", ProcedureFixed1);

    ASSERT_SYMBOLP(var);

    return Scm_SymbolValue(var, SCM_NULL);
}

/*
 * TODO:
 * - replace with a portable proc such as (eval '(set! sym val)
 *                                               (interaction-environment))
 * - make the portable proc interface similar to a de facto standard of other
 *   Scheme implementations if existing
 */
ScmObj ScmOp_set_symbol_valued(ScmObj var, ScmObj val)
{
    DECLARE_FUNCTION("set-symbol-value!", ProcedureFixed2);

    ASSERT_SYMBOLP(var);

    return SCM_SYMBOL_SET_VCELL(var, val);
}

ScmObj ScmOp_siod_eql(ScmObj obj1, ScmObj obj2)
{
    DECLARE_FUNCTION("=", ProcedureFixed2);

    if (EQ(obj1, obj2))
        return SCM_TRUE;
    else if (!INTP(obj1) || !INTP(obj2))
        return SCM_FALSE;
    else if (SCM_INT_VALUE(obj1) == SCM_INT_VALUE(obj2))
        return SCM_TRUE;

    return SCM_FALSE;
}

ScmObj ScmOp_the_environment(ScmEvalState *eval_state)
{
    DECLARE_FUNCTION("the-environment", ProcedureFixedTailRec0);

    eval_state->ret_type = SCM_RETTYPE_AS_IS;

    return eval_state->env;
}

ScmObj ScmOp_sscm_closure_code(ScmObj closure)
{
    ScmObj exp, body;
    DECLARE_FUNCTION("%%closure-code", ProcedureFixed1);

    ASSERT_CLOSUREP(closure);

    exp = SCM_CLOSURE_EXP(closure);
    if (NULLP(CDDR(exp)))
        body = CADR(exp);
    else
        body = CONS(Scm_Intern("begin"), CDR(exp));
    
    return CONS(CAR(exp), body);
}

ScmObj ScmOp_verbose(ScmObj args)
{
    DECLARE_FUNCTION("verbose", ProcedureVariadic0);

    if (!NULLP(args)) {
        ASSERT_INTP(CAR(args));

        SigScm_SetVerboseLevel(SCM_INT_VALUE(CAR(args)));
    }

    return Scm_NewInt(sscm_verbose_level);
}

ScmObj ScmOp_eof_val(void)
{
    DECLARE_FUNCTION("eof-val", ProcedureFixed0);
    return SCM_EOF;
}

ScmObj ScmExp_undefine(ScmObj var, ScmObj env)
{
    ScmObj val = SCM_FALSE;
    DECLARE_FUNCTION("undefine", SyntaxFixed1);

    ASSERT_SYMBOLP(var);

    val = Scm_LookupEnvironment(var, env);
    if (!NULLP(val))
        return SET_CAR(val, SCM_UNBOUND);

    SCM_SYMBOL_SET_VCELL(var, SCM_UNBOUND);

    return SCM_FALSE;
}

long SigScm_GetVerboseLevel(void)
{
    return sscm_verbose_level;
}

void SigScm_SetVerboseLevel(long level)
{
    if (level < 0)
        SigScm_Error("SigScm_SetVerboseLevel : negative value has been given");

    if (sscm_verbose_level == level)
        return;

    sscm_verbose_level = level;

    if (level > 5)
        level = 5;
    SigScm_SetDebugCategories(sscm_debug_mask_tbl[level]);

    if (level >= 2)
        SigScm_SetDebugCategories(SigScm_DebugCategories()
                                  | SigScm_PredefinedDebugCategories());

    if (level == 0) {
        saved_error_port = scm_current_error_port;
        saved_output_port = scm_current_output_port;

        scm_current_error_port = SCM_FALSE;
        scm_current_output_port = SCM_FALSE;
    } else {
        if (FALSEP(scm_current_error_port))
            scm_current_error_port = saved_error_port;
        if (FALSEP(scm_current_output_port))
            scm_current_output_port = saved_output_port;
    }
}
