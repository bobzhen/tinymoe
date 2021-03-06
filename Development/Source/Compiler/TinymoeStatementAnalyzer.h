#ifndef VCZH_COMPILER_TINYMOESTATEMENTANALYZER
#define VCZH_COMPILER_TINYMOESTATEMENTANALYZER

#include "TinymoeDeclarationAnalyzer.h"
#include "TinymoeExpressionAnalyzer.h"

namespace tinymoe
{
	namespace ast
	{
		class AstNode;
		class AstDeclaration;
		class AstSymbolDeclaration;
		class AstStatement;
		class AstExpression;
	}

	namespace compiler
	{
		class SymbolModule;
		class SymbolAstScope;
		struct SymbolAstContext;
		struct SymbolAstResult;

		/*************************************************************
		Statement
		*************************************************************/

		class Statement : public CodeFragment
		{
		public:
			typedef shared_ptr<Statement>								Ptr;
			typedef weak_ptr<Statement>									WeakPtr;
			typedef vector<Ptr>											List;
			typedef map<GrammarSymbol::Ptr, Expression::Ptr>			SymbolExpressionMap;

			CodeToken						keywordToken;
			Statement::WeakPtr				parentStatement;
			GrammarSymbol::Ptr				statementSymbol;
			InvokeExpression::Ptr			statementExpression;
			SymbolExpressionMap				newVariables;
			SymbolExpressionMap				blockArguments;
			Statement::List					statements;
			bool							connectToPreviousBlock = false;

			static SymbolAstResult			GenerateExitAst(shared_ptr<SymbolAstScope> scope, SymbolAstContext& context, shared_ptr<ast::AstDeclaration> state);
			static void						GenerateAssignableArgumentPairAst(shared_ptr<SymbolAstScope> scope, SymbolAstContext& context, shared_ptr<ast::AstDeclaration> state, shared_ptr<ast::AstDeclaration> signal, shared_ptr<Expression> assignable, vector<shared_ptr<ast::AstSymbolDeclaration>>::iterator& itvar, shared_ptr<ast::AstExpression>& reader, shared_ptr<ast::AstExpression>& writer);
			static void						GenerateExpressionArgumentAst(shared_ptr<SymbolAstScope> scope, SymbolAstContext& context, shared_ptr<ast::AstDeclaration> state, shared_ptr<ast::AstDeclaration> signal, shared_ptr<Expression> expression, shared_ptr<ast::AstExpression>& reader);
			void							CreateNewVariableDeclarations(shared_ptr<SymbolAstScope> scope, SymbolAstContext& context, shared_ptr<ast::AstDeclaration> state, shared_ptr<ast::AstDeclaration> signal, vector<shared_ptr<ast::AstSymbolDeclaration>>& newVariableDecls, SymbolAstResult& statResult);
			void							CreateBlockArgumentDeclarations(shared_ptr<SymbolAstScope> scope, SymbolAstContext& context, shared_ptr<ast::AstDeclaration> state, shared_ptr<ast::AstDeclaration> signal, vector<shared_ptr<ast::AstSymbolDeclaration>>& blockArgumentDecls);
			SymbolAstResult					GeneratePredefinedAst(shared_ptr<SymbolAstScope> scope, SymbolAstContext& context, shared_ptr<ast::AstDeclaration> state, shared_ptr<ast::AstDeclaration> signal);
			SymbolAstResult					GenerateAst(shared_ptr<SymbolAstScope> scope, SymbolAstContext& context, shared_ptr<ast::AstDeclaration> state, shared_ptr<ast::AstDeclaration> signal);
			SymbolAstResult					GenerateBodyAst(shared_ptr<SymbolAstScope> scope, SymbolAstContext& context, shared_ptr<ast::AstDeclaration> state, shared_ptr<ast::AstDeclaration> continuation);
		};

		/*************************************************************
		Symbol
		*************************************************************/

