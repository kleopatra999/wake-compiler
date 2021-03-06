/**************************************************
 * Source Code for the Original Compiler for the
 * Programming Language Wake
 *
 * TypeAnalyzerTest.cpp
 *
 * Licensed under the MIT license
 * See LICENSE.TXT for details
 *
 * Author: Michael Fairhurst
 * Revised By:
 *
 **************************************************/

#include "boost/test/unit_test.hpp"

#include "TypeAnalyzer.h"
#include "ClassSpaceSymbolTable.h"

/**
 *  NOTE!!! This functionality used to be part of ClassSpaceSymbolTable
 * and therefore many test cases for this class exist in ClassSpaceSymbolTableTest
 */

BOOST_AUTO_TEST_SUITE(TypeAnalyzerTest);

BOOST_AUTO_TEST_CASE(TypesAreTheirOwnSubtypes) {
	TypeAnalyzer analyzer;
	BOOST_REQUIRE(analyzer.isASubtypeOfB("test", "test"));
}

BOOST_AUTO_TEST_CASE(RealTypeIsSubtypesOfOptionalType) {
	TypeAnalyzer analyzer;

	Type realtype(TYPE_CLASS);
	realtype.typedata._class.classname = strdup("test");

	Type* wrappedtype = new Type(TYPE_CLASS);
	wrappedtype->typedata._class.classname = strdup("test");
	Type optionaltype(TYPE_OPTIONAL);
	optionaltype.typedata.optional.contained = wrappedtype;

	BOOST_REQUIRE(analyzer.isASubtypeOfB(&realtype, &optionaltype));
}

BOOST_AUTO_TEST_CASE(OptionalTypeIsNotSubtypeOfRealType) {
	TypeAnalyzer analyzer;

	Type* wrappedtype = new Type(TYPE_CLASS);
	wrappedtype->typedata._class.classname = strdup("test");
	Type optionaltype(TYPE_OPTIONAL);
	optionaltype.typedata.optional.contained = wrappedtype;

	Type realtype(TYPE_CLASS);
	realtype.typedata._class.classname = strdup("test");

	BOOST_REQUIRE(!analyzer.isASubtypeOfB(&optionaltype, &realtype));
}

BOOST_AUTO_TEST_CASE(NothingIsSubtypesOfOptionalType) {
	TypeAnalyzer analyzer;
	Type optionaltype(TYPE_OPTIONAL);
	optionaltype.typedata.optional.contained = new Type(TYPE_CLASS);
	Type nothing(TYPE_OPTIONAL);
	nothing.typedata.optional.contained = new Type(TYPE_MATCHALL);
	BOOST_REQUIRE(analyzer.isASubtypeOfB(&nothing, &optionaltype));
}

BOOST_AUTO_TEST_CASE(NothingIsNotSubtypeOfRealType) {
	TypeAnalyzer analyzer;
	Type nothing(TYPE_OPTIONAL);
	nothing.typedata.optional.contained = new Type(TYPE_MATCHALL);
	Type realtype(TYPE_CLASS);
	realtype.typedata._class.classname = strdup("test");
	BOOST_REQUIRE(!analyzer.isASubtypeOfB(&nothing, &realtype));
}

BOOST_AUTO_TEST_CASE(MatchallNameIsErroneousType) {
	TypeAnalyzer analyzer;
	Type matchall(TYPE_MATCHALL);
	BOOST_REQUIRE(analyzer.getNameForType(&matchall) == "{inferencing failed}");
}

BOOST_AUTO_TEST_CASE(ParameterizedClassTypesUnequalParametersNotSubtypes) {
	TypeAnalyzer analyzer;
	Type a(TYPE_CLASS);
	Type b(TYPE_CLASS);
	a.typedata._class.classname = strdup("hello");
	b.typedata._class.classname = strdup("hello");
	a.typedata._class.parameters = MakeTypeArray();
	AddTypeToTypeArray(new Type(TYPE_MATCHALL), a.typedata._class.parameters);
	AddTypeToTypeArray(new Type(TYPE_MATCHALL), a.typedata._class.parameters);
	b.typedata._class.parameters = MakeTypeArray();
	AddTypeToTypeArray(new Type(TYPE_MATCHALL), b.typedata._class.parameters);

	BOOST_REQUIRE(!analyzer.isASubtypeOfB(&a,&b));
	BOOST_REQUIRE(!analyzer.isASubtypeOfB(&b,&a));
}

BOOST_AUTO_TEST_CASE(ParameterizedClassTypesRequireDifferentArgumentsAreSubtypes) {
	TypeAnalyzer analyzer;
	ClassSpaceSymbolTable table;
	analyzer.reference = &table;
	Type a(TYPE_CLASS);
	Type b(TYPE_CLASS);
	Type* asub = new Type(TYPE_CLASS);
	Type* bsub = new Type(TYPE_CLASS);
	a.typedata._class.classname = strdup("hello");
	b.typedata._class.classname = strdup("hello");
	asub->typedata._class.classname = strdup("hellosub");
	bsub->typedata._class.classname = strdup("hellosub");
	a.typedata._class.parameters = MakeTypeArray();
	b.typedata._class.parameters = MakeTypeArray();

	AddTypeToTypeArray(asub, a.typedata._class.parameters);
	AddTypeToTypeArray(bsub, b.typedata._class.parameters);

	BOOST_REQUIRE(analyzer.isASubtypeOfB(&a,&b));
	BOOST_REQUIRE(analyzer.isASubtypeOfB(&b,&a));

	free(asub->typedata._class.classname);
	free(bsub->typedata._class.classname);
	asub->typedata._class.classname = strdup("hellosuba");
	bsub->typedata._class.classname = strdup("hellosubb");

	BOOST_REQUIRE(!analyzer.isASubtypeOfB(&a,&b));
	BOOST_REQUIRE(!analyzer.isASubtypeOfB(&b,&a));
}

