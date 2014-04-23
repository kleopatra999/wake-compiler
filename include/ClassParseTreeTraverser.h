#ifndef HEADER_CLASS_PARSE_TREE_TRAVERSER
#define HEADER_CLASS_PARSE_TREE_TRAVERSER

#include <string>
#include <vector>
#include <utility>
#include "ClassSpaceSymbolTable.h"
#include "ScopeSymbolTable.h"
#include "SemanticError.h"
#include "ErrorTracker.h"
#include "TypeChecker.h"
#include "MethodSignatureParseTreeTraverser.h"

extern "C" {
	#include "type.h"
	#include "node.h"
	#include "tree.h"
}

using namespace std;

class ClassParseTreeTraverser {
	public:
		ClassParseTreeTraverser(ErrorTracker* errors, ClassSpaceSymbolTable* objectsymtable, ScopeSymbolTable* scopesymtable, string classname, vector<Type*> parameterizedtypes, TypeChecker* typechecker, MethodSignatureParseTreeTraverser* methodanalyzer, PropertySymbolTable* propertysymtable);
		void firstPass(Node* tree);
		void secondPass(Node* tree);

	private:
		void checkCtorArgs(Node* tree);
		void loadCtorArgs(Node* tree);
		void typeCheckProperties(Node* tree);
		void typeCheckMethods(Node* tree);
		Node* getMethodBody(Node* methoddef);
		Type* getMethodReturn(Node* methoddef);
		vector<pair<string, TypeArray*> >* getMethodName(Node* methoddef);

		string classname;
		vector<Type*> parameterizedtypes;

		MethodSignatureParseTreeTraverser* methodanalyzer;
		TypeChecker* typechecker;
		ErrorTracker* errors;
		ScopeSymbolTable* scopesymtable;
		ClassSpaceSymbolTable* classestable;
		PropertySymbolTable* propertysymtable;
};

#endif
