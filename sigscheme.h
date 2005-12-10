/*===========================================================================
 *  FileName : sigscheme.h
 *  About    : main header file
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
#ifndef __SIGSCHEME_H
#define __SIGSCHEME_H

#ifdef __cplusplus
extern "C" {
#endif

/*=======================================
   System Include
=======================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*=======================================
   Local Include
=======================================*/
#include "config.h"
#include "encoding.h"

/*=======================================
   Macro Declarations
=======================================*/
#ifdef __GNUC__
#define SCM_NOINLINE __attribute__((noinline))
#define SCM_NORETURN __attribute__((noreturn))
#else
#define SCM_NOINLINE
#define SCM_NORETURN
#endif /* __GNUC__ */

/* Casts a machine word mainly a function pointer and (void *) */
#define SCM_WORD_CAST(type, obj) ((type)(unsigned long)(obj))
/*
 * Re-interprets a storage as-is, similarly to reinterpret_cast<type>(obj) of
 * C++. Don't change the semantics. This cast requires that obj is a pointable
 * storage.
 */
#define SCM_REINTERPRET_CAST(type, obj) (*(type *)&(obj))

/* RFC: better names for the debug printing */
#if SCM_DEBUG
#define SCM_CDBG(args) (SigScm_CategorizedDebug args)
#define SCM_DBG(args)  (SigScm_Debug args)
#else /* SCM_DEBUG */
#define SCM_CDBG(args)
#define SCM_DBG(args)
#endif /* SCM_DEBUG */

#define SCM_ASSERT(cond) \
    ((cond) || SigScm_Die("assertion failed.", __FILE__, __LINE__))

#define SCM_ERROBJP(obj)       (NFALSEP(ScmOp_error_objectp(obj)))

#define SCM_SYMBOL_BOUNDP(sym) (!SCM_EQ(SCM_SYMBOL_VCELL(sym), SCM_UNBOUND))

#define SCM_CONS(kar, kdr) (Scm_NewCons((kar), (kdr)))

#define SCM_LIST_1(elm0) \
    (SCM_CONS((elm0), SCM_NULL))
#define SCM_LIST_2(elm0, elm1) \
    (SCM_CONS((elm0), SCM_LIST_1(elm1)))
#define SCM_LIST_3(elm0, elm1, elm2) \
    (SCM_CONS((elm0), SCM_LIST_2((elm1), (elm2))))
#define SCM_LIST_4(elm0, elm1, elm2, elm3) \
    (SCM_CONS((elm0), SCM_LIST_3((elm1), (elm2), (elm3))))
#define SCM_LIST_5(elm0, elm1, elm2, elm3, elm4) \
    (SCM_CONS((elm0), SCM_LIST_4((elm1), (elm2), (elm3), (elm4))))

#define SCM_LISTP(obj)    (SCM_CONSP(obj) || SCM_NULLP(obj))
#define SCM_LIST_1_P(lst) (SCM_CONSP(lst) && SCM_NULLP(SCM_CDR(lst)))
#define SCM_LIST_2_P(lst) (SCM_CONSP(lst) && SCM_LIST_1_P(SCM_CDR(lst)))
#define SCM_LIST_3_P(lst) (SCM_CONSP(lst) && SCM_LIST_2_P(SCM_CDR(lst)))
#define SCM_LIST_4_P(lst) (SCM_CONSP(lst) && SCM_LIST_3_P(SCM_CDR(lst)))
#define SCM_LIST_5_P(lst) (SCM_CONSP(lst) && SCM_LIST_4_P(SCM_CDR(lst)))

#define SCM_EVAL(obj, env) (Scm_eval((obj), (env)))

#if SCM_GCC4_READY_GC
/*
 * Function caller with protecting Scheme objects on stack from GC
 *
 * The protection is safe against with variable reordering on a stack
 * frame performed in some compilers as anti-stack smashing or
 * optimization.
 *
 * Users should only use SCM_GC_PROTECTED_CALL() and
 * SCM_GC_PROTECTED_CALL_VOID().
 */
#define SCM_GC_PROTECTED_CALL(ret, ret_type, func, args)                     \
    SCM_GC_PROTECTED_CALL_INTERNAL(ret = , ret_type, func, args)

#define SCM_GC_PROTECTED_CALL_VOID(func, args)                               \
    SCM_GC_PROTECTED_CALL_INTERNAL((void), void, func, args)

#define SCM_GC_PROTECTED_CALL_INTERNAL(exp_ret, ret_type, func, args)        \
    do {                                                                     \
        /* ensure that func is uninlined */                                  \
        ret_type (*volatile fp)() = (ret_type (*)())&func;                   \
        ScmObj *stack_start;                                                 \
                                                                             \
        if (0) exp_ret func args;  /* compile-time type check */             \
        stack_start = SigScm_GC_ProtectStack(NULL);                          \
        exp_ret (*fp)args;                                                   \
        SigScm_GC_UnprotectStack(stack_start);                               \
    } while (/* CONSTCOND */ 0)

#endif /* SCM_GCC4_READY_GC */


/*
 * Port I/O Handling macros
 */
