#include "ParseStruct/Operation.hh"
#include "ParseStruct/Type.hh"
#include "CodeGen.hh"

#include <iostream>

namespace Dlink
{
	extern std::string tree_prefix(std::size_t depth);
	std::string operator_string(TokenType operator_type)
	{
		switch (operator_type)
		{
		case TokenType::plus:
			return "+";
		case TokenType::increment:
			return "++";
		case TokenType::plus_assign:
			return "+=";

		case TokenType::minus:
			return "-";
		case TokenType::decrement:
			return "--";
		case TokenType::minus_assign:
			return "-=";

		case TokenType::multiply:
			return "*";
		case TokenType::multiply_assign:
			return "*=";

		case TokenType::divide:
			return "/";
		case TokenType::divide_assign:
			return "/=";

		case TokenType::modulo:
			return "%";
		case TokenType::modulo_assign:
			return "%=";

		case TokenType::assign:
			return "=";
		case TokenType::equal:
			return "==";
		case TokenType::noteq:
			return "!=";
		case TokenType::greater:
			return ">";
		case TokenType::eqgreater:
			return ">=";
		case TokenType::less:
			return "<";
		case TokenType::eqless:
			return "<=";

		case TokenType::logic_and:
			return "&&";
		case TokenType::logic_or:
			return "||";

		case TokenType::bit_not:
			return "~";
		case TokenType::bit_and:
			return "&";
		case TokenType::bit_and_assign:
			return "&=";
		case TokenType::bit_or:
			return "|";
		case TokenType::bit_or_assign:
			return "|=";
		case TokenType::bit_xor:
			return "^";
		case TokenType::bit_xor_assign:
			return "^=";
		case TokenType::bit_lshift:
			return "<<";
		case TokenType::bit_lshift_assign:
			return "<<=";
		case TokenType::bit_rshift:
			return ">>";
		case TokenType::bit_rshift_assign:
			return ">>=";

		case TokenType::dot:
			return ".";

		default:
			return "";
		}
	}

	/**
	 * @brief 새 Integer32 인스턴스를 만듭니다.
	 * @details 이 함수는 예외를 발생시키지 않습니다.
	 * @param token 이 노드를 만드는데 사용된 가장 첫번째 토큰입니다.
	 * @param data 32비트 부호 있는 정수 상수입니다.
	 */
	Integer32::Integer32(const Token& token, std::int32_t data) noexcept
		: Constant(token, SimpleType::_int), data(data)
	{}

	std::string Integer32::tree_gen(std::size_t depth) const
	{
		return tree_prefix(depth) + "Integer32(" + std::to_string(data) + ')';
	}
	LLVM::Value Integer32::code_gen()
	{
		return { LLVM::builder().getInt32(data), type };
	}
	bool Integer32::evaluate(Any& out)
	{
		out = static_cast<std::int64_t>(data);
		return true;
	}

	/**
	 * @brief 새 String 인스턴스를 만듭니다.
	 * @details 이 함수는 예외를 발생시키지 않습니다.
	 * @param token 이 노드를 만드는데 사용된 가장 첫번째 토큰입니다.
	 * @param data 문자열입니다.
	 */
	String::String(const Token& token, const std::string& data) noexcept
		: Literal(token), data(data)
	{}

	std::string String::tree_gen(std::size_t depth) const
	{
		return tree_prefix(depth) + "String(" + data + ')';
	}
	LLVM::Value String::code_gen()
	{
		return LLVM::builder().CreateGlobalStringPtr(data.c_str());
	}

	/**
	 * @brief 새 Character 인스턴스를 만듭니다.
	 * @details 이 함수는 예외를 발생시키지 않습니다.
	 * @param token 이 노드를 만드는데 사용된 가장 첫번째 토큰입니다.
	 * @param data 문자입니다.
	 */
	Character::Character(const Token& token, char data) noexcept
		: Constant(token, SimpleType::_char), data(data)
	{}

	std::string Character::tree_gen(std::size_t depth) const
	{
		return tree_prefix(depth) + "Character(" + data + ')';
	}
	LLVM::Value Character::code_gen()
	{
		return { LLVM::builder().getInt8(data), type };
	}
}

