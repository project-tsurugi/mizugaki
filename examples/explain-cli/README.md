# mizugaki-explain-cli

Compiles SQL program and prints its execution plan as JSON format.

```sh
mizugaki-explain-cli [options]
```

This only compiles the SQL programs but does not execute them, so DDLs like "CREATE TABLE" does not change the database schema.
The available symbols are [listed here](#available-symbols).

Available options:

* `-text <source text>`
  * compile the given text
* `-file <source file>`
  * compile the content of the given file
* `-echo`
  * print the processing SQL snippet
* `-quiet`
  * don't print the execution plan
* `-help`
  * print help messages

## Available symbols

* tables
  * `ksv`
    * `k` (BIGINT, PRIMARY KEY)
    * `s` (DECIAL(18, 2), DEFAULT is `0.00`)
    * `v` (VARCHAR(*), DEFAULT is `''`)
* indexes
  * `ksv_s` (index of `k.s`)
* scalar functions
  * not available
* aggregate functions
  * not available

## How to install

Please build this project with `-DINSTALL_EXAMPLES=ON` option, then `cmake --build . --target install` will place `<install-prefix>/bin/mizugaki-explain-cli`.

## Examples

```sh
# compile a statement
./mizugaki-explain-cli -echo -text "SELECT * FROM ksv;"

# compile statements in file
./mizugaki-explain-cli -echo -file "example.sql"

# compile a statement and pretty print by https://stedolan.github.io/jq/
./mizugaki-explain-cli -text "TABLE ksv;" | jq .

# compile a wrong statement
./mizugaki-explain-cli -text "INSERT INTO ksv VALUES 1;"

# compile a statement with wrong symbols
./mizugaki-explain-cli -text "INSERT INTO invalid VALUES (1, 1.00, 'XXX');"
```