#define SCM_CHARPORT_ERROR(cport, msg) (SigScm_Error(msg))
#define SCM_BYTEPORT_ERROR(bport, msg) (SigScm_Error(msg))

#define SCM_ASSERT_LIVE_PORT(port)                                           \
    (SCM_PORT_IMPL(port)                                                     \
     || (SigScm_ErrorObj("operated on closed port", port), 1))

#define SCM_PORT_CLOSE_IMPL(port)                                            \
    (SCM_CHARPORT_CLOSE(SCM_PORT_IMPL(port)), SCM_PORT_SET_IMPL(port, NULL))
#define SCM_PORT_ENCODING(port)                                              \
    (SCM_ASSERT_LIVE_PORT(port), SCM_CHARPORT_ENCODING(SCM_PORT_IMPL(port)))
#define SCM_PORT_INSPECT(port)                                              \
    (SCM_ASSERT_LIVE_PORT(port), SCM_CHARPORT_INSPECT(SCM_PORT_IMPL(port)))
#define SCM_PORT_GET_CHAR(port)                                              \
    (SCM_ASSERT_LIVE_PORT(port), SCM_CHARPORT_GET_CHAR(SCM_PORT_IMPL(port)))
#define SCM_PORT_PEEK_CHAR(port)                                             \
    (SCM_ASSERT_LIVE_PORT(port), SCM_CHARPORT_PEEK_CHAR(SCM_PORT_IMPL(port)))
#define SCM_PORT_CHAR_READYP(port)                                           \
    (SCM_ASSERT_LIVE_PORT(port), SCM_CHARPORT_CHAR_READYP(SCM_PORT_IMPL(port)))
#define SCM_PORT_VPRINTF(port, str, args)                                    \
    (SCM_ASSERT_LIVE_PORT(port),                                             \
     SCM_CHARPORT_VPRINTF(SCM_PORT_IMPL(port), (str), (args)))
#define SCM_PORT_PUTS(port, str)                                             \
    (SCM_ASSERT_LIVE_PORT(port), SCM_CHARPORT_PUTS(SCM_PORT_IMPL(port), (str)))
#define SCM_PORT_PUT_CHAR(port, ch)                                          \
    (SCM_ASSERT_LIVE_PORT(port),                                             \
     SCM_CHARPORT_PUT_CHAR(SCM_PORT_IMPL(port), (ch)))
#define SCM_PORT_FLUSH(port)                                                 \
    (SCM_ASSERT_LIVE_PORT(port), SCM_CHARPORT_FLUSH(SCM_PORT_IMPL(port)))

/*
 * SCM_CHARPORT_ERROR and SCM_BYTEPORT_ERROR must be defined before this
 * inclusion
 */
#include "baseport.h"

/* FIXME: rewrite all #ifdef'ed SigScm_WriteToPortWithSharedStructure()
   invocation with this */
#define SCM_WRITESS_TO_PORT(port, obj) ((*Scm_writess_func)(port, obj))

/*=======================================
   Struct Declarations
=======================================*/
typedef void (*ScmCFunc)(void);

/* type declaration */
#if SCM_OBJ_COMPACT
#include "sigschemetype-compact.h"
#else
#include "sigschemetype.h"
#endif

enum ScmDebugCategory {
    SCM_DBG_NONE         = 0,
    SCM_DBG_ERRMSG       = 1 << 0,   /* the "Error: foo bar" style msgs */
    SCM_DBG_BACKTRACE    = 1 << 1,
    SCM_DBG_GC           = 1 << 2,
    SCM_DBG_FILE         = 1 << 3,   /* file loading */
    SCM_DBG_PARSER       = 1 << 4,
    SCM_DBG_READ         = 1 << 5,   /* print each parsed expression + misc */
    SCM_DBG_MACRO        = 1 << 6,
    SCM_DBG_ARGS         = 1 << 7,   /* number of arguments, type and so on */
    SCM_DBG_EVAL         = 1 << 8,   /* evaluation-related things */
    SCM_DBG_CONTINUATION = 1 << 9,
    SCM_DBG_EXCEPTION    = 1 << 10,
    SCM_DBG_EXPERIMENTAL = 1 << 11,  /* developed but experimental features */
    SCM_DBG_DEVEL        = 1 << 12,  /* under development */
    SCM_DBG_COMPAT       = 1 << 13,  /* warns compatibility-sensitive code */
    SCM_DBG_ENCODING     = 1 << 14,  /* multibyte handling */
    SCM_DBG_OTHER        = 1 << 30   /* all other messages */
};

/*=======================================
   Variable Declarations
=======================================*/
/* storage-gc.c */
#if SCM_GCC4_READY_GC
/*
 * The variable to ensure that a call of SigScm_GC_ProtectStack() is
 * uninlined in portable way through (*f)().
 *
 * Don't access this variables directly. Use SCM_GC_PROTECTED_CALL*() instead.
 */
extern ScmObj *(*volatile scm_gc_protect_stack)(ScmObj *);
#endif /* SCM_GCC4_READY_GC */