BOOST_AUTO_TEST_CASE(ParameterizedClassTypesArentCovariantOrContravariant) {
	TypeAnalyzer analyzer;
	ClassSpaceSymbolTable table;
	analyzer.reference = &table;

	Type a(TYPE_CLASS);
	Type b(TYPE_CLASS);

	Type* asubwrapped = new Type(TYPE_CLASS);
	Type* asub = new Type(TYPE_OPTIONAL);
	Type* bsub = new Type(TYPE_CLASS);

	a.typedata._class.classname = strdup("hello");
	b.typedata._class.classname = strdup("hello");

	asubwrapped->typedata._class.classname = strdup("hellosub");
	bsub->typedata._class.classname = strdup("hellosub");
	asub->typedata.optional.contained = asubwrapped; // easiest way to make asub a subtype of bsub, and bsub a supertype of asub
	a.typedata._class.parameters = MakeTypeArray();
	b.typedata._class.parameters = MakeTypeArray();

	AddTypeToTypeArray(asub, a.typedata._class.parameters);
	AddTypeToTypeArray(bsub, b.typedata._class.parameters);

	BOOST_REQUIRE(!analyzer.isASubtypeOfB(&a,&b));
	BOOST_REQUIRE(!analyzer.isASubtypeOfB(&b,&a));
}

BOOST_AUTO_TEST_CASE(TwoClassesAreExactTypes) {
	TypeAnalyzer analyzer;
	Type a(TYPE_CLASS);
	Type b(TYPE_CLASS);
	a.typedata._class.classname = strdup("hello");
	b.typedata._class.classname = strdup("hello");
	BOOST_REQUIRE(analyzer.isAExactlyB(&a, &b));
	BOOST_REQUIRE(analyzer.isAExactlyB(&b, &a));
}

BOOST_AUTO_TEST_CASE(MismatchedArrayTypesArentExact) {
	TypeAnalyzer analyzer;

	Type* acontained = new Type(TYPE_CLASS);
	Type* bcontained = new Type(TYPE_CLASS);
	acontained->typedata._class.classname = strdup("hello");
	bcontained->typedata._class.classname = strdup("hello");

	Type a(TYPE_LIST);
	Type b(TYPE_LIST);
	Type* binner = new Type(TYPE_LIST);
	a.typedata.list.contained = acontained;
	b.typedata.list.contained = binner;
	binner->typedata.list.contained = bcontained;

	BOOST_REQUIRE(!analyzer.isAExactlyB(acontained, &b));
	BOOST_REQUIRE(!analyzer.isAExactlyB(&b, acontained));
	BOOST_REQUIRE(!analyzer.isAExactlyB(&a, &b));
	BOOST_REQUIRE(!analyzer.isAExactlyB(&b, &a));
}

BOOST_AUTO_TEST_CASE(MismatchedOptionalTypesArentExact) {
	TypeAnalyzer analyzer;
	Type* awrapped = new Type(TYPE_CLASS);
	awrapped->typedata._class.classname = strdup("hello");

	Type* bwrapped = new Type(TYPE_CLASS);
	bwrapped->typedata._class.classname = strdup("hello");

	Type a(TYPE_OPTIONAL);
	Type* ainner = new Type(TYPE_OPTIONAL);
	ainner->typedata.optional.contained = awrapped;
	a.typedata.optional.contained = ainner;

	Type b(TYPE_OPTIONAL);
	b.typedata.optional.contained = bwrapped;

	BOOST_REQUIRE(!analyzer.isAExactlyB(awrapped, &b));
	BOOST_REQUIRE(!analyzer.isAExactlyB(&b, awrapped));
	BOOST_REQUIRE(!analyzer.isAExactlyB(&a, &b));
	BOOST_REQUIRE(!analyzer.isAExactlyB(&b, &a));
}

BOOST_AUTO_TEST_CASE(MismatchedNamesArentExact) {
	TypeAnalyzer analyzer;
	Type a(TYPE_CLASS);
	Type b(TYPE_CLASS);
	a.typedata._class.classname = strdup("hello");
	b.typedata._class.classname = strdup("nothello");
	BOOST_REQUIRE(!analyzer.isAExactlyB(&a, &b));
	BOOST_REQUIRE(!analyzer.isAExactlyB(&b, &a));
}

BOOST_AUTO_TEST_CASE(MismatchedTypesArentExact) {
	TypeAnalyzer analyzer;
	Type a(TYPE_CLASS);
	Type b(TYPE_PARAMETERIZED);
	a.typedata._class.classname = strdup("hello");
	b.typedata._class.classname = strdup("hello");
	BOOST_REQUIRE(!analyzer.isAExactlyB(&a, &b));
	BOOST_REQUIRE(!analyzer.isAExactlyB(&b, &a));
}

