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

line_break ("\n"|"\r\n")

/* <regular identifier> */
identifier_start [A-Z_a-z]
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

/* error handling */
ASCII   [\x00-\x7f]
UTF8_2  [\xc2-\xdf]
UTF8_3  [\xe0-\xef]
UTF8_4  [\xf0-\xf4]
U       [\x80-\xbf]

UTF8_CHAR {ASCII}|{UTF8_2}{U}|{UTF8_3}{U}{U}|{UTF8_4}{U}{U}{U}

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
        driver.add_comment(exit_comment(true));
        BEGIN(INITIAL);
    }
    <<EOF>> {
        // NOTE: EOF does not update yyleng
        auto loc = exit_comment(true);
        return parser_type::make_UNCLOSED_BLOCK_COMMENT(loc(0, 2));
    }
    {UTF8_CHAR} {}
    [\x00-\xff] {}
}

"--" {
    BEGIN(simple_comment);
    enter_comment();
}

<simple_comment>{
    {line_break} {
        driver.add_comment(exit_comment(false));
        BEGIN(INITIAL);
    }
    <<EOF>> {
        driver.add_comment(exit_comment(true));
        return parser_type::make_END_OF_FILE(location(true));
    }
    {UTF8_CHAR} {}
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

    /* <reserved word> */
"ABS" { return parser_type::make_ABS(location()); }
"ASYMMETRIC" { return parser_type::make_ASYMMETRIC(location()); }
"AVG" { return parser_type::make_AVG(location()); }
"BETWEEN" { return parser_type::make_BETWEEN(location()); }
"BIT_LENGTH" { return parser_type::make_BIT_LENGTH(location()); }
"BITVAR" { return parser_type::make_BITVAR(location()); }
"CARDINALITY" { return parser_type::make_CARDINALITY(location()); }
"CHAR_LENGTH" { return parser_type::make_CHAR_LENGTH(location()); }
"CHARACTER_LENGTH" { return parser_type::make_CHARACTER_LENGTH(location()); }
"COALESCE" { return parser_type::make_COALESCE(location()); }
"CONVERT" { return parser_type::make_CONVERT(location()); }
"COUNT" { return parser_type::make_COUNT(location()); }
"EXISTS" { return parser_type::make_EXISTS(location()); }
"EXTRACT" { return parser_type::make_EXTRACT(location()); }
"GENERATED" { return parser_type::make_GENERATED(location()); }
"LENGTH" { return parser_type::make_LENGTH(location()); }
"LOWER" { return parser_type::make_LOWER(location()); }
"MAX" { return parser_type::make_MAX(location()); }
"MIN" { return parser_type::make_MIN(location()); }
"MOD" { return parser_type::make_MOD(location()); }
"NULLIF" { return parser_type::make_NULLIF(location()); }
"OCTET_LENGTH" { return parser_type::make_OCTET_LENGTH(location()); }
"OVERLAPS" { return parser_type::make_OVERLAPS(location()); }
"OVERLAY" { return parser_type::make_OVERLAY(location()); }
"POSITION" { return parser_type::make_POSITION(location()); }
"SIMILAR" { return parser_type::make_SIMILAR(location()); }
"SUBLIST" { return parser_type::make_SUBLIST(location()); }
"SUBSTRING" { return parser_type::make_SUBSTRING(location()); }
"SUM" { return parser_type::make_SUM(location()); }
"SYMMETRIC" { return parser_type::make_SYMMETRIC(location()); }
"TRANSLATE" { return parser_type::make_TRANSLATE(location()); }
"TRIM" { return parser_type::make_TRIM(location()); }
"UPPER" { return parser_type::make_UPPER(location()); }

    /* <reserved word> */
"ABSOLUTE" { return parser_type::make_ABSOLUTE(get_image(driver), location()); }
"ACTION" { return parser_type::make_ACTION(get_image(driver), location()); }
"ADD" { return parser_type::make_ADD(get_image(driver), location()); }
"ADMIN" { return parser_type::make_ADMIN(get_image(driver), location()); }
"AFTER" { return parser_type::make_AFTER(get_image(driver), location()); }
"ALIAS" { return parser_type::make_ALIAS(get_image(driver), location()); }
"ALL" { return parser_type::make_ALL(location()); }
"ALTER" { return parser_type::make_ALTER(location()); }
"AND" { return parser_type::make_AND(location()); }
"ANY" { return parser_type::make_ANY(location()); }
"ARE" { return parser_type::make_ARE(location()); }
"ARRAY" { return parser_type::make_ARRAY(location()); }
"AS" { return parser_type::make_AS(location()); }
"ASC" { return parser_type::make_ASC(get_image(driver), location()); }
"ASSERTION" { return parser_type::make_ASSERTION(get_image(driver), location()); }
"AT" { return parser_type::make_AT(location()); }
"AUTHORIZATION" { return parser_type::make_AUTHORIZATION(location()); }
"BEFORE" { return parser_type::make_BEFORE(get_image(driver), location()); }
"BEGIN" { return parser_type::make_BEGIN_(location()); }
"BINARY" { return parser_type::make_BINARY(location()); }
"BIT" { return parser_type::make_BIT(location()); }
"BLOB" { return parser_type::make_BLOB(location()); }
"BOOLEAN" { return parser_type::make_BOOLEAN(location()); }
"BOTH" { return parser_type::make_BOTH(location()); }
"BY" { return parser_type::make_BY(location()); }
"CALL" { return parser_type::make_CALL(location()); }
"CASCADE" { return parser_type::make_CASCADE(get_image(driver), location()); }
"CASCADED" { return parser_type::make_CASCADED(location()); }
"CASE" { return parser_type::make_CASE(location()); }
"CAST" { return parser_type::make_CAST(location()); }
"CHAR" { return parser_type::make_CHAR(location()); }
"CHARACTER" { return parser_type::make_CHARACTER(location()); }
"CHECK" { return parser_type::make_CHECK(location()); }
"CLASS" { return parser_type::make_CLASS(location()); }
"CLOB" { return parser_type::make_CLOB(location()); }
"CLOSE" { return parser_type::make_CLOSE(location()); }
"COLLATE" { return parser_type::make_COLLATE(location()); }
"COLUMN" { return parser_type::make_COLUMN(location()); }
"COMMIT" { return parser_type::make_COMMIT(location()); }
"CONNECT" { return parser_type::make_CONNECT(location()); }
"CONSTRAINT" { return parser_type::make_CONSTRAINT(location()); }
"CONSTRAINTS" { return parser_type::make_CONSTRAINTS(location()); }
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
"DATE" { return parser_type::make_DATE(location()); }
"DAY" { return parser_type::make_DAY(location()); }
"DEALLOCATE" { return parser_type::make_DEALLOCATE(location()); }
"DEC" { return parser_type::make_DEC(location()); }
"DECIMAL" { return parser_type::make_DECIMAL(location()); }
"DECLARE" { return parser_type::make_DECLARE(location()); }
"DEFAULT" { return parser_type::make_DEFAULT(location()); }
"DELETE" { return parser_type::make_DELETE(location()); }
"DEREF" { return parser_type::make_DEREF(location()); }
"DESC" { return parser_type::make_DESC(get_image(driver), location()); }
"DESCRIBE" { return parser_type::make_DESCRIBE(location()); }
"DETERMINISTIC" { return parser_type::make_DETERMINISTIC(location()); }
"DISCONNECT" { return parser_type::make_DISCONNECT(location()); }
"DISTINCT" { return parser_type::make_DISTINCT(location()); }
"DOUBLE" { return parser_type::make_DOUBLE(location()); }
"DROP" { return parser_type::make_DROP(location()); }
"DYNAMIC" { return parser_type::make_DYNAMIC(location()); }
"EACH" { return parser_type::make_EACH(location()); }
"ELSE" { return parser_type::make_ELSE(get_image(driver), location()); }
"END" { return parser_type::make_END(location()); }
"END-EXEC" { return parser_type::make_END_EXEC(location()); }
"ESCAPE" { return parser_type::make_ESCAPE(location()); }
"EVERY" { return parser_type::make_EVERY(location()); }
"EXCEPT" { return parser_type::make_EXCEPT(location()); }
"EXEC" { return parser_type::make_EXEC(location()); }
"EXECUTE" { return parser_type::make_EXECUTE(location()); }
"EXTERNAL" { return parser_type::make_EXTERNAL(location()); }
"FALSE" { return parser_type::make_FALSE(location()); }
"FETCH" { return parser_type::make_FETCH(location()); }
"FIRST" { return parser_type::make_FIRST(get_image(driver), location()); }
"FLOAT" { return parser_type::make_FLOAT(location()); }
"FOR" { return parser_type::make_FOR(location()); }
"FOREIGN" { return parser_type::make_FOREIGN(location()); }
"FROM" { return parser_type::make_FROM(location()); }
"FULL" { return parser_type::make_FULL(location()); }
"FUNCTION" { return parser_type::make_FUNCTION(location()); }
"GET" { return parser_type::make_GET(location()); }
"GLOBAL" { return parser_type::make_GLOBAL(location()); }
"GRANT" { return parser_type::make_GRANT(location()); }
"GROUP" { return parser_type::make_GROUP(location()); }
"GROUPING" { return parser_type::make_GROUPING(location()); }
"HAVING" { return parser_type::make_HAVING(location()); }
"HOUR" { return parser_type::make_HOUR(location()); }
"IDENTITY" { return parser_type::make_IDENTITY(location()); }
"IGNORE" { return parser_type::make_IGNORE(get_image(driver), location()); }
"IN" { return parser_type::make_IN(location()); }
"INDICATOR" { return parser_type::make_INDICATOR(location()); }
"INNER" { return parser_type::make_INNER(location()); }
"INOUT" { return parser_type::make_INOUT(location()); }
"INSERT" { return parser_type::make_INSERT(location()); }
"INT" { return parser_type::make_INT(location()); }
"INTEGER" { return parser_type::make_INTEGER(location()); }
"INTERSECT" { return parser_type::make_INTERSECT(location()); }
"INTERVAL" { return parser_type::make_INTERVAL(location()); }
"INTO" { return parser_type::make_INTO(location()); }
"IS" { return parser_type::make_IS(location()); }
"JOIN" { return parser_type::make_JOIN(location()); }
"KEY" { return parser_type::make_KEY(get_image(driver), location()); }
"LANGUAGE" { return parser_type::make_LANGUAGE(location()); }
"LARGE" { return parser_type::make_LARGE(location()); }
"LAST" { return parser_type::make_LAST(get_image(driver), location()); }
"LATERAL" { return parser_type::make_LATERAL(location()); }
"LEADING" { return parser_type::make_LEADING(location()); }
"LEFT" { return parser_type::make_LEFT(location()); }
"LIKE" { return parser_type::make_LIKE(location()); }
"LIMIT" { return parser_type::make_LIMIT(location()); }
"LOCAL" { return parser_type::make_LOCAL(location()); }
"LOCALTIME" { return parser_type::make_LOCALTIME(location()); }
"LOCALTIMESTAMP" { return parser_type::make_LOCALTIMESTAMP(location()); }
"MATCH" { return parser_type::make_MATCH(location()); }
"MINUTE" { return parser_type::make_MINUTE(location()); }
"MODIFIES" { return parser_type::make_MODIFIES(location()); }
"MODULE" { return parser_type::make_MODULE(location()); }
"MONTH" { return parser_type::make_MONTH(location()); }
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
"OF" { return parser_type::make_OF(location()); }
"OLD" { return parser_type::make_OLD(location()); }
"ON" { return parser_type::make_ON(location()); }
"ONLY" { return parser_type::make_ONLY(location()); }
"OPEN" { return parser_type::make_OPEN(location()); }
"OR" { return parser_type::make_OR(location()); }
"ORDER" { return parser_type::make_ORDER(location()); }
"ORDINALITY" { return parser_type::make_ORDINALITY(get_image(driver), location()); }
"OUT" { return parser_type::make_OUT(location()); }
"OUTER" { return parser_type::make_OUTER(location()); }
"PARAMETER" { return parser_type::make_PARAMETER(location()); }
"PRECISION" { return parser_type::make_PRECISION(location()); }
"PREPARE" { return parser_type::make_PREPARE(location()); }
"PRIMARY" { return parser_type::make_PRIMARY(location()); }
"PROCEDURE" { return parser_type::make_PROCEDURE(location()); }
"REAL" { return parser_type::make_REAL(location()); }
"RECURSIVE" { return parser_type::make_RECURSIVE(location()); }
"REF" { return parser_type::make_REF(location()); }
"REFERENCES" { return parser_type::make_REFERENCES(location()); }
"REFERENCING" { return parser_type::make_REFERENCING(location()); }
"RESTRICT" { return parser_type::make_RESTRICT(get_image(driver), location()); }
"RESULT" { return parser_type::make_RESULT(location()); }
"RETURN" { return parser_type::make_RETURN(location()); }
"RETURNS" { return parser_type::make_RETURNS(location()); }
"REVOKE" { return parser_type::make_REVOKE(location()); }
"RIGHT" { return parser_type::make_RIGHT(location()); }
"ROLE" { return parser_type::make_ROLE(location()); }
"ROLLBACK" { return parser_type::make_ROLLBACK(location()); }
"ROLLUP" { return parser_type::make_ROLLUP(location()); }
"ROW" { return parser_type::make_ROW(location()); }
"ROWS" { return parser_type::make_ROWS(location()); }
"SAVEPOINT" { return parser_type::make_SAVEPOINT(location()); }
"SCHEMA" { return parser_type::make_SCHEMA(get_image(driver), location()); }
"SCOPE" { return parser_type::make_SCOPE(location()); }
"SEARCH" { return parser_type::make_SEARCH(location()); }
"SECOND" { return parser_type::make_SECOND(location()); }
"SELECT" { return parser_type::make_SELECT(location()); }
"SEQUENCE" { return parser_type::make_SEQUENCE(get_image(driver), location()); }
"SESSION_USER" { return parser_type::make_SESSION_USER(location()); }
"SET" { return parser_type::make_SET(location()); }
"SMALLINT" { return parser_type::make_SMALLINT(location()); }
"SOME" { return parser_type::make_SOME(location()); }
"SPECIFIC" { return parser_type::make_SPECIFIC(location()); }
"SQL" { return parser_type::make_SQL(location()); }
"SQLEXCEPTION" { return parser_type::make_SQLEXCEPTION(location()); }
"SQLSTATE" { return parser_type::make_SQLSTATE(location()); }
"SQLWARNING" { return parser_type::make_SQLWARNING(location()); }
"START" { return parser_type::make_START(location()); }
"STATIC" { return parser_type::make_STATIC(location()); }
"SYSTEM_USER" { return parser_type::make_SYSTEM_USER(location()); }
"TABLE" { return parser_type::make_TABLE(location()); }
"TEMPORARY" { return parser_type::make_TEMPORARY(location()); }
"THEN" { return parser_type::make_THEN(location()); }
"TIME" { return parser_type::make_TIME(location()); }
"TIMESTAMP" { return parser_type::make_TIMESTAMP(location()); }
"TIMEZONE_HOUR" { return parser_type::make_TIMEZONE_HOUR(location()); }
"TIMEZONE_MINUTE" { return parser_type::make_TIMEZONE_MINUTE(location()); }
"TO" { return parser_type::make_TO(location()); }
"TRAILING" { return parser_type::make_TRAILING(location()); }
"TRANSLATION" { return parser_type::make_TRANSLATION(location()); }
"TREAT" { return parser_type::make_TREAT(location()); }
"TRIGGER" { return parser_type::make_TRIGGER(location()); }
"TRUE" { return parser_type::make_TRUE(location()); }
"UNION" { return parser_type::make_UNION(location()); }
"UNIQUE" { return parser_type::make_UNIQUE(location()); }
"UNKNOWN" { return parser_type::make_UNKNOWN(location()); }
"UNNEST" { return parser_type::make_UNNEST(location()); }
"UPDATE" { return parser_type::make_UPDATE(location()); }
"USER" { return parser_type::make_USER(location()); }
"USING" { return parser_type::make_USING(location()); }
"VALUE" { return parser_type::make_VALUE(get_image(driver), location()); }
"VALUES" { return parser_type::make_VALUES(location()); }
"VARCHAR" { return parser_type::make_VARCHAR(location()); }
"VARYING" { return parser_type::make_VARYING(location()); }
"VIEW" { return parser_type::make_VIEW(location()); }
"WHEN" { return parser_type::make_WHEN(location()); }
"WHENEVER" { return parser_type::make_WHENEVER(location()); }
"WHERE" { return parser_type::make_WHERE(location()); }
"WITH" { return parser_type::make_WITH(location()); }
"WITHOUT" { return parser_type::make_WITHOUT(location()); }
"YEAR" { return parser_type::make_YEAR(location()); }
"ZONE" { return parser_type::make_ZONE(location()); }

    /* SQL-2003 keywords */
"ALWAYS" { return parser_type::make_ALWAYS(location()); }
"MAXVALUE" { return parser_type::make_MAXVALUE(location()); }
"MINVALUE" { return parser_type::make_MINVALUE(location()); }
"INCREMENT" { return parser_type::make_INCREMENT(location()); }
"NULLS" { return parser_type::make_NULLS(location()); }
"REPLACE" { return parser_type::make_REPLACE(location()); }
"OWNED" { return parser_type::make_OWNED(location()); }

    /* extra operators */
"<@" { return parser_type::make_CONTAINS_OPERATOR(location()); }
"@>" { return parser_type::make_IS_CONTAINED_BY_OPERATOR(location()); }
"&&" { return parser_type::make_OVERLAPS_OPERATOR(location()); }

    /* extra keywords */
"INCLUDE" { return parser_type::make_INCLUDE(location()); }
"IF" { return parser_type::make_IF(location()); }
"INDEX" { return parser_type::make_INDEX(location()); }

"VARBIT" { return parser_type::make_VARBIT(location()); }
"VARBINARY" { return parser_type::make_VARBINARY(location()); }

"PLACING" { return parser_type::make_PLACING(location()); }
"TINYINT" { return parser_type::make_TINYINT(location()); }
"BIGINT" { return parser_type::make_BIGINT(location()); }
"BIT_AND" { return parser_type::make_BOOL_AND(location()); }
"BIT_OR" { return parser_type::make_BOOL_OR(location()); }
"BOOL_AND" { return parser_type::make_BOOL_AND(location()); }
"BOOL_OR" { return parser_type::make_BOOL_OR(location()); }

{identifier} {
    auto token = get_image(driver);
    if (!driver.check_regular_identifier(token)) {
        return parser_type::make_REGULAR_IDENTIFIER_RESTRICTED(location());
    }
    return parser_type::make_REGULAR_IDENTIFIER(std::move(token), location());
}

{delimited_identifier} {
    auto token = get_image(driver);
    if (!driver.check_delimited_identifier(token)) {
        return parser_type::make_DELIMITED_IDENTIFIER_RESTRICTED(location());
    }
    return parser_type::make_DELIMITED_IDENTIFIER(std::move(token), location());
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
    {UTF8_CHAR} {
        return parser_type::make_ERROR(location());
    }
    [\x00-\xff] {
        return parser_type::make_ERROR(location());
    }
}

<<EOF>> {
    return parser_type::make_END_OF_FILE(location(true));
}

%%