/*=======================================
   Function Declarations
=======================================*/
/*===========================================================================
   SigScheme : Core Functions
===========================================================================*/
/* sigscheme.c */
void SigScm_Initialize(void);
void SigScm_Finalize(void);
void Scm_DefineAlias(const char *newsym, const char *sym);
void Scm_Provide(ScmObj feature);
int  Scm_Providedp(ScmObj feature);
int  Scm_Use(const char *feature);
ScmObj ScmExp_use(ScmObj feature, ScmObj env);
ScmObj Scm_eval_c_string(const char *exp);
#if SCM_COMPAT_SIOD
ScmObj Scm_return_value(void);
#endif

/* Procedure/Syntax Registration */
void Scm_RegisterReductionOperator(const char *name, ScmObj (*func)(ScmObj, ScmObj, enum ScmReductionState*));
void Scm_RegisterSyntaxFixed0(const char *name, ScmObj (*func)(ScmObj));
#if SCM_FUNCTYPE_MAND_MAX >= 1
void Scm_RegisterSyntaxFixed1(const char *name, ScmObj (*func)(ScmObj, ScmObj));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 2
void Scm_RegisterSyntaxFixed2(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 3
void Scm_RegisterSyntaxFixed3(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj, ScmObj));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 4
void Scm_RegisterSyntaxFixed4(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj, ScmObj, ScmObj));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 5
void Scm_RegisterSyntaxFixed5(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj, ScmObj, ScmObj, ScmObj));
#endif
void Scm_RegisterSyntaxFixedTailRec0(const char *name, ScmObj (*func)(ScmEvalState*));
#if SCM_FUNCTYPE_MAND_MAX >= 1
void Scm_RegisterSyntaxFixedTailRec1(const char *name, ScmObj (*func)(ScmObj, ScmEvalState*));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 2
void Scm_RegisterSyntaxFixedTailRec2(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmEvalState*));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 3
void Scm_RegisterSyntaxFixedTailRec3(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj, ScmEvalState*));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 4
void Scm_RegisterSyntaxFixedTailRec4(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj, ScmObj, ScmEvalState*));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 5
void Scm_RegisterSyntaxFixedTailRec5(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj, ScmObj, ScmObj, ScmEvalState*));
#endif
void Scm_RegisterSyntaxVariadic0(const char *name, ScmObj (*func)(ScmObj, ScmObj));
#if SCM_FUNCTYPE_MAND_MAX >= 1
void Scm_RegisterSyntaxVariadic1(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 2
void Scm_RegisterSyntaxVariadic2(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj, ScmObj));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 3
void Scm_RegisterSyntaxVariadic3(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj, ScmObj, ScmObj));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 4
void Scm_RegisterSyntaxVariadic4(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj, ScmObj, ScmObj, ScmObj));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 5
void Scm_RegisterSyntaxVariadic5(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj, ScmObj, ScmObj, ScmObj, ScmObj));
#endif
void Scm_RegisterSyntaxVariadicTailRec0(const char *name, ScmObj (*func)(ScmObj, ScmEvalState*));
#if SCM_FUNCTYPE_MAND_MAX >= 1
void Scm_RegisterSyntaxVariadicTailRec1(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmEvalState*));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 2
void Scm_RegisterSyntaxVariadicTailRec2(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj, ScmEvalState*));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 3
void Scm_RegisterSyntaxVariadicTailRec3(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj, ScmObj, ScmEvalState*));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 4
void Scm_RegisterSyntaxVariadicTailRec4(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj, ScmObj, ScmObj, ScmEvalState*));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 5
void Scm_RegisterSyntaxVariadicTailRec5(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj, ScmObj, ScmObj, ScmObj, ScmEvalState*));
#endif
void Scm_RegisterProcedureFixed0(const char *name, ScmObj (*func)());
#if SCM_FUNCTYPE_MAND_MAX >= 1
void Scm_RegisterProcedureFixed1(const char *name, ScmObj (*func)(ScmObj));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 2
void Scm_RegisterProcedureFixed2(const char *name, ScmObj (*func)(ScmObj, ScmObj));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 3
void Scm_RegisterProcedureFixed3(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 4
void Scm_RegisterProcedureFixed4(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj, ScmObj));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 5
void Scm_RegisterProcedureFixed5(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj, ScmObj, ScmObj));
#endif
void Scm_RegisterProcedureFixedTailRec0(const char *name, ScmObj (*func)(ScmEvalState*));
#if SCM_FUNCTYPE_MAND_MAX >= 1
void Scm_RegisterProcedureFixedTailRec1(const char *name, ScmObj (*func)(ScmObj, ScmEvalState*));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 2
void Scm_RegisterProcedureFixedTailRec2(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmEvalState*));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 3
void Scm_RegisterProcedureFixedTailRec3(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj, ScmEvalState*));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 4
void Scm_RegisterProcedureFixedTailRec4(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj, ScmObj, ScmEvalState*));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 5
void Scm_RegisterProcedureFixedTailRec5(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj, ScmObj, ScmObj, ScmEvalState*));
#endif
void Scm_RegisterProcedureVariadic0(const char *name, ScmObj (*func)(ScmObj));
#if SCM_FUNCTYPE_MAND_MAX >= 1
void Scm_RegisterProcedureVariadic1(const char *name, ScmObj (*func)(ScmObj, ScmObj));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 2
void Scm_RegisterProcedureVariadic2(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 3
void Scm_RegisterProcedureVariadic3(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj, ScmObj));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 4
void Scm_RegisterProcedureVariadic4(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj, ScmObj, ScmObj));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 5
void Scm_RegisterProcedureVariadic5(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj, ScmObj, ScmObj, ScmObj));
#endif
void Scm_RegisterProcedureVariadicTailRec0(const char *name, ScmObj (*func)(ScmObj, ScmEvalState*));
#if SCM_FUNCTYPE_MAND_MAX >= 1
void Scm_RegisterProcedureVariadicTailRec1(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmEvalState*));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 2
void Scm_RegisterProcedureVariadicTailRec2(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj, ScmEvalState*));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 3
void Scm_RegisterProcedureVariadicTailRec3(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj, ScmObj, ScmEvalState*));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 4
void Scm_RegisterProcedureVariadicTailRec4(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj, ScmObj, ScmObj, ScmEvalState*));
#endif
#if SCM_FUNCTYPE_MAND_MAX >= 5
void Scm_RegisterProcedureVariadicTailRec5(const char *name, ScmObj (*func)(ScmObj, ScmObj, ScmObj, ScmObj, ScmObj, ScmObj, ScmEvalState*));
#endif