namespace Dlink
{
	/**
	 * @brief 새 BinaryOperation 인스턴스를 만듭니다.
	 * @param token 이 노드를 만드는데 사용된 가장 첫번째 토큰입니다.
	 * @param op 연산자 타입입니다.
	 * @param lhs 좌측 피연산자입니다.
	 * @param rhs 우측 피연산자입니다.
	 */
	BinaryOperation::BinaryOperation(const Token& token, TokenType op, ExpressionPtr lhs, ExpressionPtr rhs)
		: Expression(token), op(op), lhs(lhs), rhs(rhs)
	{}
	std::string BinaryOperation::tree_gen(std::size_t depth) const
	{
		std::string tree = tree_prefix(depth) + "BinaryOperation:\n";
		++depth;
		tree += tree_prefix(depth) + "lhs:\n";
		tree += lhs->tree_gen(depth + 1) + '\n';
		tree += tree_prefix(depth) + "rhs:\n";
		tree += rhs->tree_gen(depth + 1) + '\n';
		tree += tree_prefix(depth) + "op:\n";
		tree += tree_prefix(depth + 1) + operator_string(op) + '(' + token_map.at(op) + ')';

		return tree;
	}
	LLVM::Value BinaryOperation::code_gen()
	{
		LLVM::Value lhs_value = lhs->code_gen();
		LLVM::Value rhs_value = rhs->code_gen();

		static const std::map<std::string, std::map<std::string, TypePtr>> aithmetic_type =
		{
			// char
			{ "char", {
					{ "char", SimpleType::_char },
					{ "byte", SimpleType::byte },
					{ "short", SimpleType::_short },
					{ "int", SimpleType::_int },
					{ "long", SimpleType::_long },

					{ "signed byte", SimpleType::signed_byte },
					{ "unsigned short", SimpleType::_unsigned_short },
					{ "unsigned int", SimpleType::_unsigned_int },
					{ "unsigned long", SimpleType::_unsigned_long },

					{ "half", SimpleType::half },
					{ "single", SimpleType::single },
					{ "double", SimpleType::_double },
			} },

			// byte
			{ "byte", {
				{ "char", SimpleType::byte },
				{ "byte", SimpleType::byte },
				{ "short", SimpleType::_short },
				{ "int", SimpleType::_int },
				{ "long", SimpleType::_long },

				{ "signed byte", SimpleType::byte },
				{ "unsigned short", SimpleType::_unsigned_short },
				{ "unsigned int", SimpleType::_unsigned_int },
				{ "unsigned long", SimpleType::_unsigned_long },

				{ "half", SimpleType::half },
				{ "single", SimpleType::single },
				{ "double", SimpleType::_double },
			} },
			{ "signed byte", {
				{ "char", SimpleType::signed_byte },
				{ "byte", SimpleType::byte },
				{ "short", SimpleType::_short },
				{ "int", SimpleType::_int },
				{ "long", SimpleType::_long },

				{ "signed byte", SimpleType::signed_byte },
				{ "unsigned short", SimpleType::_unsigned_short },
				{ "unsigned int", SimpleType::_unsigned_int },
				{ "unsigned long", SimpleType::_unsigned_long },

				{ "half", SimpleType::half },
				{ "single", SimpleType::single },
				{ "double", SimpleType::_double },
			} },

			// short
			{ "short", {
				{ "char", SimpleType::_short },
				{ "byte", SimpleType::_short },
				{ "short", SimpleType::_short },
				{ "int", SimpleType::_int },
				{ "long", SimpleType::_long },

				{ "signed byte", SimpleType::_short },
				{ "unsigned short", SimpleType::_unsigned_short },
				{ "unsigned int", SimpleType::_unsigned_int },
				{ "unsigned long", SimpleType::_unsigned_long },

				{ "single", SimpleType::single },
				{ "double", SimpleType::_double },
			} },
			{ "unsigned short", {
				{ "char", SimpleType::_unsigned_short },
				{ "byte", SimpleType::_unsigned_short },
				{ "short", SimpleType::_unsigned_short },
				{ "int", SimpleType::_int },
				{ "long", SimpleType::_long },

				{ "signed byte", SimpleType::_unsigned_short },
				{ "unsigned short", SimpleType::_unsigned_short },
				{ "unsigned int", SimpleType::_unsigned_int },
				{ "unsigned long", SimpleType::_unsigned_long },

				{ "single", SimpleType::single },
				{ "double", SimpleType::_double },
			} },

			// int
			{ "int", {
				{ "char", SimpleType::_int },
				{ "byte", SimpleType::_int },
				{ "short", SimpleType::_int },
				{ "int", SimpleType::_int },
				{ "long", SimpleType::_long },

				{ "signed byte", SimpleType::_int },
				{ "unsigned short", SimpleType::_int },
				{ "unsigned int", SimpleType::_unsigned_int },
				{ "unsigned long", SimpleType::_unsigned_long },

				{ "double", SimpleType::_double },
			} },
			{ "unsigned int", {
				{ "char", SimpleType::_unsigned_int },
				{ "byte", SimpleType::_unsigned_int },
				{ "short", SimpleType::_unsigned_int },
				{ "int", SimpleType::_unsigned_int },
				{ "long", SimpleType::_long },

				{ "signed byte", SimpleType::_unsigned_int },
				{ "unsigned short", SimpleType::_unsigned_int },
				{ "unsigned int", SimpleType::_unsigned_int },
				{ "unsigned long", SimpleType::_unsigned_long },

				{ "double", SimpleType::_double },
			} },

			// long
			{ "long", {
				{ "char", SimpleType::_long },
				{ "byte", SimpleType::_long },
				{ "short", SimpleType::_long },
				{ "int", SimpleType::_long },
				{ "long", SimpleType::_long },

				{ "signed byte", SimpleType::_long },
				{ "unsigned short", SimpleType::_long },
				{ "unsigned int", SimpleType::_long },
				{ "unsigned long", SimpleType::_unsigned_long },
			} },
			{ "unsigned long", {
				{ "char", SimpleType::_unsigned_long },
				{ "byte", SimpleType::_unsigned_long },
				{ "short", SimpleType::_unsigned_long },
				{ "int", SimpleType::_unsigned_long },
				{ "long", SimpleType::_unsigned_long },

				{ "signed byte", SimpleType::_unsigned_long },
				{ "unsigned short", SimpleType::_unsigned_long },
				{ "unsigned int", SimpleType::_unsigned_long },
				{ "unsigned long", SimpleType::_unsigned_long },
			} },

			// half
			{ "half", {
				{ "char", SimpleType::half },
				{ "byte", SimpleType::half },

				{ "signed byte", SimpleType::half },
			} },

			// single
			{ "single", {
				{ "char", SimpleType::single },
				{ "byte", SimpleType::single },
				{ "short", SimpleType::single },

				{ "signed byte", SimpleType::single },
				{ "unsigned short", SimpleType::single },
			} },

			// double
			{ "double", {
				{ "char", SimpleType::_double },
				{ "byte", SimpleType::_double },
				{ "short", SimpleType::_double },
				{ "int", SimpleType::_int },

				{ "signed byte", SimpleType::_double },
				{ "unsigned short", SimpleType::_double },
				{ "unsigned int", SimpleType::_double },
			} },
		};
		
		static auto simpletype2string = [](TypePtr type) -> std::string
		{
			if (dynamic_cast<SimpleType*>(type.get()))
			{
				std::shared_ptr<SimpleType> simple_type = std::dynamic_pointer_cast<SimpleType>(type);

				std::string result = simple_type->identifier;

				if (simple_type->is_unsigned)
				{
					result = "unsigned " + result;
				}

				return result;
			}

			return "";
		};

		static auto type_inf = [](LLVM::Value& lhs, LLVM::Value& rhs) -> TypePtr
		{
			if (std::find(aithmetic_type.begin(), aithmetic_type.end(), simpletype2string(lhs.type())) != aithmetic_type.end())
			{
				if (std::find(aithmetic_type.begin(), aithmetic_type.end(), simpletype2string(rhs.type())) != aithmetic_type.end())
				{
					return aithmetic_type.at(simpletype2string(lhs.type())).at(simpletype2string(rhs.type()));
				}
			}

			return nullptr;
		};

		switch (op)
		{
		case TokenType::plus:
			return { LLVM::builder().CreateAdd(lhs_value, rhs_value), type_inf(lhs_value, rhs_value) };

		case TokenType::minus:
			return { LLVM::builder().CreateSub(lhs_value, rhs_value), type_inf(lhs_value, rhs_value) };

		case TokenType::multiply:
			return { LLVM::builder().CreateMul(lhs_value, rhs_value), type_inf(lhs_value, rhs_value) };

		case TokenType::divide:
			// TODO: 임시 방안
			return { LLVM::builder().CreateSDiv(lhs_value, rhs_value), type_inf(lhs_value, rhs_value) };

		case TokenType::assign:
		{
			if (lhs_value.type()->get_type().is_const())
			{
				// TODO: 에러메세지 채워주세요.
				throw Error(token, "TODO");
			}

			llvm::LoadInst* load_inst = llvm::dyn_cast_or_null<llvm::LoadInst>(lhs_value.get());
			if (load_inst)
			{
				return { LLVM::builder().CreateStore(rhs_value, load_inst->getPointerOperand()), lhs->type };
			}
			return { LLVM::builder().CreateStore(rhs_value, lhs_value), lhs->type };
		}

		default:
			// TODO: 에러 메세지 채워주세요.
			throw Error(token, "TODO");
		}
	}
	void BinaryOperation::preprocess()
	{
		lhs->preprocess();
		rhs->preprocess();
	}
	bool BinaryOperation::evaluate(Any& out)
	{
		Any lhs_eval, rhs_eval;
		bool lhs_eval_ok = lhs->evaluate(lhs_eval);
		bool rhs_eval_ok = rhs->evaluate(rhs_eval);

		if (lhs_eval_ok && rhs_eval_ok)
		{
			Any eval;
			bool eval_ok;

			switch (op)
			{
			case TokenType::plus:
				eval_ok = any_add(lhs_eval, rhs_eval, eval);
				break;

			case TokenType::minus:
				eval_ok = any_sub(lhs_eval, rhs_eval, eval);
				break;

			case TokenType::multiply:
				eval_ok = any_mul(lhs_eval, rhs_eval, eval);
				break;

			case TokenType::divide:
				eval_ok = any_div(lhs_eval, rhs_eval, eval);
				break;

			default:
				return false;
			}

			if (eval_ok)
			{
				out = eval;
				return true;
			}
		}

		return false;
	}

