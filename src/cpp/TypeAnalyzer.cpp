/**************************************************
 * Source Code for the Original Compiler for the
 * Programming Language Wake
 *
 * TypeAnalyzer.cpp
 *
 * Licensed under the MIT license
 * See LICENSE.TXT for details
 *
 * Author: Michael Fairhurst
 * Revised By:
 *
 **************************************************/

#include "TypeAnalyzer.h"
#include "ClassSpaceSymbolTable.h"
#include "ReadOnlyPropertySymbolTable.h"
#include "CompilationExceptions.h"
#include <memory>

bool TypeAnalyzer::isASubtypeOfB(string a, string b) {
	try {
		if(a == b) return true;

		ReadOnlyPropertySymbolTable* a_data = reference->find(a);

		for(map<string, bool>::const_iterator it = a_data->getParentage().begin(); it != a_data->getParentage().end(); ++it) {
			if(isASubtypeOfB(it->first, b)) return true;
		}

	} catch(SymbolNotFoundException* e) {
		delete e;
	}

	return false;
}

bool TypeAnalyzer::isASubtypeOfB(Type* a, Type* b) {
	//if(a == NULL || b == NULL) return false;
	if(a->type == TYPE_MATCHALL || b->type == TYPE_MATCHALL) return true;
	if(a->type != b->type) {
		if(b->type == TYPE_OPTIONAL) {
			return isASubtypeOfB(a, b->typedata.optional.contained);
		}
		 // TODO: parameterized types need logic here
		return false;
	}

	if(a->type == TYPE_LAMBDA) {

		// if one or the other is a null pointer, or has no args, and the other one has arguments.
		if((a->typedata.lambda.arguments == NULL || !a->typedata.lambda.arguments->typecount) != (b->typedata.lambda.arguments == NULL || !b->typedata.lambda.arguments->typecount))
			return false;

		// Bool -- fn() is a subtype of void -- fn(), since the subtype will simply ignore the returnval
		// however, void --fn() is not a subtype of Bool -- fn() as you probably guessed
		if(a->typedata.lambda.returntype == NULL && b->typedata.lambda.returntype != NULL)
			return false;
		else if(b->typedata.lambda.returntype != NULL && !isASubtypeOfB(a->typedata.lambda.returntype, b->typedata.lambda.returntype)) // ChildClass -- fn() is a subtype of ParentClass -- fn()
			return false;

		if(a->typedata.lambda.arguments != NULL && a->typedata.lambda.arguments->typecount) {
			// A fn taking 3 arguments is not a subtype of a fn taking 2 or 4
			if(a->typedata.lambda.arguments->typecount != b->typedata.lambda.arguments->typecount)
				return false;

			int i;
			for(i = 0; i < a->typedata.lambda.arguments->typecount; i++)
			if(!isASubtypeOfB(b->typedata.lambda.arguments->types[i], a->typedata.lambda.arguments->types[i])) // fn(ParentClass) is a subtype of fn(ChildClass), but not vice versa! (contravariance)
				return false;
		}

		return true;

	} else if(a->type == TYPE_CLASS) {
		// check if one pointer exists and the other is null: !ptr == 0 and !NULL == 1
		if(!a->typedata._class.parameters != !b->typedata._class.parameters) {
			return false;
		}

		if(a->typedata._class.parameters) { // Here if A is not null, neither is B
			int len = a->typedata._class.parameters->typecount;
			if(b->typedata._class.parameters->typecount != len) {
				return false;
			}

			for(int i = 0; i < len; i++)
			if(!isAExactlyB(a->typedata._class.parameters->types[i], b->typedata._class.parameters->types[i]))
				return false;
		}

		if(string(a->typedata._class.classname) == b->typedata._class.classname) {
			return true;
		}

		try {

			ReadOnlyPropertySymbolTable* a_data = reference->find(a->typedata._class.classname);

			for(map<string, bool>::const_iterator it = a_data->getParentage().begin(); it != a_data->getParentage().end(); ++it) {
				if(isASubtypeOfB(it->first, b->typedata._class.classname)) return true;
			}

		} catch(SymbolNotFoundException* e) {
			delete e;
		}

		return false;
	} else if(a->type == TYPE_PARAMETERIZED || a->type == TYPE_PARAMETERIZED_ARG) {
		if(a->typedata.parameterized.label == string(b->typedata.parameterized.label)) return true;
		// TODO: lower/upper bounds comparison
		return false;
	} else if(a->type == TYPE_LIST) {
		return isAExactlyB(a->typedata.list.contained, b->typedata.list.contained);
	} else if(a->type == TYPE_OPTIONAL) {
		return isASubtypeOfB(a->typedata.optional.contained, b->typedata.optional.contained);
	}

}

