/**************************************************
 * Source Code for the Original Compiler for the
 * Programming Language Wake
 *
 * Arithmetic.cpp
 *
 * Licensed under the MIT license
 * See LICENSE.TXT for details
 *
 * Author: Michael Fairhurst
 * Revised By:
 *
 **************************************************/

PTT_TEST_CASE(
	MultiplyNumsAndAnythingElseIsTypeError,
	"every MyClass is:															\n\
		multiplyNumByString() { 5 * 'test'; }									\n\
		multiplyStringByNum() { 'test' * 5; }									\n\
		multiplyNumByObject(MyClass) { 5 * MyClass; }							\n\
		multiplyObjectByNum(MyClass) { MyClass * 5; }							\n\
		multiplyLambdaByNum(MyClass --fn(MyClass) lambda) { lambda * 5; }		\n\
	",
	PTT_EXPECT(TYPE_ERROR)
	PTT_EXPECT(TYPE_ERROR)
	PTT_EXPECT(TYPE_ERROR)
	PTT_EXPECT(TYPE_ERROR)
	PTT_EXPECT(TYPE_ERROR)
)

PTT_TEST_CASE(
	DivideNumsAndAnythingElseIsTypeError,
	"every MyClass is:											\n\
		divideNumByString() { 5 / 'test'; }						\n\
		divideStringByNum() { 'test' / 5; }						\n\
		divideNumByObject(MyClass) { 5 / MyClass; }				\n\
		divideObjectByNum(MyClass) { MyClass / 5; }				\n\
		divideLambdaByNum(fn() lambda) { lambda / 5; }		\n\
	",
	PTT_EXPECT(TYPE_ERROR)
	PTT_EXPECT(TYPE_ERROR)
	PTT_EXPECT(TYPE_ERROR)
	PTT_EXPECT(TYPE_ERROR)
	PTT_EXPECT(TYPE_ERROR)
)

PTT_TEST_CASE(
	AddNumsAndAnythingElseIsTypeError,
	"every MyClass is:											\n\
		addNumByString() { 5 + 'test'; }						\n\
		addStringByNum() { 'test' + 5; }						\n\
		addNumByObject(MyClass) { 5 + MyClass; }				\n\
		addObjectByNum(MyClass) { MyClass + 5; }				\n\
		addLambdaByNum(fn() lambda) { lambda + 5; }			\n\
	",
	PTT_EXPECT(TYPE_ERROR)
	PTT_EXPECT(TYPE_ERROR)
	PTT_EXPECT(TYPE_ERROR)
	PTT_EXPECT(TYPE_ERROR)
	PTT_EXPECT(TYPE_ERROR)
)

PTT_TEST_CASE(
	SubtractNumsAndAnythingElseIsTypeError,
	"every MyClass is:											\n\
		subtractNumByString() { 5 - 'test'; }					\n\
		subtractStringByNum() { 'test' - 5; }					\n\
		subtractNumByObject(MyClass) { 5 - MyClass; }			\n\
		subtractObjectByNum(MyClass) { MyClass - 5; }			\n\
		subtractLambdaByNum(fn() lambda) { lambda - 5; }		\n\
	",
	PTT_EXPECT(TYPE_ERROR)
	PTT_EXPECT(TYPE_ERROR)
	PTT_EXPECT(TYPE_ERROR)
	PTT_EXPECT(TYPE_ERROR)
	PTT_EXPECT(TYPE_ERROR)
)

PTT_TEST_CASE(
	ValidArithmeticAndComparisonsReturnProperTypes,
	"every MyClass is:																\n\
		validArithmetic() {															\n\
			5 / 5 / 5;					// done thrice to ensure int returned		\n\
			5 * 5 * 5;					// done thrice to ensure int returned		\n\
			5 + 5 + 5;					// done thrice to ensure int returned		\n\
			5 - 5 - 5;					// done thrice to ensure int returned		\n\
			'test' + 'test' + 'test';	// done thrice to ensure string returned	\n\
			'test' == 'test' == true;	// test returns truth						\n\
			'test' != 'test' == true;	// test returns truth						\n\
			5 == 5 == true;				// test returns truth						\n\
			5 != 5 == true;				// test returns truth						\n\
			5 < 5 == true;				// test returns truth						\n\
			5 > 5 == true;				// test returns truth						\n\
			5 <= 5 == true;				// test returns truth						\n\
			5 >= 5 == true;				// test returns truth						\n\
		}																			\n\
	",
	PTT_VALID
)

PTT_TEST_CASE(
	ArithmeticOperatorsNotValidOnStringPairs,
	"every MyClass is:									\n\
		subtractStrings() { 'test' - 'test'; }			\n\
		multiplyStrings() { 'test' * 'test'; }			\n\
		divideStrings() { 'test' / 'test'; }			\n\
		lessthanStrings() { 'test' < 'test'; }			\n\
		greaterthanStrings() { 'test' > 'test'; }		\n\
		lessthanequalStrings() { 'test' <= 'test'; }	\n\
		greaterthanequalStrings() { 'test' >= 'test'; }	\n\
	",
	PTT_EXPECT(TYPE_ERROR)
	PTT_EXPECT(TYPE_ERROR)
	PTT_EXPECT(TYPE_ERROR)
	PTT_EXPECT(TYPE_ERROR)
	PTT_EXPECT(TYPE_ERROR)
	PTT_EXPECT(TYPE_ERROR)
	PTT_EXPECT(TYPE_ERROR)
)

