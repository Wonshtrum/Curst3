#include <stdlib.h>
#include <stddef.h>
#include <inttypes.h>
#include <stdio.h>
#include "do.h"
#include "defer.h"


//================================================
// general helpers
//================================================
// TODO: flesh out
//#define _panic() do { fflush(stdout); fprintf(stderr, "\n%s:%d: panicked!\n", __FILE__, __LINE__); extern void exit(int); exit(-1); } while(0)
void _panic() { fflush(stdout); fprintf(stderr, "\npanicked!\n"); extern void exit(int); exit(-1); }

#define _let(T, v) _t(T) v; _defer(_call(T, drop, &v)); v = (typeof(v))

#define _strip(...) __VA_ARGS__
#define _m(T) __m T
#define _t(T) __t T
#define __m(M, T, V) M
#define __t(M, T, V) T

#define _mangle_NCT(N, Cs, Ts) CAT(N DO(_mangle_C, Cs) DO(_mangle_T, Ts))
#define _mangle_T(i, T) ,__m T
#define _mangle_C(i, C) ,_##C

#define _bind(T, V, ...) _bind2(_trait_##V(T), (__VA_ARGS__))
#define _bind2(A, B) _bind3(_strip A, B)
#define _bind3(...) _bind4(__VA_ARGS__)
#define _bind4(ret, args, exprs) DO(_bind_tmp, exprs) DO(_bind_set, args)
#define _bind_tmp(i, expr) auto __tmp_##i = expr;
#define _bind_set(i, arg) arg = __tmp_##i;

#define _fn(T, ...) __fn(T, __COUNTER__, __VA_ARGS__)
#define __fn(...) ___fn(__VA_ARGS__)
#define ___fn(T, C, ...) { T _ret_val; void *_ret_addr = &&_ret_label_##C; __VA_ARGS__; _ret_label_##C: _ret_val; }
#define _ret(V) do { _ret_val = V; goto *_ret_addr; } while(0)

#define _cmp_true ,
#define _cmp_false

#define _expect_derive_derive(...) _NARGS(__VA_ARGS__), (__VA_ARGS__)

//================================================
// trait definition
//================================================
#define _trait_debug(T) (void, (T *t))
#define _cmp_debug_debug _cmp_true

#define _trait_eq(T) (bool, (T *t, T *o))
#define _cmp_eq_eq _cmp_true

#define _trait_clone(T) (T, (T *t))
#define _cmp_clone_clone _cmp_true

#define _trait_drop(T) (void, (T *t))
#define _cmp_drop_drop _cmp_true

#define _trait_display(T) (Res(usize, _unit), (T *t, Formatter *f))
#define _cmp_display_display _cmp_true

//================================================
// compile time trait dispatch
//================================================
#define _call(T, V, ...) __call(_strip T, V, __VA_ARGS__)
#define __call(...) ___call(__VA_ARGS__)
#define ___call(M, T, Vs, V, ...) _or_impl(_bind(T, V, __VA_ARGS__), (DO_WITH(_find_trait, V, Vs)), V##_##M(__VA_ARGS__))
#define _find_trait(i, Vt, Ve) __find_trait(_cmp_##Vt##_##Ve, )
#define __find_trait(...) ___find_trait(__VA_ARGS__)
#define ___find_trait(_0, _1, ...) _1
#define _or_impl(B, D, I) __or_impl(B, _NARGS D, D, I)
#define __or_impl(...) ___or_impl(__VA_ARGS__)
#define ___or_impl(B, N, D, I) _or_impl##N(B, D, I)
#define _or_impl_00(B, D, I) I
#define _or_impl_01(B, D, I) ({ B _strip D; })

//================================================
// language types
//================================================
// primitives
typedef int8_t    _i8;
typedef int16_t   _i16;
typedef int32_t   _i32;
typedef int64_t   _i64;
typedef ptrdiff_t _isize;
typedef uint8_t   _u8;
typedef uint16_t  _u16;
typedef uint32_t  _u32;
typedef uint64_t  _u64;
typedef size_t    _usize;
typedef char      _bool;
typedef char      _char;
typedef float     _f32;
typedef double    _f64;

#define Number(M, T, F) (M, T, (\
		debug({printf(F, *t);}),\
		eq({*t == *o;}),\
		clone({*t;}),\
		drop({(void)t;})))

#define i8    Number(i8,  _i8,    "%d")
#define i16   Number(i16, _i16,   "%d")
#define i32   Number(i32, _i32,   "%"PRId32)
#define i64   Number(i64, _i64,   "%"PRId64)
#define isize Number(i64, _isize, "%zd")
#define u8    Number(u8,  _u8,    "%u")
#define u16   Number(u16, _u16,   "%u")
#define u32   Number(u32, _u32,   "%"PRIu32)
#define u64   Number(u64, _u64,   "%"PRIu64)
#define usize Number(i64, _usize, "%zu")
#define f32   Number(f32, _f32,   "%f")
#define f64   Number(f64, _f64,   "%f")

