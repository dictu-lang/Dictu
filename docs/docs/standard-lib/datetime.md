---
layout: default
title: Datetime
nav_order: 5
parent: Standard Library
---

# Datetime
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Datetime

To make use of the Datetime module an import is required.

```js
import Datetime;
```

### Constants

| Constant                   | Description                        |
| -------------------------- | ---------------------------------- |
| Datetime.SECONDS_IN_MINUTE | The number of seconds in a minute. |
| Datetime.SECONDS_IN_HOUR   | The number of seconds in an hour.  |
| Datetime.SECONDS_IN_DAY    | The number of seconds in a day.    |
| Datetime.SECONDS_IN_WEEK   | The number of seconds in a week.   |

### Datetime.now() -> String

Returns a human readable locale datetime string.

```cs
Datetime.now(); // Fri May 29 03:12:32 2020
```

### Datetime.nowUTC() -> String

Returns a human readable UTC datetime string.

```cs
Datetime.now(); // Fri May 29 02:12:32 2020
```

### Datetime formats

| Directive | Description                                                                        | Example                  |
| --------- | ---------------------------------------------------------------------------------- | ------------------------ |
| %a        | Abbreviated weekday name                                                           | Mon, Tue, ...            |
| %A        | Full weekday name                                                                  | Monday, Tuesday, ...     |
| %b        | Abbreviated month name                                                             | Jan, Feb, ...            |
| %B        | Full month name                                                                    | January, February, ...   |
| %c        | Locale datetime format                                                             | Fri May 29 03:18:03 2020 |
| %C        | The century number (year/100) as a 2-digit integer                                 | 20                       |
| %d        | The day of the month as a decimal number                                           | 01, 02, ...              |
| %D        | Equivalent to %m/%d/%y                                                             | 05/29/20                 |
| %e        | Like %d, but the leading zero is replaced by a space                               | Monday, Tuesday, ...     |
| %F        | Equivalent to %Y-%m-%d                                                             | 2020-05-29               |
| %H        | The hour as a decimal number using a 24-hour clock (00-23)                         | 00, 01, 02, ...          |
| %I        | The hour as a decimal number using a 12-hour clock (01-12)                         | 00, 01, 02, ...          |
| %j        | The day of the year as a decimal number (001-366)                                  | 001, 002, ...            |
| %m        | The month as a decimal number (01-12)                                              | 01, 02, ...              |
| %M        | The minute as a decimal number (00-59)                                             | 00, 01, 02, ...          |
| %n        | A newline character                                                                |                          |
| %p        | Either AM or PM                                                                    |                          |
| %P        | Either am or pm                                                                    |                          |
| %s        | The number of seconds since the Epoch                                              | 1590719271               |
| %S        | The second as a decimal number (range 00 to 60)                                    | 00, 01, 02, ...          |
| %t        | A tab character                                                                    |                          |
| %u        | The day of the week as a decimal (range 1 to 7)                                    | 1, 2, 3                  |
| %y        | The year as a decimal number without a century (range 00 to 99)                    | 00, 01, 02, ...          |
| %Y        | The year as a decimal number including the century.                                | 2020, 2021, 2022, ...    |
| %z        | The +hhmm or -hhmm numeric timezone (that is, the hour and minute offset from UTC) | +0100, +0200, ...        |
| %Z        | The timezone name or abbreviation                                                  | BST                      |
| %%        | A literal %                                                                        |                          |


### Datetime.strftime(String, Number: time -> Optional) -> String

Returns a user-defined datetime formatted string, see [Datetime formats](#datetime-formats). `strftime` also takes an optional argument
which is a UNIX timestamp, so the date is formatted from the given timestamp rather than
the current point in time.

```cs
Datetime.strftime("Today is %A"); // Today is Friday

var time = System.time();
Datetime.strftime("Some point in time %H:%M:%S", time);
```

### Datetime.strptime(String, String) -> Number

Returns a number which is the number of seconds from epoch. `strptime` expects two parameters
the first parameter being the date format, see [Datetime formats](#datetime-formats) and the second
the date string in the format of the first parameter.

**Note:** This is not available on windows systems.

```cs
Datetime.strptime("%Y-%m-%d %H:%M:%S", "2020-01-01 00:00:00"); // 1577836800
```