bool TypeAnalyzer::isAExactlyB(Type* a, Type* b) {
	if(a->type == TYPE_MATCHALL || b->type == TYPE_MATCHALL) return true;
	if(a->type != b->type) return false;
	if(a->type == TYPE_CLASS) {
		if(a->typedata._class.classname != string(b->typedata._class.classname)) return false;
		if(!a->typedata._class.parameters != !b->typedata._class.parameters) return false; // tests both or neither are null. Tests back this up.
		if(a->typedata._class.parameters != NULL) {
			if(a->typedata._class.parameters->typecount != b->typedata._class.parameters->typecount)
				return false;

			for(int i = 0; i < a->typedata._class.parameters->typecount; i++)
			if(!isAExactlyB(a->typedata._class.parameters->types[i], b->typedata._class.parameters->types[i]))
				return false;
		}
	} else if(a->type == TYPE_PARAMETERIZED || a->type == TYPE_PARAMETERIZED_ARG) {
		// no need to check lower/upper bound, should always be the same for all labels
		return a->typedata.parameterized.label == string(b->typedata.parameterized.label);
	} else if(a->type == TYPE_LAMBDA) {
		if(!a->typedata.lambda.returntype != !b->typedata.lambda.returntype) return false; // tests both or neither are null. Tests back this up
		if(!a->typedata.lambda.arguments != !b->typedata.lambda.arguments) return false; // tests both or neither are null. Tests back this up
		if(a->typedata.lambda.returntype != NULL && !isAExactlyB(a->typedata.lambda.returntype, b->typedata.lambda.returntype)) return false;
		if(a->typedata.lambda.arguments != NULL) {
			if(a->typedata.lambda.arguments->typecount != b->typedata.lambda.arguments->typecount)
				return false;

			for(int i = 0; i < a->typedata.lambda.arguments->typecount; i++)
			if(!isAExactlyB(a->typedata.lambda.arguments->types[i], b->typedata.lambda.arguments->types[i]))
				return false;
		}
	} else if(a->type == TYPE_LIST) {
		return isAExactlyB(a->typedata.list.contained, b->typedata.list.contained);
	} else if(a->type == TYPE_OPTIONAL) {
		return isAExactlyB(a->typedata.optional.contained, b->typedata.optional.contained);
	}
	return true;
}

void TypeAnalyzer::assertNeedIsNotCircular(string classname, Type* need) {
	if(need->type != TYPE_CLASS) return;
	if(need->typedata._class.classname == classname)
		throw new SemanticError(CIRCULAR_DEPENDENCIES, "Created by the need for class " + getNameForType(need));

	vector<Type*>* recurse = reference->find(need->typedata._class.classname)->getNeeds();
	for(vector<Type*>::iterator it = recurse->begin(); it != recurse->end(); ++it)
	try {
		assertNeedIsNotCircular(classname, *it);
	} catch(SemanticError* e) {
		e->msg += ", as required by " + getNameForType(need);
		throw e;
	}
}

void TypeAnalyzer::assertClassCanBeBound(Type* binding) {
	ReadOnlyPropertySymbolTable* bound = reference->find(binding->typedata._class.classname);
	if(bound->isAbstract())
		throw new SemanticError(ABSTRACT_PROVISION);
}

