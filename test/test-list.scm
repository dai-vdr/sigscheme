;;  FileName : test-list.scm
;;  About    : unit test for list operations
;;
;;  Copyright (C) 2005-2006 Kazuki Ohta <mover AT hct.zaq.ne.jp>
;;
;;  All rights reserved.
;;
;;  Redistribution and use in source and binary forms, with or without
;;  modification, are permitted provided that the following conditions
;;  are met:
;;
;;  1. Redistributions of source code must retain the above copyright
;;     notice, this list of conditions and the following disclaimer.
;;  2. Redistributions in binary form must reproduce the above copyright
;;     notice, this list of conditions and the following disclaimer in the
;;     documentation and/or other materials provided with the distribution.
;;  3. Neither the name of authors nor the names of its contributors
;;     may be used to endorse or promote products derived from this software
;;     without specific prior written permission.
;;
;;  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS
;;  IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
;;  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
;;  PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
;;  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
;;  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
;;  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
;;  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
;;  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
;;  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
;;  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

(load "test/unittest.scm")


(define elm0 (lambda () #f))
(define elm1 (lambda () #f))
(define elm2 (lambda () #f))
(define elm3 (lambda () #f))
(define nil  ())
(define cdr3 (cons elm3 nil))
(define cdr2 (cons elm2 cdr3))
(define cdr1 (cons elm1 cdr2))
(define cdr0 (cons elm0 cdr1))
(define lst cdr0)


; pair?
(assert-true "pair? test1" (pair? '(a . b)))
(assert-true "pair? test2" (pair? '(a b c)))
(assert-equal? "pair? test3" #f (pair? '()))
(assert-equal? "pair? test4" #f (pair? '#(a b)))

; cons
(assert-equal? "cons test1" '(a) (cons 'a '()))
(assert-equal? "cons test2" '((a) b c d) (cons '(a) '(b c d)))
(assert-equal? "cons test3" '(a . 3) (cons 'a 3))
(assert-equal? "cons test4" '((a b) . c) (cons '(a b) 'c))

; car
(assert-equal? "car test1" 'a (car '(a b c)))
(assert-equal? "car test2" '(a) (car '((a) b c)))
(assert-equal? "car test3" 1 (car '(1 . 2)))

; cdr
(assert-equal? "cdr test1" '(b c d) (cdr '((a) b c d)))
(assert-equal? "cdr test2" 2 (cdr '(1 . 2)))

; null?
(assert-true "null? test1" (null? '()))
(assert-equal? "null? test2" #f (null? "aiueo"))

; list?
(assert-true "list? test1" (list? '(a b c)))
(assert-true "list? test2" (list? '()))
(assert-false "list? test3" (list? '(a . b)))
(assert-false "list? test4" (list? '(a b . c)))
(assert-false "list? test5" (let ((x (list 'a)))
			      (set-cdr! x x)
			      (list? x)))

; list
(assert-equal? "list test1" '(a 7 c) (list 'a (+ 3 4) 'c))
(assert-equal? "list test2" '() (list))

; length
(assert-equal? "length test1" 3 (length '(a b c)))
(assert-equal? "length test2" 3 (length '(a (b) (c d e))))
(assert-equal? "length test2" 0 (length '()))

; append
(assert-equal? "append test1" '(x y) (append '(x) '(y)))
(assert-equal? "append test2" '(a b c d) (append '(a) '(b c d)))
(assert-equal? "append test3" '(a (b) (c)) (append '(a (b)) '((c))))
(define w '(n o))
(define x '(d o))
(define y '(car))
(define z '(why))
(assert-equal? "append test4" '(n o d o car why . ta) (append w x y () z 'ta))
(assert-equal? "append test5" '(n o) w)	; test non-destructiveness
(assert-eq? "append test6" x (cdr (append '((Calpis hosi-)) x))) ; share last

; reverse
(assert-equal? "reverse test1" '(c b a) (reverse '(a b c)))
(assert-equal? "reverse test2" '((e (f)) d (b c) a) (reverse '(a (b c) d (e (f)))))

; list-tail
(assert-equal? "list-tail test1" '(a b c) (list-tail '(a b c) 0))
(assert-equal? "list-tail test2" '(b c) (list-tail '(a b c) 1))
(assert-equal? "list-tail test3" '(c) (list-tail '(a b c) 2))
(assert-equal? "list-tail test4" '() (list-tail '(a b c) 3))
;;(assert-error  "list-tail test5" '() (list-tail '(a b c) 4))
(assert-equal? "list-tail test6" '() (list-tail '() 0))
;;(assert-error  "list-tail test7" (list-tail '() 1))
(assert-eq?    "list-tail test8"  cdr0 (list-tail lst 0))
(assert-eq?    "list-tail test9"  cdr1 (list-tail lst 1))
(assert-eq?    "list-tail test10" cdr2 (list-tail lst 2))
(assert-eq?    "list-tail test11" cdr3 (list-tail lst 3))
(assert-eq?    "list-tail test12" nil  (list-tail lst 4))
;;(assert-error  "list-tail test13" (list-tail lst 5))

; list-ref
(assert-equal? "list-ref test1" 'c (list-ref '(a b c d) 2))
(assert-eq?    "list-ref test2" elm0 (list-ref lst 0))
(assert-eq?    "list-ref test3" elm1 (list-ref lst 1))
(assert-eq?    "list-ref test4" elm2 (list-ref lst 2))
(assert-eq?    "list-ref test5" elm3 (list-ref lst 3))
;;(assert-error  "list-ref test6" (list-ref lst 4))

; memq
(assert-equal? "memq test1" '(a b c) (memq 'a '(a b c)))
(assert-equal? "memq test2" '(b c) (memq 'b '(a b c)))
(assert-equal? "memq test3" #f (memq 'a '(b c d)))
(assert-equal? "memq test4" #f (memq (list 'a) '(b (a) c)))

; member
(assert-equal? "member test1" '((a) c) (member (list 'a) '(b (a) c)))

; assq
(define e '((a 1) (b 2) (c 3)))
(assert-equal? "assq test1" '(a 1) (assq 'a e))
(assert-equal? "assq test2" '(b 2) (assq 'b e))
(assert-equal? "assq test3" #f (assq 'd e))
(assert-equal? "assq test4" #f (assq (list 'a) '(((a)) ((b)) ((c)))))

; assoc
(assert-equal? "assoc test1" '((a)) (assoc (list 'a) '(((a)) ((b)) ((c)))))

(total-report)
