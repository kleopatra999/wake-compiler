/**************************************************
 * Source Code for the Original Compiler for the
 * Programming Language Wake
 *
 * AstCreator.cpp
 *
 * Licensed under the MIT license
 * See LICENSE.TXT for details
 *
 * Author: Michael Fairhurst
 * Revised By:
 *
 **************************************************/

#include "AstCreator.h"
#include "ast/OtherStatement.h"
#include "ast/OtherExpression.h"
#include "ast/Declaration.h"
#include "ast/ExpressionStatement.h"
#include "ast/EmptyStatement.h"
#include "ast/Catch.h"
#include "ast/Return.h"
#include "ast/Throw.h"
#include "ast/ExpressionErrorCatcher.h"
#include "ast/ExpressionUsabilityCatcher.h"
#include "ast/For.h"
#include "ast/IfElseWhile.h"
#include "ast/Exists.h"
#include "ast/ForeachInAliased.h"
#include "ast/ForeachInExplicitType.h"
#include "ast/Retrieval.h"

wake::ast::StatementNode* wake::AstCreator::generateStatementAst(Node* node) {
	switch(node->node_type) {
		case NT_DECLARATION:
			return new wake::ast::Declaration(&node->node_data.nodes[0]->node_data.type, generateExpressionAst(node->node_data.nodes[1], true), node, classestable, scopesymtable, errors, parameterizedtypes);

		case NT_CATCH:
			return new wake::ast::Catch(node->node_data.nodes[0]->node_data.type, generateStatementAst(node->node_data.nodes[1]), node, classestable, scopesymtable, errors);

		case NT_THROW:
			return new wake::ast::Throw(generateExpressionAst(node->node_data.nodes[0], true), classestable->getAnalyzer(), errors);

		case NT_IF_ELSE:
		case NT_WHILE:
			return new wake::ast::IfElseWhile(generateExpressionAst(node->node_data.nodes[0], true), generateStatementAst(node->node_data.nodes[1]), node->subnodes == 3 ? generateStatementAst(node->node_data.nodes[2]) : NULL, classestable->getAnalyzer());

		case NT_FOR:
			return new wake::ast::For(generateStatementAst(node->node_data.nodes[0]), generateExpressionAst(node->node_data.nodes[1], true), generateStatementAst(node->node_data.nodes[2]), generateStatementAst(node->node_data.nodes[3]), scopesymtable, classestable->getAnalyzer());

		case NT_RETURN:
			if(node->subnodes) {
				return new wake::ast::Return(generateExpressionAst(node->node_data.nodes[0], true), returntype, classestable->getAnalyzer());
			} else {
				return new wake::ast::Return(NULL, returntype, classestable->getAnalyzer());
			}

		case NT_EXISTS:
			if(node->node_data.nodes[0]->node_type == NT_MEMBER_ACCESS) {
				errors->addError(new SemanticError(TYPE_ERROR, "Calling exists { } on a property is illegal as it is a shared reference and therefore might be unset amid the scope", node));
				return new wake::ast::EmptyStatement();
			}
			return new wake::ast::Exists(generateExpressionAst(node->node_data.nodes[0], true), generateStatementAst(node->node_data.nodes[1]), node->subnodes == 3 ? generateStatementAst(node->node_data.nodes[2]) : NULL, node, scopesymtable, errors);

		case NT_FOREACH:
			return new wake::ast::Foreach(generateExpressionAst(node->node_data.nodes[0], true), generateStatementAst(node->node_data.nodes[1]), node, scopesymtable, errors);

		case NT_FOREACHIN:
			if(node->node_data.nodes[0]->node_type == NT_ALIAS) {
				return new wake::ast::ForeachInAliased(generateExpressionAst(node->node_data.nodes[1], true), generateStatementAst(node->node_data.nodes[2]), node->node_data.nodes[0]->node_data.string, node, scopesymtable, errors);
			} else {
				return new wake::ast::ForeachInExplicitType(generateExpressionAst(node->node_data.nodes[1], true), generateStatementAst(node->node_data.nodes[2]),  node->node_data.nodes[0]->node_data.type, node, scopesymtable, errors, classestable->getAnalyzer());
			}

		case NT_SWITCH:
		case NT_VALUES:
		case NT_DEFAULTCASE:
		case NT_CASE:
		case NT_BLOCK:
		case NT_TRY:
		case NT_EXPRESSIONS:
		case NT_RETRIEVALS_STATEMENTS:
		case NT_BREAK:
		case NT_CONTINUE:
			{
				vector<wake::ast::StatementNode*> subnodes;

				for(int i = 0; i < node->subnodes; i++) {
					subnodes.push_back(generateStatementAst(node->node_data.nodes[i]));
				}

				return new wake::ast::OtherStatement(node, subnodes, scopesymtable);
			}

		default:
			return new wake::ast::ExpressionStatement(generateExpressionAst(node, false));

	}
}

wake::ast::ExpressionNode* wake::AstCreator::generateExpressionAst(Node* node, bool mustBeUsable) {
	wake::ast::ExpressionNode* created;
	if(node->node_type == NT_RETRIEVAL) {
		std::vector<wake::ast::ExpressionNode*> arguments;
		if(node->node_data.nodes[1]->node_type != NT_EMPTY) {
			for(int i = 0; i < node->node_data.nodes[1]->subnodes; i++) {
				arguments.push_back(generateExpressionAst(node->node_data.nodes[1]->node_data.nodes[i], true));
			}
		}
		wake::ast::ExpressionNode* provider = generateExpressionAst(node->node_data.nodes[2], true);
		created = new wake::ast::Retrieval(provider, node->node_data.nodes[0]->node_data.type, arguments, node, classestable, classestable->getAnalyzer(), errors);
	} else {

		vector<wake::ast::ExpressionNode*> subnodes;

		for(int i = 0; i < node->subnodes; i++) {
			subnodes.push_back(generateExpressionAst(node->node_data.nodes[i], true));
		}

		created = new wake::ast::OtherExpression(node, subnodes, errors, classestable, scopesymtable, methodanalyzer, thiscontext, returntype, parameterizedtypes);
	}

	if(mustBeUsable) {
		return new wake::ast::ExpressionUsabilityCatcher(new wake::ast::ExpressionErrorCatcher(created, node, errors), errors, node);
	} else {
		return new wake::ast::ExpressionErrorCatcher(created, node, errors);
	}

}
