%skeleton "lalr1.cc"
%require "3.6"

%defines
%define api.token.constructor true
%define api.value.type variant
%define api.value.automove true
%define api.namespace { mizugaki::parser }
%define api.parser.class { sql_parser_generated }
%define api.token.prefix {}
%define parse.error detailed
%define parse.lac full

%locations
%define api.location.type { ::mizugaki::ast::node_region }

%code requires {
    #include <memory>
    #include <string>

    #include <takatori/util/object_creator.h>

    #include <mizugaki/ast/node_region.h>
    #include <mizugaki/ast/common/vector.h>
    #include <mizugaki/ast/common/chars.h>

    #include <mizugaki/ast/statement/statement.h>
    #include <mizugaki/ast/statement/empty_statement.h>
    #include <mizugaki/ast/statement/select_statement.h>
    #include <mizugaki/ast/statement/insert_statement.h>
    #include <mizugaki/ast/statement/update_statement.h>
    #include <mizugaki/ast/statement/delete_statement.h>

    #include <mizugaki/ast/query/query.h>
    #include <mizugaki/ast/query/table_reference.h>
    #include <mizugaki/ast/query/table_value_constructor.h>
    #include <mizugaki/ast/query/binary_expression.h>
    #include <mizugaki/ast/query/with_expression.h>
    #include <mizugaki/ast/query/grouping_element.h>
    #include <mizugaki/ast/query/grouping_column.h>
    #include <mizugaki/ast/query/select_element.h>
    #include <mizugaki/ast/query/select_column.h>
    #include <mizugaki/ast/query/select_asterisk.h>

    #include <mizugaki/ast/table/table_reference.h>
    #include <mizugaki/ast/table/unnest.h>
    #include <mizugaki/ast/table/join.h>
    #include <mizugaki/ast/table/subquery.h>
    #include <mizugaki/ast/table/join_condition.h>
    #include <mizugaki/ast/table/join_columns.h>

    #include <mizugaki/ast/scalar/expression.h>
    #include <mizugaki/ast/scalar/literal_expression.h>
    #include <mizugaki/ast/scalar/variable_reference.h>
    #include <mizugaki/ast/scalar/host_parameter_reference.h>
    #include <mizugaki/ast/scalar/field_reference.h>
    #include <mizugaki/ast/scalar/case_expression.h>
    #include <mizugaki/ast/scalar/cast_expression.h>
    #include <mizugaki/ast/scalar/unary_expression.h>
    #include <mizugaki/ast/scalar/binary_expression.h>
    #include <mizugaki/ast/scalar/extract_expression.h>
    #include <mizugaki/ast/scalar/trim_expression.h>
    #include <mizugaki/ast/scalar/value_constructor.h>
    #include <mizugaki/ast/scalar/subquery.h>
    #include <mizugaki/ast/scalar/comparison_predicate.h>
    #include <mizugaki/ast/scalar/quantified_comparison_predicate.h>
    #include <mizugaki/ast/scalar/between_predicate.h>
    #include <mizugaki/ast/scalar/in_predicate.h>
    #include <mizugaki/ast/scalar/pattern_match_predicate.h>
    #include <mizugaki/ast/scalar/table_predicate.h>
    #include <mizugaki/ast/scalar/function_invocation.h>
    #include <mizugaki/ast/scalar/builtin_function_invocation.h>
    #include <mizugaki/ast/scalar/builtin_set_function_invocation.h>
    #include <mizugaki/ast/scalar/new_invocation.h>
    #include <mizugaki/ast/scalar/method_invocation.h>
    #include <mizugaki/ast/scalar/static_method_invocation.h>
    #include <mizugaki/ast/scalar/current_of_cursor.h>

    #include <mizugaki/ast/literal/literal.h>
    #include <mizugaki/ast/literal/boolean.h>
    #include <mizugaki/ast/literal/numeric.h>
    #include <mizugaki/ast/literal/string.h>
    #include <mizugaki/ast/literal/datetime.h>
    #include <mizugaki/ast/literal/interval.h>
    #include <mizugaki/ast/literal/special.h>

    #include <mizugaki/ast/type/type.h>
    #include <mizugaki/ast/type/simple.h>
    #include <mizugaki/ast/type/character_string.h>
    #include <mizugaki/ast/type/bit_string.h>
    #include <mizugaki/ast/type/decimal.h>
    #include <mizugaki/ast/type/binary_numeric.h>
    #include <mizugaki/ast/type/datetime.h>
    #include <mizugaki/ast/type/interval.h>
    #include <mizugaki/ast/type/row.h>
    #include <mizugaki/ast/type/user_defined.h>
    #include <mizugaki/ast/type/collection.h>

    #include <mizugaki/ast/name/name.h>
    #include <mizugaki/ast/name/simple.h>
    #include <mizugaki/ast/name/qualified.h>

    #include <mizugaki/parser/sql_parser_result.h>

    namespace mizugaki::parser {

    template<class T>
    using node_ptr = ::takatori::util::unique_object_ptr<T>;

    template<class T>
    using node_vector = ast::common::vector<node_ptr<T>>;

    template<class T>
    using element_vector = ast::common::vector<T>;

    using ast::common::regioned;

    class sql_scanner;
    class sql_driver;

    } // namespace mizugaki::parser
}

%code {

    #include <takatori/util/downcast.h>

    #include <mizugaki/parser/sql_scanner.h>
    #include <mizugaki/parser/sql_driver.h>

    namespace mizugaki::parser {

    using ::takatori::util::downcast;

    static sql_parser_generated::symbol_type yylex(sql_scanner& scanner, sql_driver& driver) {
        return scanner.next_token(driver);
    }

    void sql_parser_generated::error(location_type const& location, std::string const& message) {
        driver.error(location, message);
    }

    } // namespace mizugaki::parser
}

%param { ::mizugaki::parser::sql_scanner& scanner }
%param { ::mizugaki::parser::sql_driver& driver }

/* <SQL special character> */
%token DOUBLE_QUOTE "\""
%token PERCENT "%"
%token AMPERSAND "&"
%token QUOTE "'"
%token LEFT_PAREN "("
%token RIGHT_PAREN ")"
%token ASTERISK "*"
%token PLUS_SIGN "+"
%token COMMA ","
%token MINUS_SIGN "-"
%token PERIOD "."
%token SOLIDUS "/"
%token COLON ":"
%token SEMICOLON ";"
%token LESS_THAN_OPERATOR "<"
%token EQUALS_OPERATOR "="
%token GREATER_THAN_OPERATOR ">"
%token QUESTION_MARK "?"
%token LEFT_BRACKET "["
%token RIGHT_BRACKET "]"
%token CIRCUMFLEX "^"
%token UNDERSCORE "_"
%token VERTICAL_BAR "|"
%token LEFT_BRACE "{"
%token RIGHT_BRACE "}"

/* <delimiter token> */
%token NOT_EQUALS_OPERATOR "<>"
%token GREATER_THAN_OR_EQUALS_OPERATOR ">="
%token LESS_THAN_OR_EQUALS_OPERATOR "<="
%token CONCATENATION_OPERATOR "||"
%token RIGHT_ARROW "->"
%token DOUBLE_COLON "::"

// <non-reserved word>
%token ABS "ABS"
%token <ast::common::chars> ADA "ADA"
%token ASENSITIVE "ASENSITIVE"
%token ASSIGNMENT "ASSIGNMENT"
%token ASYMMETRIC "ASYMMETRIC"
%token ATOMIC "ATOMIC"
%token AVG "AVG"
%token BETWEEN "BETWEEN"
%token BIT_LENGTH "BIT_LENGTH"
%token BITVAR "BITVAR"
%token <ast::common::chars> C "C"
%token CALLED "CALLED"
%token CARDINALITY "CARDINALITY"
%token CATALOG_NAME "CATALOG_NAME"
%token CHAIN "CHAIN"
%token CHAR_LENGTH "CHAR_LENGTH"
%token CHARACTER_LENGTH "CHARACTER_LENGTH"
%token CHARACTER_SET_CATALOG "CHARACTER_SET_CATALOG"
%token CHARACTER_SET_NAME "CHARACTER_SET_NAME"
%token CHARACTER_SET_SCHEMA "CHARACTER_SET_SCHEMA"
%token CHECKED "CHECKED"
%token CLASS_ORIGIN "CLASS_ORIGIN"
%token COALESCE "COALESCE"
%token <ast::common::chars> COBOL "COBOL"
%token COLLATION_CATALOG "COLLATION_CATALOG"
%token COLLATION_NAME "COLLATION_NAME"
%token COLLATION_SCHEMA "COLLATION_SCHEMA"
%token COLUMN_NAME "COLUMN_NAME"
%token COMMAND_FUNCTION "COMMAND_FUNCTION"
%token COMMAND_FUNCTION_CODE "COMMAND_FUNCTION_CODE"
%token COMMITTED "COMMITTED"
%token CONDITION_NUMBER "CONDITION_NUMBER"
%token CONNECTION_NAME "CONNECTION_NAME"
%token CONSTRAINT_CATALOG "CONSTRAINT_CATALOG"
%token CONSTRAINT_NAME "CONSTRAINT_NAME"
%token CONSTRAINT_SCHEMA "CONSTRAINT_SCHEMA"
%token CONTAINS "CONTAINS"
%token CONVERT "CONVERT"
%token COUNT "COUNT"
%token CURSOR_NAME "CURSOR_NAME"
%token DATETIME_INTERVAL_CODE "DATETIME_INTERVAL_CODE"
%token DATETIME_INTERVAL_PRECISION "DATETIME_INTERVAL_PRECISION"
%token DEFINED "DEFINED"
%token DEFINER "DEFINER"
%token DISPATCH "DISPATCH"
%token DYNAMIC_FUNCTION "DYNAMIC_FUNCTION"
%token DYNAMIC_FUNCTION_CODE "DYNAMIC_FUNCTION_CODE"
%token EXISTING "EXISTING"
%token EXISTS "EXISTS"
%token EXTRACT "EXTRACT"
%token FINAL "FINAL"
%token <ast::common::chars> FORTRAN "FORTRAN"
%token <ast::common::chars> G "G"
%token GENERATED "GENERATED"
%token GRANTED "GRANTED"
%token HIERARCHY "HIERARCHY"
%token HOLD "HOLD"
%token IMPLEMENTATION "IMPLEMENTATION"
%token INFIX "INFIX"
%token INSENSITIVE "INSENSITIVE"
%token INSTANCE "INSTANCE"
%token INSTANTIABLE "INSTANTIABLE"
%token INVOKER "INVOKER"
%token <ast::common::chars> K "K"
%token KEY_MEMBER "KEY_MEMBER"
%token KEY_TYPE "KEY_TYPE"
%token LENGTH "LENGTH"
%token LOWER "LOWER"
%token <ast::common::chars> M "M"
%token MAX "MAX"
%token MIN "MIN"
%token MESSAGE_LENGTH "MESSAGE_LENGTH"
%token MESSAGE_OCTET_LENGTH "MESSAGE_OCTET_LENGTH"
%token MESSAGE_TEXT "MESSAGE_TEXT"
%token METHOD "METHOD"
%token MOD "MOD"
%token MORE "MORE"
%token <ast::common::chars> MUMPS "MUMPS"
%token NAME "NAME"
%token NULLABLE "NULLABLE"
%token NUMBER "NUMBER"
%token NULLIF "NULLIF"
%token OCTET_LENGTH "OCTET_LENGTH"
%token OPTIONS "OPTIONS"
%token OVERLAPS "OVERLAPS"
%token OVERLAY "OVERLAY"
%token OVERRIDING "OVERRIDING"
%token <ast::common::chars> PASCAL "PASCAL"
%token PARAMETER_MODE "PARAMETER_MODE"
%token PARAMETER_NAME "PARAMETER_NAME"
%token PARAMETER_ORDINAL_POSITION "PARAMETER_ORDINAL_POSITION"
%token PARAMETER_SPECIFIC_CATALOG "PARAMETER_SPECIFIC_CATALOG"
%token PARAMETER_SPECIFIC_NAME "PARAMETER_SPECIFIC_NAME"
%token PARAMETER_SPECIFIC_SCHEMA "PARAMETER_SPECIFIC_SCHEMA"
%token <ast::common::chars> PLI "PLI"
%token POSITION "POSITION"
%token REPEATABLE "REPEATABLE"
%token RETURNED_LENGTH "RETURNED_LENGTH"
%token RETURNED_OCTET_LENGTH "RETURNED_OCTET_LENGTH"
%token RETURNED_SQLSTATE "RETURNED_SQLSTATE"
%token ROUTINE_CATALOG "ROUTINE_CATALOG"
%token ROUTINE_NAME "ROUTINE_NAME"
%token ROUTINE_SCHEMA "ROUTINE_SCHEMA"
%token ROW_COUNT "ROW_COUNT"
%token SCALE "SCALE"
%token SCHEMA_NAME "SCHEMA_NAME"
%token SECURITY "SECURITY"
%token SELF "SELF"
%token SENSITIVE "SENSITIVE"
%token SERIALIZABLE "SERIALIZABLE"
%token SERVER_NAME "SERVER_NAME"
%token SIMPLE "SIMPLE"
%token SOURCE "SOURCE"
%token SPECIFIC_NAME "SPECIFIC_NAME"
%token SIMILAR "SIMILAR"
%token SUBLIST "SUBLIST"
%token SUBSTRING "SUBSTRING"
%token SUM "SUM"
%token STYLE "STYLE"
%token SUBCLASS_ORIGIN "SUBCLASS_ORIGIN"
%token SYMMETRIC "SYMMETRIC"
%token SYSTEM "SYSTEM"
%token TABLE_NAME "TABLE_NAME"
%token TRANSACTIONS_COMMITTED "TRANSACTIONS_COMMITTED"
%token TRANSACTIONS_ROLLED_BACK "TRANSACTIONS_ROLLED_BACK"
%token TRANSACTION_ACTIVE "TRANSACTION_ACTIVE"
%token TRANSFORM "TRANSFORM"
%token TRANSFORMS "TRANSFORMS"
%token TRANSLATE "TRANSLATE"
%token TRIGGER_CATALOG "TRIGGER_CATALOG"
%token TRIGGER_SCHEMA "TRIGGER_SCHEMA"
%token TRIGGER_NAME "TRIGGER_NAME"
%token TRIM "TRIM"
%token TYPE "TYPE"
%token UNCOMMITTED "UNCOMMITTED"
%token UNNAMED "UNNAMED"
%token UPPER "UPPER"
%token USER_DEFINED_TYPE_CATALOG "USER_DEFINED_TYPE_CATALOG"
%token USER_DEFINED_TYPE_NAME "USER_DEFINED_TYPE_NAME"
%token USER_DEFINED_TYPE_SCHEMA "USER_DEFINED_TYPE_SCHEMA"

