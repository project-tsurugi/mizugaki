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
* `-features=<runtime feature specification>`
  * configure the runtime features of the compiler
  * format: `[all|none,]<+|-><feature name>,<+|-><feature name>,...`
    * `all` starts from the recommended features (`yugawara::runtime_feature_all`), and it is the default
    * `none` starts from no features
    * `+<feature name>` enables the feature, and `-<feature name>` disables it
  * available feature names are the names of `yugawara::runtime_feature` (e.g. `index_join`), and an unknown name shows the list of them
  * please use the `-features=...` form, because the value may start with `-`
  * note that, the runtime features may differ from the ones of the actual database
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
  * `ksv_s` (index of `ksv.s`)

The index definitions, including the ones created by `CREATE TABLE` or `CREATE INDEX`, follow the ones in jogasaki:
the primary index has all non-key columns as its values, and the secondary index has no values.
On the other hand, this never generates a hidden primary key column for the table without primary key.
* scalar functions
  * `OCTET_LENGTH(T)`
    * `VARCHAR(*)`
    * `VARBINARY(*)`
    * `BLOB`
    * `CLOB`
* aggregate functions
  * `COUNT(*)`
* table-valued functions
  * `tvf`
    * parameters:
      * `a: BIGINT`
    * return columns:
      * `u: BIGINT`
      * `v: VARCHAR(*)`
      * `w: DECIMAL(*, *)`

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

# compile a statement without index join
./mizugaki-explain-cli -features=-index_join -text "SELECT * FROM ksv JOIN ksv AS other ON ksv.k = other.k;"

# compile a statement only with aggregate exchange
./mizugaki-explain-cli -features=none,+aggregate_exchange -text "SELECT k, COUNT(*) FROM ksv GROUP BY k;"

# compile a wrong statement
./mizugaki-explain-cli -text "INSERT INTO ksv VALUES 1;"

# compile a statement with wrong symbols
./mizugaki-explain-cli -text "INSERT INTO invalid VALUES (1, 1.00, 'XXX');"
```
