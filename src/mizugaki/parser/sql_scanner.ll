%{
#include <mizugaki/parser/sql_scanner.h>
#include <mizugaki/parser/sql_parser_generated.hpp>

#define YY_USER_ACTION user_action();

#define yyterminate() return parser_type::make_ERROR(location())

#undef YY_DECL
#define YY_DECL ::mizugaki::parser::sql_scanner::value_type mizugaki::parser::sql_scanner::next_token(::mizugaki::parser::sql_driver& driver)

#define TRACE_RETURN on_token(driver); return
#define TRACE_RETURN_EOF on_token(driver, true); return
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
        TRACE_RETURN_EOF parser_type::make_UNCLOSED_BLOCK_COMMENT(loc(0, 2));
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
        TRACE_RETURN_EOF parser_type::make_END_OF_FILE(location(true));
    }
    {UTF8_CHAR} {}
    . {}
}

{union_join} { TRACE_RETURN parser_type::make_UNION_JOIN(location()); }

{dot_asterisk} { TRACE_RETURN parser_type::make_DOT_ASTERISK(location()); }

"<>" { TRACE_RETURN parser_type::make_NOT_EQUALS_OPERATOR(location()); }
">=" { TRACE_RETURN parser_type::make_GREATER_THAN_OR_EQUALS_OPERATOR(location()); }
"<=" { TRACE_RETURN parser_type::make_LESS_THAN_OR_EQUALS_OPERATOR(location()); }
"||" { TRACE_RETURN parser_type::make_CONCATENATION_OPERATOR(location()); }
"->" { TRACE_RETURN parser_type::make_RIGHT_ARROW(location()); }
"::" { TRACE_RETURN parser_type::make_DOUBLE_COLON(location()); }

"%" { TRACE_RETURN parser_type::make_PERCENT(location()); }
"&" { TRACE_RETURN parser_type::make_AMPERSAND(location()); }
"'" { TRACE_RETURN parser_type::make_QUOTE(location()); }
"(" { TRACE_RETURN parser_type::make_LEFT_PAREN(location()); }
")" { TRACE_RETURN parser_type::make_RIGHT_PAREN(location()); }
"*" { TRACE_RETURN parser_type::make_ASTERISK(location()); }
"+" { TRACE_RETURN parser_type::make_PLUS_SIGN(location()); }
"," { TRACE_RETURN parser_type::make_COMMA(location()); }
"-" { TRACE_RETURN parser_type::make_MINUS_SIGN(location()); }
"." { TRACE_RETURN parser_type::make_PERIOD(location()); }
"/" { TRACE_RETURN parser_type::make_SOLIDUS(location()); }
":" { TRACE_RETURN parser_type::make_COLON(location()); }
";" { TRACE_RETURN parser_type::make_SEMICOLON(location()); }
"<" { TRACE_RETURN parser_type::make_LESS_THAN_OPERATOR(location()); }
"=" { TRACE_RETURN parser_type::make_EQUALS_OPERATOR(location()); }
">" { TRACE_RETURN parser_type::make_GREATER_THAN_OPERATOR(location()); }
"?" { TRACE_RETURN parser_type::make_QUESTION_MARK(location()); }
"[" { TRACE_RETURN parser_type::make_LEFT_BRACKET(location()); }
"]" { TRACE_RETURN parser_type::make_RIGHT_BRACKET(location()); }
"^" { TRACE_RETURN parser_type::make_CIRCUMFLEX(location()); }
"_" { TRACE_RETURN parser_type::make_UNDERSCORE(location()); }
"|" { TRACE_RETURN parser_type::make_VERTICAL_BAR(location()); }
"{" { TRACE_RETURN parser_type::make_LEFT_BRACE(location()); }
"}" { TRACE_RETURN parser_type::make_RIGHT_BRACE(location()); }

    /* <reserved word> */
