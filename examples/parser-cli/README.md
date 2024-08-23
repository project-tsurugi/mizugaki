# mizugaki-parser-cli

Parses SQL program and prints its syntactic structure as JSON format.

```sh
mizugaki-parser-cli [options]
```

Please take care that each SQL statement must end with `;`, like `"SELECT * FROM T0;"`.

Available options:

* `-text <source text>`
  * parses the given text
* `-file <source file>`
  * parses the content of the given file
* `-quiet`
  * only show erroneous messages
* `-repeat N`
  * repeat N times
  * default: `1`
* `-debug N`
  * parser debug level (requires `-DCMAKE_BUILD_TYPE=Debug`)
  * default: `0`
* `-node_limit`
  * limits the number of nodes in the AST
  * default: `10,000`
* `-depth_limit`
  * limits the max depth in the AST
  * default: `5,000`
* `-stats`
  * print AST statistics
* `-help`
  * print help messages

## How to install

Please build this project with `-DINSTALL_EXAMPLES=ON` option, then `cmake --build . --target install` will place `<install-prefix>/bin/mizugaki-parser-cli`.

## Examples

```sh
# parse a statement
./mizugaki-parser-cli -text "SELECT * FROM T0;"

# parse multiple statements
./mizugaki-parser-cli -text -- '
-- insert and select
INSERT INTO T0 (C0, C1) VALUES (1, 2), (3, 4);
SELECT * FROM T0;
'

# parse statements in file
./mizugaki-parser-cli -file "example.sql"

# parse a statement and pretty print by https://stedolan.github.io/jq/
./mizugaki-parser-cli -text "TABLE a;" | jq .

# parse a wrong statement
./mizugaki-parser-cli -text "INSERT INTO T1 VALUES 1;"

# parse a statement 10,000 times without printing
./mizugaki-parser-cli -repeat 10000 -quiet -text "SELECT * FROM T0;"

# parse with tracing (require -DCMAKE_BUILD_TYPE=Debug)
./mizugaki-parser-cli -debug 1 -text "SELECT * FROM T0;"
```