		class SymbolFunction
		{
		public:
			typedef shared_ptr<SymbolFunction>							Ptr;
			typedef weak_ptr<SymbolFunction>							WeakPtr;
			typedef vector<Ptr>											List;
			typedef map<GrammarSymbol::Ptr, FunctionFragment::Ptr>		SymbolFragmentMap;
			typedef map<FunctionFragment::Ptr, GrammarSymbol::Ptr>		FragmentSymbolMap;

			FunctionDeclaration::Ptr		function;					// the original function
			SymbolFunction::WeakPtr			multipleDispatchingRoot;	// the multiple-dispatching root declaration
			SymbolFragmentMap				arguments;					// all arguments
			FragmentSymbolMap				argumentTypes;				// map an argument to its receiving type

			GrammarSymbol::Ptr				resultVariable;
			GrammarSymbol::Ptr				cpsStateVariable;
			GrammarSymbol::Ptr				cpsContinuationVariable;
			GrammarSymbol::Ptr				categorySignalVariable;

			Statement::Ptr					statement;					// the function body
		};

		class SymbolModule
		{
		public:
			typedef shared_ptr<SymbolModule>							Ptr;
			typedef vector<Ptr>											List;
			typedef weak_ptr<SymbolModule>								WeakPtr;
			typedef vector<WeakPtr>										WeakList;
			typedef map<GrammarSymbol::Ptr, Declaration::Ptr>			SymbolDeclarationMap;
			typedef map<Declaration::Ptr, SymbolFunction::Ptr>			DeclarationFunctionMap;
			typedef map<Declaration::Ptr, GrammarSymbol::Ptr>			DeclarationSymbolMap;

			struct ParsingFailedException{};

			CodeFile::Ptr					codeFile;					// the original code file
			Module::Ptr						module;						// the original module
			WeakList						usingSymbolModules;			// all referenced modules
			SymbolDeclarationMap			symbolDeclarations;			// map a grammar symbol to the creator declaration
			DeclarationFunctionMap			declarationFunctions;		// map a declaration to the symbol function
			DeclarationSymbolMap			baseTypes;					// map a type to its base type

			bool							IsOverloading(GrammarSymbol::Ptr a, GrammarSymbol::Ptr b);
			bool							IsMultipleDispatchingChild(FunctionDeclaration::Ptr func);
			void							CheckOverloading(SymbolModule* modulea, GrammarSymbol::Ptr symbola, Declaration::Ptr decla, SymbolModule* moduleb, GrammarSymbol::Ptr symbolb, Declaration::Ptr declb, bool foreignCheck, CodeError::List& errors);
			void							BuildSymbols(CodeError::List& errors);									// sync step: build all declaration symbols
			void							BuildFunctions(CodeError::List& errors);								// sync step: build all symbol functions
			void							BuildFunctionLinkings(CodeError::List& errors);							// sync step: link all multiple-dispatching functions to it's parent

			void							FindOverridedSymbols(GrammarStack::Ptr stack, GrammarStackItem::Ptr item, GrammarSymbol::List& symbols);
			void							BuildNameSymbol(vector<CodeToken>& tokens, GrammarSymbol::Ptr& symbol, CodeToken& token);
			FunctionDeclaration::Ptr		FindSymbolFunction(GrammarSymbol::Ptr symbol, GrammarSymbolType acceptableType);
			Statement::Ptr					ParseBlock(CodeFile::Ptr codeFile, GrammarStack::Ptr stack, Statement::Ptr statement, int& lineIndex, int endLineIndex, CodeError::List& errors);
			GrammarSymbol::Ptr				FindType(SymbolName::Ptr name, GrammarStack::Ptr stack, CodeError::List& errors);
			Declaration::Ptr				FindDeclaration(GrammarSymbol::Ptr symbol);
			void							BuildStatements(GrammarStack::Ptr stack, CodeError::List& errors);		// sync step: parse all statements
		};

		class SymbolAssembly
		{
		public:
			typedef shared_ptr<SymbolAssembly>		Ptr;

			SymbolModule::List				symbolModules;

			static SymbolAssembly::Ptr		Parse(vector<string>& codes, CodeError::List& errors);
		};
	}
}

#endif