// <reserved word>
%token ABSOLUTE "ABSOLUTE"
%token ACTION "ACTION"
%token ADD "ADD"
%token ADMIN "ADMIN"
%token AFTER "AFTER"
%token AGGREGATE "AGGREGATE"
%token ALIAS "ALIAS"
%token ALL "ALL"
%token ALLOCATE "ALLOCATE"
%token ALTER "ALTER"
%token AND "AND"
%token ANY "ANY"
%token ARE "ARE"
%token ARRAY "ARRAY"
%token AS "AS"
%token ASC "ASC"
%token ASSERTION "ASSERTION"
%token AT "AT"
%token AUTHORIZATION "AUTHORIZATION"
%token BEFORE "BEFORE"
%token BEGIN_ "BEGIN"
%token BINARY "BINARY"
%token BIT "BIT"
%token BLOB "BLOB"
%token BOOLEAN "BOOLEAN"
%token BOTH "BOTH"
%token BREADTH "BREADTH"
%token BY "BY"
%token CALL "CALL"
%token CASCADE "CASCADE"
%token CASCADED "CASCADED"
%token CASE "CASE"
%token CAST "CAST"
%token CATALOG "CATALOG"
%token CHAR "CHAR"
%token CHARACTER "CHARACTER"
%token CHECK "CHECK"
%token CLASS "CLASS"
%token CLOB "CLOB"
%token CLOSE "CLOSE"
%token COLLATE "COLLATE"
%token COLLATION "COLLATION"
%token COLUMN "COLUMN"
%token COMMIT "COMMIT"
%token COMPLETION "COMPLETION"
%token CONNECT "CONNECT"
%token CONNECTION "CONNECTION"
%token CONSTRAINT "CONSTRAINT"
%token CONSTRAINTS "CONSTRAINTS"
%token CONSTRUCTOR "CONSTRUCTOR"
%token CONTINUE "CONTINUE"
%token CORRESPONDING "CORRESPONDING"
%token CREATE "CREATE"
%token CROSS "CROSS"
%token CUBE "CUBE"
%token CURRENT "CURRENT"
%token CURRENT_DATE "CURRENT_DATE"
%token CURRENT_PATH "CURRENT_PATH"
%token CURRENT_ROLE "CURRENT_ROLE"
%token CURRENT_TIME "CURRENT_TIME"
%token CURRENT_TIMESTAMP "CURRENT_TIMESTAMP"
%token CURRENT_USER "CURRENT_USER"
%token CURSOR "CURSOR"
%token CYCLE "CYCLE"
%token DATA "DATA"
%token DATE "DATE"
%token DAY "DAY"
%token DEALLOCATE "DEALLOCATE"
%token DEC "DEC"
%token DECIMAL "DECIMAL"
%token DECLARE "DECLARE"
%token DEFAULT "DEFAULT"
%token DEFERRABLE "DEFERRABLE"
%token DEFERRED "DEFERRED"
%token DELETE "DELETE"
%token DEPTH "DEPTH"
%token DEREF "DEREF"
%token DESC "DESC"
%token DESCRIBE "DESCRIBE"
%token DESCRIPTOR "DESCRIPTOR"
%token DESTROY "DESTROY"
%token DESTRUCTOR "DESTRUCTOR"
%token DETERMINISTIC "DETERMINISTIC"
%token DICTIONARY "DICTIONARY"
%token DIAGNOSTICS "DIAGNOSTICS"
%token DISCONNECT "DISCONNECT"
%token DISTINCT "DISTINCT"
%token DOMAIN "DOMAIN"
%token DOUBLE "DOUBLE"
%token DROP "DROP"
%token DYNAMIC "DYNAMIC"
%token EACH "EACH"
%token ELSE "ELSE"
%token END "END"
%token END_EXEC "END-EXEC"
%token EQUALS "EQUALS"
%token ESCAPE "ESCAPE"
%token EVERY "EVERY"
%token EXCEPT "EXCEPT"
%token EXCEPTION "EXCEPTION"
%token EXEC "EXEC"
%token EXECUTE "EXECUTE"
%token EXTERNAL "EXTERNAL"
%token FALSE "FALSE"
%token FETCH "FETCH"
%token FIRST "FIRST"
%token FLOAT "FLOAT"
%token FOR "FOR"
%token FOREIGN "FOREIGN"
%token FOUND "FOUND"
%token FROM "FROM"
%token FREE "FREE"
%token FULL "FULL"
%token FUNCTION "FUNCTION"
%token GENERAL "GENERAL"
%token GET "GET"
%token GLOBAL "GLOBAL"
%token GO "GO"
%token GOTO "GOTO"
%token GRANT "GRANT"
%token GROUP "GROUP"
%token GROUPING "GROUPING"
%token HAVING "HAVING"
%token HOST "HOST"
%token HOUR "HOUR"
%token IDENTITY "IDENTITY"
%token IGNORE "IGNORE"
%token IMMEDIATE "IMMEDIATE"
%token IN "IN"
%token INDICATOR "INDICATOR"
%token INITIALIZE "INITIALIZE"
%token INITIALLY "INITIALLY"
%token INNER "INNER"
%token INOUT "INOUT"
%token INPUT "INPUT"
%token INSERT "INSERT"
%token INT "INT"
%token INTEGER "INTEGER"
%token INTERSECT "INTERSECT"
%token INTERVAL "INTERVAL"
%token INTO "INTO"
%token IS "IS"
%token ISOLATION "ISOLATION"
%token ITERATE "ITERATE"
%token JOIN "JOIN"
%token KEY "KEY"
%token LANGUAGE "LANGUAGE"
%token LARGE "LARGE"
%token LAST "LAST"
%token LATERAL "LATERAL"
%token LEADING "LEADING"
%token LEFT "LEFT"
%token LESS "LESS"
%token LEVEL "LEVEL"
%token LIKE "LIKE"
%token LIMIT "LIMIT"
%token LOCAL "LOCAL"
%token LOCALTIME "LOCALTIME"
%token LOCALTIMESTAMP "LOCALTIMESTAMP"
%token LOCATOR "LOCATOR"
%token MAP "MAP"
%token MATCH "MATCH"
%token MINUTE "MINUTE"
%token MODIFIES "MODIFIES"
%token MODIFY "MODIFY"
%token MODULE "MODULE"
%token MONTH "MONTH"
%token NAMES "NAMES"
%token NATIONAL "NATIONAL"
%token NATURAL "NATURAL"
%token NCHAR "NCHAR"
%token NCLOB "NCLOB"
%token NEW "NEW"
%token NEXT "NEXT"
%token NO "NO"
%token NONE "NONE"
%token NOT "NOT"
%token NULL_ "NULL"
%token NUMERIC "NUMERIC"
%token OBJECT "OBJECT"
%token OF "OF"
%token OFF "OFF"
%token OLD "OLD"
%token ON "ON"
%token ONLY "ONLY"
%token OPEN "OPEN"
%token OPERATION "OPERATION"
%token OPTION "OPTION"
%token OR "OR"
%token ORDER "ORDER"
%token ORDINALITY "ORDINALITY"
%token OUT "OUT"
%token OUTER "OUTER"
%token OUTPUT "OUTPUT"
%token PAD "PAD"
%token PARAMETER "PARAMETER"
%token PARAMETERS "PARAMETERS"
%token PARTIAL "PARTIAL"
%token PATH "PATH"
%token POSTFIX "POSTFIX"
%token PRECISION "PRECISION"
%token PREFIX "PREFIX"
%token PREORDER "PREORDER"
%token PREPARE "PREPARE"
%token PRESERVE "PRESERVE"
%token PRIMARY "PRIMARY"
%token PRIOR "PRIOR"
%token PRIVILEGES "PRIVILEGES"
%token PROCEDURE "PROCEDURE"
%token PUBLIC "PUBLIC"
%token READ "READ"
%token READS "READS"
%token REAL "REAL"
%token RECURSIVE "RECURSIVE"
%token REF "REF"
%token REFERENCES "REFERENCES"
%token REFERENCING "REFERENCING"
%token RELATIVE "RELATIVE"
%token RESTRICT "RESTRICT"
%token RESULT "RESULT"
%token RETURN "RETURN"
%token RETURNS "RETURNS"
%token REVOKE "REVOKE"
%token RIGHT "RIGHT"
%token ROLE "ROLE"
%token ROLLBACK "ROLLBACK"
%token ROLLUP "ROLLUP"
%token ROUTINE "ROUTINE"
%token ROW "ROW"
%token ROWS "ROWS"
%token SAVEPOINT "SAVEPOINT"
%token SCHEMA "SCHEMA"
%token SCROLL "SCROLL"
%token SCOPE "SCOPE"
%token SEARCH "SEARCH"
%token SECOND "SECOND"
%token SECTION "SECTION"
%token SELECT "SELECT"
%token SEQUENCE "SEQUENCE"
%token SESSION "SESSION"
%token SESSION_USER "SESSION_USER"
%token SET "SET"
%token SETS "SETS"
%token SIZE "SIZE"
%token SMALLINT "SMALLINT"
%token SOME "SOME"
%token SPACE "SPACE"
%token SPECIFIC "SPECIFIC"
%token SPECIFICTYPE "SPECIFICTYPE"
%token SQL "SQL"
%token SQLEXCEPTION "SQLEXCEPTION"
%token SQLSTATE "SQLSTATE"
%token SQLWARNING "SQLWARNING"
%token START "START"
%token STATE "STATE"
%token STATEMENT "STATEMENT"
%token STATIC "STATIC"
%token STRUCTURE "STRUCTURE"
%token SYSTEM_USER "SYSTEM_USER"
%token TABLE "TABLE"
%token TEMPORARY "TEMPORARY"
%token TERMINATE "TERMINATE"
%token THAN "THAN"
%token THEN "THEN"
%token TIME "TIME"
%token TIMESTAMP "TIMESTAMP"
%token TIMEZONE_HOUR "TIMEZONE_HOUR"
%token TIMEZONE_MINUTE "TIMEZONE_MINUTE"
%token TO "TO"
%token TRAILING "TRAILING"
%token TRANSACTION "TRANSACTION"
%token TRANSLATION "TRANSLATION"
%token TREAT "TREAT"
%token TRIGGER "TRIGGER"
%token TRUE "TRUE"
%token UNDER "UNDER"
%token UNION "UNION"
%token UNIQUE "UNIQUE"
%token UNKNOWN "UNKNOWN"
%token UNNEST "UNNEST"
%token UPDATE "UPDATE"
%token USAGE "USAGE"
%token USER "USER"
%token USING "USING"
%token VALUE "VALUE"
%token VALUES "VALUES"
%token VARCHAR "VARCHAR"
%token VARIABLE "VARIABLE"
%token VARYING "VARYING"
%token VIEW "VIEW"
%token WHEN "WHEN"
%token WHENEVER "WHENEVER"
%token WHERE "WHERE"
%token WITH "WITH"
%token WITHOUT "WITHOUT"
%token WORK "WORK"
%token WRITE "WRITE"
%token YEAR "YEAR"
%token ZONE "ZONE"