/* storage.c */
ScmObj Scm_NewCons(ScmObj a, ScmObj b);
ScmObj Scm_NewInt(int val);
ScmObj Scm_NewSymbol(char *name, ScmObj v_cell);
ScmObj Scm_NewChar(int val);
ScmObj Scm_NewString(char *str, int is_immutable);
ScmObj Scm_NewImmutableString(char *str);
ScmObj Scm_NewImmutableStringCopying(const char *str);
ScmObj Scm_NewMutableString(char *str);
ScmObj Scm_NewMutableStringCopying(const char *str);
ScmObj Scm_NewFunc(enum ScmFuncTypeCode type, ScmFuncType func);
ScmObj Scm_NewClosure(ScmObj exp, ScmObj env);
ScmObj Scm_NewVector(ScmObj *vec, int len);
ScmObj Scm_NewPort(ScmCharPort *cport, enum ScmPortFlag flag);
ScmObj Scm_NewContinuation(void);
#if !SCM_USE_VALUECONS
ScmObj Scm_NewValuePacket(ScmObj values);
#endif
#if SCM_USE_NONSTD_FEATURES
ScmObj Scm_NewCPointer(void *data);
ScmObj Scm_NewCFuncPointer(ScmCFunc func);
#endif

/* storage-gc.c */
void SigScm_GC_Protect(ScmObj *var);
void SigScm_GC_Unprotect(ScmObj *var);
#if SCM_GCC4_READY_GC
/*
 * Ordinary programs should not call these functions directly. Use
 * SCM_GC_PROTECTED_CALL*() instead.
 */
#ifdef __GNUC__
#define SigScm_GC_ProtectStack SigScm_GC_ProtectStackInternal
#else /* __GNUC__ */
#define SigScm_GC_ProtectStack (*scm_gc_protect_stack)
#endif /* __GNUC__ */

ScmObj *SigScm_GC_ProtectStackInternal(ScmObj *designated_stack_start) SCM_NOINLINE;
#else /* SCM_GCC4_READY_GC */
void   SigScm_GC_ProtectStack(ScmObj *stack_start);
#endif /* SCM_GCC4_READY_GC */
void   SigScm_GC_UnprotectStack(ScmObj *stack_start);    

/* storage-symbol.c */
ScmObj Scm_Intern(const char *name);
ScmObj Scm_SymbolBoundTo(ScmObj obj);

/* eval.c */
ScmObj ScmOp_eval(ScmObj obj, ScmObj env);
ScmObj ScmOp_apply(ScmObj proc, ScmObj arg0, ScmObj rest, ScmEvalState *eval_state);
ScmObj ScmExp_quote(ScmObj datum, ScmObj env);
ScmObj ScmExp_lambda(ScmObj formals, ScmObj body, ScmObj env);
ScmObj ScmExp_if(ScmObj test, ScmObj conseq, ScmObj rest, ScmEvalState *eval_state);
ScmObj ScmExp_setd(ScmObj var, ScmObj val, ScmObj env);
ScmObj ScmExp_cond(ScmObj args, ScmEvalState *eval_state);
ScmObj ScmExp_case(ScmObj key, ScmObj args, ScmEvalState *eval_state);
ScmObj ScmExp_and(ScmObj args, ScmEvalState *eval_state);
ScmObj ScmExp_or(ScmObj args, ScmEvalState *eval_state);
ScmObj ScmExp_let(ScmObj args, ScmEvalState *eval_state);
ScmObj ScmExp_letstar(ScmObj bindings, ScmObj body, ScmEvalState *eval_state);
ScmObj ScmExp_letrec(ScmObj bindings, ScmObj body, ScmEvalState *eval_state);
ScmObj ScmExp_begin(ScmObj args, ScmEvalState *eval_state);
ScmObj ScmExp_do(ScmObj bindings, ScmObj testframe, ScmObj commands, ScmEvalState *eval_state);
ScmObj ScmExp_delay(ScmObj expr, ScmObj env);
ScmObj ScmExp_quasiquote(ScmObj datum, ScmObj env);
ScmObj ScmExp_unquote(ScmObj dummy, ScmObj env);
ScmObj ScmExp_unquote_splicing(ScmObj dummy, ScmObj env);
ScmObj ScmExp_define(ScmObj var, ScmObj rest, ScmObj env);
ScmObj ScmOp_scheme_report_environment(ScmObj version);
ScmObj ScmOp_null_environment(ScmObj version);
ScmObj ScmOp_interaction_environment(void);

