# mizugaki-explain-cli

Compiles SQL program and prints its execution plan as JSON format.

```sh
mizugaki-explain-cli [options]
```

This only compiles the SQL programs and then interprets only DDL statements, such as `CREATE TABLE`, `CREATE INDEX`, `DROP TABLE`, and `DROP INDEX`.
The other DML statements, such as `SELECT` and `INSERT`, are only show their execution plan.
Note that, the defined tables and indexes are only available in the same execution, so that you can only access them in the trailing statements like `"CREATE TABLE t (...); SELECT * FROM t"`.

The pre-defined symbols are [listed here](#pre-defined-symbols).

Available options:

* `-text <source text>`
  * compile the given text
* `-file <source file>`
  * compile the content of the given file
* `-placeholders <placeholder definitions>`
  * define placeholders for the SQL statements
  * format: `<name1>=<type1>,<name2>=<type2>,...`
  * available types:
    * `int`
    * `bigint`
    * `decimal`
    * `varchar`
* `-echo`
  * print the processing SQL snippet
* `-quiet`
  * don't print the execution plan
* `-help`
  * print help messages

## Pre-defined symbols

* tables
  * `ksv`
    * `k` (BIGINT, PRIMARY KEY)
    * `s` (DECIAL(18, 2), DEFAULT is `0.00`)
    * `v` (VARCHAR(*), DEFAULT is `''`)
* indexes
  * `ksv_s` (index of `k.s`)
* scalar functions
  * `OCTET_LENGTH(T)`
    * `VARCHAR(*)`
    * `VARBINARY(*)`
    * `BLOB`
    * `CLOB`
* aggregate functions
  * `COUNT(*)`

## How to install

Please build this project with `-DINSTALL_EXAMPLES=ON` option, then `cmake --build . --target install` will place `<install-prefix>/bin/mizugaki-explain-cli`.

## Examples

```sh
# compile a statement
./mizugaki-explain-cli -echo -text "SELECT * FROM ksv;"

# compile statements in file
./mizugaki-explain-cli -echo -file "example.sql"

# compile a DDL and then access to the defined table
./mizugaki-explain-cli -echo -text "CREATE TABLE other (k BIGINT PRIMARY KEY); SELECT * FROM ksv JOIN other ON ksv.k = other.k;"

# compile a statement and pretty print by https://stedolan.github.io/jq/
./mizugaki-explain-cli -text "TABLE ksv;" | jq .

# compile a wrong statement
./mizugaki-explain-cli -text "INSERT INTO ksv VALUES 1;"

# compile a statement with wrong symbols
./mizugaki-explain-cli -text "INSERT INTO invalid VALUES (1, 1.00, 'XXX');"
```