// extra operators
%token CONTAINS_OPERATOR "<@"
%token IS_CONTAINED_BY_OPERATOR "@>"
%token OVERLAPS_OPERATOR "&&"

// extra keywords
%token PLACING "PLACING"

%token TINYINT "TINYINT"
%token BIGINT "BIGINT"

%token BIT_AND "BIT_AND"
%token BIT_OR "BIT_OR"
%token BOOL_AND "BOOL_AND"
%token BOOL_OR "BOOL_OR"

%token DOT_ASTERISK ". *"
%token UNION_JOIN "UNION JOIN"

%token <ast::common::chars> REGULAR_IDENTIFIER
%token <ast::common::chars> DELIMITED_IDENTIFIER

%token <ast::common::chars> UNSIGNED_INTEGER
%token <ast::common::chars> EXACT_NUMERIC_LITERAL
%token <ast::common::chars> APPROXIMATE_NUMERIC_LITERAL
%token <ast::common::chars> CHARACTER_STRING_LITERAL
%token <ast::common::chars> BIT_STRING_LITERAL
%token <ast::common::chars> HEX_STRING_LITERAL

%token <ast::common::chars> HOST_PARAMETER_NAME

%token ERROR "<ERROR>"

%token END_OF_FILE 0 "<END_OF_FILE>"

%nterm <node_ptr<ast::statement::statement>> statement
%nterm <node_vector<ast::statement::statement>> statement_list

%nterm <node_ptr<ast::name::name>> target_table

%nterm <element_vector<ast::statement::set_element>> set_clause_list
%nterm <ast::statement::set_element> set_clause

%nterm <node_ptr<ast::scalar::expression>> manipulate_where_clause_opt

%nterm <node_ptr<ast::query::expression>> query_expression
%nterm <node_ptr<ast::query::expression>> query_expression_body
%nterm <node_ptr<ast::query::expression>> query_expression_primary

%nterm <element_vector<ast::query::with_element>> with_list
%nterm <ast::query::with_element> with_element
%nterm <node_vector<ast::name::simple>> with_column_list_opt

%nterm <ast::common::regioned<bool>> recursive_opt

%nterm <std::optional<ast::common::regioned<ast::scalar::set_quantifier>>> set_quantifier_opt
%nterm <std::optional<ast::query::corresponding_clause>> corresponding_spec_opt

%nterm <node_vector<ast::query::select_element>> select_list
%nterm <node_ptr<ast::query::select_element>> select_element
%nterm <node_ptr<ast::name::simple>> as_clause_opt
%nterm <node_ptr<ast::scalar::expression>> where_clause_opt
%nterm <std::optional<ast::query::group_by_clause>> group_by_clause_opt
%nterm <node_vector<ast::query::grouping_element>> grouping_column_reference_list
%nterm <node_ptr<ast::query::grouping_element>> grouping_column_reference
%nterm <node_ptr<ast::name::name>> collate_clause_opt
%nterm <node_ptr<ast::scalar::expression>> having_clause_opt
%nterm <element_vector<ast::common::sort_element>> order_by_clause_opt
%nterm <element_vector<ast::common::sort_element>> sort_specification_list
%nterm <ast::common::sort_element> sort_specification
%nterm <std::optional<ast::common::regioned<ast::common::ordering_specification>>> ordering_specification_opt
%nterm <node_ptr<ast::scalar::expression>> limit_clause_opt

%nterm <regioned<ast::table::join_type>> join_type
%nterm <regioned<ast::table::join_type>> natural_join_type
%nterm <node_ptr<ast::table::join_specification>> join_specification

%nterm <node_vector<ast::table::expression>> table_reference_list
%nterm <node_ptr<ast::table::expression>> table_reference
%nterm <node_ptr<ast::table::expression>> table_primary

%nterm <ast::table::correlation_clause> correlation_clause
%nterm <std::optional<ast::table::correlation_clause>> correlation_clause_opt
%nterm <node_ptr<ast::name::simple>> correlation_name
%nterm <node_vector<ast::name::simple>> derived_column_list_opt
%nterm <ast::common::regioned<bool>> with_ordinality_opt

%nterm <node_vector<ast::scalar::expression>> explicit_row_value_expression_list
%nterm <node_ptr<ast::scalar::expression>> explicit_row_value_expression

%nterm <node_vector<ast::scalar::expression>> value_expression_list
%nterm <node_ptr<ast::scalar::expression>> value_expression
%nterm <node_ptr<ast::scalar::expression>> predicate_expression
%nterm <node_ptr<ast::query::expression>> table_value_expression
%nterm <node_ptr<ast::scalar::expression>> row_value_expression
%nterm <node_ptr<ast::scalar::expression>> scalar_value_expression

%nterm <ast::common::regioned<ast::scalar::comparison_operator>> comparison_operator
%nterm <ast::common::regioned<ast::scalar::binary_operator>> extended_comparison_operator
%nterm <ast::common::regioned<bool>> not_opt
%nterm <std::optional<ast::common::regioned<ast::scalar::between_operator>>> symmetric_opt
%nterm <node_ptr<ast::scalar::expression>> escape_opt

%nterm <node_ptr<ast::scalar::expression>> value_expression_primary

%nterm <node_ptr<ast::scalar::expression>> system_function_invocation
%nterm <ast::common::regioned<ast::scalar::builtin_function_kind>> simple_system_function_name
%nterm <ast::common::regioned<ast::scalar::builtin_function_kind>> precise_system_function_name
%nterm <ast::common::regioned<ast::scalar::extract_field_kind>> extract_field
%nterm <ast::common::regioned<ast::scalar::trim_specification>> trim_specification

%nterm <node_ptr<ast::scalar::expression>> system_set_function_invocation
%nterm <ast::common::regioned<ast::scalar::builtin_set_function_kind>> computational_operation_except_count

%nterm <element_vector<ast::scalar::case_when_clause>> case_when_clause_list
%nterm <ast::scalar::case_when_clause> case_when_clause
%nterm <node_ptr<ast::scalar::expression>> else_expression_opt

%nterm <node_vector<ast::scalar::expression>> routine_invocation_arguments
%nterm <node_vector<ast::scalar::expression>> sql_argument_list_opt
%nterm <node_vector<ast::scalar::expression>> sql_argument_list
%nterm <node_ptr<ast::scalar::expression>> sql_argument

%nterm <ast::common::regioned<ast::scalar::quantifier>> quantifier

%nterm <node_ptr<ast::query::expression>> table_subquery
%nterm <node_ptr<ast::query::expression>> row_subquery
%nterm <node_ptr<ast::query::expression>> scalar_subquery
%nterm <node_ptr<ast::query::expression>> subquery

%nterm <node_ptr<ast::literal::literal>> literal
%nterm <node_ptr<ast::literal::literal>> unsigned_integer
%nterm <node_ptr<ast::literal::literal>> truth_literal
%nterm <std::optional<ast::common::regioned<ast::literal::sign>>> sign_opt
%nterm <element_vector<ast::common::regioned<ast::common::chars>>> concatenations_list_opt

%nterm <node_ptr<ast::type::type>> data_type
%nterm <node_ptr<ast::type::type>> data_type_system
%nterm <node_ptr<ast::type::type>> data_type_composite
%nterm <node_ptr<ast::type::type>> data_type_user

%nterm character_type_name
%nterm character_varying_type_name
%nterm <ast::common::regioned<ast::type::kind>> decimal_type_name
%nterm integer_type_name

%nterm <std::optional<ast::common::regioned<std::size_t>>> parenthesized_size_maybe_flexible_opt
%nterm <std::optional<ast::common::regioned<std::size_t>>> parenthesized_size_opt
%nterm <std::optional<ast::common::regioned<std::size_t>>> size_maybe_flexible_opt
%nterm <ast::common::regioned<std::size_t>> size_maybe_flexible
%nterm <ast::common::regioned<std::size_t>> size

%nterm <std::optional<ast::common::regioned<bool>>> with_or_without_time_zone_opt

%nterm <ast::type::field_definition> field_definition
%nterm <element_vector<ast::type::field_definition>> field_definition_list

%nterm <node_vector<ast::name::simple>> column_name_list

%nterm <node_ptr<ast::name::simple>> query_name
%nterm <node_ptr<ast::name::name>> table_name
%nterm <node_ptr<ast::name::simple>> column_name
%nterm <node_ptr<ast::name::simple>> field_name
%nterm <node_ptr<ast::name::simple>> cursor_name

%nterm <node_ptr<ast::name::simple>> host_parameter_name

%nterm <node_ptr<ast::name::name>> identifier_chain
%nterm <node_ptr<ast::name::simple>> identifier
%nterm <node_ptr<ast::name::simple>> contextual_identifier

// query expression
%left "UNION" "EXCEPT"
%left "INTERSECT"

// predicate expression
%left "OR"
%left "AND"
%precedence UNARY_NOT

// value expression
%left "+" "-" "||"
%left "*" "/"
%precedence UNARY_PLUS UNARY_MINUS

%start program

%%

program
    : statement_list[L] END_OF_FILE
        {
            driver.success($L);
        }
    ;

statement_list
    : statement_list[L] statement[s]
        {
            $$ = $L; $$.emplace_back($s);
        }
    | %empty
        {
            $$ = driver.node_vector<ast::statement::statement>();
        }
    ;