ScmObj Scm_call(ScmObj proc, ScmObj args);

/* operations.c */
ScmObj ScmOp_eqvp(ScmObj obj1, ScmObj obj2);
ScmObj ScmOp_eqp(ScmObj obj1, ScmObj obj2);
ScmObj ScmOp_equalp(ScmObj obj1, ScmObj obj2);
ScmObj ScmOp_add(ScmObj left, ScmObj right, enum ScmReductionState *state);
ScmObj ScmOp_subtract(ScmObj left, ScmObj right, enum ScmReductionState *state);
ScmObj ScmOp_multiply(ScmObj left, ScmObj right, enum ScmReductionState *state);
ScmObj ScmOp_divide(ScmObj left, ScmObj right, enum ScmReductionState *state);
ScmObj ScmOp_equal(ScmObj left, ScmObj right, enum ScmReductionState *state);
ScmObj ScmOp_less(ScmObj left, ScmObj right, enum ScmReductionState *state);
ScmObj ScmOp_less_eq(ScmObj left, ScmObj right, enum ScmReductionState *state);
ScmObj ScmOp_greater(ScmObj left, ScmObj right, enum ScmReductionState *state);
ScmObj ScmOp_greater_eq(ScmObj left, ScmObj right, enum ScmReductionState *state);
ScmObj ScmOp_numberp(ScmObj obj);
ScmObj ScmOp_zerop(ScmObj scm_num);
ScmObj ScmOp_positivep(ScmObj scm_num);
ScmObj ScmOp_negativep(ScmObj scm_num);
ScmObj ScmOp_oddp(ScmObj scm_num);
ScmObj ScmOp_evenp(ScmObj scm_num);
ScmObj ScmOp_max(ScmObj left, ScmObj right, enum ScmReductionState *state);
ScmObj ScmOp_min(ScmObj left, ScmObj right, enum ScmReductionState *state);
ScmObj ScmOp_abs(ScmObj scm_num);
ScmObj ScmOp_quotient(ScmObj scm_n1, ScmObj scm_n2);
ScmObj ScmOp_modulo(ScmObj scm_n1, ScmObj scm_n2);
ScmObj ScmOp_remainder(ScmObj scm_n1, ScmObj scm_n2);
ScmObj ScmOp_number2string (ScmObj num, ScmObj args);
ScmObj ScmOp_string2number(ScmObj str, ScmObj args);
ScmObj ScmOp_not(ScmObj obj);
ScmObj ScmOp_booleanp(ScmObj obj);
ScmObj ScmOp_car(ScmObj obj);
ScmObj ScmOp_cdr(ScmObj obj);
ScmObj ScmOp_pairp(ScmObj obj);
ScmObj ScmOp_cons(ScmObj car, ScmObj cdr);
ScmObj ScmOp_set_card(ScmObj pair, ScmObj car);
ScmObj ScmOp_set_cdrd(ScmObj pair, ScmObj cdr);
ScmObj ScmOp_caar(ScmObj lst);
ScmObj ScmOp_cadr(ScmObj lst);
ScmObj ScmOp_cdar(ScmObj lst);
ScmObj ScmOp_cddr(ScmObj lst);
ScmObj ScmOp_caddr(ScmObj lst);
ScmObj ScmOp_cdddr(ScmObj lst);
ScmObj ScmOp_list(ScmObj args);
ScmObj ScmOp_nullp(ScmObj obj);
ScmObj ScmOp_listp(ScmObj obj);
ScmObj ScmOp_length(ScmObj obj);
ScmObj ScmOp_append(ScmObj args);
ScmObj ScmOp_reverse(ScmObj lst);
ScmObj ScmOp_list_tail(ScmObj lst, ScmObj scm_k);
ScmObj ScmOp_list_ref(ScmObj lst, ScmObj scm_k);
ScmObj ScmOp_memq(ScmObj obj, ScmObj lst);
ScmObj ScmOp_memv(ScmObj obj, ScmObj lst);
ScmObj ScmOp_member(ScmObj obj, ScmObj lst);
ScmObj ScmOp_assq(ScmObj obj, ScmObj alist);
ScmObj ScmOp_assv(ScmObj obj, ScmObj alist);
ScmObj ScmOp_assoc(ScmObj obj, ScmObj alist);
ScmObj ScmOp_symbolp(ScmObj obj);
ScmObj ScmOp_symbol2string(ScmObj obj);
ScmObj ScmOp_string2symbol(ScmObj str);