"ABS" { TRACE_RETURN parser_type::make_ABS(location()); }
"ASYMMETRIC" { TRACE_RETURN parser_type::make_ASYMMETRIC(location()); }
"AVG" { TRACE_RETURN parser_type::make_AVG(location()); }
"BETWEEN" { TRACE_RETURN parser_type::make_BETWEEN(location()); }
"BIT_LENGTH" { TRACE_RETURN parser_type::make_BIT_LENGTH(location()); }
"BITVAR" { TRACE_RETURN parser_type::make_BITVAR(location()); }
"CARDINALITY" { TRACE_RETURN parser_type::make_CARDINALITY(location()); }
"CHAR_LENGTH" { TRACE_RETURN parser_type::make_CHAR_LENGTH(location()); }
"CHARACTER_LENGTH" { TRACE_RETURN parser_type::make_CHARACTER_LENGTH(location()); }
"COALESCE" { TRACE_RETURN parser_type::make_COALESCE(location()); }
"CONVERT" { TRACE_RETURN parser_type::make_CONVERT(location()); }
"COUNT" { TRACE_RETURN parser_type::make_COUNT(location()); }
"EXISTS" { TRACE_RETURN parser_type::make_EXISTS(location()); }
"EXTRACT" { TRACE_RETURN parser_type::make_EXTRACT(location()); }
"GENERATED" { TRACE_RETURN parser_type::make_GENERATED(location()); }
"LENGTH" { TRACE_RETURN parser_type::make_LENGTH(location()); }
"LOWER" { TRACE_RETURN parser_type::make_LOWER(location()); }
"MAX" { TRACE_RETURN parser_type::make_MAX(location()); }
"MIN" { TRACE_RETURN parser_type::make_MIN(location()); }
"MOD" { TRACE_RETURN parser_type::make_MOD(location()); }
"NULLIF" { TRACE_RETURN parser_type::make_NULLIF(location()); }
"OCTET_LENGTH" { TRACE_RETURN parser_type::make_OCTET_LENGTH(location()); }
"OVERLAPS" { TRACE_RETURN parser_type::make_OVERLAPS(location()); }
"OVERLAY" { TRACE_RETURN parser_type::make_OVERLAY(location()); }
"POSITION" { TRACE_RETURN parser_type::make_POSITION(location()); }
"SIMILAR" { TRACE_RETURN parser_type::make_SIMILAR(location()); }
"SUBLIST" { TRACE_RETURN parser_type::make_SUBLIST(location()); }
"SUBSTRING" { TRACE_RETURN parser_type::make_SUBSTRING(location()); }
"SUM" { TRACE_RETURN parser_type::make_SUM(location()); }
"SYMMETRIC" { TRACE_RETURN parser_type::make_SYMMETRIC(location()); }
"TRANSLATE" { TRACE_RETURN parser_type::make_TRANSLATE(location()); }
"TRIM" { TRACE_RETURN parser_type::make_TRIM(location()); }
"UPPER" { TRACE_RETURN parser_type::make_UPPER(location()); }

    /* <reserved word> */
