#include "GLBdecl.h"
#include "Log.h"

bonobo::DataType bonobo::Type(types::DataType dt)
{
#	define TYPE_OPERATION(C_TYPE, DATA_TYPE, SIZE, GL_DATA_TYPE) \
		case DATA_TYPE: return (bonobo::DataType) GL_DATA_TYPE;
	switch (dt) {
#		include "Types.inl"
		default: break;
	}
	LogWarning("GL type conversion failed");
	return bonobo::TYPE_INT;
#	undef TYPE_OPERATION
}