BOOST_AUTO_TEST_CASE(OnlyOneParameterizedArentExact) {
	TypeAnalyzer analyzer;
	Type a(TYPE_CLASS);
	Type b(TYPE_CLASS);
	a.typedata._class.classname = strdup("hello");
	b.typedata._class.classname = strdup("hello");
	a.typedata._class.parameters = MakeTypeArray();
	AddTypeToTypeArray(new Type(TYPE_MATCHALL), a.typedata._class.parameters);
	BOOST_REQUIRE(!analyzer.isAExactlyB(&a, &b));
	BOOST_REQUIRE(!analyzer.isAExactlyB(&b, &a));
}

BOOST_AUTO_TEST_CASE(MismatchedParameterizationLengthsArentExact) {
	TypeAnalyzer analyzer;
	Type a(TYPE_CLASS);
	Type b(TYPE_CLASS);
	a.typedata._class.classname = strdup("hello");
	b.typedata._class.classname = strdup("hello");
	a.typedata._class.parameters = MakeTypeArray();
	b.typedata._class.parameters = MakeTypeArray();
	AddTypeToTypeArray(new Type(TYPE_MATCHALL), a.typedata._class.parameters);
	AddTypeToTypeArray(new Type(TYPE_MATCHALL), a.typedata._class.parameters);
	AddTypeToTypeArray(new Type(TYPE_MATCHALL), b.typedata._class.parameters);
	BOOST_REQUIRE(!analyzer.isAExactlyB(&a, &b));
	BOOST_REQUIRE(!analyzer.isAExactlyB(&b, &a));
}

BOOST_AUTO_TEST_CASE(MismatchedParameterizationsArentExact) {
	TypeAnalyzer analyzer;
	Type a(TYPE_CLASS);
	Type b(TYPE_CLASS);
	Type* asub = new Type(TYPE_CLASS);
	Type* bsub = new Type(TYPE_CLASS);
	a.typedata._class.classname = strdup("hello");
	b.typedata._class.classname = strdup("hello");
	asub->typedata._class.classname = strdup("hellosuba");
	bsub->typedata._class.classname = strdup("hellosubb");
	a.typedata._class.parameters = MakeTypeArray();
	b.typedata._class.parameters = MakeTypeArray();
	AddTypeToTypeArray(asub, a.typedata._class.parameters);
	AddTypeToTypeArray(bsub, b.typedata._class.parameters);
	BOOST_REQUIRE(!analyzer.isAExactlyB(&a, &b));
	BOOST_REQUIRE(!analyzer.isAExactlyB(&b, &a));
}

BOOST_AUTO_TEST_CASE(ExactParameterizationsArentExact) {
	TypeAnalyzer analyzer;
	Type a(TYPE_CLASS);
	Type b(TYPE_CLASS);
	Type* asub = new Type(TYPE_CLASS);
	Type* bsub = new Type(TYPE_CLASS);
	a.typedata._class.classname = strdup("hello");
	b.typedata._class.classname = strdup("hello");
	asub->typedata._class.classname = strdup("hellosub");
	bsub->typedata._class.classname = strdup("hellosub");
	a.typedata._class.parameters = MakeTypeArray();
	b.typedata._class.parameters = MakeTypeArray();
	AddTypeToTypeArray(asub, a.typedata._class.parameters);
	AddTypeToTypeArray(bsub, b.typedata._class.parameters);
	BOOST_REQUIRE(analyzer.isAExactlyB(&a, &b));
	BOOST_REQUIRE(analyzer.isAExactlyB(&b, &a));
}

BOOST_AUTO_TEST_CASE(ParameterizedTypesAreExactWithExactLabel) {
	TypeAnalyzer analyzer;
	Type a(TYPE_PARAMETERIZED);
	Type b(TYPE_PARAMETERIZED);
	a.typedata.parameterized.label = strdup("hello");
	b.typedata.parameterized.label = strdup("hello");
	BOOST_REQUIRE(analyzer.isAExactlyB(&a, &b));
	BOOST_REQUIRE(analyzer.isAExactlyB(&b, &a));
}

BOOST_AUTO_TEST_CASE(ParameterizedTypesAreNotExactWithDifferentLabel) {
	TypeAnalyzer analyzer;
	Type a(TYPE_PARAMETERIZED);
	Type b(TYPE_PARAMETERIZED);
	a.typedata.parameterized.label = strdup("hello");
	b.typedata.parameterized.label = strdup("nothello");
	BOOST_REQUIRE(!analyzer.isAExactlyB(&a, &b));
	BOOST_REQUIRE(!analyzer.isAExactlyB(&b, &a));
}

BOOST_AUTO_TEST_CASE(LambdaTypesAreNotExactWithVoidVsNonVoid) {
	TypeAnalyzer analyzer;
	Type a(TYPE_LAMBDA);
	Type b(TYPE_LAMBDA);
	a.typedata.lambda.returntype = new Type(TYPE_MATCHALL);
	BOOST_REQUIRE(!analyzer.isAExactlyB(&a, &b));
	BOOST_REQUIRE(!analyzer.isAExactlyB(&b, &a));
}

BOOST_AUTO_TEST_CASE(LambdaTypesAreNotExactWithDifferentReturnType) {
	TypeAnalyzer analyzer;
	Type a(TYPE_LAMBDA);
	Type b(TYPE_LAMBDA);
	a.typedata.lambda.returntype = new Type(TYPE_CLASS);
	b.typedata.lambda.returntype = new Type(TYPE_CLASS);
	a.typedata.lambda.returntype->typedata._class.classname = strdup("hello");
	b.typedata.lambda.returntype->typedata._class.classname = strdup("nothello");
	BOOST_REQUIRE(!analyzer.isAExactlyB(&a, &b));
	BOOST_REQUIRE(!analyzer.isAExactlyB(&b, &a));
}