	/**
	 * @brief 새 UnaryOperation 인스턴스를 만듭니다.
	 * @param token 이 노드를 만드는데 사용된 가장 첫번째 토큰입니다.
	 * @param op 연산자 타입입니다.
	 * @param rhs 피연산자입니다.
	 */
	UnaryOperation::UnaryOperation(const Token& token, TokenType op, ExpressionPtr rhs)
		: Expression(token), op(op), rhs(rhs)
	{}
	std::string UnaryOperation::tree_gen(std::size_t depth) const
	{
		std::string tree = tree_prefix(depth) + "UnaryOperation:\n";
		++depth;
		tree += tree_prefix(depth) + "rhs:\n";
		tree += rhs->tree_gen(depth + 1) + '\n';
		tree += tree_prefix(depth) + "op:\n";
		tree += tree_prefix(depth + 1) + operator_string(op) + '(' + token_map.at(op) + ')';

		return tree;
	}
	LLVM::Value UnaryOperation::code_gen()
	{
		LLVM::Value rhs_value = rhs->code_gen();

		switch (op)
		{
		case TokenType::plus:
			return { LLVM::builder().CreateMul(LLVM::builder().getInt32(1), rhs_value), rhs->type };

		case TokenType::minus:
			return { LLVM::builder().CreateMul(LLVM::builder().getInt32(-1), rhs_value), rhs->type };

		case TokenType::multiply: // 값 참조 연산
		{
			return LLVM::builder().CreateLoad(rhs_value);
		}

		case TokenType::bit_and: // 주소 참조 연산
		{
			if (rhs->is_lvalue())
			{
				llvm::LoadInst* temp = nullptr;
				if ((temp = llvm::dyn_cast_or_null<llvm::LoadInst>(rhs_value.get())))
				{
					return { temp->getPointerOperand(),
						std::make_shared<Pointer>(Token::empty, rhs->type) };
				}
			}

			throw Error(token, "Expected lvalue for operand of reference operator");
		}

		default:
			// TODO: 에러 메세지 채워주세요.
			throw Error(token, "TODO");
		}
	}
	void UnaryOperation::preprocess()
	{
		rhs->preprocess();
	}
	bool UnaryOperation::evaluate(Any& out)
	{
		Any rhs_eval;
		bool rhs_eval_ok = rhs->evaluate(rhs_eval);

		if (rhs_eval_ok)
		{
			Any eval;
			bool eval_ok;

			switch (op)
			{
			case TokenType::plus:
				eval_ok = any_add(0, rhs_eval, eval);
				break;

			case TokenType::minus:
				eval_ok = any_sub(0, rhs_eval, eval);
				break;

			default:
				return false;
			}

			if (eval_ok)
			{
				out = eval;
				return true;
			}
		}

		return false;
	}