void TypeAnalyzer::assertClassCanProvide(string provider, Type* binding) {
	string symname = getNameForType(binding) + "<-";
	if(binding->specialty != NULL) symname += binding->specialty;
	if(!reference->find(provider)->find(symname))
		throw new SemanticError(PROPERTY_OR_METHOD_NOT_FOUND, symname + " not found on class " + provider);

	if(reference->findModifiable(provider)->isBehavioralProvision(symname)) return;

	vector<Type*>* recurse = reference->find(binding->typedata._class.classname)->getNeeds();
	for(vector<Type*>::iterator it = recurse->begin(); it != recurse->end(); ++it)
	try {
		assertClassCanProvide(provider, *it);
	} catch(SemanticError* e) {
		e->msg += ", when trying to provide " + symname;
		throw e;
	}
}

void TypeAnalyzer::assertClassCanProvide(Type* provider, Type* binding) {
	assertClassCanProvide(string(provider->typedata._class.classname), binding);
}

boost::optional<Type*> TypeAnalyzer::getCommonSubtypeOf(Type* a, Type* b) {
	if(a->type == TYPE_UNUSABLE || b->type == TYPE_UNUSABLE) return boost::optional<Type*>(); // this maybe should return TYPE_UNUSABLE...but unusable types aren't necessarily similar at all
	if(a->type == TYPE_MATCHALL) return boost::optional<Type*>(new Type(*b));
	if(b->type == TYPE_MATCHALL) return boost::optional<Type*>(new Type(*a));

	if(a->type == TYPE_OPTIONAL || b->type == TYPE_OPTIONAL) {
		Type* optional = a->type == TYPE_OPTIONAL ? a : b;
		Type* other = a == optional ? b : a;
		if(other->type == TYPE_OPTIONAL) other = other->typedata.optional.contained;

		// [nothing, Text?] is common to type Text?
		// [X, Y?] is common to type Z? where Z is the common type to [X, Y]
		boost::optional<Type*> nonoptcommon = getCommonSubtypeOf(optional->typedata.optional.contained, other);

		if(!nonoptcommon) {
			return nonoptcommon;
		}

		Type* common = new Type(TYPE_OPTIONAL);

		// [Text?, Text] becomes Text?
		common->typedata.optional.contained = *nonoptcommon;
		return boost::optional<Type*>(common);
	}

	if(a->type != b->type) return boost::optional<Type*>();

	if(a->type == TYPE_LIST) {
		// [Text[], Num[]] and [Printer[], DisabledPrinter[]] are both common to nothing
		if(!isAExactlyB(a, b)) return boost::optional<Type*>();
		// but [Text[], Text[]] is common to Text[]
		return boost::optional<Type*>(new Type(*a));

	} else if(a->type == TYPE_CLASS) {
		// check if one pointer exists and the other is null: !ptr == 0 and !NULL == 1
		if(!a->typedata._class.parameters != !b->typedata._class.parameters) {
			return boost::optional<Type*>();
		}

		if(a->typedata._class.parameters) { // Here if A is not null, neither is B
			int len = a->typedata._class.parameters->typecount;
			if(b->typedata._class.parameters->typecount != len) {
				return boost::optional<Type*>();
			}

			for(int i = 0; i < len; i++)
			if(!isAExactlyB(a->typedata._class.parameters->types[i], b->typedata._class.parameters->types[i]))
				return boost::optional<Type*>();
		}

		if(isASubtypeOfB(a, b)) return boost::optional<Type*>(new Type(*b));

		if(isASubtypeOfB(b, a)) return boost::optional<Type*>(new Type(*a));

		boost::optional<pair<int, Type*> > common = getCommonClassnamesWithDepth(*a, *b, 0);
		if(!common || common->second == NULL) {
			return boost::optional<Type*>();
		} else {
			return boost::optional<Type*>(common->second);
		}
	} else if(a->type == TYPE_PARAMETERIZED || a->type == TYPE_PARAMETERIZED_ARG) {
		if(a->typedata.parameterized.label == string(b->typedata.parameterized.label)) return boost::optional<Type*>(new Type(*a));
		// TODO: lower/upper bounds comparison
		return boost::optional<Type*>();
	}

	if(a->type == TYPE_LAMBDA) {

		// if one or the other is a null, and the other one has arguments
		if((a->typedata.lambda.arguments == NULL || !a->typedata.lambda.arguments->typecount) != (b->typedata.lambda.arguments == NULL || !b->typedata.lambda.arguments->typecount))
			return boost::optional<Type*>();

		if(a->typedata.lambda.arguments != NULL && a->typedata.lambda.arguments->typecount) {
			// A fn taking 3 arguments is not a subtype of a fn taking 2 or 4
			if(a->typedata.lambda.arguments->typecount != b->typedata.lambda.arguments->typecount)
				return boost::optional<Type*>();

			int i;
			for(i = 0; i < a->typedata.lambda.arguments->typecount; i++)
			if(!isAExactlyB(a->typedata.lambda.arguments->types[i], b->typedata.lambda.arguments->types[i]))
				return boost::optional<Type*>();
		}

		// Bool -- fn() is a subtype of void -- fn(), since the subtype will simply ignore the returnval
		// however, void --fn() is not a subtype of Bool -- fn() as you probably guessed
		if(a->typedata.lambda.returntype == NULL || b->typedata.lambda.returntype == NULL) {
			Type* newlambda = new Type(a);

			if(newlambda->typedata.lambda.returntype) {
				delete newlambda->typedata.lambda.returntype;
			}

			newlambda->typedata.lambda.returntype = NULL;
			return boost::optional<Type*>(newlambda);
		}


		boost::optional<Type*> commonReturn = getCommonSubtypeOf(a->typedata.lambda.returntype, b->typedata.lambda.returntype);
		if(!commonReturn) {
			Type* newlambda = new Type(a);
			delete newlambda->typedata.lambda.returntype;
			newlambda->typedata.lambda.returntype = NULL;
			return boost::optional<Type*>(newlambda);
		}

		Type* newlambda = new Type(a);
		delete newlambda->typedata.lambda.returntype;
		newlambda->typedata.lambda.returntype = *commonReturn;;
		return boost::optional<Type*>(newlambda);

	}
}

