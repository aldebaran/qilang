namespace qilang {

namespace detail {

std::string builtinTypeToCpp(BuiltinType type, bool constref);

}

template <typename T>
CppTypeFormatter<T>::CppTypeFormatter()
{
}

template <typename T>
CppTypeFormatter<T>::CppTypeFormatter(std::stringstream& ss, int indent)
  : T(ss, indent)
{
}

template <typename T>
void CppTypeFormatter<T>::unconstify(TypeExprNodePtr node) {
  ScopedFormatAttrBlock _(constattr);
  this->accept(node);
}

template <typename T>
void CppTypeFormatter<T>::constify(const TypeExprNodePtr& node) {
  ScopedFormatAttrActivate _(constattr);
  this->accept(node);
}

template <typename T>
void CppTypeFormatter<T>::visitTypeExpr(BuiltinTypeExprNode* node) {
  this->out() << detail::builtinTypeToCpp(node->builtinType, constattr.isActive());
}

template <typename T>
void CppTypeFormatter<T>::visitTypeExpr(CustomTypeExprNode* node) {

  std::string ns = formatNs(node->resolved_package);

  this->out() << constattr("const ");
  if (!ns.empty())
    // only for objects for the moment
    this->out() << ns << "::";
  this->out() << node->resolved_value;
  if (node->resolved_kind == TypeKind_Interface)
    this->out() << "Ptr";
  this->out() << constattr("&");
}

template <typename T>
void CppTypeFormatter<T>::visitTypeExpr(ListTypeExprNode* node) {
  this->out() << constattr("const ") << "std::vector< ";
  unconstify(node->element);
  this->out() << " >" << constattr("&");
}

template <typename T>
void CppTypeFormatter<T>::visitTypeExpr(MapTypeExprNode* node) {
  this->out() << constattr("const ") << "std::map< ";
  unconstify(node->key);
  this->out() << ", ";
  unconstify(node->value);
  this->out() << " >" << constattr("&");
}

template <typename T>
void CppTypeFormatter<T>::visitTypeExpr(TupleTypeExprNode* node) {
  if (node->elements.size() == 2) {
    this->out() << constattr("const ") << "std::pair< ";
    unconstify(node->elements.at(0));
    this->out() << ", ";
    unconstify(node->elements.at(1));
    this->out() << " >" << constattr("&");
  }
  else
    this->out() << "TUPLENOTIMPL";
}

template <typename T>
void CppTypeFormatter<T>::visitTypeExpr(VarArgTypeExprNode* node) {
  this->out() << constattr("const ") << "qi::VarArguments< ";
  unconstify(node->element);
  this->out() << " >" << constattr("&");
}

template <typename T>
void CppTypeFormatter<T>::visitTypeExpr(KeywordArgTypeExprNode* node) {
  this->out() << constattr("const ") << "qi::KeywordArguments< ";
  unconstify(node->value);
  this->out() << " >" << constattr("&");
}

template <typename T>
void CppTypeFormatter<T>::visitData(BoolLiteralNode *node) {
  if (node->value)
    this->out() << "true";
  else
    this->out() << "false";
}

template <typename T>
void CppTypeFormatter<T>::visitData(IntLiteralNode *node) {
  this->out() << node->value;
}

template <typename T>
void CppTypeFormatter<T>::visitData(FloatLiteralNode *node) {
  this->out() << node->value;
}

template <typename T>
void CppTypeFormatter<T>::visitData(StringLiteralNode *node) {
  this->out() << node->value;
}

template <typename T>
void CppTypeFormatter<T>::visitData(TupleLiteralNode* node) {
  this->out() << "(" << "FAIL" << ")";
}

template <typename T>
void CppTypeFormatter<T>::visitData(ListLiteralNode* node) {
  this->out() << "[" << "FAIL" << "]";
}

template <typename T>
void CppTypeFormatter<T>::visitData(DictLiteralNode* node) {
  this->out() << "{" << "FAIL" << "}";
}

template <typename T>
void CppTypeFormatter<T>::visitExpr(BinaryOpExprNode *node) {
  throw std::runtime_error("unimplemented");
}

template <typename T>
void CppTypeFormatter<T>::visitExpr(UnaryOpExprNode *node) {
  throw std::runtime_error("unimplemented");
}

template <typename T>
void CppTypeFormatter<T>::visitExpr(VarExprNode *node) {
  //throw std::runtime_error("unimplemented");
}

template <typename T>
void CppTypeFormatter<T>::visitExpr(LiteralExprNode* node) {
  throw std::runtime_error("unimplemented");
}

template <typename T>
void CppTypeFormatter<T>::visitExpr(CallExprNode* node) {
  this->out() << node->name << "(";
  for (unsigned i = 0; i < node->args.size(); ++i) {
    this->accept(node->args.at(i));
    if (i + 1 != node->args.size())
      this->out() << ", ";
  }
  this->out() << ")";
}

namespace detail {

std::string toName(const std::string& name, int counter);

template <typename T>
void cppFormatParam(CppTypeFormatter<T>* fmt, ParamFieldDeclNodePtr node, CppParamsFormat cfpt, int counter) {
  for (unsigned i = 0; i < node->names.size(); ++i) {
    switch(node->paramType) {
      case ParamFieldType_Normal: {
        if (cfpt != CppParamsFormat_NameOnly) {
          fmt->constify(node->effectiveType());
          fmt->out() << " ";
        }
        if (cfpt != CppParamsFormat_TypeOnly)
          fmt->out() << toName(node->names.at(i), counter);
        break;
      }
      case ParamFieldType_VarArgs: {
        if (cfpt != CppParamsFormat_NameOnly) {
          ScopedFormatAttrActivate _(fmt->constattr);
          fmt->accept(node->effectiveType());
        }
        if (cfpt != CppParamsFormat_TypeOnly)
          fmt->out() << toName(node->names.at(i), counter);
        break;
      }
      case ParamFieldType_KeywordArgs: {
        if (cfpt != CppParamsFormat_NameOnly) {
          ScopedFormatAttrActivate _(fmt->constattr);
          fmt->accept(node->effectiveType());
        }
        if (cfpt != CppParamsFormat_TypeOnly)
          fmt->out() << toName(node->names.at(i), counter);
        break;
      }
    }
  }
}

}

template <typename T>
void cppParamsFormat(CppTypeFormatter<T>* typeformat, ParamFieldDeclNodePtrVector params, CppParamsFormat cfpt) {
  for (unsigned i = 0; i < params.size(); ++i) {
    detail::cppFormatParam(typeformat, params.at(i), cfpt, i);
    if (i + 1 < params.size())
      typeformat->out() << ", ";
  }
}

}
