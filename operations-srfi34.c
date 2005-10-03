/*===========================================================================
 *  FileName : operations-srfi34.c
 *  About    : Exception Handling for Programs
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
#include <setjmp.h>

/*=======================================
  Local Include
=======================================*/
#include "sigscheme.h"
#include "sigschemeinternal.h"

/*=======================================
  File Local Struct Declarations
=======================================*/

/*=======================================
  File Local Macro Declarations
=======================================*/
#define CURRENT_EXCEPTION_HANDLER()             \
    (CAR(scm_exception_handlers))
#define PUSH_EXCEPTION_HANDLER(handler)                                 \
    (scm_exception_handlers = CONS((handler), scm_exception_handlers))
#define POP_EXCEPTION_HANDLER()                 \
    (scm_exception_handlers = CDR(scm_exception_handlers))

#define CURRENT_EXCEPTION_CONTINUATION()        \
    (CAR(scm_exception_continuations))
#define PUSH_EXCEPTION_CONTINUATION(cont)                               \
    (scm_exception_continuations = CONS((cont), scm_exception_continuations))
#define POP_EXCEPTION_CONTINUATION()            \
    (scm_exception_continuations = CDR(scm_exception_continuations))

#define CONTINUATION_JMPENV     SCM_CONTINUATION_OPAQUE0
#define CONTINUATION_SET_JMPENV SCM_CONTINUATION_SET_OPAQUE0

/*=======================================
  Variable Declarations
=======================================*/
ScmObj scm_exception_handlers      = NULL;
ScmObj scm_exception_continuations = NULL;

static ScmObj exception_thrown_obj = NULL;

/*=======================================
  File Local Function Declarations
=======================================*/
static ScmObj guard_handle_clauses(ScmObj clauses, ScmObj env);

/*=======================================
  Function Implementations
=======================================*/
ScmObj ScmOp_SRFI34_with_exception_handler(ScmObj handler, ScmObj thunk)
{
    jmp_buf jmpenv;
    ScmObj ret  = SCM_FALSE;
    ScmObj cont = Scm_NewContinuation();
    DECLARE_FUNCTION("with-exception-handler", ProcedureFixed);

    ASSERT_PROCEDUREP(handler);
    ASSERT_PROCEDUREP(thunk);

    CONTINUATION_SET_JMPENV(cont, &jmpenv);
    if (setjmp(CONTINUATION_JMPENV(cont))) {
        ret = Scm_call(CURRENT_EXCEPTION_HANDLER(), LIST_1(exception_thrown_obj));
        POP_EXCEPTION_CONTINUATION();
        POP_EXCEPTION_HANDLER();
        exception_thrown_obj = SCM_FALSE; /* make sweepable */
        return ret;
    }

    PUSH_EXCEPTION_HANDLER(handler);
    PUSH_EXCEPTION_CONTINUATION(cont);
    ret = Scm_call(thunk, SCM_NULL);
    POP_EXCEPTION_CONTINUATION();
    POP_EXCEPTION_HANDLER();

    return ret;
}

/* FIXME:
 * - Simplify with POP_ARG macros
 */
ScmObj ScmExp_SRFI34_guard(ScmObj var_and_clauses, ScmObj body, ScmObj env)
{
    /* (guard (var clauses) body) */
    jmp_buf jmpenv;
    ScmObj var     = CAR(var_and_clauses);
    ScmObj clauses = CDR(var_and_clauses);
    ScmObj expr    = SCM_FALSE;
    ScmObj cont    = Scm_NewContinuation();
    DECLARE_FUNCTION("guard", SyntaxVariadic1);

    ASSERT_SYMBOLP(var);

    /* check if return from "raise" */
    CONTINUATION_SET_JMPENV(cont, &jmpenv);
    if (setjmp(CONTINUATION_JMPENV(cont))) {
        POP_EXCEPTION_CONTINUATION();
        env = Scm_ExtendEnvironment(LIST_1(var), LIST_1(exception_thrown_obj), env);
        return guard_handle_clauses(clauses, env);
    }

    PUSH_EXCEPTION_CONTINUATION(cont);
    while (!NO_MORE_ARG(body)) {
        expr = POP_ARG(body);
        expr = EVAL(expr, env);
    }
    POP_EXCEPTION_CONTINUATION();

    return expr;
}

/* FIXME:
 * - Simplify with ScmExp_cond()
 */
static ScmObj guard_handle_clauses(ScmObj clauses, ScmObj env)
{
    ScmObj thrown  = exception_thrown_obj;
    ScmObj clause  = SCM_FALSE;
    ScmObj test    = SCM_FALSE;
    ScmObj exps    = SCM_FALSE;
    ScmObj proc    = SCM_FALSE;
    ScmObj ret     = SCM_FALSE;

    /* make sweepable */
    exception_thrown_obj = SCM_FALSE;

    /* handle "cond" like clause */
    for (; !NULLP(clauses); clauses = CDR(clauses)) {
        clause = CAR(clauses);
        if (!CONSP(clause))
            SigScm_ErrorObj("guard : bad clause: ", clause);

        test = CAR(clause);
        exps = CDR(clause);

        /* evaluate test */
        test = EVAL(test, env);

        if (NFALSEP(test)) {
            /*
             * if the selected <clause> contains only the <test> and no <expression>s,
             * then the value of the <test> is returned as the result.
             */
            if (NULLP(exps))
                return test;

            /*
             * If the selected <clause> uses the => alternate form, then the <expression>
             * is evaluated. Its value must be a procedure that accepts one argument;
             * this procedure is then called on the value of the <test> and the value
             * returned by this procedure is returned by the guard expression.
             */
            /* FIXME: remove expensive Scm_Intern() */
            if (EQ(Scm_Intern("=>"), CAR(exps))) {
                proc = EVAL(CADR(exps), env);
                if (FALSEP(ScmOp_procedurep(proc)))
                    SigScm_ErrorObj("guard : the value of exp after => must be the procedure but got ", proc);

                return Scm_call(proc, LIST_1(test));
            }

            for (; !NULLP(exps); exps = CDR(exps))
                ret = EVAL(CAR(exps), env);

            return ret;
        }
    }

    /* "reraise" exception */
    if (NULLP(CURRENT_EXCEPTION_CONTINUATION()))
        SigScm_Error("guard : cannot reraise exception");
    ScmOp_SRFI34_raise(thrown);

    /* never reaches here */
    return SCM_UNDEF;
}

ScmObj ScmOp_SRFI34_raise(ScmObj obj)
{
    jmp_buf *env;
    DECLARE_FUNCTION("raise", ProcedureFixed1);

    exception_thrown_obj = obj;

    env = CONTINUATION_JMPENV(CURRENT_EXCEPTION_CONTINUATION());
    longjmp(*env, 1);

    /* never reaches here */
    return SCM_UNDEF;
}