	/**
	 * @brief 새 FunctionCallOperation 인스턴스를 만듭니다.
	 * @param token 이 노드를 만드는데 사용된 가장 첫번째 토큰입니다.
	 * @param identifier 호출할 함수의 식별자입니다.
	 * @param argument 인수입니다.
	 */
	FunctionCallOperation::FunctionCallOperation(const Token& token, ExpressionPtr func_expr, const std::vector<ExpressionPtr>& arugment)
		: Expression(token), func_expr(func_expr), argument(arugment)
	{}
	std::string FunctionCallOperation::tree_gen(std::size_t depth) const
	{
		std::string result;
		result += tree_prefix(depth) + "FunctionCallOperation:\n";
		++depth;
		result += tree_prefix(depth) + "func_expr:\n";
		result += func_expr->tree_gen(depth + 1) + '\n';
		result += tree_prefix(depth) + "argument:\n";
		++depth;
		for (auto arg : argument)
		{
			result += arg->tree_gen(depth) + '\n';
		}

		return result;
	}
	LLVM::Value FunctionCallOperation::code_gen()
	{
		llvm::Function* function;

		Identifier* dest;
		if ((dest = dynamic_cast<Identifier*>(func_expr.get())))
		{
			function = llvm::dyn_cast<llvm::Function>(symbol_table->find(dest->id).get());
		}
		else
		{
			function = llvm::dyn_cast<llvm::Function>(func_expr->code_gen().get());
		}


		if (function)
		{
			std::vector<llvm::Value*> arg_real;

			for (auto arg : argument)
			{
				arg_real.push_back(arg->code_gen());
			}

			return LLVM::builder().CreateCall(function, arg_real);
		}
		else
		{
			throw Error(token, "Expected callable function expression");
		}
	}
	void FunctionCallOperation::preprocess()
	{
		func_expr->preprocess();

		for (ExpressionPtr arg : argument)
		{
			arg->preprocess();
		}
	}

