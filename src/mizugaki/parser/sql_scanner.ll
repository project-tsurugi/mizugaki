%{
#include <mizugaki/parser/sql_scanner.h>
#include <mizugaki/parser/sql_parser_generated.hpp>

#define YY_USER_ACTION user_action();

#define yyterminate() return parser_type::make_ERROR(location())

#undef YY_DECL
#define YY_DECL ::mizugaki::parser::sql_scanner::value_type mizugaki::parser::sql_scanner::next_token(::mizugaki::parser::sql_driver& driver)
%}

%option noyywrap
%option c++
%option caseless
%option prefix="_generated_mizugaki__parser__sql_scanner_"
%option yyclass="::mizugaki::parser::sql_scanner"

space [[:space:]]+

/* <regular identifier> FIXME: more characters */
identifier_start [A-Za-z]
identifier_part [A-Za-z_0-9]
identifier_body {identifier_start}{identifier_part}*
identifier {identifier_body}

/* <delimited identifier> */
delimited_identifier {double_quote}{delimited_identifier_body}{double_quote}
delimited_identifier_body {delimited_identifier_part}+
delimited_identifier_part ({nondoublequote_character}|{doublequote_symbol})
double_quote \"
nondoublequote_character [^\"]
doublequote_symbol {double_quote}{double_quote}

/* <character string literal> */
character_string_literal {quote}{character_representation}*{quote}
quote "'"
character_representation ({nonquote_character}|{quote_symbol})
nonquote_character [^']
quote_symbol {quote}{quote}

/* <bit string literal> */
bit_string_literal "B"{character_string_literal}

/* <hex string literal> */
hex_string_literal "X"{character_string_literal}

/* <exact numeric literal> */
sign [+\-]
digit [0-9]
period "."
unsigned_integer {digit}+
signed_integer {sign}?{unsigned_integer}
decimal {unsigned_integer}{period}{unsigned_integer}

/* <approximate numeric literal> */
approximate_numeric_literal {mantissa}"E"{exponent}
mantissa ({unsigned_integer}|{decimal})
exponent {signed_integer}

/* special phrases */
union_join "UNION"{space}+"JOIN"
dot_asterisk "."{space}*"*"

host_parameter_name ":"{identifier}

%x bracketed_comment
%x simple_comment

%%

{space} {}

"/*" {
    BEGIN(bracketed_comment);
    enter_comment();
}

<bracketed_comment>{
    "*/" {
        driver.add_comment(exit_comment());
        BEGIN(INITIAL);
    }
    <<EOF>> {
        return parser_type::make_ERROR(location());
    }
    . {}
}

"--" {
    BEGIN(simple_comment);
    enter_comment();
}

<simple_comment>{
    "\n" {
        driver.add_comment(exit_comment());
        BEGIN(INITIAL);
    }
    <<EOF>> {
        return parser_type::make_END_OF_FILE(location());
    }
    . {}
}

{union_join} { return parser_type::make_UNION_JOIN(location()); }

{dot_asterisk} { return parser_type::make_DOT_ASTERISK(location()); }

"<>" { return parser_type::make_NOT_EQUALS_OPERATOR(location()); }
">=" { return parser_type::make_GREATER_THAN_OR_EQUALS_OPERATOR(location()); }
"<=" { return parser_type::make_LESS_THAN_OR_EQUALS_OPERATOR(location()); }
"||" { return parser_type::make_CONCATENATION_OPERATOR(location()); }
"->" { return parser_type::make_RIGHT_ARROW(location()); }
"::" { return parser_type::make_DOUBLE_COLON(location()); }

"%" { return parser_type::make_PERCENT(location()); }
"&" { return parser_type::make_AMPERSAND(location()); }
"'" { return parser_type::make_QUOTE(location()); }
"(" { return parser_type::make_LEFT_PAREN(location()); }
")" { return parser_type::make_RIGHT_PAREN(location()); }
"*" { return parser_type::make_ASTERISK(location()); }
"+" { return parser_type::make_PLUS_SIGN(location()); }
"," { return parser_type::make_COMMA(location()); }
"-" { return parser_type::make_MINUS_SIGN(location()); }
"." { return parser_type::make_PERIOD(location()); }
"/" { return parser_type::make_SOLIDUS(location()); }
":" { return parser_type::make_COLON(location()); }
";" { return parser_type::make_SEMICOLON(location()); }
"<" { return parser_type::make_LESS_THAN_OPERATOR(location()); }
"=" { return parser_type::make_EQUALS_OPERATOR(location()); }
">" { return parser_type::make_GREATER_THAN_OPERATOR(location()); }
"?" { return parser_type::make_QUESTION_MARK(location()); }
"[" { return parser_type::make_LEFT_BRACKET(location()); }
"]" { return parser_type::make_RIGHT_BRACKET(location()); }
"^" { return parser_type::make_CIRCUMFLEX(location()); }
"_" { return parser_type::make_UNDERSCORE(location()); }
"|" { return parser_type::make_VERTICAL_BAR(location()); }
"{" { return parser_type::make_LEFT_BRACE(location()); }
"}" { return parser_type::make_RIGHT_BRACE(location()); }

"ABS" { return parser_type::make_ABS(location()); }
"ADA" { return parser_type::make_ADA(get_image(driver), location()); }
"ASENSITIVE" { return parser_type::make_ASENSITIVE(location()); }
"ASSIGNMENT" { return parser_type::make_ASSIGNMENT(location()); }
"ASYMMETRIC" { return parser_type::make_ASYMMETRIC(location()); }
"ATOMIC" { return parser_type::make_ATOMIC(location()); }
"AVG" { return parser_type::make_AVG(location()); }
"BETWEEN" { return parser_type::make_BETWEEN(location()); }
"BIT_LENGTH" { return parser_type::make_BIT_LENGTH(location()); }
"BITVAR" { return parser_type::make_BITVAR(location()); }
"C" { return parser_type::make_C(get_image(driver), location()); }
"CALLED" { return parser_type::make_CALLED(location()); }
"CARDINALITY" { return parser_type::make_CARDINALITY(location()); }
"CATALOG_NAME" { return parser_type::make_CATALOG_NAME(location()); }
"CHAIN" { return parser_type::make_CHAIN(location()); }
"CHAR_LENGTH" { return parser_type::make_CHAR_LENGTH(location()); }
"CHARACTER_LENGTH" { return parser_type::make_CHARACTER_LENGTH(location()); }
"CHARACTER_SET_CATALOG" { return parser_type::make_CHARACTER_SET_CATALOG(location()); }
"CHARACTER_SET_NAME" { return parser_type::make_CHARACTER_SET_NAME(location()); }
"CHARACTER_SET_SCHEMA" { return parser_type::make_CHARACTER_SET_SCHEMA(location()); }
"CHECKED" { return parser_type::make_CHECKED(location()); }
"CLASS_ORIGIN" { return parser_type::make_CLASS_ORIGIN(location()); }
"COALESCE" { return parser_type::make_COALESCE(location()); }
"COBOL" { return parser_type::make_COBOL(get_image(driver), location()); }
"COLLATION_CATALOG" { return parser_type::make_COLLATION_CATALOG(location()); }
"COLLATION_NAME" { return parser_type::make_COLLATION_NAME(location()); }
"COLLATION_SCHEMA" { return parser_type::make_COLLATION_SCHEMA(location()); }
"COLUMN_NAME" { return parser_type::make_COLUMN_NAME(location()); }
"COMMAND_FUNCTION" { return parser_type::make_COMMAND_FUNCTION(location()); }
"COMMAND_FUNCTION_CODE" { return parser_type::make_COMMAND_FUNCTION_CODE(location()); }
"COMMITTED" { return parser_type::make_COMMITTED(location()); }
"CONDITION_NUMBER" { return parser_type::make_CONDITION_NUMBER(location()); }
"CONNECTION_NAME" { return parser_type::make_CONNECTION_NAME(location()); }
"CONSTRAINT_CATALOG" { return parser_type::make_CONSTRAINT_CATALOG(location()); }
"CONSTRAINT_NAME" { return parser_type::make_CONSTRAINT_NAME(location()); }
"CONSTRAINT_SCHEMA" { return parser_type::make_CONSTRAINT_SCHEMA(location()); }
"CONTAINS" { return parser_type::make_CONTAINS(location()); }
"CONVERT" { return parser_type::make_CONVERT(location()); }
"COUNT" { return parser_type::make_COUNT(location()); }
"CURSOR_NAME" { return parser_type::make_CURSOR_NAME(location()); }
"DATETIME_INTERVAL_CODE" { return parser_type::make_DATETIME_INTERVAL_CODE(location()); }
"DATETIME_INTERVAL_PRECISION" { return parser_type::make_DATETIME_INTERVAL_PRECISION(location()); }
"DEFINED" { return parser_type::make_DEFINED(location()); }
"DEFINER" { return parser_type::make_DEFINER(location()); }
"DISPATCH" { return parser_type::make_DISPATCH(location()); }
"DYNAMIC_FUNCTION" { return parser_type::make_DYNAMIC_FUNCTION(location()); }
"DYNAMIC_FUNCTION_CODE" { return parser_type::make_DYNAMIC_FUNCTION_CODE(location()); }
"EXISTING" { return parser_type::make_EXISTING(location()); }
"EXISTS" { return parser_type::make_EXISTS(location()); }
"EXTRACT" { return parser_type::make_EXTRACT(location()); }
"FINAL" { return parser_type::make_FINAL(location()); }
"FORTRAN" { return parser_type::make_FORTRAN(get_image(driver), location()); }
"G" { return parser_type::make_G(get_image(driver), location()); }
"GENERATED" { return parser_type::make_GENERATED(location()); }
"GRANTED" { return parser_type::make_GRANTED(location()); }
"HIERARCHY" { return parser_type::make_HIERARCHY(location()); }
"HOLD" { return parser_type::make_HOLD(location()); }
"IMPLEMENTATION" { return parser_type::make_IMPLEMENTATION(location()); }
"INFIX" { return parser_type::make_INFIX(location()); }
"INSENSITIVE" { return parser_type::make_INSENSITIVE(location()); }
"INSTANCE" { return parser_type::make_INSTANCE(location()); }
"INSTANTIABLE" { return parser_type::make_INSTANTIABLE(location()); }
"INVOKER" { return parser_type::make_INVOKER(location()); }
"K" { return parser_type::make_K(get_image(driver), location()); }
"KEY_MEMBER" { return parser_type::make_KEY_MEMBER(location()); }
"KEY_TYPE" { return parser_type::make_KEY_TYPE(location()); }
"LENGTH" { return parser_type::make_LENGTH(location()); }
"LOWER" { return parser_type::make_LOWER(location()); }
"M" { return parser_type::make_M(get_image(driver), location()); }
"MAX" { return parser_type::make_MAX(location()); }
"MIN" { return parser_type::make_MIN(location()); }
"MESSAGE_LENGTH" { return parser_type::make_MESSAGE_LENGTH(location()); }
"MESSAGE_OCTET_LENGTH" { return parser_type::make_MESSAGE_OCTET_LENGTH(location()); }
"MESSAGE_TEXT" { return parser_type::make_MESSAGE_TEXT(location()); }
"METHOD" { return parser_type::make_METHOD(location()); }
"MOD" { return parser_type::make_MOD(location()); }
"MORE" { return parser_type::make_MORE(location()); }
"MUMPS" { return parser_type::make_MUMPS(get_image(driver), location()); }
"NAME" { return parser_type::make_NAME(location()); }
"NULLABLE" { return parser_type::make_NULLABLE(location()); }
"NUMBER" { return parser_type::make_NUMBER(location()); }
"NULLIF" { return parser_type::make_NULLIF(location()); }
"OCTET_LENGTH" { return parser_type::make_OCTET_LENGTH(location()); }
"OPTIONS" { return parser_type::make_OPTIONS(location()); }
"OVERLAPS" { return parser_type::make_OVERLAPS(location()); }
"OVERLAY" { return parser_type::make_OVERLAY(location()); }
"OVERRIDING" { return parser_type::make_OVERRIDING(location()); }
"PASCAL" { return parser_type::make_PASCAL(get_image(driver), location()); }
"PARAMETER_MODE" { return parser_type::make_PARAMETER_MODE(location()); }
"PARAMETER_NAME" { return parser_type::make_PARAMETER_NAME(location()); }
"PARAMETER_ORDINAL_POSITION" { return parser_type::make_PARAMETER_ORDINAL_POSITION(location()); }
"PARAMETER_SPECIFIC_CATALOG" { return parser_type::make_PARAMETER_SPECIFIC_CATALOG(location()); }
"PARAMETER_SPECIFIC_NAME" { return parser_type::make_PARAMETER_SPECIFIC_NAME(location()); }
"PARAMETER_SPECIFIC_SCHEMA" { return parser_type::make_PARAMETER_SPECIFIC_SCHEMA(location()); }
"PLI" { return parser_type::make_PLI(get_image(driver), location()); }
"POSITION" { return parser_type::make_POSITION(location()); }
"REPEATABLE" { return parser_type::make_REPEATABLE(location()); }
"RETURNED_LENGTH" { return parser_type::make_RETURNED_LENGTH(location()); }
"RETURNED_OCTET_LENGTH" { return parser_type::make_RETURNED_OCTET_LENGTH(location()); }
"RETURNED_SQLSTATE" { return parser_type::make_RETURNED_SQLSTATE(location()); }
"ROUTINE_CATALOG" { return parser_type::make_ROUTINE_CATALOG(location()); }
"ROUTINE_NAME" { return parser_type::make_ROUTINE_NAME(location()); }
"ROUTINE_SCHEMA" { return parser_type::make_ROUTINE_SCHEMA(location()); }
"ROW_COUNT" { return parser_type::make_ROW_COUNT(location()); }
"SCALE" { return parser_type::make_SCALE(location()); }
"SCHEMA_NAME" { return parser_type::make_SCHEMA_NAME(location()); }
"SECURITY" { return parser_type::make_SECURITY(location()); }
"SELF" { return parser_type::make_SELF(location()); }
"SENSITIVE" { return parser_type::make_SENSITIVE(location()); }
"SERIALIZABLE" { return parser_type::make_SERIALIZABLE(location()); }
"SERVER_NAME" { return parser_type::make_SERVER_NAME(location()); }
"SIMPLE" { return parser_type::make_SIMPLE(location()); }
"SOURCE" { return parser_type::make_SOURCE(location()); }
"SPECIFIC_NAME" { return parser_type::make_SPECIFIC_NAME(location()); }
"SIMILAR" { return parser_type::make_SIMILAR(location()); }
"SUBLIST" { return parser_type::make_SUBLIST(location()); }
"SUBSTRING" { return parser_type::make_SUBSTRING(location()); }
"SUM" { return parser_type::make_SUM(location()); }
"STYLE" { return parser_type::make_STYLE(location()); }
"SUBCLASS_ORIGIN" { return parser_type::make_SUBCLASS_ORIGIN(location()); }
"SYMMETRIC" { return parser_type::make_SYMMETRIC(location()); }
"SYSTEM" { return parser_type::make_SYSTEM(location()); }
"TABLE_NAME" { return parser_type::make_TABLE_NAME(location()); }
"TRANSACTIONS_COMMITTED" { return parser_type::make_TRANSACTIONS_COMMITTED(location()); }
"TRANSACTIONS_ROLLED_BACK" { return parser_type::make_TRANSACTIONS_ROLLED_BACK(location()); }
"TRANSACTION_ACTIVE" { return parser_type::make_TRANSACTION_ACTIVE(location()); }
"TRANSFORM" { return parser_type::make_TRANSFORM(location()); }
"TRANSFORMS" { return parser_type::make_TRANSFORMS(location()); }
"TRANSLATE" { return parser_type::make_TRANSLATE(location()); }
"TRIGGER_CATALOG" { return parser_type::make_TRIGGER_CATALOG(location()); }
"TRIGGER_SCHEMA" { return parser_type::make_TRIGGER_SCHEMA(location()); }
"TRIGGER_NAME" { return parser_type::make_TRIGGER_NAME(location()); }
"TRIM" { return parser_type::make_TRIM(location()); }
"TYPE" { return parser_type::make_TYPE(location()); }
"UNCOMMITTED" { return parser_type::make_UNCOMMITTED(location()); }
"UNNAMED" { return parser_type::make_UNNAMED(location()); }
"UPPER" { return parser_type::make_UPPER(location()); }
"USER_DEFINED_TYPE_CATALOG" { return parser_type::make_USER_DEFINED_TYPE_CATALOG(location()); }
"USER_DEFINED_TYPE_NAME" { return parser_type::make_USER_DEFINED_TYPE_NAME(location()); }
"USER_DEFINED_TYPE_SCHEMA" { return parser_type::make_USER_DEFINED_TYPE_SCHEMA(location()); }
"ABSOLUTE" { return parser_type::make_ABSOLUTE(location()); }
"ACTION" { return parser_type::make_ACTION(location()); }
"ADD" { return parser_type::make_ADD(location()); }
"ADMIN" { return parser_type::make_ADMIN(location()); }
"AFTER" { return parser_type::make_AFTER(location()); }
"AGGREGATE" { return parser_type::make_AGGREGATE(location()); }
"ALIAS" { return parser_type::make_ALIAS(location()); }
"ALL" { return parser_type::make_ALL(location()); }
"ALLOCATE" { return parser_type::make_ALLOCATE(location()); }
"ALTER" { return parser_type::make_ALTER(location()); }
"AND" { return parser_type::make_AND(location()); }
"ANY" { return parser_type::make_ANY(location()); }
"ARE" { return parser_type::make_ARE(location()); }
"ARRAY" { return parser_type::make_ARRAY(location()); }
"AS" { return parser_type::make_AS(location()); }
"ASC" { return parser_type::make_ASC(location()); }
"ASSERTION" { return parser_type::make_ASSERTION(location()); }
"AT" { return parser_type::make_AT(location()); }
"AUTHORIZATION" { return parser_type::make_AUTHORIZATION(location()); }
"BEFORE" { return parser_type::make_BEFORE(location()); }
"BEGIN" { return parser_type::make_BEGIN_(location()); }
"BINARY" { return parser_type::make_BINARY(location()); }
"BIT" { return parser_type::make_BIT(location()); }
"BLOB" { return parser_type::make_BLOB(location()); }
"BOOLEAN" { return parser_type::make_BOOLEAN(location()); }
"BOTH" { return parser_type::make_BOTH(location()); }
"BREADTH" { return parser_type::make_BREADTH(location()); }
"BY" { return parser_type::make_BY(location()); }
"CALL" { return parser_type::make_CALL(location()); }
"CASCADE" { return parser_type::make_CASCADE(location()); }
"CASCADED" { return parser_type::make_CASCADED(location()); }
"CASE" { return parser_type::make_CASE(location()); }
"CAST" { return parser_type::make_CAST(location()); }
"CATALOG" { return parser_type::make_CATALOG(location()); }
"CHAR" { return parser_type::make_CHAR(location()); }
"CHARACTER" { return parser_type::make_CHARACTER(location()); }
"CHECK" { return parser_type::make_CHECK(location()); }
"CLASS" { return parser_type::make_CLASS(location()); }
"CLOB" { return parser_type::make_CLOB(location()); }
"CLOSE" { return parser_type::make_CLOSE(location()); }
"COLLATE" { return parser_type::make_COLLATE(location()); }
"COLLATION" { return parser_type::make_COLLATION(location()); }
"COLUMN" { return parser_type::make_COLUMN(location()); }
"COMMIT" { return parser_type::make_COMMIT(location()); }
"COMPLETION" { return parser_type::make_COMPLETION(location()); }
"CONNECT" { return parser_type::make_CONNECT(location()); }
"CONNECTION" { return parser_type::make_CONNECTION(location()); }
"CONSTRAINT" { return parser_type::make_CONSTRAINT(location()); }
"CONSTRAINTS" { return parser_type::make_CONSTRAINTS(location()); }
"CONSTRUCTOR" { return parser_type::make_CONSTRUCTOR(location()); }
"CONTINUE" { return parser_type::make_CONTINUE(location()); }
"CORRESPONDING" { return parser_type::make_CORRESPONDING(location()); }
"CREATE" { return parser_type::make_CREATE(location()); }
"CROSS" { return parser_type::make_CROSS(location()); }
"CUBE" { return parser_type::make_CUBE(location()); }
"CURRENT" { return parser_type::make_CURRENT(location()); }
"CURRENT_DATE" { return parser_type::make_CURRENT_DATE(location()); }
"CURRENT_PATH" { return parser_type::make_CURRENT_PATH(location()); }
"CURRENT_ROLE" { return parser_type::make_CURRENT_ROLE(location()); }
"CURRENT_TIME" { return parser_type::make_CURRENT_TIME(location()); }
"CURRENT_TIMESTAMP" { return parser_type::make_CURRENT_TIMESTAMP(location()); }
"CURRENT_USER" { return parser_type::make_CURRENT_USER(location()); }
"CURSOR" { return parser_type::make_CURSOR(location()); }
"CYCLE" { return parser_type::make_CYCLE(location()); }
"DATA" { return parser_type::make_DATA(location()); }
"DATE" { return parser_type::make_DATE(location()); }
"DAY" { return parser_type::make_DAY(location()); }
"DEALLOCATE" { return parser_type::make_DEALLOCATE(location()); }
"DEC" { return parser_type::make_DEC(location()); }
"DECIMAL" { return parser_type::make_DECIMAL(location()); }
"DECLARE" { return parser_type::make_DECLARE(location()); }
"DEFAULT" { return parser_type::make_DEFAULT(location()); }
"DEFERRABLE" { return parser_type::make_DEFERRABLE(location()); }
"DEFERRED" { return parser_type::make_DEFERRED(location()); }
"DELETE" { return parser_type::make_DELETE(location()); }
"DEPTH" { return parser_type::make_DEPTH(location()); }
"DEREF" { return parser_type::make_DEREF(location()); }
"DESC" { return parser_type::make_DESC(location()); }
"DESCRIBE" { return parser_type::make_DESCRIBE(location()); }
"DESCRIPTOR" { return parser_type::make_DESCRIPTOR(location()); }
"DESTROY" { return parser_type::make_DESTROY(location()); }
"DESTRUCTOR" { return parser_type::make_DESTRUCTOR(location()); }
"DETERMINISTIC" { return parser_type::make_DETERMINISTIC(location()); }
"DICTIONARY" { return parser_type::make_DICTIONARY(location()); }
"DIAGNOSTICS" { return parser_type::make_DIAGNOSTICS(location()); }
"DISCONNECT" { return parser_type::make_DISCONNECT(location()); }
"DISTINCT" { return parser_type::make_DISTINCT(location()); }
"DOMAIN" { return parser_type::make_DOMAIN(location()); }
"DOUBLE" { return parser_type::make_DOUBLE(location()); }
"DROP" { return parser_type::make_DROP(location()); }
"DYNAMIC" { return parser_type::make_DYNAMIC(location()); }
"EACH" { return parser_type::make_EACH(location()); }
"ELSE" { return parser_type::make_ELSE(location()); }
"END" { return parser_type::make_END(location()); }
"END-EXEC" { return parser_type::make_END_EXEC(location()); }
"EQUALS" { return parser_type::make_EQUALS(location()); }
"ESCAPE" { return parser_type::make_ESCAPE(location()); }
"EVERY" { return parser_type::make_EVERY(location()); }
"EXCEPT" { return parser_type::make_EXCEPT(location()); }
"EXCEPTION" { return parser_type::make_EXCEPTION(location()); }
"EXEC" { return parser_type::make_EXEC(location()); }
"EXECUTE" { return parser_type::make_EXECUTE(location()); }
"EXTERNAL" { return parser_type::make_EXTERNAL(location()); }
"FALSE" { return parser_type::make_FALSE(location()); }
"FETCH" { return parser_type::make_FETCH(location()); }
"FIRST" { return parser_type::make_FIRST(location()); }
"FLOAT" { return parser_type::make_FLOAT(location()); }
"FOR" { return parser_type::make_FOR(location()); }
"FOREIGN" { return parser_type::make_FOREIGN(location()); }
"FOUND" { return parser_type::make_FOUND(location()); }
"FROM" { return parser_type::make_FROM(location()); }
"FREE" { return parser_type::make_FREE(location()); }
"FULL" { return parser_type::make_FULL(location()); }
"FUNCTION" { return parser_type::make_FUNCTION(location()); }
"GENERAL" { return parser_type::make_GENERAL(location()); }
"GET" { return parser_type::make_GET(location()); }
"GLOBAL" { return parser_type::make_GLOBAL(location()); }
"GO" { return parser_type::make_GO(location()); }
"GOTO" { return parser_type::make_GOTO(location()); }
"GRANT" { return parser_type::make_GRANT(location()); }
"GROUP" { return parser_type::make_GROUP(location()); }
"GROUPING" { return parser_type::make_GROUPING(location()); }
"HAVING" { return parser_type::make_HAVING(location()); }
"HOST" { return parser_type::make_HOST(location()); }
"HOUR" { return parser_type::make_HOUR(location()); }
"IDENTITY" { return parser_type::make_IDENTITY(location()); }
"IGNORE" { return parser_type::make_IGNORE(location()); }
"IMMEDIATE" { return parser_type::make_IMMEDIATE(location()); }
"IN" { return parser_type::make_IN(location()); }
"INDICATOR" { return parser_type::make_INDICATOR(location()); }
"INITIALIZE" { return parser_type::make_INITIALIZE(location()); }
"INITIALLY" { return parser_type::make_INITIALLY(location()); }
"INNER" { return parser_type::make_INNER(location()); }
"INOUT" { return parser_type::make_INOUT(location()); }
"INPUT" { return parser_type::make_INPUT(location()); }
"INSERT" { return parser_type::make_INSERT(location()); }
"INT" { return parser_type::make_INT(location()); }
"INTEGER" { return parser_type::make_INTEGER(location()); }
"INTERSECT" { return parser_type::make_INTERSECT(location()); }
"INTERVAL" { return parser_type::make_INTERVAL(location()); }
"INTO" { return parser_type::make_INTO(location()); }
"IS" { return parser_type::make_IS(location()); }
"ISOLATION" { return parser_type::make_ISOLATION(location()); }
"ITERATE" { return parser_type::make_ITERATE(location()); }
"JOIN" { return parser_type::make_JOIN(location()); }
"KEY" { return parser_type::make_KEY(location()); }
"LANGUAGE" { return parser_type::make_LANGUAGE(location()); }
"LARGE" { return parser_type::make_LARGE(location()); }
"LAST" { return parser_type::make_LAST(location()); }
"LATERAL" { return parser_type::make_LATERAL(location()); }
"LEADING" { return parser_type::make_LEADING(location()); }
"LEFT" { return parser_type::make_LEFT(location()); }
"LESS" { return parser_type::make_LESS(location()); }
"LEVEL" { return parser_type::make_LEVEL(location()); }
"LIKE" { return parser_type::make_LIKE(location()); }
"LIMIT" { return parser_type::make_LIMIT(location()); }
"LOCAL" { return parser_type::make_LOCAL(location()); }
"LOCALTIME" { return parser_type::make_LOCALTIME(location()); }
"LOCALTIMESTAMP" { return parser_type::make_LOCALTIMESTAMP(location()); }
"LOCATOR" { return parser_type::make_LOCATOR(location()); }
"MAP" { return parser_type::make_MAP(location()); }
"MATCH" { return parser_type::make_MATCH(location()); }
"MINUTE" { return parser_type::make_MINUTE(location()); }
"MODIFIES" { return parser_type::make_MODIFIES(location()); }
"MODIFY" { return parser_type::make_MODIFY(location()); }
"MODULE" { return parser_type::make_MODULE(location()); }
"MONTH" { return parser_type::make_MONTH(location()); }
"NAMES" { return parser_type::make_NAMES(location()); }
"NATIONAL" { return parser_type::make_NATIONAL(location()); }
"NATURAL" { return parser_type::make_NATURAL(location()); }
"NCHAR" { return parser_type::make_NCHAR(location()); }
"NCLOB" { return parser_type::make_NCLOB(location()); }
"NEW" { return parser_type::make_NEW(location()); }
"NEXT" { return parser_type::make_NEXT(location()); }
"NO" { return parser_type::make_NO(location()); }
"NONE" { return parser_type::make_NONE(location()); }
"NOT" { return parser_type::make_NOT(location()); }
"NULL" { return parser_type::make_NULL_(location()); }
"NUMERIC" { return parser_type::make_NUMERIC(location()); }
"OBJECT" { return parser_type::make_OBJECT(location()); }
"OF" { return parser_type::make_OF(location()); }
"OFF" { return parser_type::make_OFF(location()); }
"OLD" { return parser_type::make_OLD(location()); }
"ON" { return parser_type::make_ON(location()); }
"ONLY" { return parser_type::make_ONLY(location()); }
"OPEN" { return parser_type::make_OPEN(location()); }
"OPERATION" { return parser_type::make_OPERATION(location()); }
"OPTION" { return parser_type::make_OPTION(location()); }
"OR" { return parser_type::make_OR(location()); }
"ORDER" { return parser_type::make_ORDER(location()); }
"ORDINALITY" { return parser_type::make_ORDINALITY(location()); }
"OUT" { return parser_type::make_OUT(location()); }
"OUTER" { return parser_type::make_OUTER(location()); }
"OUTPUT" { return parser_type::make_OUTPUT(location()); }
"PAD" { return parser_type::make_PAD(location()); }
"PARAMETER" { return parser_type::make_PARAMETER(location()); }
"PARAMETERS" { return parser_type::make_PARAMETERS(location()); }
"PARTIAL" { return parser_type::make_PARTIAL(location()); }
"PATH" { return parser_type::make_PATH(location()); }
"POSTFIX" { return parser_type::make_POSTFIX(location()); }
"PRECISION" { return parser_type::make_PRECISION(location()); }
"PREFIX" { return parser_type::make_PREFIX(location()); }
"PREORDER" { return parser_type::make_PREORDER(location()); }
"PREPARE" { return parser_type::make_PREPARE(location()); }
"PRESERVE" { return parser_type::make_PRESERVE(location()); }
"PRIMARY" { return parser_type::make_PRIMARY(location()); }
"PRIOR" { return parser_type::make_PRIOR(location()); }
"PRIVILEGES" { return parser_type::make_PRIVILEGES(location()); }
"PROCEDURE" { return parser_type::make_PROCEDURE(location()); }
"PUBLIC" { return parser_type::make_PUBLIC(location()); }
"READ" { return parser_type::make_READ(location()); }
"READS" { return parser_type::make_READS(location()); }
"REAL" { return parser_type::make_REAL(location()); }
"RECURSIVE" { return parser_type::make_RECURSIVE(location()); }
"REF" { return parser_type::make_REF(location()); }
"REFERENCES" { return parser_type::make_REFERENCES(location()); }
"REFERENCING" { return parser_type::make_REFERENCING(location()); }
"RELATIVE" { return parser_type::make_RELATIVE(location()); }
"RESTRICT" { return parser_type::make_RESTRICT(location()); }
"RESULT" { return parser_type::make_RESULT(location()); }
"RETURN" { return parser_type::make_RETURN(location()); }
"RETURNS" { return parser_type::make_RETURNS(location()); }
"REVOKE" { return parser_type::make_REVOKE(location()); }
"RIGHT" { return parser_type::make_RIGHT(location()); }
"ROLE" { return parser_type::make_ROLE(location()); }
"ROLLBACK" { return parser_type::make_ROLLBACK(location()); }
"ROLLUP" { return parser_type::make_ROLLUP(location()); }
"ROUTINE" { return parser_type::make_ROUTINE(location()); }
"ROW" { return parser_type::make_ROW(location()); }
"ROWS" { return parser_type::make_ROWS(location()); }
"SAVEPOINT" { return parser_type::make_SAVEPOINT(location()); }
"SCHEMA" { return parser_type::make_SCHEMA(location()); }
"SCROLL" { return parser_type::make_SCROLL(location()); }
"SCOPE" { return parser_type::make_SCOPE(location()); }
"SEARCH" { return parser_type::make_SEARCH(location()); }
"SECOND" { return parser_type::make_SECOND(location()); }
"SECTION" { return parser_type::make_SECTION(location()); }
"SELECT" { return parser_type::make_SELECT(location()); }
"SEQUENCE" { return parser_type::make_SEQUENCE(location()); }
"SESSION" { return parser_type::make_SESSION(location()); }
"SESSION_USER" { return parser_type::make_SESSION_USER(location()); }
"SET" { return parser_type::make_SET(location()); }
"SETS" { return parser_type::make_SETS(location()); }
"SIZE" { return parser_type::make_SIZE(location()); }
"SMALLINT" { return parser_type::make_SMALLINT(location()); }
"SOME" { return parser_type::make_SOME(location()); }
"SPACE" { return parser_type::make_SPACE(location()); }
"SPECIFIC" { return parser_type::make_SPECIFIC(location()); }
"SPECIFICTYPE" { return parser_type::make_SPECIFICTYPE(location()); }
"SQL" { return parser_type::make_SQL(location()); }
"SQLEXCEPTION" { return parser_type::make_SQLEXCEPTION(location()); }
"SQLSTATE" { return parser_type::make_SQLSTATE(location()); }
"SQLWARNING" { return parser_type::make_SQLWARNING(location()); }
"START" { return parser_type::make_START(location()); }
"STATE" { return parser_type::make_STATE(location()); }
"STATEMENT" { return parser_type::make_STATEMENT(location()); }
"STATIC" { return parser_type::make_STATIC(location()); }
"STRUCTURE" { return parser_type::make_STRUCTURE(location()); }
"SYSTEM_USER" { return parser_type::make_SYSTEM_USER(location()); }
"TABLE" { return parser_type::make_TABLE(location()); }
"TEMPORARY" { return parser_type::make_TEMPORARY(location()); }
"TERMINATE" { return parser_type::make_TERMINATE(location()); }
"THAN" { return parser_type::make_THAN(location()); }
"THEN" { return parser_type::make_THEN(location()); }
"TIME" { return parser_type::make_TIME(location()); }
"TIMESTAMP" { return parser_type::make_TIMESTAMP(location()); }
"TIMEZONE_HOUR" { return parser_type::make_TIMEZONE_HOUR(location()); }
"TIMEZONE_MINUTE" { return parser_type::make_TIMEZONE_MINUTE(location()); }
"TO" { return parser_type::make_TO(location()); }
"TRAILING" { return parser_type::make_TRAILING(location()); }
"TRANSACTION" { return parser_type::make_TRANSACTION(location()); }
"TRANSLATION" { return parser_type::make_TRANSLATION(location()); }
"TREAT" { return parser_type::make_TREAT(location()); }
"TRIGGER" { return parser_type::make_TRIGGER(location()); }
"TRUE" { return parser_type::make_TRUE(location()); }
"UNDER" { return parser_type::make_UNDER(location()); }
"UNION" { return parser_type::make_UNION(location()); }
"UNIQUE" { return parser_type::make_UNIQUE(location()); }
"UNKNOWN" { return parser_type::make_UNKNOWN(location()); }
"UNNEST" { return parser_type::make_UNNEST(location()); }
"UPDATE" { return parser_type::make_UPDATE(location()); }
"USAGE" { return parser_type::make_USAGE(location()); }
"USER" { return parser_type::make_USER(location()); }
"USING" { return parser_type::make_USING(location()); }
"VALUE" { return parser_type::make_VALUE(location()); }
"VALUES" { return parser_type::make_VALUES(location()); }
"VARCHAR" { return parser_type::make_VARCHAR(location()); }
"VARIABLE" { return parser_type::make_VARIABLE(location()); }
"VARYING" { return parser_type::make_VARYING(location()); }
"VIEW" { return parser_type::make_VIEW(location()); }
"WHEN" { return parser_type::make_WHEN(location()); }
"WHENEVER" { return parser_type::make_WHENEVER(location()); }
"WHERE" { return parser_type::make_WHERE(location()); }
"WITH" { return parser_type::make_WITH(location()); }
"WITHOUT" { return parser_type::make_WITHOUT(location()); }
"WORK" { return parser_type::make_WORK(location()); }
"WRITE" { return parser_type::make_WRITE(location()); }
"YEAR" { return parser_type::make_YEAR(location()); }
"ZONE" { return parser_type::make_ZONE(location()); }

"<@" { return parser_type::make_CONTAINS_OPERATOR(location()); }
"@>" { return parser_type::make_IS_CONTAINED_BY_OPERATOR(location()); }
"&&" { return parser_type::make_OVERLAPS_OPERATOR(location()); }

"PLACING" { return parser_type::make_PLACING(location()); }
"TINYINT" { return parser_type::make_TINYINT(location()); }
"BIGINT" { return parser_type::make_BIGINT(location()); }
"BIT_AND" { return parser_type::make_BOOL_AND(location()); }
"BIT_OR" { return parser_type::make_BOOL_OR(location()); }
"BOOL_AND" { return parser_type::make_BOOL_AND(location()); }
"BOOL_OR" { return parser_type::make_BOOL_OR(location()); }

{identifier} {
    return parser_type::make_REGULAR_IDENTIFIER(get_image(driver), location());
}

{delimited_identifier} {
    return parser_type::make_DELIMITED_IDENTIFIER(get_image(driver), location());
}

{host_parameter_name} {
    return parser_type::make_HOST_PARAMETER_NAME(get_image(driver), location());
}

{unsigned_integer} {
    return parser_type::make_UNSIGNED_INTEGER(get_image(driver), location());
}

{decimal} {
    return parser_type::make_EXACT_NUMERIC_LITERAL(get_image(driver), location());
}

{approximate_numeric_literal} {
    return parser_type::make_APPROXIMATE_NUMERIC_LITERAL(get_image(driver), location());
}

{character_string_literal} {
    return parser_type::make_CHARACTER_STRING_LITERAL(get_image(driver), location());
}

{bit_string_literal} {
    return parser_type::make_BIT_STRING_LITERAL(get_image(driver), location());
}

{hex_string_literal} {
    return parser_type::make_HEX_STRING_LITERAL(get_image(driver), location());
}

<*>{
    . { return parser_type::make_ERROR(location()); }
}

<<EOF>> { return parser_type::make_END_OF_FILE(location()); }

%%
