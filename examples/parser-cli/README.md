# mizugaki-parser-cli

Parses SQL program and prints its syntactic structure as JSON format.

```sh
mizugaki-parser-cli [options] [--] sql-program
```

Please take care that each SQL statement must end with `;`, like `"SELECT * FROM T0;"`.

Available options:

* `-quiet`
  * only show erroneous messages
* `-repeat N`
  * repeat N times
  * default: `1`
* `-debug N`
  * parser debug level
  * default: `0`
* `-help`
  * print help messages

## How to install

Please build this project with `-DINSTALL_EXAMPLES=ON` option.

## Examples

```sh
# parse a statement
mizugaki-parser-cli "SELECT * FROM T0;"

# parse multiple statements
mizugaki-parser-cli -- '
-- insert and select
INSERT INTO T0 (C0, C1) VALUES (1, 2), (3, 4);
SELECT * FROM T0;
'

# parse a statement and pretty print by https://stedolan.github.io/jq/
mizugaki-parser-cli "TABLE a;" | jq .

# parse a wrong statement
mizugaki-parser-cli "INSERT INTO T1 VALUES 1;"

# parse a statement 10,000 times without printing
mizugaki-parser-cli -repeat 10000 -quiet "SELECT * FROM T0;"

# parse with parser tracing
mizugaki-parser-cli -debug 1 "SELECT * FROM T0;"
```
