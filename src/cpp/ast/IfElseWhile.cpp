#include "ast/IfElseWhile.h"
#include "TypeError.h"
#include <memory>

void wake::ast::IfElseWhile::typeCheck() {
	auto_ptr<Type*> conditionType(typeCheckUsable(tree->node_data.nodes[0], forceArrayIdentifier));
	delete block->typeCheck();

	if(otherwise != NULL) {
		otherwise->typeCheck();
	}

	if(!analyzer->isPrimitiveTypeNum(conditionType.get()) && !analyzer->isPrimitiveTypeBool(conditionType.get())) {
		EXPECTED	"Bool";
		ERRONEOUS	analyzer->getNameForType(conditionType.get());
		THROW		("If/While conditions must be Bool or Num");
	}
}