"ABSOLUTE" { TRACE_RETURN parser_type::make_ABSOLUTE(get_image(driver), location()); }
"ACTION" { TRACE_RETURN parser_type::make_ACTION(get_image(driver), location()); }
"ADD" { TRACE_RETURN parser_type::make_ADD(get_image(driver), location()); }
"ADMIN" { TRACE_RETURN parser_type::make_ADMIN(get_image(driver), location()); }
"AFTER" { TRACE_RETURN parser_type::make_AFTER(get_image(driver), location()); }
"ALIAS" { TRACE_RETURN parser_type::make_ALIAS(get_image(driver), location()); }
"ALL" { TRACE_RETURN parser_type::make_ALL(location()); }
"ALTER" { TRACE_RETURN parser_type::make_ALTER(location()); }
"AND" { TRACE_RETURN parser_type::make_AND(location()); }
"ANY" { TRACE_RETURN parser_type::make_ANY(location()); }
"ARE" { TRACE_RETURN parser_type::make_ARE(location()); }
"ARRAY" { TRACE_RETURN parser_type::make_ARRAY(location()); }
"AS" { TRACE_RETURN parser_type::make_AS(location()); }
"ASC" { TRACE_RETURN parser_type::make_ASC(get_image(driver), location()); }
"ASSERTION" { TRACE_RETURN parser_type::make_ASSERTION(get_image(driver), location()); }
"AT" { TRACE_RETURN parser_type::make_AT(location()); }
"AUTHORIZATION" { TRACE_RETURN parser_type::make_AUTHORIZATION(location()); }
"BEFORE" { TRACE_RETURN parser_type::make_BEFORE(get_image(driver), location()); }
"BEGIN" { TRACE_RETURN parser_type::make_BEGIN_(location()); }
"BINARY" { TRACE_RETURN parser_type::make_BINARY(location()); }
"BIT" { TRACE_RETURN parser_type::make_BIT(location()); }
"BLOB" { TRACE_RETURN parser_type::make_BLOB(location()); }
"BOOLEAN" { TRACE_RETURN parser_type::make_BOOLEAN(location()); }
"BOTH" { TRACE_RETURN parser_type::make_BOTH(location()); }
"BY" { TRACE_RETURN parser_type::make_BY(location()); }
"CALL" { TRACE_RETURN parser_type::make_CALL(location()); }
"CASCADE" { TRACE_RETURN parser_type::make_CASCADE(get_image(driver), location()); }
"CASCADED" { TRACE_RETURN parser_type::make_CASCADED(location()); }
"CASE" { TRACE_RETURN parser_type::make_CASE(location()); }
"CAST" { TRACE_RETURN parser_type::make_CAST(location()); }
"CHAR" { TRACE_RETURN parser_type::make_CHAR(location()); }
"CHARACTER" { TRACE_RETURN parser_type::make_CHARACTER(location()); }
"CHECK" { TRACE_RETURN parser_type::make_CHECK(location()); }
"CLASS" { TRACE_RETURN parser_type::make_CLASS(location()); }
"CLOB" { TRACE_RETURN parser_type::make_CLOB(location()); }
"CLOSE" { TRACE_RETURN parser_type::make_CLOSE(location()); }
"COLLATE" { TRACE_RETURN parser_type::make_COLLATE(location()); }
"COLUMN" { TRACE_RETURN parser_type::make_COLUMN(location()); }
"COMMIT" { TRACE_RETURN parser_type::make_COMMIT(location()); }
"CONNECT" { TRACE_RETURN parser_type::make_CONNECT(location()); }
"CONSTRAINT" { TRACE_RETURN parser_type::make_CONSTRAINT(location()); }
"CONSTRAINTS" { TRACE_RETURN parser_type::make_CONSTRAINTS(location()); }
"CORRESPONDING" { TRACE_RETURN parser_type::make_CORRESPONDING(location()); }
"CREATE" { TRACE_RETURN parser_type::make_CREATE(location()); }
"CROSS" { TRACE_RETURN parser_type::make_CROSS(location()); }
"CUBE" { TRACE_RETURN parser_type::make_CUBE(location()); }
"CURRENT" { TRACE_RETURN parser_type::make_CURRENT(location()); }
"CURRENT_DATE" { TRACE_RETURN parser_type::make_CURRENT_DATE(location()); }
"CURRENT_PATH" { TRACE_RETURN parser_type::make_CURRENT_PATH(location()); }
"CURRENT_ROLE" { TRACE_RETURN parser_type::make_CURRENT_ROLE(location()); }
"CURRENT_TIME" { TRACE_RETURN parser_type::make_CURRENT_TIME(location()); }
"CURRENT_TIMESTAMP" { TRACE_RETURN parser_type::make_CURRENT_TIMESTAMP(location()); }
"CURRENT_USER" { TRACE_RETURN parser_type::make_CURRENT_USER(location()); }
"CURSOR" { TRACE_RETURN parser_type::make_CURSOR(location()); }
"CYCLE" { TRACE_RETURN parser_type::make_CYCLE(location()); }
"DATE" { TRACE_RETURN parser_type::make_DATE(location()); }
"DAY" { TRACE_RETURN parser_type::make_DAY(location()); }
"DEALLOCATE" { TRACE_RETURN parser_type::make_DEALLOCATE(location()); }
"DEC" { TRACE_RETURN parser_type::make_DEC(location()); }
"DECIMAL" { TRACE_RETURN parser_type::make_DECIMAL(location()); }
"DECLARE" { TRACE_RETURN parser_type::make_DECLARE(location()); }
"DEFAULT" { TRACE_RETURN parser_type::make_DEFAULT(location()); }
"DELETE" { TRACE_RETURN parser_type::make_DELETE(location()); }
"DEREF" { TRACE_RETURN parser_type::make_DEREF(location()); }
"DESC" { TRACE_RETURN parser_type::make_DESC(get_image(driver), location()); }
"DESCRIBE" { TRACE_RETURN parser_type::make_DESCRIBE(location()); }
"DETERMINISTIC" { TRACE_RETURN parser_type::make_DETERMINISTIC(location()); }
"DISCONNECT" { TRACE_RETURN parser_type::make_DISCONNECT(location()); }
"DISTINCT" { TRACE_RETURN parser_type::make_DISTINCT(location()); }
"DOUBLE" { TRACE_RETURN parser_type::make_DOUBLE(location()); }
"DROP" { TRACE_RETURN parser_type::make_DROP(location()); }
"DYNAMIC" { TRACE_RETURN parser_type::make_DYNAMIC(location()); }
"EACH" { TRACE_RETURN parser_type::make_EACH(location()); }
"ELSE" { TRACE_RETURN parser_type::make_ELSE(get_image(driver), location()); }
"END" { TRACE_RETURN parser_type::make_END(location()); }
"END-EXEC" { TRACE_RETURN parser_type::make_END_EXEC(location()); }
"ESCAPE" { TRACE_RETURN parser_type::make_ESCAPE(location()); }
"EVERY" { TRACE_RETURN parser_type::make_EVERY(location()); }
"EXCEPT" { TRACE_RETURN parser_type::make_EXCEPT(location()); }
"EXEC" { TRACE_RETURN parser_type::make_EXEC(location()); }
"EXECUTE" { TRACE_RETURN parser_type::make_EXECUTE(location()); }
"EXTERNAL" { TRACE_RETURN parser_type::make_EXTERNAL(location()); }
"FALSE" { TRACE_RETURN parser_type::make_FALSE(location()); }
"FETCH" { TRACE_RETURN parser_type::make_FETCH(location()); }
"FIRST" { TRACE_RETURN parser_type::make_FIRST(get_image(driver), location()); }
"FLOAT" { TRACE_RETURN parser_type::make_FLOAT(location()); }
"FOR" { TRACE_RETURN parser_type::make_FOR(location()); }
"FOREIGN" { TRACE_RETURN parser_type::make_FOREIGN(location()); }
"FROM" { TRACE_RETURN parser_type::make_FROM(location()); }
"FULL" { TRACE_RETURN parser_type::make_FULL(location()); }
"FUNCTION" { TRACE_RETURN parser_type::make_FUNCTION(location()); }
"GET" { TRACE_RETURN parser_type::make_GET(location()); }
"GLOBAL" { TRACE_RETURN parser_type::make_GLOBAL(location()); }
"GRANT" { TRACE_RETURN parser_type::make_GRANT(location()); }
"GROUP" { TRACE_RETURN parser_type::make_GROUP(location()); }
"GROUPING" { TRACE_RETURN parser_type::make_GROUPING(location()); }
"HAVING" { TRACE_RETURN parser_type::make_HAVING(location()); }
"HOUR" { TRACE_RETURN parser_type::make_HOUR(location()); }
"IDENTITY" { TRACE_RETURN parser_type::make_IDENTITY(location()); }
"IGNORE" { TRACE_RETURN parser_type::make_IGNORE(get_image(driver), location()); }
"IN" { TRACE_RETURN parser_type::make_IN(location()); }
"INDICATOR" { TRACE_RETURN parser_type::make_INDICATOR(location()); }
"INNER" { TRACE_RETURN parser_type::make_INNER(location()); }
"INOUT" { TRACE_RETURN parser_type::make_INOUT(location()); }
"INSERT" { TRACE_RETURN parser_type::make_INSERT(location()); }
"INT" { TRACE_RETURN parser_type::make_INT(location()); }
"INTEGER" { TRACE_RETURN parser_type::make_INTEGER(location()); }
"INTERSECT" { TRACE_RETURN parser_type::make_INTERSECT(location()); }
"INTERVAL" { TRACE_RETURN parser_type::make_INTERVAL(location()); }
"INTO" { TRACE_RETURN parser_type::make_INTO(location()); }
"IS" { TRACE_RETURN parser_type::make_IS(location()); }
"JOIN" { TRACE_RETURN parser_type::make_JOIN(location()); }
"KEY" { TRACE_RETURN parser_type::make_KEY(get_image(driver), location()); }
"LANGUAGE" { TRACE_RETURN parser_type::make_LANGUAGE(location()); }
"LARGE" { TRACE_RETURN parser_type::make_LARGE(location()); }
"LAST" { TRACE_RETURN parser_type::make_LAST(get_image(driver), location()); }
"LATERAL" { TRACE_RETURN parser_type::make_LATERAL(location()); }
"LEADING" { TRACE_RETURN parser_type::make_LEADING(location()); }
"LEFT" { TRACE_RETURN parser_type::make_LEFT(location()); }
"LIKE" { TRACE_RETURN parser_type::make_LIKE(location()); }
"LIMIT" { TRACE_RETURN parser_type::make_LIMIT(location()); }
"LOCAL" { TRACE_RETURN parser_type::make_LOCAL(location()); }
"LOCALTIME" { TRACE_RETURN parser_type::make_LOCALTIME(location()); }
"LOCALTIMESTAMP" { TRACE_RETURN parser_type::make_LOCALTIMESTAMP(location()); }
"MATCH" { TRACE_RETURN parser_type::make_MATCH(location()); }
"MINUTE" { TRACE_RETURN parser_type::make_MINUTE(location()); }
"MODIFIES" { TRACE_RETURN parser_type::make_MODIFIES(location()); }
"MODULE" { TRACE_RETURN parser_type::make_MODULE(location()); }
"MONTH" { TRACE_RETURN parser_type::make_MONTH(location()); }
"NATIONAL" { TRACE_RETURN parser_type::make_NATIONAL(location()); }
"NATURAL" { TRACE_RETURN parser_type::make_NATURAL(location()); }
"NCHAR" { TRACE_RETURN parser_type::make_NCHAR(location()); }
"NCLOB" { TRACE_RETURN parser_type::make_NCLOB(location()); }
"NEW" { TRACE_RETURN parser_type::make_NEW(location()); }
"NEXT" { TRACE_RETURN parser_type::make_NEXT(location()); }
"NO" { TRACE_RETURN parser_type::make_NO(location()); }
"NONE" { TRACE_RETURN parser_type::make_NONE(location()); }
"NOT" { TRACE_RETURN parser_type::make_NOT(location()); }
"NULL" { TRACE_RETURN parser_type::make_NULL_(location()); }
"NUMERIC" { TRACE_RETURN parser_type::make_NUMERIC(location()); }
"OBJECT" { TRACE_RETURN parser_type::make_OBJECT(location()); }
"OF" { TRACE_RETURN parser_type::make_OF(location()); }
"OLD" { TRACE_RETURN parser_type::make_OLD(location()); }
"ON" { TRACE_RETURN parser_type::make_ON(location()); }
"ONLY" { TRACE_RETURN parser_type::make_ONLY(location()); }
"OPEN" { TRACE_RETURN parser_type::make_OPEN(location()); }
"OR" { TRACE_RETURN parser_type::make_OR(location()); }
"ORDER" { TRACE_RETURN parser_type::make_ORDER(location()); }
"ORDINALITY" { TRACE_RETURN parser_type::make_ORDINALITY(get_image(driver), location()); }
"OUT" { TRACE_RETURN parser_type::make_OUT(location()); }
"OUTER" { TRACE_RETURN parser_type::make_OUTER(location()); }
"PARAMETER" { TRACE_RETURN parser_type::make_PARAMETER(location()); }
"PRECISION" { TRACE_RETURN parser_type::make_PRECISION(location()); }
"PREPARE" { TRACE_RETURN parser_type::make_PREPARE(location()); }
"PRIMARY" { TRACE_RETURN parser_type::make_PRIMARY(location()); }
"PROCEDURE" { TRACE_RETURN parser_type::make_PROCEDURE(location()); }
"REAL" { TRACE_RETURN parser_type::make_REAL(location()); }
"RECURSIVE" { TRACE_RETURN parser_type::make_RECURSIVE(location()); }
"REF" { TRACE_RETURN parser_type::make_REF(location()); }
"REFERENCES" { TRACE_RETURN parser_type::make_REFERENCES(location()); }
"REFERENCING" { TRACE_RETURN parser_type::make_REFERENCING(location()); }
"RESTRICT" { TRACE_RETURN parser_type::make_RESTRICT(get_image(driver), location()); }
"RESULT" { TRACE_RETURN parser_type::make_RESULT(location()); }
"RETURN" { TRACE_RETURN parser_type::make_RETURN(location()); }
"RETURNS" { TRACE_RETURN parser_type::make_RETURNS(location()); }
"REVOKE" { TRACE_RETURN parser_type::make_REVOKE(location()); }
"RIGHT" { TRACE_RETURN parser_type::make_RIGHT(location()); }
"ROLE" { TRACE_RETURN parser_type::make_ROLE(location()); }
"ROLLBACK" { TRACE_RETURN parser_type::make_ROLLBACK(location()); }
"ROLLUP" { TRACE_RETURN parser_type::make_ROLLUP(location()); }
"ROW" { TRACE_RETURN parser_type::make_ROW(location()); }
"ROWS" { TRACE_RETURN parser_type::make_ROWS(location()); }
"SAVEPOINT" { TRACE_RETURN parser_type::make_SAVEPOINT(location()); }
"SCHEMA" { TRACE_RETURN parser_type::make_SCHEMA(get_image(driver), location()); }
"SCOPE" { TRACE_RETURN parser_type::make_SCOPE(location()); }
"SEARCH" { TRACE_RETURN parser_type::make_SEARCH(location()); }
"SECOND" { TRACE_RETURN parser_type::make_SECOND(location()); }
"SELECT" { TRACE_RETURN parser_type::make_SELECT(location()); }
"SEQUENCE" { TRACE_RETURN parser_type::make_SEQUENCE(get_image(driver), location()); }
"SESSION_USER" { TRACE_RETURN parser_type::make_SESSION_USER(location()); }
"SET" { TRACE_RETURN parser_type::make_SET(location()); }
"SMALLINT" { TRACE_RETURN parser_type::make_SMALLINT(location()); }
"SOME" { TRACE_RETURN parser_type::make_SOME(location()); }
"SPECIFIC" { TRACE_RETURN parser_type::make_SPECIFIC(location()); }
"SQL" { TRACE_RETURN parser_type::make_SQL(location()); }
"SQLEXCEPTION" { TRACE_RETURN parser_type::make_SQLEXCEPTION(location()); }
"SQLSTATE" { TRACE_RETURN parser_type::make_SQLSTATE(location()); }
"SQLWARNING" { TRACE_RETURN parser_type::make_SQLWARNING(location()); }
"START" { TRACE_RETURN parser_type::make_START(location()); }
"STATIC" { TRACE_RETURN parser_type::make_STATIC(location()); }
"SYSTEM_USER" { TRACE_RETURN parser_type::make_SYSTEM_USER(location()); }
"TABLE" { TRACE_RETURN parser_type::make_TABLE(location()); }
"TEMPORARY" { TRACE_RETURN parser_type::make_TEMPORARY(location()); }
"THEN" { TRACE_RETURN parser_type::make_THEN(location()); }
"TIME" { TRACE_RETURN parser_type::make_TIME(location()); }
"TIMESTAMP" { TRACE_RETURN parser_type::make_TIMESTAMP(location()); }
"TIMEZONE_HOUR" { TRACE_RETURN parser_type::make_TIMEZONE_HOUR(location()); }
"TIMEZONE_MINUTE" { TRACE_RETURN parser_type::make_TIMEZONE_MINUTE(location()); }
"TO" { TRACE_RETURN parser_type::make_TO(location()); }
"TRAILING" { TRACE_RETURN parser_type::make_TRAILING(location()); }
"TRANSLATION" { TRACE_RETURN parser_type::make_TRANSLATION(location()); }
"TREAT" { TRACE_RETURN parser_type::make_TREAT(location()); }
"TRIGGER" { TRACE_RETURN parser_type::make_TRIGGER(location()); }
"TRUE" { TRACE_RETURN parser_type::make_TRUE(location()); }
"UNION" { TRACE_RETURN parser_type::make_UNION(location()); }
"UNIQUE" { TRACE_RETURN parser_type::make_UNIQUE(location()); }
"UNKNOWN" { TRACE_RETURN parser_type::make_UNKNOWN(location()); }
"UNNEST" { TRACE_RETURN parser_type::make_UNNEST(location()); }
"UPDATE" { TRACE_RETURN parser_type::make_UPDATE(location()); }
"USER" { TRACE_RETURN parser_type::make_USER(location()); }
"USING" { TRACE_RETURN parser_type::make_USING(location()); }
"VALUE" { TRACE_RETURN parser_type::make_VALUE(get_image(driver), location()); }
"VALUES" { TRACE_RETURN parser_type::make_VALUES(location()); }
"VARCHAR" { TRACE_RETURN parser_type::make_VARCHAR(location()); }
"VARYING" { TRACE_RETURN parser_type::make_VARYING(location()); }
"VIEW" { TRACE_RETURN parser_type::make_VIEW(location()); }
"WHEN" { TRACE_RETURN parser_type::make_WHEN(location()); }
"WHENEVER" { TRACE_RETURN parser_type::make_WHENEVER(location()); }
"WHERE" { TRACE_RETURN parser_type::make_WHERE(location()); }
"WITH" { TRACE_RETURN parser_type::make_WITH(location()); }
"WITHOUT" { TRACE_RETURN parser_type::make_WITHOUT(location()); }
"YEAR" { TRACE_RETURN parser_type::make_YEAR(location()); }
"ZONE" { TRACE_RETURN parser_type::make_ZONE(location()); }

    /* SQL-2003 keywords */
