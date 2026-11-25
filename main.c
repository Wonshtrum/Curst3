#include "curst.h"

#define Test(A, B) _struct(Test, (), (A, B),\
	_derive(debug, clone, drop, eq),\
	_field(a, A),\
	_field(b, B))

#define Opt2 _enum(Opt2, (), (),\
	_derive(debug),\
	_variant(Some, _struct(OptInner, (), (),\
		_derive(),\
		_field(a, i32),\
		_field(a, i32))),\
	_variant(None, _T()))


int test() {
	_fn_prelude;
	_defer(printf("test\n"));
	_for (int i=0; i<10; i++) {
		_let(Box(i32), b1) Box_new(i);
		_let(Box(i32), b2) Box_new(i);
		printf("i=%d\n", i);
		_if (i > 2) _return 42;
	}
	return 0;
}

int main() {
	_bool eq;

	test();

	_t(Vec(i32)) v0 = Vec_new();
	_t(Vec(i32)) v1 = Vec_new(1, 2, 3);
	_t(Vec(Vec(i32))) v2 = Vec_new(v0, v1);
	_call(Vec(Vec(i32)), debug, &v2);
	eq = _call(Vec(Vec(i32)), eq, &v2, &v2);
	printf("\nv2==v2: %d\n", eq);

	_t(_T(i32, Vec(i32))) t = { ._0 = 42, ._1 = v1 };
	_call(_T(i32, Vec(i32)), debug, &t);

	eq = _call(_T(i32, Vec(i32)), eq, &t, &t);
	printf("\nt==t: %d\n", eq);

	_t(Test(i32, Vec(i32))) test = { .a = 42, .b = v1 };
	_call(Test(i32, Vec(i32)), debug, &test);
	printf("\n");

	_t(Test(i32, i32)) test1 = { .a = 42, .b = 69 };
	_t(Test(i32, i32)) test2 = _call(Test(i32, i32), clone, &test1);
	test2.a = 1337;
	printf("test1: ");
	_call(Test(i32, i32), debug, &test1);
	printf("\ntest2: ");
	_call(Test(i32, i32), debug, &test2);
	eq =_call(Test(i32, i32), eq, &test1, &test1);
	printf("\ntest1==test1: %d\n", eq);
	eq =_call(Test(i32, i32), eq, &test1, &test2);
	printf("test1==test2: %d\n", eq);
	_call(Test(i32, i32), drop, &test1);
	_call(Test(i32, i32), drop, &test2);

	_t(Opt(i32)) o1 = _build(Opt, Some, 42);
	_t(Opt(i32)) o2 = _build(Opt, None);
	printf("\no1: ");
	_call(Opt(i32), debug, &o1);
	printf("\no2: ");
	_call(Opt(i32), debug, &o2);
	eq = _call(Opt(i32), eq, &o1, &o1);
	printf("\no1==o1: %d", eq);
	eq = _call(Opt(i32), eq, &o1, &o2);
	printf("\no1==o2: %d", eq);
	printf("\n");
}
