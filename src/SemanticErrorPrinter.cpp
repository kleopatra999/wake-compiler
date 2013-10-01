#include <iostream>
#include "SemanticErrorPrinter.h"

void SemanticErrorPrinter::print(SemanticError* e) {

	cout << "[ ERROR on line " << e->token->line << ", column " << e->token->col << " ]" << endl;

	if(e->context != "") cout << "[ -CONTEXT: " << e->context << " ]" << endl;

	cout << "[ -TYPE: ";
	switch(e->code) {
		case MULTIPLE_CLASS_DEFINITION:
			cout << "Class is defined twice";
			break;
		case SELF_INHERITANCE:
			cout << "Class inherits from itself";
			break;
		case CIRCULAR_INHERITANCE:
			cout << "Circular inheritance formation";
			break;
		case MORE_THAN_ONE_SUBCLASS:
			cout << "Muliple subclassing with 'a/an' is not allowed. Perhaps use 'capable' instead?";
			break;
		case MULTIPLE_INHERITANCE:
			cout << "Class inherits twice from the same class";
			break;
		case MULTIPLE_METHOD_DEFINITION:
			cout << "Multiple methods for one class defined with the same signature";
			break;
		case CLASSNAME_NOT_FOUND:
			cout << "Class by this name not found";
			break;
		case SYMBOL_ALREADY_DEFINED:
			cout << "Variable declaration would overwrite previous. Either use a new alias, or shadow this declaration, or shadow the previous declaration";
			break;
		case SYMBOL_NOT_DEFINED:
			cout << "Variable (by class name, shadow, or alias) is not recognized";
			break;
		case TYPE_ERROR:
			cout << "Invalid type";
			break;
		case PROPERTY_OR_METHOD_NOT_FOUND:
			cout << "Property or method on class is not valid";
			break;
		default:
			cout << "bad error code provided";
	}
	cout << " ]" << endl;

	if(e->msg != "") cout << "[ -MSG: " << e->msg << " ]" << endl;

	cout << endl;
}