#define bool  Number(bool, _bool, *t ? "true" : "false")
#define char  Number(char, _char, "'%c'")

// anonymous product types (tuples)
#define _T(...) __T(CAT(_q DO(_mangle_T, (__VA_ARGS__)), p), _NARGS(__VA_ARGS__), __VA_ARGS__)
#define __T(...) ___T(__VA_ARGS__)
#define ___T(M, N, ...) (M, struct M { _DO##N(_tuple_field_def, __VA_ARGS__) }, (\
		debug({ printf("("); _DO##N(_tuple_field_debug, __VA_ARGS__) printf(")"); }),\
		eq({ true _DO##N(_tuple_field_eq, __VA_ARGS__); }),\
		clone({ struct M res; _DO##N(_tuple_field_clone, __VA_ARGS__) res; }),\
		drop({ _DO##N(_tuple_field_drop, __VA_ARGS__) }),\
	))
#define _tuple_field_def(i, F) __t F _##i;
#define _tuple_field_debug(i, F) if(i)printf(", "); _call(F, debug, &t->_##i);
#define _tuple_field_eq(i, F) && _call(F, eq, &t->_##i, &o->_##i)
#define _tuple_field_clone(i, F) res._##i = _call(F, clone, &t->_##i);
#define _tuple_field_drop(i, F) _call(F, drop, &t->_##i);

typedef _t(_T()) _unit;

// named product types (structs)
#define _struct(N, Cs, Ts, Ds, ...) __struct(_mangle_NCT(N, Cs, Ts), _expect_derive##Ds, _NARGS(__VA_ARGS__), __VA_ARGS__)
#define __struct(...) ___struct(__VA_ARGS__)
#define ___struct(M, ND, Ds, N, ...) (M, struct M { CAT_EACH##N(_struct_def, __VA_ARGS__) }, _struct_derive_expand(ND, (M, N, __VA_ARGS__), _strip Ds))
#define _struct_def
#define _struct_def_field(N, T) __t T N;
#define _struct_derive_expand(ND, W, ...) (_DO##ND##_WITH(_struct_derive, W, __VA_ARGS__))
#define _struct_derive(i, W, V) V({ _struct_derive_##V W }),

#define _struct_derive_debug(M, N, ...) printf(#M " { "); CAT_EACH##N(_struct_debug, __VA_ARGS__) printf("}");
#define _struct_debug_field(N, T) printf(#N ": "); _call(T, debug, &t->N); printf(", ");
#define _struct_debug

#define _struct_derive_clone(M, N, ...) struct M res; CAT_EACH##N(_struct_clone, __VA_ARGS__) res;
#define _struct_clone_field(N, T) res.N = _call(T, clone, &t->N);
#define _struct_clone

#define _struct_derive_eq(M, N, ...) true CAT_EACH##N(_struct_eq, __VA_ARGS__);
#define _struct_eq_field(N, T) && _call(T, eq, &t->N, &o->N)
#define _struct_eq

#define _struct_derive_drop(M, N, ...) CAT_EACH##N(_struct_drop, __VA_ARGS__)
#define _struct_drop_field(N, T) _call(T, drop, &t->N);
#define _struct_drop

// sum types (tagged enum)
#define _build(N, V, ...) __build(N, V, ##__VA_ARGS__, (_unit){})
#define __build(N, V, x, ...) { .__tag = N##_##V##_TAG, .V = x }

#define _enum(N, Cs, Ts, Ds, ...) __enum(N, _mangle_NCT(N, Cs, Ts), _expect_derive##Ds, _NARGS(__VA_ARGS__), __VA_ARGS__)
#define __enum(...) ___enum(__VA_ARGS__)
#define ___enum(N, M, ND, Ds, NF, ...) (M, struct M { typeof(enum N##_TAGS { _DO##NF##_WITH(_enum_tag, N, __VA_ARGS__) }) __tag; union { CAT_EACH##NF(_enum_def, __VA_ARGS__) }; }, _enum_derive_expand(ND, (N, M, NF, __VA_ARGS__), _strip Ds))
#define _enum_def
#define _enum_def_variant(N, T) __t T N;
#define _enum_tag(i, N, V) __enum_tag(N, _strip##V)
#define __enum_tag(...) ___enum_tag(__VA_ARGS__)
#define ___enum_tag(N, Vn, Vt) N##_##Vn##_TAG,
#define _enum_derive_expand(ND, W, ...) (_DO##ND##_WITH(_enum_derive, W, __VA_ARGS__))

#define _strip_variant(N, T) N, T
#define _enum_derive(i, W, V) V({ __enum_derive(V, _strip W) }),
#define __enum_derive(...) ___enum_derive(__VA_ARGS__)
#define ___enum_derive(V, N, M, NF, ...) _enum_derive_##V(N, M, __DO##NF##_WITH(_enum_derive_variant, (V, N), __VA_ARGS__))
#define _enum_derive_variant(i, W, V) __enum_derive_variant(_strip W, _strip##V)
#define __enum_derive_variant(...) ___enum_derive_variant(__VA_ARGS__)
#define ___enum_derive_variant(V, N, Vn, Vt) _enum_##V##_variant(N, Vn, Vt)

#define _enum_derive_debug(N, M, C) if (false); C else _panic();
#define _enum_debug_variant(N, Vn, Vt) else if (t->__tag == N##_##Vn##_TAG) { printf(#N "::" #Vn "("); _call(Vt, debug, &t->Vn); printf(")"); }

#define _enum_derive_eq(N, M, C) _bool __res; if (t->__tag != o->__tag) __res=false; C else _panic(); __res;
#define _enum_eq_variant(N, Vn, Vt) else if (t->__tag == N##_##Vn##_TAG) __res=_call(Vt, eq, &t->Vn, &o->Vn);

#define _enum_derive_clone(N, M, C) struct M __res = { .__tag = t->__tag }; if (false) C else _panic(); __res;
#define _enum_clone_variant(N, Vn, Vt) else if (t->__tag == N##_##Vn##_TAG) __res.Vn=_call(Vt, clone, &t->Vn);

#define _enum_derive_drop(N, M, C) if (false) C else _panic();
#define _enum_drop_variant(N, Vn, Vt) else if (t->__tag == N##_##Vn##_TAG) _call(Vt, drop, &t->Vn);

//================================================
// std types
//================================================
// options
#define Opt(T) _enum(Opt, (), (T),\
	_derive(debug, eq, clone, drop),\
	_variant(None, _T()),\
	_variant(Some, T))

// results
#define Res(T, E) _enum(Res, (), (T, E),\
	_derive(debug, eq, clone, drop),\
	_variant(Ok, T),\
	_variant(Err, E))

// boxes
#define Box(T) _Box T
#define _Box(M, T, Vs) (_b##M, struct _b##M { T *ptr; }, (\
		debug({ __call(M, T, Vs, debug, t->ptr); }),\
		eq({ __call(M, T, Vs, eq, t->ptr, o->ptr); }),\
		clone({ struct _b##M __res; __res.ptr = malloc(sizeof(T)); *__res.ptr = __call(M, T, Vs, clone, t->ptr); }),\
		drop({ __call(M, T, Vs, drop, t->ptr); printf("Box::drop()\n"); free(t->ptr); }),\
	))
#define Box_new(x) { .ptr = ({ typeof(x)* ptr = malloc(sizeof(x)); *ptr=x; ptr; }) }

// vectors
#define Vec(T) _Vec T
#define _Vec(M, T, Vs) (_v##M, struct _v##M { int len; int cap; T *ptr; }, (\
		debug({\
			printf("[");\
			for(int i=0;i<t->len;i++) {\
				if (i) printf(", ");\
				___call(M, T, Vs, debug, t->ptr+i);\
			}\
			printf("]");}),\
		eq(_fn(_bool,\
			if (t->len != o->len) _ret(false);\
			for(int i=0;i<t->len;i++) {\
				if (!___call(M, T, Vs, eq, t->ptr+i, o->ptr+i)) _ret(false);\
			}\
			_ret(true);)),\
		clone({\
			void *ptr = malloc(sizeof(T)*t->len);\
			struct _v##M res = { .len = t->len, .cap = t->len, .ptr = ptr };\
			for(int i=0;i<t->len;i++) {\
				res.ptr[i] = ___call(M, T, Vs, clone, t->ptr+i);\
			}\
			res;}),\
		drop({\
			for(int i=0;i<t->len;i++) {\
				___call(M, T, Vs, drop, t->ptr+i);\
			}\
			free(t->ptr);}),\
	))
#define Vec_new(...) _Vec_new((__VA_ARGS__), __VA_OPT__(,) 1, 0)
#define _Vec_new(Es, _0, _1, ...) _Vec_new_##_1 Es
#define _Vec_new_0() { .len = 0, .cap = 0, .ptr = 0 }
#define _Vec_new_1(E, ...) {\
	.len = sizeof((typeof(E)[]){E, __VA_ARGS__})/sizeof(E),\
	.cap = sizeof((typeof(E)[]){E, __VA_ARGS__})/sizeof(E),\
	.ptr = ({\
		struct { typeof((typeof(E)[]){E, __VA_ARGS__}) arr; } *ptr = malloc(sizeof(*ptr));\
		*ptr = (typeof(*ptr)){{E, __VA_ARGS__}}; (typeof(E)*)ptr;}) }