statement
    : ";"
        {
            $$ = driver.node<ast::statement::empty_statement>(@$);
        }
    | query_expression[e] ";"
        {
            $$ = driver.node<ast::statement::select_statement>(
                    $e,
                    driver.element_vector<ast::statement::target_element>(),
                    @$);
        }
    | "INSERT" "INTO" table_name[n] "DEFAULT" "VALUES" ";"
        {
            $$ = driver.node<ast::statement::insert_statement>(
                    $n,
                    driver.node_vector<ast::name::simple>(),
                    nullptr,
                    @$);
        }
    | "INSERT" "INTO" table_name[n] query_expression[e] ";"
        {
            $$ = driver.node<ast::statement::insert_statement>(
                    $n,
                    driver.node_vector<ast::name::simple>(),
                    $e,
                    @$);
        }
    | "INSERT" "INTO" table_name[n] "(" column_name_list[c] ")" query_expression[e] ";"
        {
            // NOTE: don't insert reduce point before left paren
            //       because both insert column list and query expression accepts it
            $$ = driver.node<ast::statement::insert_statement>(
                    $n,
                    $c,
                    $e,
                    @$);
        }
    | "UPDATE" target_table[t] "SET" set_clause_list[s] manipulate_where_clause_opt[w] ";"
        {
            $$ = driver.node<ast::statement::update_statement>(
                    $t,
                    $s,
                    $w,
                    @$);
        }
    | "DELETE" "FROM" target_table[t] manipulate_where_clause_opt[w] ";"
        {
            $$ = driver.node<ast::statement::delete_statement>(
                    $t,
                    $w,
                    @$);
        }
    ;

target_table
    : table_name[n]
        {
            $$ = $n;
        }
    ;

set_clause_list
    : set_clause_list[L] "," set_clause[c]
        {
            $$ = $L;
            $$.emplace_back($c);
        }
    | set_clause[c]
        {
            $$ = driver.element_vector<ast::statement::set_element>();
            $$.emplace_back($c);
        }
    ;

set_clause
    : identifier_chain[n] "=" value_expression[e]
        {
            $$ = ast::statement::set_element {
                    $n,
                    $e,
            };
        }
    ;

manipulate_where_clause_opt
    : %empty
        {
            $$ = nullptr;
        }
    | "WHERE" value_expression[e]
        {
            $$ = $e;
        }
    | "WHERE" "CURRENT"[c] "OF" cursor_name[n]
        {
            $$ = driver.node<ast::scalar::current_of_cursor>(
                    $n,
                    @c | @n);
        }
    ;

query_expression
    : "WITH" recursive_opt[r] with_list[w] query_expression_body[e]
        {
            $$ = driver.node<ast::query::with_expression>(
                    $r,
                    $w,
                    $e,
                    @$);
        }
    | query_expression_body[e]
        {
            $$ = $e;
        }
    ;

recursive_opt
    : %empty
        {
            $$ = {};
        }
    | "RECURSIVE"
        {
            $$ = { true, @$ };
        }
    ;

with_list
    : with_list[L] "," with_element[c]
        {
            $$ = $L;
            $$.emplace_back($c);
        }
    | with_element[c]
        {
            $$ = driver.element_vector<ast::query::with_element>();
            $$.emplace_back($c);
        }
    ;

with_element
    : query_name[n] with_column_list_opt[c] "AS" "(" query_expression[q] ")"
        {
            $$ = ast::query::with_element {
                    $n,
                    $c,
                    $q,
                    @$,
            };
        }
    ;

with_column_list_opt
    : %empty
        {
            $$ = driver.node_vector<ast::name::simple>();
        }
    | "(" column_name_list[L] ")"
        {
            $$ = $L;
        }
    ;

query_expression_body
    : query_expression_body[l] "UNION"[o] set_quantifier_opt[q] corresponding_spec_opt[c] query_expression_body[r]
        {
            $$ = driver.node<ast::query::binary_expression>(
                    $l,
                    regioned { ast::query::binary_operator::union_, @o },
                    $q,
                    $c,
                    $r,
                    @$);
        }
    | query_expression_body[l] "EXCEPT"[o] set_quantifier_opt[q] corresponding_spec_opt[c] query_expression_body[r]
        {
            $$ = driver.node<ast::query::binary_expression>(
                    $l,
                    regioned { ast::query::binary_operator::except, @o },
                    $q,
                    $c,
                    $r,
                    @$);
        }
    | query_expression_body[l] "INTERSECT"[o] set_quantifier_opt[q] corresponding_spec_opt[c] query_expression_body[r]
        {
            $$ = driver.node<ast::query::binary_expression>(
                    $l,
                    regioned { ast::query::binary_operator::intersect, @o },
                    $q,
                    $c,
                    $r,
                    @$);
        }
    // FIXME: to avoid conflict, parenthesized expression was moved from _primary
    | "(" query_expression_body[e] ")"
        {
            $$ = $e;
        }
    | query_expression_primary[e]
        {
            $$ = $e;
        }
    ;

query_expression_primary
    : "SELECT"
            set_quantifier_opt[q]
            select_list[s]
            "FROM" table_reference_list[t]
            where_clause_opt[w]
            group_by_clause_opt[g]
            having_clause_opt[h]
            order_by_clause_opt[o]
            limit_clause_opt[l]
        {
            $$ = driver.node<ast::query::query>(
                    $q,
                    $s,
                    $t,
                    $w,
                    $g,
                    $h,
                    $o,
                    $l,
                    @$);
        }
    | "TABLE" table_name[n]
        {
            $$ = driver.node<ast::query::table_reference>(
                    $n,
                    @$);
        }
    | "VALUES" explicit_row_value_expression_list[l]
        {
            $$ = driver.node<ast::query::table_value_constructor>(
                    $l,
                    @$);
        }
    ;

select_list
    : select_list[L] "," select_element[e]
        {
            $$ = $L;
            $$.emplace_back($e);
        }
    | select_element[e]
        {
            $$ = driver.node_vector<ast::query::select_element>();
            $$.emplace_back($e);
        }
    ;

select_element
    : "*"
        {
            $$ = driver.node<ast::query::select_asterisk>(
                nullptr,
                @$);
        }
    | value_expression[q] ". *"
        {
            $$ = driver.node<ast::query::select_asterisk>(
                $q,
                @$);
        }
    | value_expression[e] as_clause_opt[a]
        {
            $$ = driver.node<ast::query::select_column>(
                $e,
                $a,
                @$);
        }
    ;

as_clause_opt
    : %empty
        {
            $$ = nullptr;
        }
    | "AS" column_name[n]
        {
            $$ = $n;
        }
    ;

table_reference_list
    : table_reference_list[L] "," table_reference[e]
        {
            $$ = $L;
            $$.emplace_back($e);
        }
    | table_reference[e]
        {
            $$ = driver.node_vector<ast::table::expression>();
            $$.emplace_back($e);
        }
    ;

where_clause_opt
    : %empty
        {
            $$ = nullptr;
        }
    | "WHERE" value_expression[e]
        {
            $$ = $e;
        }
    ;

group_by_clause_opt
    : %empty
        {
            $$ = std::nullopt;
        }
    | "GROUP" "BY" "(" ")"
        {
            $$ = ast::query::group_by_clause {
                    driver.node_vector<ast::query::grouping_element>(), // grand total
                    @$,
            };
        }
    | "GROUP" "BY" grouping_column_reference_list[l]
        {
            $$ = ast::query::group_by_clause {
                    $l,
                    @$,
            };
        }
    // FIXME: 7.9 <group by clause>
    ;

grouping_column_reference_list
    : grouping_column_reference_list[L] "," grouping_column_reference[e]
        {
            $$ = $L;
            $$.emplace_back($e);
        }
    | grouping_column_reference[e]
        {
            $$ = driver.node_vector<ast::query::grouping_element>();
            $$.emplace_back($e);
        }
    ;

grouping_column_reference
    : value_expression[e] collate_clause_opt[c]
        {
            $$ = driver.node<ast::query::grouping_column>(
                    $e,
                    $c,
                    @$);
        }
    ;

collate_clause_opt
    : %empty
        {
            $$ = nullptr;
        }
    | "COLLATE" identifier_chain[n]
        {
            $$ = $n;
        }
    ;

having_clause_opt
    : %empty
        {
            $$ = nullptr;
        }
    | "HAVING" value_expression[e]
        {
            $$ = $e;
        }
    ;

order_by_clause_opt
    : %empty
        {
            $$ = driver.element_vector<ast::common::sort_element>();
        }
    | "ORDER" "BY" sort_specification_list[l]
        {
            $$ = $l;
        }
    ;

sort_specification_list
    : sort_specification_list[L] "," sort_specification[e]
        {
            $$ = $L;
            $$.emplace_back($e);
        }
    | sort_specification[e]
        {
            $$ = driver.element_vector<ast::common::sort_element>();
            $$.emplace_back($e);
        }
    ;

sort_specification
    : value_expression[e] collate_clause_opt[c] ordering_specification_opt[o]
        {
            $$ = ast::common::sort_element {
                    $e,
                    $c,
                    $o,
                    @$,
            };
        }
    ;

ordering_specification_opt
    : %empty
        {
            $$ = std::nullopt;
        }
    | "ASC"
        {
            $$ = regioned { ast::common::ordering_specification::asc, @$ };
        }
    | "DESC"
        {
            $$ = regioned { ast::common::ordering_specification::desc, @$ };
        }
    ;

limit_clause_opt
    : %empty
        {
            $$ = nullptr;
        }
    | "LIMIT" value_expression[e]
        {
            $$ = $e;
        }
    ;

table_reference
    : table_reference[l] "CROSS"[t] "JOIN"[u] table_primary[r]
        {
            $$ = driver.node<ast::table::join>(
                    $l,
                    regioned { ast::table::join_type::cross, @t | @u },
                    $r,
                    nullptr,
                    @$);
        }
    | table_reference[l] join_type[t] table_reference[r] join_specification[s]
        {
            $$ = driver.node<ast::table::join>(
                    $l,
                    $t,
                    $r,
                    $s,
                    @$);
        }
    | table_reference[l] natural_join_type[t] table_primary[r]
        {
            $$ = driver.node<ast::table::join>(
                    $l,
                    $t,
                    $r,
                    nullptr,
                    @$);
        }
    | table_reference[l] "UNION JOIN"[t] table_primary[r]
        {
            $$ = driver.node<ast::table::join>(
                    $l,
                    regioned { ast::table::join_type::union_, @t },
                    $r,
                    nullptr,
                    @$);
        }
    | table_primary[t]
        {
            $$ = $t;
        }
    ;

join_type
    : "JOIN"
        {
            $$ = regioned { ast::table::join_type::inner, @$ };
        }
    | "INNER" "JOIN"
        {
            $$ = regioned { ast::table::join_type::inner, @$ };
        }
    | "LEFT" "JOIN"
        {
            $$ = regioned { ast::table::join_type::left_outer, @$ };
        }
    | "LEFT" "OUTER" "JOIN"
        {
            $$ = regioned { ast::table::join_type::left_outer, @$ };
        }
    | "RIGHT" "JOIN"
        {
            $$ = regioned { ast::table::join_type::right_outer, @$ };
        }
    | "RIGHT" "OUTER" "JOIN"
        {
            $$ = regioned { ast::table::join_type::right_outer, @$ };
        }
    | "FULL" "JOIN"
        {
            $$ = regioned { ast::table::join_type::full_outer, @$ };
        }
    | "FULL" "OUTER" "JOIN"
        {
            $$ = regioned { ast::table::join_type::full_outer, @$ };
        }
    ;

natural_join_type
    : "NATURAL" "JOIN"
        {
            $$ = regioned { ast::table::join_type::natural_inner, @$ };
        }
    | "NATURAL" "INNER" "JOIN"
        {
            $$ = regioned { ast::table::join_type::natural_inner, @$ };
        }
    | "NATURAL" "LEFT" "JOIN"
        {
            $$ = regioned { ast::table::join_type::natural_left_outer, @$ };
        }
    | "NATURAL" "LEFT" "OUTER" "JOIN"
        {
            $$ = regioned { ast::table::join_type::natural_left_outer, @$ };
        }
    | "NATURAL" "RIGHT" "JOIN"
        {
            $$ = regioned { ast::table::join_type::natural_right_outer, @$ };
        }
    | "NATURAL" "RIGHT" "OUTER" "JOIN"
        {
            $$ = regioned { ast::table::join_type::natural_right_outer, @$ };
        }
    | "NATURAL" "FULL" "JOIN"
        {
            $$ = regioned { ast::table::join_type::natural_full_outer, @$ };
        }
    | "NATURAL" "FULL" "OUTER" "JOIN"
        {
            $$ = regioned { ast::table::join_type::natural_full_outer, @$ };
        }
    ;