ScmObj ScmOp_charp(ScmObj obj);
ScmObj ScmOp_charequalp(ScmObj ch1, ScmObj ch2);
/* TODO : many comparing functions around char is unimplemented */
ScmObj ScmOp_char_alphabeticp(ScmObj obj);
ScmObj ScmOp_char_numericp(ScmObj obj);
ScmObj ScmOp_char_whitespacep(ScmObj obj);
ScmObj ScmOp_char_upper_casep(ScmObj obj);
ScmObj ScmOp_char_lower_casep(ScmObj obj);
ScmObj ScmOp_char2integer(ScmObj obj);
ScmObj ScmOp_integer2char(ScmObj obj);
ScmObj ScmOp_char_upcase(ScmObj obj);
ScmObj ScmOp_char_downcase(ScmObj obj);

ScmObj ScmOp_stringp(ScmObj obj);
ScmObj ScmOp_make_string(ScmObj length, ScmObj args);
ScmObj ScmOp_string(ScmObj args);
ScmObj ScmOp_string_length(ScmObj str);
ScmObj ScmOp_string_ref(ScmObj str, ScmObj k);
ScmObj ScmOp_string_setd(ScmObj str, ScmObj k, ScmObj ch);
ScmObj ScmOp_stringequalp(ScmObj str1, ScmObj str2);
/* TODO : many comparing functions around string is unimplemented */
ScmObj ScmOp_substring(ScmObj str, ScmObj start, ScmObj end);
ScmObj ScmOp_string_append(ScmObj args);
ScmObj ScmOp_string2list(ScmObj str);
ScmObj ScmOp_list2string(ScmObj lst);
ScmObj ScmOp_string_copy(ScmObj str);
ScmObj ScmOp_string_filld(ScmObj str, ScmObj ch);
ScmObj ScmOp_vectorp(ScmObj obj);
ScmObj ScmOp_make_vector(ScmObj vector_len, ScmObj args);
ScmObj ScmOp_vector(ScmObj args);
ScmObj ScmOp_vector_length(ScmObj vec);
ScmObj ScmOp_vector_ref(ScmObj vec, ScmObj scm_k);
ScmObj ScmOp_vector_setd(ScmObj vec, ScmObj scm_k, ScmObj obj);
ScmObj ScmOp_vector2list(ScmObj vec);
ScmObj ScmOp_list2vector(ScmObj lst);
ScmObj ScmOp_vector_filld(ScmObj vec, ScmObj fill);
ScmObj ScmOp_procedurep(ScmObj obj);
ScmObj ScmOp_map(ScmObj proc, ScmObj args);
ScmObj ScmOp_for_each(ScmObj proc, ScmObj args);
ScmObj ScmOp_force(ScmObj closure);
ScmObj ScmOp_call_with_current_continuation(ScmObj proc, ScmEvalState *eval_state);
ScmObj ScmOp_values(ScmObj args);
ScmObj ScmOp_call_with_values(ScmObj producer, ScmObj consumer, ScmEvalState *eval_state);
ScmObj ScmOp_dynamic_wind(ScmObj before, ScmObj thunk, ScmObj after);

/* operations-r5rs-deepcadrs.c */
#if SCM_USE_DEEP_CADRS
ScmObj ScmOp_caaar(ScmObj lst);
ScmObj ScmOp_caadr(ScmObj lst);
ScmObj ScmOp_cadar(ScmObj lst);
ScmObj ScmOp_cdaar(ScmObj lst);
ScmObj ScmOp_cdadr(ScmObj lst);
ScmObj ScmOp_cddar(ScmObj lst);
ScmObj ScmOp_caaaar(ScmObj lst);
ScmObj ScmOp_caaadr(ScmObj lst);
ScmObj ScmOp_caadar(ScmObj lst);
ScmObj ScmOp_caaddr(ScmObj lst);
ScmObj ScmOp_cadaar(ScmObj lst);
ScmObj ScmOp_cadadr(ScmObj lst);
ScmObj ScmOp_caddar(ScmObj lst);
ScmObj ScmOp_cadddr(ScmObj lst);
ScmObj ScmOp_cdaaar(ScmObj lst);
ScmObj ScmOp_cdaadr(ScmObj lst);
ScmObj ScmOp_cdadar(ScmObj lst);
ScmObj ScmOp_cdaddr(ScmObj lst);
ScmObj ScmOp_cddaar(ScmObj lst);
ScmObj ScmOp_cddadr(ScmObj lst);
ScmObj ScmOp_cdddar(ScmObj lst);
ScmObj ScmOp_cddddr(ScmObj lst);
#endif /* SCM_USE_DEEP_CADRS */

/* operations-nonstd.c */
#if SCM_USE_NONSTD_FEATURES
void SigScm_Initialize_NONSTD_FEATURES(void);
ScmObj ScmOp_symbol_boundp(ScmObj sym, ScmObj rest);
ScmObj ScmOp_load_path(void);
/* FIXME: add ScmObj SigScm_require(const char *c_filename); */
ScmObj ScmOp_require(ScmObj filename);
ScmObj ScmOp_provide(ScmObj feature);
ScmObj ScmOp_providedp(ScmObj feature);
ScmObj ScmOp_file_existsp(ScmObj filepath);
ScmObj ScmOp_delete_file(ScmObj filepath);
#endif

/* io.c */
void   SigScm_set_lib_path(const char *path);
ScmObj Scm_MakeSharedFilePort(FILE *file, const char *aux_info,
                              enum ScmPortFlag flag);
