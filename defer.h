#define _with(M, ...) M(__VA_ARGS__)

#define UNWIND_NONE 0
#define UNWIND_BREAK 1
#define UNWIND_CONTINUE 2
#define UNWIND_RETURN 3
#define UNWIND_ABORT 4

#define _break do { __is_in_loop_check; __unwind_type=UNWIND_BREAK; goto *__last_defer; } while(0)
#define _continue do { __is_in_loop_check; __unwind_type=UNWIND_CONTINUE; goto *__last_defer; } while(0)
#define _return _with(__return, __COUNTER__)
#define _abort _with(__abort, __COUNTER__)
#define __return(N) if (1) { __fn_exit=&&__go_ret_##N; __unwind_type=UNWIND_RETURN; goto *__last_defer; } else __go_ret_##N: return
#define __abort(N) if (1) { __fn_exit=&&__go_ret_##N; __unwind_type=UNWIND_ABORT; goto *__last_defer; } else __go_ret_##N: return

#define _defer(...) _with(__defer, __COUNTER__, __VA_ARGS__)
#define __defer(N, ...) do {\
	void *__prev_defer_##N=__last_defer;\
	__last_defer=&&__lbl_defer_##N;\
	if (0) { __lbl_defer_##N:\
		__VA_ARGS__;\
		goto *__prev_defer_##N;\
	}} while(0)

#define _errdefer(...) _with(__errdefer, __COUNTER__, __VA_ARGS__)
#define __errdefer(N, ...) do {\
	void *__prev_defer_##N=__last_defer;\
	__last_defer=&&__lbl_defer_##N;\
	if (0) { __lbl_defer_##N:\
		if (__unwind_type==UNWIND_ABORT) { __VA_ARGS__; }\
		goto *__prev_defer_##N;\
	}} while(0)

#define _if(...) if (__VA_ARGS__) _scope
#define _else else _scope
#define _for(...) for (__VA_ARGS__) _loop

#define _scope _with(__scope, __COUNTER__)
#define __scope(N) if (0) {\
__scope_anchor_##N:\
	if (__unwind_type==UNWIND_NONE); /*fallthrough*/\
	else goto *__last_defer;\
} else for (void *__i=0,*__last_defer=&&__scope_anchor_##N;;__i=(void*)1) if(__i) {\
	goto *__last_defer; /*always drain current chain*/\
} else

#define _loop _with(__loop, __COUNTER__)
#define __loop(N) if (0) {\
__scope_anchor_##N:\
	if (__unwind_type==UNWIND_NONE); /*fallthrough*/\
	else if (__unwind_type==UNWIND_BREAK) break;\
	else if (__unwind_type==UNWIND_CONTINUE) continue;\
	else goto *__last_defer;\
} else for (void *__i=0,*__is_in_loop_check,*__last_defer=&&__scope_anchor_##N;;__i=(void*)1) if(__i) {\
	(void)__is_in_loop_check;\
	goto *__last_defer; /*always drain current chain*/\
} else

#define _fn_prelude \
	int __unwind_type=UNWIND_NONE;\
	void *__fn_exit=0,*__last_defer=&&__fn_anchor;\
	if (0) __fn_anchor: goto *__fn_exit;