join_specification
    : "ON" value_expression[e]
        {
            $$ = driver.node<ast::table::join_condition>($e, @$);
        }
    | "USING" "(" column_name_list[c] ")"
        {
            $$ = driver.node<ast::table::join_columns>($c, @$);
        }
    ;

table_primary
    : table_name[n] correlation_clause_opt[c]
        {
            $$ = driver.node<ast::table::table_reference>(
                    regioned { false },
                    $n,
                    $c,
                    @$);
        }
    | "(" query_expression[e] ")" correlation_clause[c]
        {
            $$ = driver.node<ast::table::subquery>(
                    regioned { false },
                    $e,
                    $c,
                    @$);
        }
    | "LATERAL"[q] "(" query_expression[e] ")" correlation_clause[c]
        {
            $$ = driver.node<ast::table::subquery>(
                    regioned { true, @q },
                    $e,
                    $c,
                    @$);
        }
    | "UNNEST" "(" value_expression[e] ")" with_ordinality_opt[o] correlation_clause[c]
        {
            $$ = driver.node<ast::table::unnest>(
                    $e,
                    $o,
                    $c,
                    @$);
        }
    | "ONLY" "(" table_name[n] ")" correlation_clause_opt[c]
        {
            $$ = driver.node<ast::table::table_reference>(
                    regioned { false },
                    $n,
                    $c,
                    @$);
        }
    ;

field_definition_list
    : field_definition_list[L] "," field_definition[e]
        {
            $$ = $L;
            $$.emplace_back($e);
        }
    | field_definition[e]
        {
            $$ = driver.element_vector<ast::type::field_definition>();
            $$.emplace_back($e);
        }
    ;

field_definition
    : field_name[n] data_type[t] collate_clause_opt[c]
        {
            $$ = ast::type::field_definition {
                    $n,
                    $t,
                    $c,
                    @$,
            };
        }
    ;

with_ordinality_opt
    : %empty
        {
            $$ = regioned { false };
        }
    | "WITH" "ORDINALITY"
        {
            $$ = regioned { true, @$ };
        }
    ;

correlation_clause_opt
    : %empty
        {
            $$ = std::nullopt;
        }
    | correlation_clause[c]
        {
            $$ = $c;
        }
    ;

correlation_clause
    : correlation_name[n] derived_column_list_opt[c]
        {
            $$ = ast::table::correlation_clause {
                    $n,
                    $c,
                    @$,
            };
        }
    | "AS" correlation_name[n] derived_column_list_opt[c]
        {
            $$ = ast::table::correlation_clause {
                    $n,
                    $c,
                    @$,
            };
        }
    ;

derived_column_list_opt
    : %empty
        {
            $$ = driver.node_vector<ast::name::simple>();
        }
    | "(" column_name_list[L] ")"
        {
            $$ = $L;
        }
    ;

// NOTE: explicitly require () to avoid syntactic conflict
explicit_row_value_expression_list
    : explicit_row_value_expression_list[L] "," explicit_row_value_expression[e]
        {
            $$ = $L;
            $$.emplace_back($e);
        }
    | explicit_row_value_expression[e]
        {
            $$ = driver.node_vector<ast::scalar::expression>();
            $$.emplace_back($e);
        }
    ;

explicit_row_value_expression
    : "ROW"[k] "(" value_expression_list[e] ")"
        {
            $$ = driver.node<ast::scalar::value_constructor>(
                    regioned { ast::scalar::value_constructor_kind::row, @k },
                    $e,
                    @$);
        }
    | "(" value_expression_list[e] ")"
        {
            $$ = driver.node<ast::scalar::value_constructor>(
                    regioned { ast::scalar::value_constructor_kind::row },
                    $e,
                    @$);
        }
    | row_subquery[e]
        {
            $$ = driver.node<ast::scalar::subquery>($e, @$);
        }
    ;

value_expression_list
    : value_expression_list[L] "," value_expression[e]
        {
            $$ = $L;
            $$.emplace_back($e);
        }
    | value_expression[e]
        {
            $$ = driver.node_vector<ast::scalar::expression>();
            $$.emplace_back($e);
        }
    ;

value_expression
    : value_expression[l] "OR"[o] value_expression[r]
        {
            $$ = driver.node<ast::scalar::binary_expression>(
                    $l,
                    regioned { ast::scalar::binary_operator::or_, @o },
                    $r,
                    @$);
        }
    | value_expression[l] "AND"[o] value_expression[r]
        {
            $$ = driver.node<ast::scalar::binary_expression>(
                    $l,
                    regioned { ast::scalar::binary_operator::and_, @o },
                    $r,
                    @$);
        }
    | "NOT"[o] value_expression[e] %prec UNARY_NOT
        {
            $$ = driver.node<ast::scalar::unary_expression>(
                    regioned { ast::scalar::unary_operator::not_, @o },
                    $e,
                    @$);
        }
    | predicate_expression[l] "IS"[o] not_opt[n] truth_literal[r]
        {
            regioned op = { ast::scalar::binary_operator::is, @o };
            if ($n) {
                op = { ast::scalar::binary_operator::is_not, @o | @n };
            }
            $$ = driver.node<ast::scalar::binary_expression>(
                    $l,
                    op,
                    driver.node<ast::scalar::literal_expression>($r, @r),
                    @$);
        }
    | predicate_expression[e]
        {
            $$ = $e;
        }
    ;

predicate_expression
    : row_value_expression[l] comparison_operator[o] row_value_expression[r]
        {
            $$ = driver.node<ast::scalar::comparison_predicate>(
                    $l,
                    $o,
                    $r,
                    @$);
        }
    | row_value_expression[l] extended_comparison_operator[o] row_value_expression[r]
        {
            $$ = driver.node<ast::scalar::binary_expression>(
                    $l,
                    $o,
                    $r,
                    @$);
        }
    | row_value_expression[l] comparison_operator[o] quantifier[q] table_subquery[r]
        {
            $$ = driver.node<ast::scalar::quantified_comparison_predicate>(
                    $l,
                    $o,
                    $q,
                    $r,
                    @$);
        }
    | row_value_expression[t] not_opt[n] "BETWEEN"[o] symmetric_opt[s]
            row_value_expression[l] "AND" row_value_expression[r]
        {
            $$ = driver.node<ast::scalar::between_predicate>(
                    $t,
                    $n,
                    $s,
                    $l,
                    $r,
                    @$);
        }
    | row_value_expression[l] not_opt[n] "IN"[o] table_value_expression[r]
        {
            $$ = driver.node<ast::scalar::in_predicate>(
                    $l,
                    $n,
                    $r,
                    @$);
        }
    | row_value_expression[m] not_opt[n] "LIKE"[o] row_value_expression[p] escape_opt[e]
        {
            $$ = driver.node<ast::scalar::pattern_match_predicate>(
                    $m,
                    $n,
                    regioned { ast::scalar::pattern_match_operator::like, @o },
                    $p,
                    $e,
                    @$);
        }
    | row_value_expression[m] not_opt[n] "SIMILAR"[s] "TO"[t] row_value_expression[p] escape_opt[e]
        {
            $$ = driver.node<ast::scalar::pattern_match_predicate>(
                    $m,
                    $n,
                    regioned { ast::scalar::pattern_match_operator::similar_to, @s | @t },
                    $p,
                    $e,
                    @$);
        }
    | "EXISTS"[o] table_subquery[e]
        {
            $$ = driver.node<ast::scalar::table_predicate>(
                    regioned { ast::scalar::table_operator::exists, @o },
                    $e,
                    @$);
        }
    | "UNIQUE"[o] table_subquery[e]
        {
            $$ = driver.node<ast::scalar::table_predicate>(
                    regioned { ast::scalar::table_operator::unique, @o },
                    $e,
                    @$);
        }
    // 8.12 <overlaps predicate>
    | row_value_expression[l] "OVERLAPS"[o] row_value_expression[r]
        {
            $$ = driver.node<ast::scalar::binary_expression>(
                    $l,
                    regioned { ast::scalar::binary_operator::overlaps, @o },
                    $r,
                    @$);
        }
    // FIXME: 8.13 <distinct predicate> - avoid conflict
/*
    | row_value_expression[l] "IS"[i] "DISTINCT"[d] "FROM"[f] row_value_expression[r]
        {
            $$ = driver.node<ast::scalar::binary_expression>(
                    $l,
                    regioned { ast::scalar::binary_operator::is_distinct_from, @i | @f },
                    $r,
                    @$);
        }
*/
    | row_value_expression[e]
        {
            $$ = $e;
        }
    ;

comparison_operator
    : "="
        {
            $$ = { ast::scalar::comparison_operator::equals, @$ };
        }
    | "<>"
        {
            $$ = { ast::scalar::comparison_operator::not_equals, @$ };
        }
    | "<"
        {
            $$ = { ast::scalar::comparison_operator::less_than, @$ };
        }
    | ">"
        {
            $$ = { ast::scalar::comparison_operator::greater_than, @$ };
        }
    | "<="
        {
            $$ = { ast::scalar::comparison_operator::less_than_or_equals, @$ };
        }
    | ">="
        {
            $$ = { ast::scalar::comparison_operator::greater_than_or_equals, @$ };
        }
    ;

extended_comparison_operator
    : "<@"
        {
            $$ = { ast::scalar::binary_operator::contains, @$ };
        }
    | "@>"
        {
            $$ = { ast::scalar::binary_operator::is_contained_by, @$ };
        }
    | "&&"
        {
            $$ = { ast::scalar::binary_operator::have_elements_in_common, @$ };
        }
    ;

not_opt
    : %empty
        {
            $$ = { false };
        }
    | "NOT"
        {
            $$ = { true, @$ };
        }
    ;

symmetric_opt
    : %empty
        {
            $$ = std::nullopt;
        }
    | "ASYMMETRIC"
        {
            $$ = { ast::scalar::between_operator::asymmetric, @$ };
        }
    | "SYMMETRIC"
        {
            $$ = { ast::scalar::between_operator::symmetric, @$ };
        }
    ;

escape_opt
    : %empty
        {
            $$ = nullptr;
        }
    | "ESCAPE" row_value_expression[e]
        {
            $$ = $e;
        }
    ;

table_value_expression
    : table_subquery[e]
        {
            $$ = $e;
        }
    | "(" value_expression_list[l] ")"
        {
            $$ = driver.node<ast::query::table_value_constructor>(
                    $l,
                    @$);
        }
    ;

row_value_expression
    : "ROW"[k] "(" value_expression_list[e] ")"
        {
            $$ = driver.node<ast::scalar::value_constructor>(
                    regioned { ast::scalar::value_constructor_kind::row, @k },
                    $e,
                    @$);
        }
    // NOTE: to avoid conflict, "row value constructor" without "ROW" was moved in value_expression_primary
    | scalar_value_expression[e]
         {
             $$ = $e;
         }
     ;

