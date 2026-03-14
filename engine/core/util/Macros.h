#pragma once

#define OLY_REMOVE_PARENS_IMPL(...) __VA_ARGS__
#define OLY_REMOVE_PARENS(X) OLY_REMOVE_PARENS_IMPL X
#define OLY_CONCAT(...) __VA_ARGS__
#define OLY_FLATTEN(...) OLY_REMOVE_PARENS((OLY_CONCAT(__VA_ARGS__)))

#define _OLY_REPEAT_1(M) M(1)
#define _OLY_REPEAT_2(M) _OLY_REPEAT_1(M) M(2)
#define _OLY_REPEAT_3(M) _OLY_REPEAT_2(M) M(3)
#define _OLY_REPEAT_4(M) _OLY_REPEAT_3(M) M(4)
#define _OLY_REPEAT_5(M) _OLY_REPEAT_4(M) M(5)
#define _OLY_REPEAT_6(M) _OLY_REPEAT_5(M) M(6)
#define _OLY_REPEAT_7(M) _OLY_REPEAT_6(M) M(7)
#define _OLY_REPEAT_8(M) _OLY_REPEAT_7(M) M(8)
#define _OLY_REPEAT_9(M) _OLY_REPEAT_8(M) M(9)
#define _OLY_REPEAT_10(M) _OLY_REPEAT_9(M) M(10)
#define _OLY_REPEAT_11(M) _OLY_REPEAT_10(M) M(11)
#define _OLY_REPEAT_12(M) _OLY_REPEAT_11(M) M(12)
#define _OLY_REPEAT_13(M) _OLY_REPEAT_12(M) M(13)
#define _OLY_REPEAT_14(M) _OLY_REPEAT_13(M) M(14)
#define _OLY_REPEAT_15(M) _OLY_REPEAT_14(M) M(15)
#define _OLY_REPEAT_16(M) _OLY_REPEAT_15(M) M(16)
#define _OLY_REPEAT(M, N) _OLY_REPEAT_##N(M)

#define _OLY_REPEAT_COMMA_1(M) M(1)
#define _OLY_REPEAT_COMMA_2(M) _OLY_REPEAT_COMMA_1(M), M(2)
#define _OLY_REPEAT_COMMA_3(M) _OLY_REPEAT_COMMA_2(M), M(3)
#define _OLY_REPEAT_COMMA_4(M) _OLY_REPEAT_COMMA_3(M), M(4)
#define _OLY_REPEAT_COMMA_5(M) _OLY_REPEAT_COMMA_4(M), M(5)
#define _OLY_REPEAT_COMMA_6(M) _OLY_REPEAT_COMMA_5(M), M(6)
#define _OLY_REPEAT_COMMA_7(M) _OLY_REPEAT_COMMA_6(M), M(7)
#define _OLY_REPEAT_COMMA_8(M) _OLY_REPEAT_COMMA_7(M), M(8)
#define _OLY_REPEAT_COMMA_9(M) _OLY_REPEAT_COMMA_8(M), M(9)
#define _OLY_REPEAT_COMMA_10(M) _OLY_REPEAT_COMMA_9(M), M(10)
#define _OLY_REPEAT_COMMA_11(M) _OLY_REPEAT_COMMA_10(M), M(11)
#define _OLY_REPEAT_COMMA_12(M) _OLY_REPEAT_COMMA_11(M), M(12)
#define _OLY_REPEAT_COMMA_13(M) _OLY_REPEAT_COMMA_12(M), M(13)
#define _OLY_REPEAT_COMMA_14(M) _OLY_REPEAT_COMMA_13(M), M(14)
#define _OLY_REPEAT_COMMA_15(M) _OLY_REPEAT_COMMA_14(M), M(15)
#define _OLY_REPEAT_COMMA_16(M) _OLY_REPEAT_COMMA_15(M), M(16)
#define _OLY_REPEAT_COMMA(M, N) _OLY_REPEAT_COMMA_##N(M)

#define _OLY_POLYKLASS_CASES_BEGIN(obj) \
    auto& _polyklass_obj = obj; \
    do\
	{

#define _OLY_POLYKLASS_IF_CASE(name)\
		if (klass == #name)\
		{\
			if (!_polyklass_obj.castable<name>())\
				_polyklass_obj = make_polymorphic<name>();\
		}

#define _OLY_POLYKLASS_ELSE_IF_CASE(name)\
		else _OLY_POLYKLASS_IF_CASE(name)

#define _OLY_POLYKLASS_CASES_END \
    } while(false);