"ALWAYS" { TRACE_RETURN parser_type::make_ALWAYS(location()); }
"MAXVALUE" { TRACE_RETURN parser_type::make_MAXVALUE(location()); }
"MINVALUE" { TRACE_RETURN parser_type::make_MINVALUE(location()); }
"INCREMENT" { TRACE_RETURN parser_type::make_INCREMENT(location()); }
"NULLS" { TRACE_RETURN parser_type::make_NULLS(location()); }
"REPLACE" { TRACE_RETURN parser_type::make_REPLACE(location()); }
"OWNED" { TRACE_RETURN parser_type::make_OWNED(location()); }

    /* extra operators */
"<@" { TRACE_RETURN parser_type::make_CONTAINS_OPERATOR(location()); }
"@>" { TRACE_RETURN parser_type::make_IS_CONTAINED_BY_OPERATOR(location()); }
"&&" { TRACE_RETURN parser_type::make_OVERLAPS_OPERATOR(location()); }

    /* extra keywords */
"INCLUDE" { TRACE_RETURN parser_type::make_INCLUDE(location()); }
"IF" { TRACE_RETURN parser_type::make_IF(location()); }
"INDEX" { TRACE_RETURN parser_type::make_INDEX(location()); }

"VARBIT" { TRACE_RETURN parser_type::make_VARBIT(location()); }
"VARBINARY" { TRACE_RETURN parser_type::make_VARBINARY(location()); }