BOOST_AUTO_TEST_CASE(LambdaTypesAreNotExactWithDifferentArgumentCounts) {
	TypeAnalyzer analyzer;
	Type a(TYPE_LAMBDA);
	Type b(TYPE_LAMBDA);
	a.typedata.lambda.arguments = MakeTypeArray();
	b.typedata.lambda.arguments = MakeTypeArray();
	AddTypeToTypeArray(new Type(TYPE_MATCHALL), a.typedata.lambda.arguments);
	BOOST_REQUIRE(!analyzer.isAExactlyB(&a, &b));
	BOOST_REQUIRE(!analyzer.isAExactlyB(&b, &a));
}

BOOST_AUTO_TEST_CASE(LambdaTypesAreNotExactWithNullNonNullArguments) {
	TypeAnalyzer analyzer;
	Type a(TYPE_LAMBDA);
	Type b(TYPE_LAMBDA);
	a.typedata.lambda.arguments = MakeTypeArray();
	BOOST_REQUIRE(!analyzer.isAExactlyB(&a, &b));
	BOOST_REQUIRE(!analyzer.isAExactlyB(&b, &a));
}

BOOST_AUTO_TEST_CASE(LambdaTypesAreNotExactWithDifferentArguments) {
	TypeAnalyzer analyzer;
	Type a(TYPE_LAMBDA);
	Type b(TYPE_LAMBDA);
	Type* aarg = new Type(TYPE_CLASS);
	aarg->typedata._class.classname = strdup("holl");
	Type* barg = new Type(TYPE_CLASS);
	barg->typedata._class.classname = strdup("holle");
	a.typedata.lambda.arguments = MakeTypeArray();
	AddTypeToTypeArray(aarg, a.typedata.lambda.arguments);
	b.typedata.lambda.arguments = MakeTypeArray();
	AddTypeToTypeArray(barg, b.typedata.lambda.arguments);
	BOOST_REQUIRE(!analyzer.isAExactlyB(&a, &b));
	BOOST_REQUIRE(!analyzer.isAExactlyB(&b, &a));
}

/**
 * This is a bit of a tricky one. On the one hand, if someone
 * makes an array [1, TYPE_ERROR], that turns into [1, TYPE_MATCHALL],
 * which we then don't understand and should assume TYPE_MATCHALL[].
 * But on the other hand, this way we can represent `[]` and `nothing`
 * as a TYPE_LIST[TYPE_MATCHALL] and TYPE_OPTIONAL[TYPE_MATCHALL]
 */
BOOST_AUTO_TEST_CASE(MatchallTypesCommonAreTheirNeighbor) {
	TypeAnalyzer analyzer;
	Type a(TYPE_MATCHALL);
	Type b(TYPE_OPTIONAL);
	b.typedata.optional.contained = new Type(TYPE_MATCHALL);
	Type c(TYPE_CLASS);
	c.typedata._class.classname = strdup("Test");
	Type d(TYPE_LAMBDA);
	Type e(TYPE_LIST);
	e.typedata.optional.contained = new Type(TYPE_MATCHALL);
	boost::optional<Type*> ret;

	ret = analyzer.getCommonSubtypeOf(&a, &a);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE((*ret)->type == TYPE_MATCHALL); freeType(*ret);
	ret = analyzer.getCommonSubtypeOf(&a, &b);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE((*ret)->type == TYPE_OPTIONAL); freeType(*ret);
	ret = analyzer.getCommonSubtypeOf(&a, &c);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE((*ret)->type == TYPE_CLASS); freeType(*ret);
	ret = analyzer.getCommonSubtypeOf(&a, &d);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE((*ret)->type == TYPE_LAMBDA); freeType(*ret);
	ret = analyzer.getCommonSubtypeOf(&a, &e);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE((*ret)->type == TYPE_LIST); freeType(*ret);
	ret = analyzer.getCommonSubtypeOf(&b, &a);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE((*ret)->type == TYPE_OPTIONAL); freeType(*ret);
	ret = analyzer.getCommonSubtypeOf(&c, &a);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE((*ret)->type == TYPE_CLASS); freeType(*ret);
	ret = analyzer.getCommonSubtypeOf(&d, &a);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE((*ret)->type == TYPE_LAMBDA); freeType(*ret);
	ret = analyzer.getCommonSubtypeOf(&e, &a);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE((*ret)->type == TYPE_LIST); freeType(*ret);
}

BOOST_AUTO_TEST_CASE(NothingOptionalCommonTypeIsOptionalType) {
	TypeAnalyzer analyzer;
	Type optional(TYPE_OPTIONAL);
	Type* contained = new Type(TYPE_CLASS);
	optional.typedata.optional.contained = contained;
	contained->typedata._class.classname = strdup("Classname");
	Type nothing(TYPE_OPTIONAL);
	nothing.typedata.optional.contained = new Type(TYPE_MATCHALL);

	boost::optional<Type*> ret = analyzer.getCommonSubtypeOf(&optional, &nothing);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE((*ret)->type == TYPE_OPTIONAL);
	BOOST_REQUIRE((*ret)->typedata.optional.contained->type == TYPE_CLASS);
	BOOST_REQUIRE((*ret)->typedata.optional.contained->typedata._class.classname == string("Classname"));
	freeType(*ret);
	ret = analyzer.getCommonSubtypeOf(&nothing, &optional);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE((*ret)->type == TYPE_OPTIONAL);
	BOOST_REQUIRE((*ret)->typedata.optional.contained->type == TYPE_CLASS);
	BOOST_REQUIRE((*ret)->typedata.optional.contained->typedata._class.classname == string("Classname"));
	freeType(*ret);
}