	/**
	 * @brief 새 ArrayInitList 인스턴스를 만듭니다.
	 * @param token 이 노드를 만드는데 사용된 가장 첫번째 토큰입니다.
	 * @param elements 배열 리스트의 원소들입니다.
	 */
	ArrayInitList::ArrayInitList(const Token& token, const std::vector<ExpressionPtr>& elements)
		: Constant(token), elements(elements)
	{}
	std::string ArrayInitList::tree_gen(std::size_t depth) const
	{
		std::string result;
		result += tree_prefix(depth) + "ArrayInitList:\n";
		++depth;
		result += tree_prefix(depth) + "elements:\n";
		++depth;
		for (auto element : elements)
		{
			result += element->tree_gen(depth) + '\n';
		}

		return result;
	}
	LLVM::Value ArrayInitList::code_gen()
	{
		throw Error(token, "Expected expression");
	}
	void ArrayInitList::preprocess()
	{
		for (ExpressionPtr expr : elements)
		{
			expr->preprocess();
		}
	}

	/**
	 * @brief 새 UnsafeExpression 인스턴스를 만듭니다.
	 * @param token 이 노드를 만드는데 사용된 가장 첫번째 토큰입니다.
	 * @param expression 안전하지 않은 식입니다.
	 */
	UnsafeExpression::UnsafeExpression(const Token& token, ExpressionPtr expression)
		: Expression(token), expression(expression)
	{}
	std::string UnsafeExpression::tree_gen(std::size_t depth) const
	{
		return tree_prefix(depth) + "UnsafeExpression:\n" +
			tree_prefix(depth + 1) + "expression:\n" +
			expression->tree_gen(depth + 2);
	}
	LLVM::Value UnsafeExpression::code_gen()
	{
		LLVM::Value result = nullptr;

		if (in_unsafe_block)
		{
			get_current_assembler().get_warnings().add_warning(Warning(token, "Unnecessary unsafe expression"));

			result = expression->code_gen();
		}
		else
		{
			in_unsafe_block = true;
			result = expression->code_gen();
			in_unsafe_block = false;
		}

		return result;
	}
	void UnsafeExpression::preprocess()
	{
		expression->preprocess();
	}
}