scalar_value_expression
    : scalar_value_expression[l] "+"[o] scalar_value_expression[r]
        {
            $$ = driver.node<ast::scalar::binary_expression>(
                    $l,
                    regioned { ast::scalar::binary_operator::plus, @o },
                    $r,
                    @$);
        }
    | scalar_value_expression[l] "-"[o] scalar_value_expression[r]
        {
            $$ = driver.node<ast::scalar::binary_expression>(
                    $l,
                    regioned { ast::scalar::binary_operator::minus, @o },
                    $r,
                    @$);
        }
    | scalar_value_expression[l] "*"[o] scalar_value_expression[r]
        {
            $$ = driver.node<ast::scalar::binary_expression>(
                    $l,
                    regioned { ast::scalar::binary_operator::asterisk, @o },
                    $r,
                    @$);
        }
    | scalar_value_expression[l] "/"[o] scalar_value_expression[r]
        {
            $$ = driver.node<ast::scalar::binary_expression>(
                    $l,
                    regioned { ast::scalar::binary_operator::solidus, @o },
                    $r,
                    @$);
        }
    | scalar_value_expression[l] "||"[o] scalar_value_expression[r]
        {
            $$ = driver.node<ast::scalar::binary_expression>(
                    $l,
                    regioned { ast::scalar::binary_operator::concatenation, @o },
                    $r,
                    @$);
        }
    | "+"[o] scalar_value_expression[e] %prec UNARY_PLUS
        {
            $$ = driver.node<ast::scalar::unary_expression>(
                    regioned { ast::scalar::unary_operator::plus, @o },
                    $e,
                    @$);
        }
    | "-"[o] scalar_value_expression[e] %prec UNARY_MINUS
        {
            $$ = driver.node<ast::scalar::unary_expression>(
                    regioned { ast::scalar::unary_operator::minus, @o },
                    $e,
                    @$);
        }
    | value_expression_primary[e]
         {
             $$ = $e;
         }
     ;

value_expression_primary
    : literal[l]
        {
            $$ = driver.node<ast::scalar::literal_expression>(
                    $l,
                    @$);
        }
    // 6.6 <column reference>
    // 6.7 <SQL parameter reference>
    | identifier[n]
        {
            // NOTE: to avoid conflict, variable reference only can handle simple name
            $$ = driver.node<ast::scalar::variable_reference>(
                    $n,
                    @$);
        }
    | host_parameter_name[n]
        {
            $$ = driver.node<ast::scalar::host_parameter_reference>(
                    $n,
                    @$);
        }
    // 6.8 <field reference>
    | value_expression_primary[q] "."[o] identifier[n]
        {
            auto q = $q;
            auto n = $n;
            if (auto c = driver.try_build_identifier_chain(q, n)) {
                $$ = driver.node<ast::scalar::variable_reference>(
                        std::move(c),
                        @$);
            } else {
                $$ = driver.node<ast::scalar::field_reference>(
                        std::move(q),
                        regioned { ast::scalar::reference_operator::period, @o },
                        std::move(n),
                        @$);
            }
        }
    // 10.4 <routine invocation>
    | identifier[n] routine_invocation_arguments[l]
        {
            $$ = driver.node<ast::scalar::function_invocation>(
                    $n,
                    $l,
                    @$);
        }
    // 6.10 <method reference>
    | value_expression_primary[q] "->"[o] identifier[n] routine_invocation_arguments[l]
        {
            $$ = driver.node<ast::scalar::method_invocation>(
                    $q,
                    regioned { ast::scalar::reference_operator::arrow, @o },
                    $n,
                    $l,
                    @$);
        }
    // 6.11 <method invocation>
    | value_expression_primary[q] "."[o] identifier[n] routine_invocation_arguments[l]
        {
            auto q = $q;
            auto n = $n;
            auto l = $l;
            if (auto c = driver.try_build_identifier_chain(q, n)) {
                $$ = driver.node<ast::scalar::function_invocation>(
                        std::move(c),
                        std::move(l),
                        @$);
            } else {
                $$ = driver.node<ast::scalar::method_invocation>(
                        std::move(q),
                        regioned { ast::scalar::reference_operator::period, @o },
                        std::move(n),
                        std::move(l),
                        @$);
            }
        }
    // 6.12 <static method invocation>
    | value_expression_primary[q] "::" identifier[n] routine_invocation_arguments[l]
        {
            auto q = $q;
            if (auto t = driver.try_build_type(q)) {
                $$ = driver.node<ast::scalar::static_method_invocation>(
                        std::move(t),
                        $n,
                        $l,
                        @$);
            } else {
                // syntax error
                driver.error(@q, "syntax error: must be a type");
                $$ = nullptr;
            }
        }
    // FIXME: user defined aggregate functions
    // 6.13 <element reference>
    | value_expression_primary[l] "["[o] value_expression[r] "]"
        {
            $$ = driver.node<ast::scalar::binary_expression>(
                    $l,
                    regioned { ast::scalar::binary_operator::element_reference, @o },
                    $r,
                    @$);
        }
    // 6.14 <dereference operation>
    | value_expression_primary[q] "->"[o] identifier[n]
        {
            $$ = driver.node<ast::scalar::field_reference>(
                    $q,
                    regioned { ast::scalar::reference_operator::arrow, @o },
                    $n,
                    @$);
        }
    // 6.15 <reference resolution>
    | "DEREF"[o] "(" value_expression[e] ")"
        {
            $$ = driver.node<ast::scalar::unary_expression>(
                    regioned { ast::scalar::unary_operator::reference_resolution, @o },
                    $e,
                    @$);
        }
    // 6.16 <set function specification>
    | system_set_function_invocation[e]
        {
            $$ = $e;
        }
    // 6.17 <numeric value function>
    // 6.18 <string value function>
    // 6.19 <datetime value function>
    // 6.20 <interval value function>
    | system_function_invocation[e]
        {
            $$ = $e;
        }
    // 6.21 <case expression> - <case abbreviation>
    | "NULLIF"[f] "(" value_expression[l] "," value_expression[r] ")"
        {
            $$ = driver.node<ast::scalar::builtin_function_invocation>(
                    regioned { ast::scalar::builtin_function_kind::nullif, @f },
                    driver.to_node_vector<ast::scalar::expression>($l, $r),
                    @$);
        }
    // 6.21 <case expression> - <case abbreviation>
    | "COALESCE"[f] "(" value_expression_list[l] ")"
        {
            $$ = driver.node<ast::scalar::builtin_function_invocation>(
                    regioned { ast::scalar::builtin_function_kind::coalesce, @f },
                    $l,
                    @$);
        }
    // 6.21 <case expression> - <simple case>
    | "CASE" value_expression[e] case_when_clause_list[w] else_expression_opt[l] "END"
        {
            $$ = driver.node<ast::scalar::case_expression>(
                    $e,
                    $w,
                    $l,
                    @$);
        }
    // 6.21 <case expression> - <searched case>
    | "CASE" case_when_clause_list[w] else_expression_opt[l] "END"
        {
            $$ = driver.node<ast::scalar::case_expression>(
                    nullptr,
                    $w,
                    $l,
                    @$);
        }
    // 6.22 <cast specification>
    | "CAST"[o] "(" value_expression[e] "AS" data_type[t] ")"
        {
            $$ = driver.node<ast::scalar::cast_expression>(
                    regioned { ast::scalar::cast_operator::cast, @o },
                    $e,
                    $t,
                    @$);
        }
    // 6.24 <new specification>
    | "NEW" data_type_user[t] "(" sql_argument_list_opt[l] ")"
        {
            $$ = driver.node<ast::scalar::new_invocation>(
                    $t,
                    $l,
                    @$);
        }
    // 6.25 <subtype treatment>
    | "TREAT"[o] "(" value_expression[e] "AS" data_type[t] ")"
        {
            $$ = driver.node<ast::scalar::cast_expression>(
                    regioned { ast::scalar::cast_operator::treat, @o },
                    $e,
                    $t,
                    @$);
        }
    // FIXME: 6.29 <interval value expression> <interval qualifier>
    // 6.32 <array value constructor>
    | "ARRAY"[o] "[" value_expression_list[l] "]"
        {
                $$ = driver.node<ast::scalar::value_constructor>(
                        regioned { ast::scalar::value_constructor_kind::array, @o },
                        $l,
                        @$);
        }
    // 7.14 <subquery>
    | scalar_subquery[e]
        {
            $$ = driver.node<ast::scalar::subquery>($e, @$);
        }
    // 6.11 <method invocation> - <generalized invocation>
    | "(" value_expression[e] "AS" data_type[t] ")"
        {
            $$ = driver.node<ast::scalar::cast_expression>(
                    regioned { ast::scalar::cast_operator::generalize },
                    $e,
                    $t,
                    @$);
        }
    // FIXME 6.27 <string value expression> - <collate clause>
    // 6.28 <datetime value expression> - <time zone>
    | "(" value_expression_list[e] ")"
        {
            auto es = $e;
            if (es.size() == 1) { // may be a parenthesized expression
                $$ = std::move(es[0]);
            } else {
                $$ = driver.node<ast::scalar::value_constructor>(
                        regioned { ast::scalar::value_constructor_kind::row },
                        std::move(es),
                        @$);
            }
        }
    ;

// 6.20 <interval value function>
system_function_invocation
    : simple_system_function_name[f] "(" value_expression_list[l] ")"
        {
            $$ = driver.node<ast::scalar::builtin_function_invocation>(
                    $f,
                    $l,
                    @$);
        }
    // 6.17 <numeric value function>
    | "POSITION"[f] "(" scalar_value_expression[l] "IN" scalar_value_expression[r] ")"
        {
            $$ = driver.node<ast::scalar::builtin_function_invocation>(
                    regioned { ast::scalar::builtin_function_kind::position, @f },
                    driver.to_node_vector<ast::scalar::expression>($l, $r),
                    @$);
        }
    | "EXTRACT"[f] "(" extract_field[k] "FROM" scalar_value_expression[e] ")"
        {
            $$ = driver.node<ast::scalar::extract_expression>(
                    $k,
                    $e,
                    @$);
        }
    // 6.18 <string value function>
    | "SUBSTRING"[f] "(" scalar_value_expression[l]
            "FROM" scalar_value_expression[r] ")"
        {
            $$ = driver.node<ast::scalar::builtin_function_invocation>(
                    regioned { ast::scalar::builtin_function_kind::substring, @f },
                    driver.to_node_vector<ast::scalar::expression>($l, $r),
                    @$);
        }
    | "SUBSTRING"[f] "(" scalar_value_expression[l]
            "FROM" scalar_value_expression[r]
            "FOR" scalar_value_expression[e] ")"
        {
            $$ = driver.node<ast::scalar::builtin_function_invocation>(
                    regioned { ast::scalar::builtin_function_kind::substring, @f },
                    driver.to_node_vector<ast::scalar::expression>($l, $r, $e),
                    @$);
        }
    | "CONVERT"[f] "(" scalar_value_expression[e] "USING" identifier_chain[n] ")"
        {
            $$ = driver.node<ast::scalar::builtin_function_invocation>(
                    regioned { ast::scalar::builtin_function_kind::convert, @f },
                    driver.to_node_vector<ast::scalar::expression>(
                            $e,
                            driver.node<ast::scalar::variable_reference>($n, @n)),
                    @$);
        }
    | "TRANSLATE"[f] "(" scalar_value_expression[e] "USING" identifier_chain[n] ")"
        {
            $$ = driver.node<ast::scalar::builtin_function_invocation>(
                    regioned { ast::scalar::builtin_function_kind::translate, @f },
                    driver.to_node_vector<ast::scalar::expression>(
                            $e,
                            driver.node<ast::scalar::variable_reference>($n, @n)),
                    @$);
        }
    | "TRIM"[f] "(" trim_specification[s] scalar_value_expression[c] "FROM" scalar_value_expression[e] ")"
        {
            $$ = driver.node<ast::scalar::trim_expression>(
                    $s,
                    $c,
                    $e,
                    @$);
        }
    | "TRIM"[f] "(" trim_specification[s] "FROM" scalar_value_expression[e] ")"
        {
            $$ = driver.node<ast::scalar::trim_expression>(
                    $s,
                    nullptr,
                    $e,
                    @$);
        }
    | "TRIM"[f] "(" scalar_value_expression[c] "FROM" scalar_value_expression[e] ")"
        {
            $$ = driver.node<ast::scalar::trim_expression>(
                    std::nullopt,
                    $c,
                    $e,
                    @$);
        }
    | "TRIM"[f] "(" "FROM" scalar_value_expression[e] ")"
        {
            $$ = driver.node<ast::scalar::trim_expression>(
                    std::nullopt,
                    nullptr,
                    $e,
                    @$);
        }
    | "TRIM"[f] "(" scalar_value_expression[e] ")"
        {
            $$ = driver.node<ast::scalar::trim_expression>(
                    std::nullopt,
                    nullptr,
                    $e,
                    @$);
        }
    | "OVERLAY"[f] "(" scalar_value_expression[e]
            "PLACING" scalar_value_expression[p]
            "FROM" scalar_value_expression[u]
            "FOR" scalar_value_expression[v] ")"
        {
            $$ = driver.node<ast::scalar::builtin_function_invocation>(
                    regioned { ast::scalar::builtin_function_kind::overlay, @f },
                    driver.to_node_vector<ast::scalar::expression>($e, $p, $u, $v),
                    @$);
        }
    | "OVERLAY"[f] "(" scalar_value_expression[e]
            "PLACING" scalar_value_expression[p]
            "FROM" scalar_value_expression[u] ")"
        {
            $$ = driver.node<ast::scalar::builtin_function_invocation>(
                    regioned { ast::scalar::builtin_function_kind::overlay, @f },
                    driver.to_node_vector<ast::scalar::expression>($e, $p, $u),
                    @$);
        }
    // 6.19 <datetime value function>
    | "CURRENT_DATE"[f]
        {
            $$ = driver.node<ast::scalar::builtin_function_invocation>(
                    regioned { ast::scalar::builtin_function_kind::current_date, @f },
                    driver.to_node_vector<ast::scalar::expression>(),
                    @$);
        }
    | precise_system_function_name[f]
        {
            $$ = driver.node<ast::scalar::builtin_function_invocation>(
                    $f,
                    driver.to_node_vector<ast::scalar::expression>(),
                    @$);
        }
    | precise_system_function_name[f] "(" value_expression[e] ")"
        {
            $$ = driver.node<ast::scalar::builtin_function_invocation>(
                    $f,
                    driver.to_node_vector<ast::scalar::expression>($e),
                    @$);
        }
    ;