BOOST_AUTO_TEST_CASE(NothingNothingCommonTypeIsNothing) {
	TypeAnalyzer analyzer;
	Type nothing(TYPE_OPTIONAL);
	nothing.typedata.optional.contained = new Type(TYPE_MATCHALL);
	boost::optional<Type*> ret = analyzer.getCommonSubtypeOf(&nothing, &nothing);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE((*ret)->type == TYPE_OPTIONAL);
	BOOST_REQUIRE((*ret)->typedata.optional.contained->type == TYPE_MATCHALL);
	freeType(*ret);
}

BOOST_AUTO_TEST_CASE(MismatchedTypesWithNoCommonType) {
	TypeAnalyzer analyzer;
	Type lambda(TYPE_LAMBDA);
	Type clazz(TYPE_CLASS); clazz.typedata._class.classname = strdup("Test");
	Type list(TYPE_LIST);
	list.typedata.list.contained = new Type(clazz);
	Type unusable(TYPE_UNUSABLE);

	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&lambda, &clazz));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&lambda, &list));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&lambda, &unusable));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&clazz, &list));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&clazz, &unusable));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&list, &unusable));

	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&clazz, &lambda));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&list, &lambda));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&unusable, &lambda));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&list, &clazz));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&unusable, &clazz));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&unusable, &list));
}

BOOST_AUTO_TEST_CASE(CommonTypeNothingNotOptionalNotNothingIsOptionalOtherType) {
	TypeAnalyzer analyzer;
	Type clazz(TYPE_CLASS);
	clazz.typedata._class.classname = strdup("Classname");
	Type nothing(TYPE_OPTIONAL);
	nothing.typedata.optional.contained = new Type(TYPE_MATCHALL);

	boost::optional<Type*> ret = analyzer.getCommonSubtypeOf(&clazz, &nothing);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE((*ret)->type == TYPE_OPTIONAL);
	BOOST_REQUIRE((*ret)->typedata.optional.contained->type == TYPE_CLASS);
	BOOST_REQUIRE((*ret)->typedata.optional.contained->typedata._class.classname == string("Classname"));
	freeType(*ret);
	ret = analyzer.getCommonSubtypeOf(&nothing, &clazz);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE((*ret)->type == TYPE_OPTIONAL);
	BOOST_REQUIRE((*ret)->typedata.optional.contained->type == TYPE_CLASS);
	BOOST_REQUIRE((*ret)->typedata.optional.contained->typedata._class.classname == string("Classname"));
	freeType(*ret);
}

BOOST_AUTO_TEST_CASE(ListTypesNotExactlyEqualHaveNoCommonType) {
	TypeAnalyzer analyzer;
	Type listOptNums(TYPE_LIST);
	Type* optNums = new Type(TYPE_OPTIONAL);
	Type* num = new Type(TYPE_CLASS);

	listOptNums.typedata.list.contained = optNums;
	optNums->typedata.optional.contained = num;
	num->typedata._class.classname = strdup("Num");

	Type listNums(TYPE_LIST);
	Type* num2 = new Type(TYPE_CLASS);

	listNums.typedata.list.contained = num2;
	num2->typedata._class.classname = strdup("Num");

	Type listTexts(TYPE_LIST);
	Type* text = new Type(TYPE_CLASS);

	listTexts.typedata.list.contained = text;
	text->typedata._class.classname = strdup("Text");

	Type listListTexts(TYPE_LIST);
	Type* listListTextsInner = new Type(TYPE_LIST);
	Type* text2 = new Type(TYPE_CLASS);

	listListTextsInner->typedata.list.contained = text2;
	listListTexts.typedata.list.contained = listListTextsInner;
	text2->typedata._class.classname = strdup("Text");

	Type listListOptNums(TYPE_LIST);
	Type* listListOptNumsInner = new Type(TYPE_LIST);
	Type* optNums2 = new Type(TYPE_OPTIONAL);
	Type* num3 = new Type(TYPE_CLASS);

	listListOptNums.typedata.list.contained = listListOptNumsInner;
	listListOptNumsInner->typedata.list.contained = optNums2;
	optNums2->typedata.optional.contained = num3;
	num3->typedata._class.classname = strdup("Num");

	Type listListNums(TYPE_LIST);
	Type* listListNumsInner = new Type(TYPE_LIST);
	Type* num4 = new Type(TYPE_CLASS);

	listListNums.typedata.list.contained = listListNumsInner;
	listListNumsInner->typedata.list.contained = num4;
	num4->typedata._class.classname = strdup("Num");

	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listOptNums, &listNums));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listOptNums, &listTexts));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listOptNums, &listListTexts));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listOptNums, &listListOptNums));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listOptNums, &listListNums));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listNums, &listTexts));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listNums, &listListTexts));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listNums, &listListOptNums));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listNums, &listListNums));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listTexts, &listListTexts));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listTexts, &listListOptNums));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listTexts, &listListNums));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listListTexts, &listListOptNums));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listListTexts, &listListNums));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listListOptNums, &listListNums));

	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listNums, &listOptNums));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listTexts, &listOptNums));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listListTexts, &listOptNums));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listListOptNums, &listOptNums));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listListNums, &listOptNums));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listTexts, &listNums));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listListTexts, &listNums));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listListOptNums, &listNums));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listListNums, &listNums));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listListTexts, &listTexts));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listListOptNums, &listTexts));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listListNums, &listTexts));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listListOptNums, &listListTexts));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listListNums, &listListTexts));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&listListNums, &listListOptNums));

	boost::optional<Type*> ret;
	ret = analyzer.getCommonSubtypeOf(&listOptNums, &listOptNums);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE(analyzer.isAExactlyB(*ret, &listOptNums));
	freeType(*ret);
	ret = analyzer.getCommonSubtypeOf(&listNums, &listNums);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE(analyzer.isAExactlyB(*ret, &listNums));
	freeType(*ret);
	ret = analyzer.getCommonSubtypeOf(&listTexts, &listTexts);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE(analyzer.isAExactlyB(*ret, &listTexts));
	freeType(*ret);
	ret = analyzer.getCommonSubtypeOf(&listListTexts, &listListTexts);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE(analyzer.isAExactlyB(*ret, &listListTexts));
	freeType(*ret);
	ret = analyzer.getCommonSubtypeOf(&listListOptNums, &listListOptNums);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE(analyzer.isAExactlyB(*ret, &listListOptNums));
	freeType(*ret);
	ret = analyzer.getCommonSubtypeOf(&listListNums, &listListNums);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE(analyzer.isAExactlyB(*ret, &listListNums));
	freeType(*ret);
}

