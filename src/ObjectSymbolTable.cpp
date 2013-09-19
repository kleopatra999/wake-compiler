#include "ObjectSymbolTable.h"
#include <algorithm>
#include "CompilationExceptions.h"

ObjectSymbolTable::~ObjectSymbolTable() {
	for(map<string, PropertySymbolTable*>::iterator it = classes.begin(); it != classes.end(); ++it) {
		delete it->second;
	}
}

bool ObjectSymbolTable::isASubtypeOfB(string a, string b) {
	try {
		if(a == b) return true;

		PropertySymbolTable* a_data = find(a);

		for(map<string, bool>::iterator it = a_data->parentage.begin(); it != a_data->parentage.end(); ++it) {
			if(isASubtypeOfB(it->first, b)) return true;
		}

	} catch(SymbolNotFoundException* e) {
		delete e;
	}

	return false;
}

void ObjectSymbolTable::addClass(string name) {
	addingclass_name = name;
	addingclass_symbol = new PropertySymbolTable();
	addingclass_hassubclass = false;

	if(classes.count(addingclass_name)) {
		throw new SemanticError(MULTIPLE_CLASS_DEFINITION);
	}

	classes[addingclass_name] = addingclass_symbol;
}

void ObjectSymbolTable::addInheritance(string childname, bool as_subclass) {
	if(addingclass_hassubclass && as_subclass) {
		throw new SemanticError(MORE_THAN_ONE_SUBCLASS);
	}

	if(childname == addingclass_name) {
		throw new SemanticError(SELF_INHERITANCE);
	}

	if(isASubtypeOfB(childname, addingclass_name)) {
		throw new SemanticError(CIRCULAR_INHERITANCE);
	}

	if(addingclass_symbol->parentage.count(childname)) {
		throw new SemanticError(MULTIPLE_INHERITANCE);
	}

	addingclass_hassubclass = addingclass_symbol->parentage[childname] = as_subclass;
}

PropertySymbolTable* ObjectSymbolTable::find(string name) {
	std::map<string,PropertySymbolTable*>::iterator searcher = classes.find(name);
	if(!classes.count(name)) {
		SymbolNotFoundException* error = new SymbolNotFoundException();
		error->errormsg = "Could not find symbol ";
		error->errormsg += name;
		error->errormsg += " in object table";
		throw error;
	}

	return searcher->second;
}

void ObjectSymbolTable::assertTypeIsValid(Type* type) {
	if(type->type == TYPE_CLASS) {
		if(classes.count(type->typedata._class.classname)) return;
		throw new SemanticError(CLASSNAME_NOT_FOUND);
	} else {
		if(type->typedata.lambda.returntype != NULL)
			assertTypeIsValid(type->typedata.lambda.returntype);
		if(type->typedata.lambda.arguments == NULL) return;

		int i;
		for(i = 0; i < type->typedata.lambda.arguments->typecount; i++)
			assertTypeIsValid(type->typedata.lambda.arguments->types[i]);
	}
}