simple_system_function_name
    // 6.17 <numeric value function>
    : "CHAR_LENGTH"
        {
            $$ = { ast::scalar::builtin_function_kind::character_length, @$ };
        }
    | "CHARACTER_LENGTH"
        {
            $$ = { ast::scalar::builtin_function_kind::character_length, @$ };
        }
    | "OCTET_LENGTH"
        {
            $$ = { ast::scalar::builtin_function_kind::octet_length, @$ };
        }
    | "BIT_LENGTH"
        {
            $$ = { ast::scalar::builtin_function_kind::bit_length, @$ };
        }
    | "CARDINALITY"
        {
            $$ = { ast::scalar::builtin_function_kind::cardinality, @$ };
        }
    | "ABS"
        {
            $$ = { ast::scalar::builtin_function_kind::abs, @$ };
        }
    | "MOD"
        {
            $$ = { ast::scalar::builtin_function_kind::mod, @$ };
        }
    // 6.18 <string value function>
    | "UPPER"
        {
            $$ = { ast::scalar::builtin_function_kind::upper, @$ };
        }
    | "LOWER"
        {
            $$ = { ast::scalar::builtin_function_kind::lower, @$ };
        }
    // extensions
    | "LENGTH"
        {
            $$ = { ast::scalar::builtin_function_kind::length, @$ };
        }
    ;

precise_system_function_name
    : "CURRENT_TIME"
        {
            $$ = { ast::scalar::builtin_function_kind::current_time, @$ };
        }
    | "LOCALTIME"
        {
            $$ = { ast::scalar::builtin_function_kind::localtime, @$ };
        }
    | "CURRENT_TIMESTAMP"
        {
            $$ = { ast::scalar::builtin_function_kind::current_timestamp, @$ };
        }
    | "LOCALTIMESTAMP"
        {
            $$ = { ast::scalar::builtin_function_kind::localtimestamp, @$ };
        }
    ;

extract_field
    : "YEAR"
        {
            $$ = { ast::scalar::extract_field_kind::year, @$ };
        }
    | "MONTH"
        {
            $$ = { ast::scalar::extract_field_kind::month, @$ };
        }
    | "DAY"
        {
            $$ = { ast::scalar::extract_field_kind::day, @$ };
        }
    | "HOUR"
        {
            $$ = { ast::scalar::extract_field_kind::hour, @$ };
        }
    | "MINUTE"
        {
            $$ = { ast::scalar::extract_field_kind::minute, @$ };
        }
    | "SECOND"
        {
            $$ = { ast::scalar::extract_field_kind::second, @$ };
        }
    | "TIMEZONE_HOUR"
        {
            $$ = { ast::scalar::extract_field_kind::timezone_hour, @$ };
        }
    | "TIMEZONE_MINUTE"
        {
            $$ = { ast::scalar::extract_field_kind::timezone_minute, @$ };
        }
    ;

trim_specification
    : "LEADING"
        {
            $$ = { ast::scalar::trim_specification::leading, @$ };
        }
    | "TRAILING"
        {
            $$ = { ast::scalar::trim_specification::trailing, @$ };
        }
    | "BOTH"
        {
            $$ = { ast::scalar::trim_specification::both, @$ };
        }
    ;

// 6.16 <set function specification>
system_set_function_invocation
    : "COUNT"[f] "(" "*" ")"
        {
            $$ = driver.node<ast::scalar::builtin_set_function_invocation>(
                    regioned { ast::scalar::builtin_set_function_kind::count, @f },
                    std::nullopt,
                    driver.node_vector<ast::scalar::expression>(),
                    @$);
        }
    | "COUNT"[f] "(" set_quantifier_opt[q] value_expression[e] ")"
        {
            $$ = driver.node<ast::scalar::builtin_set_function_invocation>(
                    regioned { ast::scalar::builtin_set_function_kind::count, @f },
                    $q,
                    driver.to_node_vector<ast::scalar::expression>($e),
                    @$);
        }
    | computational_operation_except_count[f] "(" set_quantifier_opt[q] value_expression[e] ")"
        {
            $$ = driver.node<ast::scalar::builtin_set_function_invocation>(
                    $f,
                    $q,
                    driver.to_node_vector<ast::scalar::expression>($e),
                    @$);
        }
    ;

computational_operation_except_count
    : "AVG"
        {
            $$ = { ast::scalar::builtin_set_function_kind::avg, @$ };
        }
    | "MAX"
        {
            $$ = { ast::scalar::builtin_set_function_kind::max, @$ };
        }
    | "MIN"
        {
            $$ = { ast::scalar::builtin_set_function_kind::min, @$ };
        }
    | "SUM"
        {
            $$ = { ast::scalar::builtin_set_function_kind::sum, @$ };
        }
    | "EVERY"
        {
            $$ = { ast::scalar::builtin_set_function_kind::every, @$ };
        }
    | "BIT_AND"
        {
            $$ = { ast::scalar::builtin_set_function_kind::bit_and, @$ };
        }
    | "BIT_OR"
        {
            $$ = { ast::scalar::builtin_set_function_kind::bit_or, @$ };
        }
    | "BOOL_AND"
        {
            $$ = { ast::scalar::builtin_set_function_kind::bool_and, @$ };
        }
    | "BOOL_OR"
        {
            $$ = { ast::scalar::builtin_set_function_kind::bool_or, @$ };
        }
    // NOTE: ANY, SOME are ambiguous with `x = SOME ((SELECT ...))`
    ;

quantifier
    : "ALL"
        {
            $$ = { ast::scalar::quantifier::all, @$ };
        }
    | "SOME"
        {
            $$ = { ast::scalar::quantifier::some, @$ };
        }
    | "ANY"
        {
            $$ = { ast::scalar::quantifier::any, @$ };
        }
    ;

case_when_clause_list
    : case_when_clause_list[L] case_when_clause[e]
        {
            $$ = $L;
            $$.emplace_back($e);
        }
    | case_when_clause[e]
        {
            $$ = driver.element_vector<ast::scalar::case_when_clause>();
            $$.emplace_back($e);
        }
    ;

case_when_clause
    : "WHEN" value_expression[w] "THEN" value_expression[r]
        {
            $$ = ast::scalar::case_when_clause {
                    $w,
                    $r,
                    @$,
            };
        }
    ;

else_expression_opt
    : %empty
        {
            $$ = nullptr;
        }
    | "ELSE" value_expression[e]
        {
            $$ = $e;
        }
    ;

routine_invocation_arguments
    : "(" sql_argument_list_opt[l] ")"
        {
            $$ = $l;
        }
    ;

sql_argument_list_opt
    : %empty
        {
            $$ = driver.node_vector<ast::scalar::expression>();
        }
    | sql_argument_list[l]
        {
            $$ = $l;
        }
    ;

sql_argument_list
    : sql_argument_list_opt[L] "," sql_argument[e]
        {
            $$ = $L;
            $$.emplace_back($e);
        }
    | sql_argument[e]
        {
            $$ = driver.to_node_vector<ast::scalar::expression>($e);
        }
    ;

sql_argument
    : value_expression[e]
        {
            $$ = $e;
        }
    | value_expression[e] "AS" data_type[t]
        {
            $$ = driver.node<ast::scalar::cast_expression>(
                    regioned { ast::scalar::cast_operator::generalize },
                    $e,
                    $t,
                    @$);
        }
    ;


table_subquery
    : subquery[e]
        {
            $$ = $e;
        }
    ;

scalar_subquery
    : subquery[e]
        {
            $$ = $e;
        }
    ;

row_subquery
    : subquery[e]
        {
            $$ = $e;
        }
    ;

subquery
    // FIXME: to avoid conflict, subquery contains _primary instead of query expression
    : "(" query_expression_primary[e] ")"
        {
            $$ = $e;
        }
    ;

literal
    : unsigned_integer[v]
        {
            $$ = $v;
        }
    | EXACT_NUMERIC_LITERAL[t]
        {
            $$ = driver.node<ast::literal::numeric>(
                    ast::literal::kind::exact_numeric,
                    std::nullopt,
                    $t,
                    @$);
        }
    | APPROXIMATE_NUMERIC_LITERAL[t]
        {
            $$ = driver.node<ast::literal::numeric>(
                    ast::literal::kind::approximate_numeric,
                    std::nullopt,
                    $t,
                    @$);
        }
    | CHARACTER_STRING_LITERAL[t] concatenations_list_opt[c]
        {
            $$ = driver.node<ast::literal::string>(
                    regioned { ast::literal::kind::character_string },
                    regioned { $t, @t },
                    $c,
                    @$);
        }
    | BIT_STRING_LITERAL[t] concatenations_list_opt[c]
        {
            $$ = driver.node<ast::literal::string>(
                    regioned { ast::literal::kind::bit_string, @t(0, 1) },
                    regioned { $t.substr(1), @t(1) },
                    $c,
                    @$);
        }
    | HEX_STRING_LITERAL[t] concatenations_list_opt[c]
        {
            $$ = driver.node<ast::literal::string>(
                    regioned { ast::literal::kind::hex_string, @t(0, 1) },
                    regioned { $t.substr(1), @t(1) },
                    $c,
                    @$);
        }
    | "DATE"[k] CHARACTER_STRING_LITERAL[t]
        {
            $$ = driver.node<ast::literal::datetime>(
                    regioned { ast::literal::kind::date, @k },
                    $t,
                    @$);
        }
    | "TIME"[k] CHARACTER_STRING_LITERAL[t]
        {
            $$ = driver.node<ast::literal::datetime>(
                    regioned { ast::literal::kind::time, @k },
                    regioned { $t, @t },
                    @$);
        }
    | "TIMESTAMP"[k] CHARACTER_STRING_LITERAL[t]
        {
            $$ = driver.node<ast::literal::datetime>(
                    regioned { ast::literal::kind::timestamp, @k },
                    regioned { $t, @t },
                    @$);
        }
    | "INTERVAL"[k] sign_opt[s] CHARACTER_STRING_LITERAL[t] // FIXME: interval qualifier
        {
            $$ = driver.node<ast::literal::interval>(
                    $s,
                    regioned { $t, @t },
                    @$);
        }
    | truth_literal[v]
        {
            $$ = $v;
        }
    | "ARRAY" "[" "]"
        {
            $$ = driver.node<ast::literal::special<ast::literal::kind::empty>>(@$);
        }
    | "DEFAULT"
        {
            $$ = driver.node<ast::literal::special<ast::literal::kind::default_>>(@$);
        }
    ;