BOOST_AUTO_TEST_CASE(TestExactClassesAreTheirOwnCommonType) {
	TypeAnalyzer analyzer;
	Type text1(TYPE_CLASS);
	text1.typedata._class.classname = strdup("Text");
	text1.typedata._class.shadow = 0;
	Type text2(TYPE_CLASS);
	text2.typedata._class.classname = strdup("Text");
	text2.typedata._class.shadow = 1;

	boost::optional<Type*> ret = analyzer.getCommonSubtypeOf(&text1, &text2);
	BOOST_REQUIRE(*ret);
	BOOST_REQUIRE((*ret)->type == TYPE_CLASS);
	BOOST_REQUIRE((*ret)->typedata._class.classname == string("Text"));
	freeType(*ret);
	ret = analyzer.getCommonSubtypeOf(&text2, &text1);
	BOOST_REQUIRE(*ret);
	BOOST_REQUIRE((*ret)->type == TYPE_CLASS);
	BOOST_REQUIRE((*ret)->typedata._class.classname == string("Text"));
	freeType(*ret);
}

BOOST_AUTO_TEST_CASE(TestClassesWithMismatchedParameterExistenceHaveNoCommonType) {
	TypeAnalyzer analyzer;
	Type text1(TYPE_CLASS);
	text1.typedata._class.classname = strdup("Text");
	text1.typedata._class.shadow = 0;
	text1.typedata._class.parameters = MakeTypeArray();
	AddTypeToTypeArray(new Type(TYPE_MATCHALL), text1.typedata._class.parameters);
	Type text2(TYPE_CLASS);
	text2.typedata._class.classname = strdup("Text");
	text2.typedata._class.shadow = 1;

	boost::optional<Type*> ret;
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&text1, &text2));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&text2, &text1));
}

BOOST_AUTO_TEST_CASE(TestClassesWithParameterValuesDifferentHaveNoCommonType) {
	TypeAnalyzer analyzer;
	ClassSpaceSymbolTable reference;
	analyzer.reference = &reference;
	Type text1(TYPE_CLASS);
	text1.typedata._class.classname = strdup("Text");
	text1.typedata._class.shadow = 0;
	text1.typedata._class.parameters = MakeTypeArray();
	AddTypeToTypeArray(new Type(TYPE_MATCHALL), text1.typedata._class.parameters);
	Type text2(TYPE_CLASS);
	text2.typedata._class.classname = strdup("Text");
	text2.typedata._class.shadow = 1;
	text2.typedata._class.parameters = MakeTypeArray();
	AddTypeToTypeArray(new Type(TYPE_MATCHALL), text1.typedata._class.parameters);

	boost::optional<Type*> ret = analyzer.getCommonSubtypeOf(&text1, &text2);
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&text1, &text2));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&text2, &text1));
}

BOOST_AUTO_TEST_CASE(TestClassWithParentIsCommonToParent) {
	TypeAnalyzer analyzer;
	ClassSpaceSymbolTable reference;
	analyzer.reference = &reference;
	Type printer(TYPE_CLASS);
	printer.typedata._class.classname = strdup("Printer");
	printer.typedata._class.shadow = 0;
	Type disabledPrinter(TYPE_CLASS);
	disabledPrinter.typedata._class.classname = strdup("DisabledPrinter");
	disabledPrinter.typedata._class.shadow = 1;

	reference.addClass("Printer");
	reference.addClass("DisabledPrinter");
	reference.addInheritance("Printer", true);

	boost::optional<Type*> ret = analyzer.getCommonSubtypeOf(&printer, &disabledPrinter);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE((*ret)->type == TYPE_CLASS);
	BOOST_REQUIRE((*ret)->typedata._class.classname == string("Printer"));
	freeType(*ret);
	ret = analyzer.getCommonSubtypeOf(&disabledPrinter, &printer);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE((*ret)->type == TYPE_CLASS);
	BOOST_REQUIRE((*ret)->typedata._class.classname == string("Printer"));
	freeType(*ret);
}