boost::optional<pair<int, Type*> > TypeAnalyzer::getCommonClassnamesWithDepth(Type& a, Type& b, int depth) {
	try {
		ReadOnlyPropertySymbolTable* a_data = reference->find(a.typedata._class.classname);

		boost::optional<boost::optional<pair<int, Type*> > > contender;
		bool duplicated = false;
		for(map<string, bool>::const_iterator it = a_data->getParentage().begin(); it != a_data->getParentage().end(); ++it) {
			Type lowered(TYPE_CLASS);
			lowered.typedata._class.classname = strdup(it->first.c_str());
			if(it->first == b.typedata._class.classname) {
				if(contender && (*contender)->first == depth) {
					//cout << "Checking " << a.typedata._class.classname << "/" << b.typedata._class.classname << ", " << depth;
					//cout << " is duped!" << endl;
					return boost::optional<pair<int, Type*> >(pair<int, Type*>(depth, NULL));
				}

				boost::optional<pair<int, Type*> > found(pair<int, Type*>(depth, new Type(lowered)));
				contender = boost::optional<boost::optional<pair<int, Type*> > >(found);
				duplicated = false;
			} else {
				boost::optional<pair<int, Type*> > recurse = getCommonClassnamesWithDepth(b, lowered, depth + 1);

				if(recurse) {
					if(contender && (*contender)->first == recurse->first) {
						if((*contender)->second && recurse->second && (*contender)->second->typedata._class.classname != string(recurse->second->typedata._class.classname)) {
							duplicated = true;
						}
					} else {
						if(contender && (*contender)->first > recurse->first) {
							delete (*contender)->second;
						} else if(!contender || recurse->second) {
							contender = boost::optional<boost::optional<pair<int, Type*> > >(recurse);
						}
						if(recurse->second) duplicated = false;
					}
				}

				recurse = getCommonClassnamesWithDepth(lowered, b, depth + 1);

				if(recurse) {
					if(contender && (*contender)->first == recurse->first) {
						if((*contender)->second && recurse->second && (*contender)->second->typedata._class.classname != string(recurse->second->typedata._class.classname)) {
							duplicated = true;
						}
					} else {
						if(contender && (*contender)->first > recurse->first) {
							delete (*contender)->second;
						} else if(!contender || recurse->second) {
							contender = boost::optional<boost::optional<pair<int, Type*> > >(recurse);
						}
						if(recurse->second) duplicated = false;
					}
				}
			}
		}
		//cout << "Checking " << a.typedata._class.classname << "/" << b.typedata._class.classname << ", " << depth;

		if(duplicated) {
			//cout << " has dupe parents!" << endl;
			delete (*contender)->second;
			return boost::optional<pair<int, Type*> >(pair<int, Type*>(depth, NULL));
		}

		if(!contender) {
			//cout << " has no child matches!" << endl;
			return boost::optional<pair<int, Type*> >();
		}
		//cout << " has match " << (*contender)->first << ", " << ((*contender)->second ? (*contender)->second->typedata._class.classname : "NULL") << "!" << endl;
		return *contender;
	} catch(SymbolNotFoundException* e) {
		delete e;
	}
	return boost::optional<pair<int, Type*> >();
}