void SigScm_PortPrintf(ScmObj port, const char *fmt, ...);
void SigScm_VPortPrintf(ScmObj port, const char *fmt, va_list args);
void SigScm_PortNewline(ScmObj port);
void SigScm_ErrorPrintf(const char *fmt, ...);
void SigScm_VErrorPrintf(const char *fmt, va_list args);
void SigScm_ErrorNewline(void);

ScmObj ScmOp_call_with_input_file(ScmObj filepath, ScmObj proc);
ScmObj ScmOp_call_with_output_file(ScmObj filepath, ScmObj proc);
ScmObj ScmOp_input_portp(ScmObj obj);
ScmObj ScmOp_output_portp(ScmObj obj);
ScmObj ScmOp_current_input_port(void);
ScmObj ScmOp_current_output_port(void);
ScmObj ScmOp_with_input_from_file(ScmObj filepath, ScmObj thunk);
ScmObj ScmOp_with_output_to_file(ScmObj filepath, ScmObj thunk);
ScmObj ScmOp_open_input_file(ScmObj filepath);
ScmObj ScmOp_open_output_file(ScmObj filepath);
ScmObj ScmOp_close_input_port(ScmObj port);
ScmObj ScmOp_close_output_port(ScmObj port);

ScmObj ScmOp_read(ScmObj args);
ScmObj ScmOp_read_char(ScmObj args);
ScmObj ScmOp_peek_char(ScmObj args);
ScmObj ScmOp_eof_objectp(ScmObj obj);
ScmObj ScmOp_char_readyp(ScmObj args);
ScmObj ScmOp_write(ScmObj obj, ScmObj args);
ScmObj ScmOp_display(ScmObj obj, ScmObj args);
ScmObj ScmOp_newline(ScmObj args);
ScmObj ScmOp_write_char(ScmObj obj, ScmObj args);

ScmObj SigScm_load(const char *c_filename);
ScmObj ScmOp_load(ScmObj filename);

/* read.c */
ScmObj SigScm_Read(ScmObj port);
ScmObj SigScm_Read_Char(ScmObj port);

/* error.c */
int  SigScm_Die(const char *msg, const char *filename, int line);
void SigScm_Error(const char *msg, ...) SCM_NORETURN;
void SigScm_ErrorObj(const char *msg, ScmObj obj) SCM_NORETURN;
void SigScm_ShowBacktrace(ScmObj trace_stack);
ScmObj Scm_MakeErrorObj(ScmObj reason, ScmObj objs);
void Scm_RaiseError(ScmObj err_obj) SCM_NORETURN;
void Scm_FatalError(const char *msg) SCM_NORETURN;
void Scm_SetFatalErrorCallback(void (*cb)(void));
ScmObj ScmOp_error_objectp(ScmObj obj);
ScmObj ScmOp_fatal_error(ScmObj err_obj) SCM_NORETURN;
ScmObj ScmOp_inspect_error(ScmObj err_obj);
ScmObj ScmOp_backtrace(void);

/* debug.c */
int  SigScm_DebugCategories(void);
void SigScm_SetDebugCategories(int categories);
int  SigScm_PredefinedDebugCategories(void);
void SigScm_CategorizedDebug(int category, const char *msg, ...);
void SigScm_Debug(const char *msg, ...);
void SigScm_Display(ScmObj obj);
void SigScm_WriteToPort(ScmObj port, ScmObj obj);
void SigScm_DisplayToPort(ScmObj port, ScmObj obj);
#if SCM_USE_SRFI38
void SigScm_WriteToPortWithSharedStructure(ScmObj port, ScmObj obj);
#endif