BOOST_AUTO_TEST_CASE(TestClassesWithRootParentIsRootParent) {
	TypeAnalyzer analyzer;
	ClassSpaceSymbolTable reference;
	analyzer.reference = &reference;
	Type stdErrPrinter(TYPE_CLASS);
	stdErrPrinter.typedata._class.classname = strdup("StdErrPrinter");
	stdErrPrinter.typedata._class.shadow = 0;
	Type disabledPrinter(TYPE_CLASS);
	disabledPrinter.typedata._class.classname = strdup("DisabledPrinter");
	disabledPrinter.typedata._class.shadow = 1;

	reference.addClass("Printer");
	reference.addClass("DisabledPrinter"); reference.addInheritance("Printer", true);
	reference.addClass("StdErrPrinter"); reference.addInheritance("Printer", true);

	boost::optional<Type*> ret = analyzer.getCommonSubtypeOf(&stdErrPrinter, &disabledPrinter);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE((*ret)->type == TYPE_CLASS);
	BOOST_REQUIRE((*ret)->typedata._class.classname == string("Printer"));
	freeType(*ret);
	ret = analyzer.getCommonSubtypeOf(&disabledPrinter, &stdErrPrinter);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE((*ret)->type == TYPE_CLASS);
	BOOST_REQUIRE((*ret)->typedata._class.classname == string("Printer"));
	freeType(*ret);
}

BOOST_AUTO_TEST_CASE(TestClassesWithDifferenntLengthPathsToRootParentIsRootParent) {
	TypeAnalyzer analyzer;
	ClassSpaceSymbolTable reference;
	analyzer.reference = &reference;
	Type stdErrPrinter(TYPE_CLASS);
	stdErrPrinter.typedata._class.classname = strdup("StdErrPrinter");
	stdErrPrinter.typedata._class.shadow = 0;
	Type logger(TYPE_CLASS);
	logger.typedata._class.classname = strdup("Logger");
	logger.typedata._class.shadow = 1;

	reference.addClass("OStream");
	reference.addClass("Printer"); reference.addInheritance("OStream", true);
	reference.addClass("StdErrPrinter"); reference.addInheritance("Printer", true);
	reference.addClass("Logger"); reference.addInheritance("OStream", true);

	boost::optional<Type*> ret = analyzer.getCommonSubtypeOf(&stdErrPrinter, &logger);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE((*ret)->type == TYPE_CLASS);
	BOOST_REQUIRE((*ret)->typedata._class.classname == string("OStream"));
	freeType(*ret);
	ret = analyzer.getCommonSubtypeOf(&logger, &stdErrPrinter);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE((*ret)->type == TYPE_CLASS);
	BOOST_REQUIRE((*ret)->typedata._class.classname == string("OStream"));
	freeType(*ret);
}

BOOST_AUTO_TEST_CASE(TestClassesWithRootRootParentIsRootRootParent) {
	TypeAnalyzer analyzer;
	ClassSpaceSymbolTable reference;
	analyzer.reference = &reference;
	Type stdErrPrinter(TYPE_CLASS);
	stdErrPrinter.typedata._class.classname = strdup("StdErrPrinter");
	stdErrPrinter.typedata._class.shadow = 0;
	Type disabledLogger(TYPE_CLASS);
	disabledLogger.typedata._class.classname = strdup("DisabledLogger");
	disabledLogger.typedata._class.shadow = 1;

	reference.addClass("OStream");
	reference.addClass("Printer"); reference.addInheritance("OStream", true);
	reference.addClass("StdErrPrinter"); reference.addInheritance("Printer", true);
	reference.addClass("Logger"); reference.addInheritance("OStream", true);
	reference.addClass("DisabledLogger"); reference.addInheritance("Logger", true);

	boost::optional<Type*> ret = analyzer.getCommonSubtypeOf(&stdErrPrinter, &disabledLogger);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE((*ret)->type == TYPE_CLASS);
	BOOST_REQUIRE((*ret)->typedata._class.classname == string("OStream"));
	freeType(*ret);
	ret = analyzer.getCommonSubtypeOf(&disabledLogger, &stdErrPrinter);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE((*ret)->type == TYPE_CLASS);
	BOOST_REQUIRE((*ret)->typedata._class.classname == string("OStream"));
	freeType(*ret);
}

BOOST_AUTO_TEST_CASE(TestTwoEquallyViableParentClassesHasNoCommonType) {
	TypeAnalyzer analyzer;
	ClassSpaceSymbolTable reference;
	analyzer.reference = &reference;
	Type printer(TYPE_CLASS);
	printer.typedata._class.classname = strdup("Printer");
	printer.typedata._class.shadow = 0;
	Type logger(TYPE_CLASS);
	logger.typedata._class.classname = strdup("Logger");
	logger.typedata._class.shadow = 1;

	reference.addClass("OStream");
	reference.addClass("Serializable");
	reference.addClass("Printer"); reference.addInheritance("OStream", true); reference.addInheritance("Serializable", false);
	reference.addClass("Logger"); reference.addInheritance("OStream", true); reference.addInheritance("Serializable", false);

	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&printer, &logger));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&logger, &printer));
}