bool TypeAnalyzer::isPrimitiveTypeNum(Type* type) {
	if(type->type == TYPE_MATCHALL) return true;
	return type->type == TYPE_CLASS && type->typedata._class.classname == string("Num");
}

bool TypeAnalyzer::isPrimitiveTypeText(Type* type) {
	if(type->type == TYPE_MATCHALL) return true;
	return type->type == TYPE_CLASS && type->typedata._class.classname == string("Text");
}

bool TypeAnalyzer::isPrimitiveTypeBool(Type* type) {
	if(type->type == TYPE_MATCHALL) return true;
	return type->type == TYPE_CLASS && type->typedata._class.classname == string("Bool");
}

bool TypeAnalyzer::isAutoboxedType(Type* type, Type** boxed) {
	if(type->type == TYPE_MATCHALL) return false;

	// shouldn't be here...throw exception?
	if(type->type == TYPE_UNUSABLE) return false;

	// need to return a List<T>
	if(type->type == TYPE_LIST) {
		*boxed = MakeType(TYPE_CLASS);
		Type* loweredtype = new Type(*type->typedata.list.contained);
		(*boxed)->typedata._class.classname = strdup("List");
		(*boxed)->typedata._class.parameters = MakeTypeArray();
		AddTypeToTypeArray(loweredtype, (*boxed)->typedata._class.parameters);
		return true;
	}

	// need to return a generic autoboxed type according to arguments/return
	if(type->type == TYPE_LAMBDA) {
		*boxed = MakeType(TYPE_CLASS);
		(*boxed)->typedata._class.classname = strdup("lambda");
		return true;
	}

	// shouldn't ever get here,..unless I decide to make, say, T?.exists() and/or T?.applyIfExists(fn(T))
	if(type->type == TYPE_OPTIONAL) return false;

	if(isPrimitiveTypeBool(type)) {
		*boxed = MakeType(TYPE_CLASS);
		(*boxed)->typedata._class.classname = strdup("Bool");
		return true;
	}

	if(isPrimitiveTypeText(type)) {
		*boxed = MakeType(TYPE_CLASS);
		(*boxed)->typedata._class.classname = strdup("Text");
		return true;
	}

	if(isPrimitiveTypeNum(type)) {
		*boxed = MakeType(TYPE_CLASS);
		(*boxed)->typedata._class.classname = strdup("Num");
		return true;
	}

	return false;
}

bool TypeAnalyzer::isException(Type* type) {
	Type* exception = MakeType(TYPE_CLASS);
	exception->typedata._class.classname = strdup("Exception");
	bool ret = isASubtypeOfB(type, exception);
	freeType(exception);
	return ret;
}