namespace Dlink
{
	/**
	 * @brief 새 ReturnStatement 인스턴스를 만듭니다.
	 * @param token 이 노드를 만드는데 사용된 가장 첫번째 토큰입니다.
	 * @param return_expr 반환할 식입니다.
	 */
	ReturnStatement::ReturnStatement(const Token& token, ExpressionPtr return_expr)
		: Statement(token), return_expr(return_expr)
	{}

	std::string ReturnStatement::tree_gen(std::size_t depth) const
	{
		std::string tree = tree_prefix(depth) + "ReturnStatement:\n";
		if (return_expr)
		{
			tree += return_expr->tree_gen(depth + 1);
		}
		else
		{
			tree += tree_prefix(depth + 1) + "empty";
		}

		return tree;
	}
	LLVM::Value ReturnStatement::code_gen()
	{
		if (return_expr)
		{
			if (LLVM::builder().getCurrentFunctionReturnType() == LLVM::builder().getVoidTy())
			{
				throw Error(token, "Unexpected value return statement in void function");
			}
			return { LLVM::builder().CreateRet(return_expr->code_gen()), nullptr };
		}
		else
		{
			if (LLVM::builder().getCurrentFunctionReturnType() != LLVM::builder().getVoidTy())
			{
				throw Error(token, "Expected value return statement in non-void returning function");
			}
			return { LLVM::builder().CreateRetVoid(), nullptr };
		}
	}
	void ReturnStatement::preprocess()
	{
		return_expr->preprocess();
	}

	/**
	 * @brief 새 UnsafeStatement 인스턴스를 만듭니다.
	 * @param token 이 노드를 만드는데 사용된 가장 첫번째 토큰입니다.
	 * @param statement 안전하지 않은 문입니다.
	 */
	UnsafeStatement::UnsafeStatement(const Token& token, StatementPtr statement)
		: Statement(token), statement(statement)
	{}
	std::string UnsafeStatement::tree_gen(std::size_t depth) const
	{
		return tree_prefix(depth) + "UnsafeStatement:\n" +
			tree_prefix(depth + 1) + "statement:\n" +
			statement->tree_gen(depth + 2);
	}
	LLVM::Value UnsafeStatement::code_gen()
	{
		LLVM::Value result = nullptr;

		if (in_unsafe_block)
		{
			get_current_assembler().get_warnings().add_warning(Warning(token, "Unnecessary unsafe statement"));

			result = statement->code_gen();
		}
		else
		{
			in_unsafe_block = true;
			result = statement->code_gen();
			in_unsafe_block = false;
		}

		return result;
	}
	void UnsafeStatement::preprocess()
	{
		statement->preprocess();
	}
}