"PLACING" { TRACE_RETURN parser_type::make_PLACING(location()); }
"TINYINT" { TRACE_RETURN parser_type::make_TINYINT(location()); }
"BIGINT" { TRACE_RETURN parser_type::make_BIGINT(location()); }
"BIT_AND" { TRACE_RETURN parser_type::make_BOOL_AND(location()); }
"BIT_OR" { TRACE_RETURN parser_type::make_BOOL_OR(location()); }
"BOOL_AND" { TRACE_RETURN parser_type::make_BOOL_AND(location()); }
"BOOL_OR" { TRACE_RETURN parser_type::make_BOOL_OR(location()); }

{identifier} {
    auto token = get_image(driver);
    if (!driver.check_regular_identifier(token)) {
        TRACE_RETURN parser_type::make_REGULAR_IDENTIFIER_RESTRICTED(location());
    }
    TRACE_RETURN parser_type::make_REGULAR_IDENTIFIER(std::move(token), location());
}

{delimited_identifier} {
    auto token = get_image(driver);
    if (!driver.check_delimited_identifier(token)) {
        TRACE_RETURN parser_type::make_DELIMITED_IDENTIFIER_RESTRICTED(location());
    }
    TRACE_RETURN parser_type::make_DELIMITED_IDENTIFIER(std::move(token), location());
}

