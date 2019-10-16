#include "DBEntity_VariableType.h"

namespace NS_VariableType
{

  VariableType FromString(std::string& type)
  {
    if (type == "char")
    {
      return VT_CHAR;
    }
    else if (type == "bool")
    {
      return VT_BOOLEAN;
    }
    else if (type == "int")
    {
      return VT_INTEGER;
    }
    else if (type == "long")
    {
      return VT_LONG;
    }
    else if (type == "float")
    {
      return VT_FLOAT;
    }
    else if (type == "double")
    {
      return VT_DOUBLE;
    }
    else if (type == "float_complex")
    {
      return VT_FLOAT_COMPLEX;
    }
    else if (type == "double_complex")
    {
      return VT_DOUBLE_COMPLEX;
    }
    else if (type == "charptr")
    {
      return VT_CHARPTR;
    }

    return VT_UNKNOWN_TYPE;
  }

  std::string ToString(VariableType type)
  {
    switch (type)
    {
      case VT_CHAR:
        return "char";
        break;
      case VT_BOOLEAN:
        return "bool";
        break;
      case VT_INTEGER:
        return "int";
        break;
      case VT_LONG:
        return "long";
        break;
      case VT_FLOAT:
        return "float";
        break;
      case VT_DOUBLE:
        return "double";
        break;
      case VT_FLOAT_COMPLEX:
        return "float_complex";
        break;
      case VT_DOUBLE_COMPLEX:
        return "double_complex";
        break;
      case VT_CHARPTR:
        return "charptr";
        break;
      default:;
    }
    return "unknown";
  }

  bool IsComparableTypes(VariableType type1, VariableType type2)
  {
    if (type1 == type2)
    {
      return true;
    }

    if (
      (type1 == VT_FLOAT || type1 == VT_FLOAT_COMPLEX || type1 == VT_DOUBLE || type1 == VT_DOUBLE_COMPLEX)
      &&
      (type2 == VT_FLOAT || type2 == VT_FLOAT_COMPLEX || type2 == VT_DOUBLE || type2 == VT_DOUBLE_COMPLEX)
      )
    {
      return true;
    }

    if (
      (type1 == VT_LONG || type1 == VT_INTEGER)
      &&
      (type2 == VT_LONG || type2 == VT_INTEGER)
      )
    {
      return true;
    }

    return false;
  }

}
