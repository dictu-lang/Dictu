---
layout: default
title: Sqlite
nav_order: 19
parent: Standard Library
---

# Sqlite
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Sqlite
To make use of the Sqlite module an import is required.

Note: Unlike SQLite and most other libraries, foreign keys **are** enabled by default when a connection is opened.

```js
import Sqlite;
```

### Sqlite.connect(String: database, timeout: number -> optional) -> Result\<SQLite>

This opens a connection to a SQLite database. Returns a Result type and on success wraps an abstract SQLite type.

The second argument to connect is the amount of time it will sleep (total) in milliseconds if the database is currently locked. If 
not set it will default to 5 seconds.

Note: You can pass ":memory:" to open the SQLite database in memory rather than a file.

```cs
Sqlite.connect(":memory:").unwrap();
Sqlite.connect("my/database/file.db").unwrap();
```

### sqlite.execute(String: query, List: arguments -> Optional) -> Result\<Nil>

The `execute` method is ran on the abstract that is returned from `.connect` rather than the `Sqlite` module, hence the
lower case `sqlite`. The `execute` method executes an SQL query and can return one of 3 values.

A Result type is returned, the value being wrapped depends on the query:
  If the query is a type that does not return data (such as UPDATE / DELETE), then `nil` is wrapped on successful queries.
  If the query is a SELECT statement then a list is wrapped on successful queries (empty list for a select query that returns no rows).

```cs
sqlite.execute("UPDATE mytable SET mycolumn = 10"); // <Result Suc>
var query = sqlite.execute("UPDATE unknown_table SET mycolumn = 10"); // <Result Err>
print(query.unwrapError()); // no such table: unknown_table
```

#### Arguments
Passing user input to a query should always be done through the method of binding parameters and never through string interpolation, 
otherwise your application will be at risk to SQL injection. Binding values to placeholders is incredibly simple.

```cs
sqlite.execute("SELECT * FROM mytable WHERE mycolumn = ?", [
    "test"
]);
```

The `?` will take the value of the item in the list. If there are multiple placeholders then it is done by index, for example.

```cs
sqlite.execute("SELECT * FROM mytable WHERE mycolumn = ? AND mycolumn1 = ?", [
    "test",
    "next value"
]);
```
The first `?` matches with the first value in the list, and the second `?` with the second value in the list, and so on.

### sqlite.close()

Closes the database.

```cs
sqlite.close();
```