{host_parameter_name} {
    TRACE_RETURN parser_type::make_HOST_PARAMETER_NAME(get_image(driver), location());
}

{unsigned_integer} {
    TRACE_RETURN parser_type::make_UNSIGNED_INTEGER(get_image(driver), location());
}

{decimal} {
    TRACE_RETURN parser_type::make_EXACT_NUMERIC_LITERAL(get_image(driver), location());
}

{approximate_numeric_literal} {
    TRACE_RETURN parser_type::make_APPROXIMATE_NUMERIC_LITERAL(get_image(driver), location());
}

{character_string_literal} {
    TRACE_RETURN parser_type::make_CHARACTER_STRING_LITERAL(get_image(driver), location());
}

{bit_string_literal} {
    TRACE_RETURN parser_type::make_BIT_STRING_LITERAL(get_image(driver), location());
}

{hex_string_literal} {
    TRACE_RETURN parser_type::make_HEX_STRING_LITERAL(get_image(driver), location());
}

<*>{
    {UTF8_CHAR} {
        TRACE_RETURN parser_type::make_ERROR(location());
    }
    [\x00-\xff] {
        TRACE_RETURN parser_type::make_ERROR(location());
    }
}

<<EOF>> {
    TRACE_RETURN_EOF parser_type::make_END_OF_FILE(location(true));
}

%%
