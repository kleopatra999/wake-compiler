#include "PropertySymbolTable.h"
#include "SemanticError.h"
#include "DerivedPropertySymbolTable.h"
#include "TypeParameterizer.h"

PropertySymbolTable::PropertySymbolTable(TypeAnalyzer* analyzer) {
	this->analyzer = analyzer;
	abstract = false;
	declaredtypeparameters = new vector<Type*>(); // @TODO this is a hack
}

const map<string, bool>& PropertySymbolTable::getParentage() {
	return parentage;
}

boost::optional<SemanticError*> PropertySymbolTable::addMethod(Type* returntype, vector<pair<string, TypeArray*> >* segments_arguments, int flags) {
	string name = getSymbolNameOf(segments_arguments);

	if(properties.count(name)) {
		string temp = "duplicate method signature is " + name;
		return boost::optional<SemanticError*>(new SemanticError(MULTIPLE_METHOD_DEFINITION, temp));
	}

	if(flags & PROPERTY_ABSTRACT) {
		abstract = true;
	}

	Type* method = MakeType(TYPE_LAMBDA);
	method->typedata.lambda.returntype = copyType(returntype);

	TypeArray* conglomerate = MakeTypeArray();
	for(vector<pair<string, TypeArray*> >::iterator it = segments_arguments->begin(); it != segments_arguments->end(); ++it) {
		int i;
		for(i = 0; i < it->second->typecount; i++)
			AddTypeToTypeArray(copyType(it->second->types[i]), conglomerate);
	}

	method->typedata.lambda.arguments = conglomerate;

	ObjectProperty* prop = new ObjectProperty;
	prop->flags = flags;
	prop->type = method;
	prop->casing = getCasingNameOf(segments_arguments);
	prop->address = name;

	properties[name] = prop;

	return boost::optional<SemanticError*>();
}

boost::optional<SemanticError*> PropertySymbolTable::addProperty(Type* property, int flags) {
	string name = analyzer->getNameForTypeAsProperty(property);

	if(properties.count(name)) {
		string temp = "duplicate property name is " + name;
		return boost::optional<SemanticError*>(new SemanticError(MULTIPLE_PROVISION_DEFINITION, temp));
	}

	ObjectProperty* prop = new ObjectProperty;
	prop->casing = name;
	prop->flags = flags;
	prop->type = property;

	properties[name] = prop;
	return boost::optional<SemanticError*>();
}

boost::optional<SemanticError*> PropertySymbolTable::addProvision(Type* provided) {
	string name = analyzer->getNameForType(provided) + "<-";
	if(provided->specialty != NULL) name += provided->specialty;

	if(properties.count(name)) {
		string temp = "duplicate provisoning is " + name;
		return boost::optional<SemanticError*>(new SemanticError(MULTIPLE_PROVISION_DEFINITION, temp));
	}

	Type* method = MakeType(TYPE_LAMBDA);
	method->typedata.lambda.returntype = copyType(provided);

	method->typedata.lambda.arguments = MakeTypeArray(); //TODO injections with curried ctors or arguments!

	ObjectProperty* prop = new ObjectProperty;
	prop->type = method;
	prop->casing = name;

	properties[name] = prop;
	return boost::optional<SemanticError*>();
}

boost::optional<Type*> PropertySymbolTable::find(string name) {
	if(!properties.count(name))
		//throw new SemanticError(PROPERTY_OR_METHOD_NOT_FOUND, "Cannot find method with signature " + name);
		return boost::optional<Type*>();

	return boost::optional<Type*>(properties.find(name)->second->type);
}

string PropertySymbolTable::getAddress(string name) {
	return properties.find(name)->second->address;
}

string PropertySymbolTable::getProvisionSymbol(Type* provided) {
	string name = analyzer->getNameForType(provided) + "<-";
	if(provided->specialty != NULL) name += provided->specialty;
	return name;
}

string PropertySymbolTable::getCasingNameOf(vector<pair<string, TypeArray*> >* segments_arguments) {
	string name;
	for(vector<pair<string, TypeArray*> >::iterator it = segments_arguments->begin(); it != segments_arguments->end(); ++it) {
		name += it->first;
		name += "(";
		int i;
		for(i = 0; i < it->second->typecount; i++) {
			if(i) name += ",";
			name += '#';
		}
		name += ")";
	}

	return name;
}