BOOST_AUTO_TEST_CASE(TestTwoDistantEquallyViableParentClassesHasNoCommonType) {
	TypeAnalyzer analyzer;
	ClassSpaceSymbolTable reference;
	analyzer.reference = &reference;
	Type disabledPrinter(TYPE_CLASS);
	disabledPrinter.typedata._class.classname = strdup("DisabledPrinter");
	disabledPrinter.typedata._class.shadow = 0;
	Type stdErrLogger(TYPE_CLASS);
	stdErrLogger.typedata._class.classname = strdup("StdErrLogger");
	stdErrLogger.typedata._class.shadow = 1;

	reference.addClass("OStream");
	reference.addClass("Serializable");
	reference.addClass("Printer"); reference.addInheritance("OStream", true); reference.addInheritance("Serializable", false);
	reference.addClass("Logger"); reference.addInheritance("OStream", true); reference.addInheritance("Serializable", false);
	reference.addClass("DisabledPrinter"); reference.addInheritance("Printer", true);
	reference.addClass("StdErrLogger"); reference.addInheritance("Logger", true);

	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&disabledPrinter, &stdErrLogger));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&stdErrLogger, &disabledPrinter));
}

BOOST_AUTO_TEST_CASE(TestTwoLopsidedEquallyViableParentClassesHasNoCommonType) {
	TypeAnalyzer analyzer;
	ClassSpaceSymbolTable reference;
	analyzer.reference = &reference;
	Type disabledPrinter(TYPE_CLASS);
	disabledPrinter.typedata._class.classname = strdup("DisabledPrinter");
	disabledPrinter.typedata._class.shadow = 0;
	Type logger(TYPE_CLASS);
	logger.typedata._class.classname = strdup("Logger");
	logger.typedata._class.shadow = 1;

	reference.addClass("OStream");
	reference.addClass("Serializable");
	reference.addClass("Printer"); reference.addInheritance("OStream", true); reference.addInheritance("Serializable", false);
	reference.addClass("Logger"); reference.addInheritance("OStream", true); reference.addInheritance("Serializable", false);
	reference.addClass("DisabledPrinter"); reference.addInheritance("Printer", true);

	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&disabledPrinter, &logger));
	BOOST_REQUIRE(!analyzer.getCommonSubtypeOf(&logger, &disabledPrinter));
}

BOOST_AUTO_TEST_CASE(TestDistantEquallyViableParentClassesBelowValidParentClassIsNotAnIssue) {
	TypeAnalyzer analyzer;
	ClassSpaceSymbolTable reference;
	analyzer.reference = &reference;
	Type disabledPrinter(TYPE_CLASS);
	disabledPrinter.typedata._class.classname = strdup("DisabledPrinter");
	disabledPrinter.typedata._class.shadow = 0;
	Type stdErrLogger(TYPE_CLASS);
	stdErrLogger.typedata._class.classname = strdup("StdErrLogger");
	stdErrLogger.typedata._class.shadow = 1;

	reference.addClass("OStream");
	reference.addClass("Serializable");
	reference.addClass("Debuggable");
	reference.addClass("Printer"); reference.addInheritance("OStream", true); reference.addInheritance("Serializable", false);
	reference.addClass("Logger"); reference.addInheritance("OStream", true); reference.addInheritance("Serializable", false);
	reference.addClass("DisabledPrinter"); reference.addInheritance("Printer", true); reference.addInheritance("Debuggable", false);
	reference.addClass("StdErrLogger"); reference.addInheritance("Logger", true); reference.addInheritance("Debuggable", false);

	boost::optional<Type*> ret = analyzer.getCommonSubtypeOf(&stdErrLogger, &disabledPrinter);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE((*ret)->type == TYPE_CLASS);
	BOOST_REQUIRE((*ret)->typedata._class.classname == string("Debuggable"));
	freeType(*ret);
	ret = analyzer.getCommonSubtypeOf(&disabledPrinter, &stdErrLogger);
	BOOST_REQUIRE(ret);
	BOOST_REQUIRE((*ret)->type == TYPE_CLASS);
	BOOST_REQUIRE((*ret)->typedata._class.classname == string("Debuggable"));
	freeType(*ret);
}

BOOST_AUTO_TEST_CASE(TestGetArrayReferenceLevelForClassIs0) {
	TypeAnalyzer analyzer;
	Type clazz(TYPE_CLASS);
	BOOST_REQUIRE(analyzer.getArrayReferenceLevel(clazz) == 0);
}

BOOST_AUTO_TEST_CASE(TestGetArrayReferenceLevelForListIs1) {
	TypeAnalyzer analyzer;
	Type list(TYPE_LIST);
	list.typedata.list.contained = new Type(TYPE_CLASS);
	BOOST_REQUIRE(analyzer.getArrayReferenceLevel(list) == 1);
}

BOOST_AUTO_TEST_CASE(TestGetArrayReferenceLevelForListListIs2) {
	TypeAnalyzer analyzer;
	Type list(TYPE_LIST);
	list.typedata.list.contained = new Type(TYPE_LIST);
	list.typedata.list.contained->typedata.list.contained = new Type(TYPE_CLASS);
	BOOST_REQUIRE(analyzer.getArrayReferenceLevel(list) == 2);
}

BOOST_AUTO_TEST_CASE(TestGetArrayReferenceLevelForListOptionalListIs2) {
	TypeAnalyzer analyzer;
	Type list(TYPE_LIST);
	list.typedata.list.contained = new Type(TYPE_OPTIONAL);
	list.typedata.list.contained->typedata.optional.contained = new Type(TYPE_LIST);
	list.typedata.list.contained->typedata.optional.contained->typedata.list.contained = new Type(TYPE_CLASS);
	BOOST_REQUIRE(analyzer.getArrayReferenceLevel(list) == 2);
}

BOOST_AUTO_TEST_SUITE_END();