string TypeAnalyzer::getNameForType(Type* type) {
	string name;

	if(type == NULL || type->type == TYPE_UNUSABLE) {
		name = "VOID";
		return name;
	}

	if(type->type == TYPE_PARAMETERIZED || type->type == TYPE_PARAMETERIZED_ARG) {
		name = type->typedata.parameterized.label;
	}

	if(type->type == TYPE_MATCHALL) {
		return "{inferencing failed}";
	}

	if(type->type == TYPE_CLASS) {
		name = type->typedata._class.classname;
		if(type->typedata._class.parameters != NULL) {
			name += "{";
			int i;
			for(i = 0; i < type->typedata._class.parameters->typecount; i++) {
				if(i) name += ",";
				name += getNameForType(type->typedata._class.parameters->types[i]);
			}
			name += "}";
		}
	} else if(type->type == TYPE_LAMBDA) {
		name = getNameForType(type->typedata.lambda.returntype);
		name += "--(";

		if(type->typedata.lambda.arguments != NULL) {
			int i;
			for(i = 0; i < type->typedata.lambda.arguments->typecount; i++) {
				if(i) name += ",";
				name += getNameForType(type->typedata.lambda.arguments->types[i]);
			}
		}
		name += ")";
	}

	if(type->type == TYPE_LIST) {
		name = getNameForType(type->typedata.list.contained);
		name += "[]";
	}

	if(type->type == TYPE_OPTIONAL) {
		name = getNameForType(type->typedata.optional.contained);
		name += "?";
	}

	return name;
}

string TypeAnalyzer::getNameForTypeAsProperty(Type* type) {
	if(type->alias != NULL) {
		return type->alias;
	} else {
		if(type->type == TYPE_CLASS) {
			return string(type->typedata._class.shadow, '$') + type->typedata._class.classname;
		} else if(type->type == TYPE_LIST) {
			Type* noList_noOpt = type->typedata.list.contained;

			while(noList_noOpt->type == TYPE_LIST || noList_noOpt->type == TYPE_OPTIONAL) {
				noList_noOpt = noList_noOpt->type == TYPE_LIST ? noList_noOpt->typedata.list.contained : noList_noOpt->typedata.optional.contained;
			}

			return getNameForTypeAsProperty(noList_noOpt) + "[]";
		} else if(type->type == TYPE_OPTIONAL) {
			return getNameForTypeAsProperty(type->typedata.optional.contained);
		} else if(type->type == TYPE_PARAMETERIZED || type->type == TYPE_PARAMETERIZED_ARG) {
			return string(type->typedata.parameterized.shadow, '$') + type->typedata.parameterized.label;
		}
	}
}

string TypeAnalyzer::getProvisionSymbol(Type* provided, vector<Type*> &arguments) {
	string name = getNameForType(provided);
	if(arguments.size()) {
		name += "(";
		for(vector<Type*>::iterator it = arguments.begin(); it != arguments.end(); ++it) {
			if(it != arguments.begin()) name += ",";
			name += getNameForType(*it);
		}
		name += ")";
	}
	name += "<-";
	if(provided->specialty != NULL) name += provided->specialty;
	return name;
}

int TypeAnalyzer::getArrayReferenceLevel(Type& type) {
	if(type.type == TYPE_LIST) return 1 + getArrayReferenceLevel(*type.typedata.list.contained);
	if(type.type == TYPE_OPTIONAL) return getArrayReferenceLevel(*type.typedata.optional.contained);
	return 0;
}

bool TypeAnalyzer::hasArgParameterization(Type* type) {
	if(type == NULL) return false;
	switch(type->type) {
		case TYPE_OPTIONAL:
			return hasArgParameterization(type->typedata.optional.contained);

		case TYPE_LIST:
			return hasArgParameterization(type->typedata.list.contained);

		case TYPE_PARAMETERIZED_ARG:
			return true;

		case TYPE_CLASS:
			return hasArgParameterization(type->typedata._class.parameters);

		case TYPE_LAMBDA:
			if(hasArgParameterization(type->typedata.lambda.arguments)) {
				return true;
			}
			return type->typedata.lambda.returntype != NULL && hasArgParameterization(type->typedata.lambda.returntype);

		default:
			return false;
	}
}

bool TypeAnalyzer::hasArgParameterization(TypeArray* typearray) {
	if(typearray == NULL) {
		return false;
	}

	for(int i = 0; i < typearray->typecount; i++)
	if(hasArgParameterization(typearray->types[i])) {
		return true;
	}

	return false;
}