/*===========================================================================
   SigScheme : Optional Funtions
===========================================================================*/
#if SCM_USE_SRFI1
/* operations-srfi1.c */
void   SigScm_Initialize_SRFI1(void);
ScmObj ScmOp_SRFI1_xcons(ScmObj a, ScmObj b);
ScmObj ScmOp_SRFI1_consstar(ScmObj args);
ScmObj ScmOp_SRFI1_make_list(ScmObj length, ScmObj args);
ScmObj ScmOp_SRFI1_list_tabulate(ScmObj scm_n, ScmObj args);
ScmObj ScmOp_SRFI1_list_copy(ScmObj lst);
ScmObj ScmOp_SRFI1_circular_list(ScmObj args);
ScmObj ScmOp_SRFI1_iota(ScmObj scm_count, ScmObj args);
ScmObj ScmOp_SRFI1_proper_listp(ScmObj lst);
ScmObj ScmOp_SRFI1_circular_listp(ScmObj lst);
ScmObj ScmOp_SRFI1_dotted_listp(ScmObj lst);
ScmObj ScmOp_SRFI1_not_pairp(ScmObj pair);
ScmObj ScmOp_SRFI1_null_listp(ScmObj lst);
ScmObj ScmOp_SRFI1_listequal(ScmObj eqproc, ScmObj args);
ScmObj ScmOp_SRFI1_first(ScmObj lst);
ScmObj ScmOp_SRFI1_second(ScmObj lst);
ScmObj ScmOp_SRFI1_third(ScmObj lst);
ScmObj ScmOp_SRFI1_fourth(ScmObj lst);
ScmObj ScmOp_SRFI1_fifth(ScmObj lst);
ScmObj ScmOp_SRFI1_sixth(ScmObj lst);
ScmObj ScmOp_SRFI1_seventh(ScmObj lst);
ScmObj ScmOp_SRFI1_eighth(ScmObj lst);
ScmObj ScmOp_SRFI1_ninth(ScmObj lst);
ScmObj ScmOp_SRFI1_tenth(ScmObj lst);
ScmObj ScmOp_SRFI1_carpluscdr(ScmObj lst);
ScmObj ScmOp_SRFI1_take(ScmObj lst, ScmObj scm_idx);
ScmObj ScmOp_SRFI1_drop(ScmObj lst, ScmObj scm_idx);
ScmObj ScmOp_SRFI1_take_right(ScmObj lst, ScmObj scm_elem);
ScmObj ScmOp_SRFI1_drop_right(ScmObj lst, ScmObj scm_elem);
ScmObj ScmOp_SRFI1_taked(ScmObj lst, ScmObj scm_idx);
ScmObj ScmOp_SRFI1_drop_rightd(ScmObj lst, ScmObj scm_idx);
ScmObj ScmOp_SRFI1_split_at(ScmObj lst, ScmObj idx);
ScmObj ScmOp_SRFI1_split_atd(ScmObj lst, ScmObj idx);
ScmObj ScmOp_SRFI1_last(ScmObj lst);
ScmObj ScmOp_SRFI1_last_pair(ScmObj lst);
ScmObj ScmOp_SRFI1_lengthplus(ScmObj lst);
ScmObj ScmOp_SRFI1_concatenate(ScmObj args);
#endif
#if SCM_USE_SRFI2
/* operations-srfi2.c */
void   SigScm_Initialize_SRFI2(void);
ScmObj ScmExp_SRFI2_and_letstar(ScmObj claws, ScmObj body, ScmEvalState *eval_state);
#endif
#if SCM_USE_SRFI6
/* operations-srfi6.c */
void   SigScm_Initialize_SRFI6(void);
ScmObj ScmOp_SRFI6_open_input_string(ScmObj str);
ScmObj ScmOp_SRFI6_open_output_string(void);
ScmObj ScmOp_SRFI6_get_output_string(ScmObj port);
#endif
#if SCM_USE_SRFI8
/* operations-srfi8.c */
void   SigScm_Initialize_SRFI8(void);
ScmObj ScmExp_SRFI8_receive(ScmObj formals, ScmObj expr, ScmObj body, ScmEvalState *eval_state);
#endif
#if SCM_USE_SRFI23
/* operations-srfi23.c */
void   SigScm_Initialize_SRFI23(void);
ScmObj ScmOp_SRFI23_error(ScmObj reason, ScmObj args);
#endif
#if SCM_USE_SRFI34
/* operations-srfi34.c */
void  SigScm_Initialize_SRFI34(void);
ScmObj ScmOp_SRFI34_with_exception_handler(ScmObj handler, ScmObj thunk);
ScmObj ScmExp_SRFI34_guard(ScmObj cond_catch, ScmObj body,
                           ScmEvalState *eval_state);
ScmObj ScmOp_SRFI34_raise(ScmObj obj);
#endif
#if SCM_USE_SRFI38
/* operations-srfi38.c */
void   SigScm_Initialize_SRFI38(void);
ScmObj ScmOp_SRFI38_write_with_shared_structure(ScmObj obj, ScmObj args);
#endif
#if SCM_USE_SRFI60
/* operations-srfi60.c */
void   SigScm_Initialize_SRFI60(void);
ScmObj ScmOp_SRFI60_logand(ScmObj left, ScmObj right,
                           enum ScmReductionState *state);
ScmObj ScmOp_SRFI60_logior(ScmObj left, ScmObj right,
                           enum ScmReductionState *state);
ScmObj ScmOp_SRFI60_logxor(ScmObj left, ScmObj right,
                           enum ScmReductionState *state);
ScmObj ScmOp_SRFI60_lognot(ScmObj n);
ScmObj ScmOp_SRFI60_bitwise_if(ScmObj mask, ScmObj n0, ScmObj n1);
ScmObj ScmOp_SRFI60_logtest(ScmObj j, ScmObj k);
#endif
#if SCM_COMPAT_SIOD
/* operations-siod.c */
void   SigScm_Initialize_SIOD(void);
ScmObj ScmOp_symbol_value(ScmObj var);
ScmObj ScmOp_set_symbol_valued(ScmObj var, ScmObj val);
ScmObj ScmOp_SIOD_equal(ScmObj obj1, ScmObj obj2);
ScmObj ScmOp_the_environment(ScmEvalState *eval_state);
ScmObj ScmOp_closure_code(ScmObj closure);
ScmObj ScmOp_verbose(ScmObj args);
ScmObj ScmOp_eof_val(void);
ScmObj ScmExp_undefine(ScmObj var, ScmObj env);
long   SigScm_GetVerboseLevel(void);
void   SigScm_SetVerboseLevel(long level);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SIGSCHEME_H */