unsigned_integer
    : UNSIGNED_INTEGER[t]
         {
             $$ = driver.node<ast::literal::numeric>(
                     ast::literal::kind::exact_numeric,
                     std::nullopt,
                     $t,
                     @$);
         }
     ;

truth_literal
    : "TRUE"
        {
            $$ = driver.node<ast::literal::boolean>(ast::literal::boolean_kind::true_, @$);
        }
    | "FALSE"
        {
            $$ = driver.node<ast::literal::boolean>(ast::literal::boolean_kind::false_, @$);
        }
    | "UNKNOWN"
        {
            $$ = driver.node<ast::literal::boolean>(ast::literal::boolean_kind::unknown, @$);
        }
    | "NULL"
        {
            $$ = driver.node<ast::literal::special<ast::literal::kind::null>>(@$);
        }
    ;

sign_opt
    : %empty
        {
            $$ = std::nullopt;
        }
    | "+"
        {
            $$ = { ast::literal::sign::plus, @$ };
        }
    | "-"
        {
            $$ = { ast::literal::sign::minus, @$ };
        }
    ;

concatenations_list_opt
    : concatenations_list_opt[L] CHARACTER_STRING_LITERAL[t]
        {
            $$ = $L;
            $$.emplace_back($t, @t);
        }
    | %empty
        {
            $$ = driver.element_vector<ast::common::regioned<ast::common::chars>>();
        }
    ;

data_type
    : data_type_system[t]
        {
            $$ = $t;
        }
    | data_type_composite[t]
        {
            $$ = $t;
        }
    | data_type_user[t]
        {
            $$ = $t;
        }
    ;

data_type_system
    : character_type_name[k] parenthesized_size_opt[s] // FIXME: charset
        {
            $$ = driver.node<ast::type::character_string>(
                    regioned { ast::type::kind::character, @k },
                    $s,
                    @$);
        }
    | character_varying_type_name[k] parenthesized_size_maybe_flexible_opt[s] // FIXME: charset
        {
            $$ = driver.node<ast::type::character_string>(
                    regioned { ast::type::kind::character_varying, @k },
                    $s,
                    @$);
        }
    | "BIT"[k] parenthesized_size_opt[s]
        {
            $$ = driver.node<ast::type::bit_string>(
                    regioned { ast::type::kind::bit, @k },
                    $s,
                    @$);
        }
    | "BIT"[k] "VARYING"[v] parenthesized_size_maybe_flexible_opt[s]
        {
            $$ = driver.node<ast::type::bit_string>(
                    regioned { ast::type::kind::bit_varying, @k | @v },
                    $s,
                    @$);
        }
    | decimal_type_name[k]
        {
            $$ = driver.node<ast::type::decimal>(
                    $k,
                    std::nullopt,
                    std::nullopt,
                    @$);
        }
    | decimal_type_name[k] "(" size_maybe_flexible[p] ")"
        {
            $$ = driver.node<ast::type::decimal>(
                    $k,
                    $p,
                    std::nullopt,
                    @$);
        }
    | decimal_type_name[k] "(" size_maybe_flexible[p] "," size[q] ")"
        {
            $$ = driver.node<ast::type::decimal>(
                    $k,
                    $p,
                    $q,
                    @$);
        }
    | integer_type_name
        {
            $$ = driver.node<ast::type::simple>(ast::type::kind::integer, @$);
        }
    | "TINYINT"
        {
            $$ = driver.node<ast::type::simple>(ast::type::kind::tiny_integer, @$);
        }
    | "SMALLINT"
        {
            $$ = driver.node<ast::type::simple>(ast::type::kind::small_integer, @$);
        }
    | "BIGINT"
        {
            $$ = driver.node<ast::type::simple>(ast::type::kind::big_integer, @$);
        }
    | "FLOAT"
        {
            $$ = driver.node<ast::type::simple>(ast::type::kind::float_, @$);
        }
    | "REAL"
        {
            $$ = driver.node<ast::type::simple>(ast::type::kind::real, @$);
        }
    | "DOUBLE" "PRECISION"
        {
            $$ = driver.node<ast::type::simple>(ast::type::kind::double_precision, @$);
        }
    | "DOUBLE"
        {
            $$ = driver.node<ast::type::simple>(ast::type::kind::double_precision, @$);
        }
    | integer_type_name[k] "(" size[s] ")"
        {
            $$ = driver.node<ast::type::binary_numeric>(
                    regioned { ast::type::kind::binary_integer, @k },
                    $s,
                    @$);
        }
    | "FLOAT"[k] "(" size[s] ")"
        {
            $$ = driver.node<ast::type::binary_numeric>(
                    regioned { ast::type::kind::binary_float, @k },
                    $s,
                    @$);
        }
    | "BOOLEAN"
        {
            $$ = driver.node<ast::type::simple>(ast::type::kind::boolean, @$);
        }
    | "DATE"
        {
            $$ = driver.node<ast::type::simple>(ast::type::kind::date, @$);
        }
    | "TIME"[k] with_or_without_time_zone_opt[z]
        {
            $$ = driver.node<ast::type::datetime>(
                    regioned { ast::type::kind::time, @k },
                    $z,
                    @$);
        }
    | "TIMESTAMP"[k] with_or_without_time_zone_opt[z]
        {
            $$ = driver.node<ast::type::datetime>(
                    regioned { ast::type::kind::timestamp, @k },
                    $z,
                    @$);
        }
    | "INTERVAL" // FIXME: qualifier
        {
            $$ = driver.node<ast::type::interval>(@$);
        }
    // FIXME: large object types
    ;

data_type_composite
    : "ROW" "(" field_definition_list[l] ")"
        {
            $$ = driver.node<ast::type::row>(
                    $l,
                    @$);
        }
    | data_type[c] "ARRAY" "[" size_maybe_flexible_opt[s] "]"
        {
            $$ = driver.node<ast::type::collection>(
                    $c,
                    $s,
                    @$);
        }
    | data_type[c] "[" size_maybe_flexible_opt[s] "]"
        {
            $$ = driver.node<ast::type::collection>(
                    $c,
                    $s,
                    @$);
        }
    ;

data_type_user
    : identifier_chain[n]
        {
            $$ = driver.node<ast::type::user_defined>(
                    $n,
                    @$);
        }
    ;

character_type_name
    : "CHARACTER"
    | "CHAR"
    ;

character_varying_type_name
    : "CHARACTER" "VARYING"
    | "CHAR" "VARYING"
    | "VARCHAR"
    ;

decimal_type_name
    : "DECIMAL"
        {
            $$ = { ast::type::kind::decimal, @$ };
        }
    | "DEC"
        {
            $$ = { ast::type::kind::decimal, @$ };
        }
    | "NUMERIC"
        {
            $$ = { ast::type::kind::numeric, @$ };
        }
    ;

integer_type_name
    : "INT"
    | "INTEGER"
    ;

parenthesized_size_maybe_flexible_opt
    : %empty
        {
            $$ = std::nullopt;
        }
    | "(" size_maybe_flexible[s] ")"
        {
            $$ = $s;
        }
    ;

parenthesized_size_opt
    : %empty
        {
            $$ = std::nullopt;
        }
    | "(" size[s] ")"
        {
            $$ = $s;
        }
    ;

size_maybe_flexible_opt
    : %empty
        {
            $$ = std::nullopt;
        }
    | size_maybe_flexible[s]
        {
            $$ = $s;
        }
    ;

size_maybe_flexible
    : size[s]
        {
            $$ = $s;
        }
    | "*"
        {
            $$ = { static_cast<std::size_t>(-1), @$ };
        }
    ;

size
    : UNSIGNED_INTEGER[t]
        {
            $$ = { driver.to_size($t), @t };
        }
    ;

with_or_without_time_zone_opt
    : %empty
        {
            $$ = std::nullopt;
        }
    | "WITH" "TIME" "ZONE"
        {
            $$ = { true, @$ };
        }
    | "WITHOUT" "TIME" "ZONE"
        {
            $$ = { false, @$ };
        }
    ;

set_quantifier_opt
    : %empty
        {
            $$ = {};
        }
    | "ALL"
        {
            $$ = { ast::scalar::set_quantifier::all, @$ };
        }
    | "DISTINCT"
        {
            $$ = { ast::scalar::set_quantifier::distinct, @$ };
        }
    ;

corresponding_spec_opt
    : %empty
        {
            $$ = {};
        }
    | "CORRESPONDING" "BY" "(" column_name_list[L] ")"
        {
            $$ = ast::query::corresponding_clause {
                    $L,
                    @$,
            };
        }
    | "CORRESPONDING"
        {
            $$ = ast::query::corresponding_clause {
                    driver.node_vector<ast::name::simple>(),
                    @$,
            };
        }
    ;

column_name_list
    : column_name_list[L] "," column_name[n]
        {
            $$ = $L;
            $$.emplace_back($n);
        }
    | column_name[n]
        {
            $$ = driver.node_vector<ast::name::simple>();
            $$.emplace_back($n);
        }
    ;

query_name
    : identifier[n]
        {
            $$ = $n;
        }
    ;

table_name
    : identifier_chain[n]
        {
            $$ = $n;
        }
    ;

correlation_name
    : identifier[n]
        {
            $$ = $n;
        }
    ;

column_name
    : identifier[n]
        {
            $$ = $n;
        }
    ;

field_name
    : identifier[n]
        {
            $$ = $n;
        }
    ;

cursor_name
    : identifier[n]
        {
            $$ = $n;
        }
    ;

host_parameter_name
    : HOST_PARAMETER_NAME[t]
        {
            $$ = driver.node<ast::name::simple>($t, @$);
        }
    ;

identifier_chain
    : identifier_chain[q] "." identifier[n]
        {
            $$ = driver.node<ast::name::qualified>(
                    $q,
                    $n,
                    @$);
        }
    | identifier[n]
        {
            $$ = $n;
        }
    ;

identifier
    : REGULAR_IDENTIFIER[t]
        {
            $$ = driver.node<ast::name::simple>($t, @$);
        }
    | DELIMITED_IDENTIFIER[t]
        {
            $$ = driver.node<ast::name::simple>(driver.parse_delimited_identifier($t), @$);
        }
    // FIXME: move to individual name
    | contextual_identifier[n]
        {
            $$ = $n;
        }
    ;

contextual_identifier
    : "ADA"[t] { $$ = driver.node<ast::name::simple>($t, @$); }
    | "C"[t] { $$ = driver.node<ast::name::simple>($t, @$); }
    | "COBOL"[t] { $$ = driver.node<ast::name::simple>($t, @$); }
    | "FORTRAN"[t] { $$ = driver.node<ast::name::simple>($t, @$); }
    | "G"[t] { $$ = driver.node<ast::name::simple>($t, @$); }
    | "K"[t] { $$ = driver.node<ast::name::simple>($t, @$); }
    | "M"[t] { $$ = driver.node<ast::name::simple>($t, @$); }
    | "MUMPS"[t] { $$ = driver.node<ast::name::simple>($t, @$); }
    | "PASCAL"[t] { $$ = driver.node<ast::name::simple>($t, @$); }
    | "PLI"[t] { $$ = driver.node<ast::name::simple>($t, @$); }
    ;