string PropertySymbolTable::getSymbolNameOf(vector<pair<string, TypeArray*> >* segments_arguments) {
	string name;
	for(vector<pair<string, TypeArray*> >::iterator it = segments_arguments->begin(); it != segments_arguments->end(); ++it) {
		name += it->first;
		name += "(";
		int i;
		for(i = 0; i < it->second->typecount; i++) {
			if(i) name += ",";
			name += analyzer->getNameForType(it->second->types[i]);
		}
		name += ")";
	}

	return name;
}

void PropertySymbolTable::printEntryPoints(EntryPointAnalyzer* entryanalyzer) {
	for(map<string, ObjectProperty*>::iterator it = properties.begin(); it != properties.end(); ++it) {
		if(entryanalyzer->checkMethodCanBeMain(it->first, it->second->type))
			entryanalyzer->printMethod(it->first);
	}
}

void PropertySymbolTable::addNeed(Type* needed) {
	needs.push_back(needed);
}

vector<Type*>* PropertySymbolTable::getNeeds() {
	return &needs;
}

bool PropertySymbolTable::isAbstract() {
	return abstract;
}

PropertySymbolTable::~PropertySymbolTable() {
	for(map<string, ObjectProperty*>::iterator it = properties.begin(); it != properties.end(); ++it) {
		freeType(it->second->type);
		delete it->second;
	}
}

void propagateInheritanceTables(PropertySymbolTable* child, PropertySymbolTable* parent, bool extend) {
	for(map<string, ObjectProperty*>::iterator it = parent->properties.begin(); it != parent->properties.end(); ++it) {
		map<string, ObjectProperty*>::iterator searcher = child->properties.find(it->first);
		if(searcher == child->properties.end()) {
			ObjectProperty* propagate = new ObjectProperty;
			propagate->type = copyType(it->second->type);
			propagate->flags = it->second->flags;
			propagate->casing = it->second->casing;
			propagate->address = it->second->address;
			if(!extend) {
				propagate->flags |= PROPERTY_ABSTRACT;
				child->abstract = true;
			} else if(propagate->flags & PROPERTY_ABSTRACT) {
				child->abstract = true;
			}
			child->properties[it->first] = propagate;
		} else {
			searcher->second->address = it->second->address;
		}
	}
}

bool PropertySymbolTable::isPublic(string name) {
	return properties.find(name)->second->flags & PROPERTY_PUBLIC;
}

void PropertySymbolTable::setParameters(vector<Type*>* parameters) {
	declaredtypeparameters = parameters;
}

const vector<Type*>& PropertySymbolTable::getParameters() {
	return *declaredtypeparameters;
}

ReadOnlyPropertySymbolTable* PropertySymbolTable::resolveParameters(vector<Type*>& parameters) {
	TypeParameterizer parameterizer;
	map<string, ObjectProperty*>* newprops = new map<string, ObjectProperty*>();
	for(map<string, ObjectProperty*>::iterator it = properties.begin(); it != properties.end(); ++it) {
		string casing, oldcasing = it->second->casing;
		ObjectProperty* newprop = new ObjectProperty();
		newprop->type = copyType(it->second->type);
		newprop->flags = it->second->flags;
		parameterizer.applyParameterizations(&newprop->type, getParameters(), parameters);
		int i = 0, c = 0;
		while(c < oldcasing.length()) {
			if(oldcasing.at(c) == '#') {
				casing += analyzer->getNameForType(newprop->type->typedata.lambda.arguments->types[i]);
				i++;
			} else {
				casing += oldcasing.at(c);
			}
			c++;
		}
		newprop->casing = casing;
		(*newprops)[casing] = newprop;
	}
	return new DerivedPropertySymbolTable(*analyzer, needs, *newprops, parentage);
	return this;
}

Type* PropertySymbolTable::getAsType() {
	Type* ret = MakeType(TYPE_CLASS);
	ret->typedata._class.classname = strdup(classname.c_str());
	if(getParameters().size()) {
		ret->typedata._class.parameters = MakeTypeArray();
		for(auto it = getParameters().begin(); it != getParameters().end(); ++it) {
			AddTypeToTypeArray(*it, ret->typedata._class.parameters);
		}
	}

	return ret;
}
