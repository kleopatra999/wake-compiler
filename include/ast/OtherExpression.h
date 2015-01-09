#ifndef HEADER_AST_OTHER_EXPRESSION
#define HEADER_AST_OTHER_EXPRESSION

#include <vector>
#include "ast/ExpressionNode.h"
#include "ErrorTracker.h"
#include "ClassSpaceSymbolTable.h"
#include "ScopeSymbolTable.h"
#include "MethodSignatureParseTreeTraverser.h"
#include "Type.h"
#include "node.h"

namespace wake {

	namespace ast {

		class OtherExpression : public ExpressionNode {

			public:
				OtherExpression(Node* node, std::vector<ExpressionNode*> children, ErrorTracker* errors, ClassSpaceSymbolTable* objectsymtable, ScopeSymbolTable* scopesymtable, MethodSignatureParseTreeTraverser* methodanalyzer, Type* returntype, Type* thiscontext, const vector<Type>& parameterizedtypes)
					: node(node), children(children), errors(errors), objectsymtable(objectsymtable), scopesymtable(scopesymtable), methodanalyzer(methodanalyzer), returntype(returntype), thiscontext(thiscontext), parameterizedtypes(parameterizedtypes);
				Type* typeCheck(bool forceArrayIdentifier);

			private:
				Node* node;
				std::vector<StatementNode*> children;
				MethodSignatureParseTreeTraverser* methodanalyzer;
				ErrorTracker* errors;
				ClassSpaceSymbolTable* classestable;
				ScopeSymbolTable* scopesymtable;
				Type* returntype;
				Type* thiscontext;
				vector<Type*> parameterizedtypes;

		}

	}

}

#endif