---
layout: default
title: Datetime
nav_order: 4
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

```cs
import Datetime;
```

### Constants

| Constant                   | Description                        |
| -------------------------- | ---------------------------------- |
| Datetime.SECONDS_IN_MINUTE | The number of seconds in a minute. |
| Datetime.SECONDS_IN_HOUR   | The number of seconds in an hour.  |
| Datetime.SECONDS_IN_DAY    | The number of seconds in a day.    |
| Datetime.SECONDS_IN_WEEK   | The number of seconds in a week.   |


### Datetime.new() -> Datetime

Returns a datetime object with current datetime.

```cs
var datetime = Datetime.new(); 
datetime.strftime(); // Fri May 29 03:12:32 2020
```

### Datetime.new(Number) -> Datetime

Returns a datetime object for given number of seconds from epoch.

```cs
const datetime = Datetime.new(1577836800);
datetime.strftime(); // Wed Jan  1 00:00:00 2020
```


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


### Datetime.strptime(String, String) -> Datetime

Returns a datetime object for given time string format.

**Note:** This is not available on windows systems.

```cs
var datetime = Datetime.strptime("%Y-%m-%d %H:%M:%S", "2020-01-01 00:00:00"); 
datetime.strftime(); // Wed Jan  1 00:00:00 2020
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


### datetimeObj.getTime() -> Number

Returns a number which is the number of seconds from epoch, for a given datetime

**Note:** This is not available on windows systems.

```cs
const datetime = Datetime.strptime("%Y-%m-%d %H:%M:%S", "2020-01-01 00:00:00");
datetime.getTime(); // 1577836800
```

### datetimeObj.strftime(String -> Optional) -> String

Returns a user-defined datetime formatted string for a datetime object, see [Datetime formats](#datetime-formats).

```cs
const datetime = Datetime.new();
datetime.strftime("Today is %A"); // Today is Friday
```

Returns in default format "%a %b %d %H:%M:%S %Y", when user defined format is not provided 


```cs
const datetime = Datetime.new();
datetime.strftime(); // Sat Oct 21 21:44:25 2023
```


## Duration

A Duration object represents a time delta, the difference between two datetimes. This Duration object can be used to create new datetime objects, by performing operations such as addition and subtraction.

### Datetime.duration(Dict) -> Duration

Duration object is constructed by passing dictionary duration properties. weeks, days, hours, minutes and seconds are allowed properties.

```cs
import Datetime;
const duration = Datetime.duration({'weeks':0, 'days': 1, 'seconds': 0, 'minutes': 0, 'hours': 0});
```

### datetimeObj.add(Duration) -> Datetime

Returns a new Datetime object after adding given duration.

```cs
const duration = Datetime.duration({'weeks':0, 'days': 1, 'seconds': 0, 'minutes': 0, 'hours': 0});
const datetime = Datetime.strptime("%Y-%m-%d %H:%M:%S", "2020-01-01 00:00:00");
datetime.add(duration).strftime(); // Thu Jan 02 00:00:00 2020
```


### datetimeObj.sub(Duration) -> Datetime

Returns a new Datetime object after subtracting given duration.

```cs
const duration = Datetime.duration({'weeks':53});
const datetime = Datetime.strptime("%Y-%m-%d %H:%M:%S", "2020-01-01 00:00:00");
datetime.sub(duration).strftime(); // Wed Jan 02 00:00:00 2019
```

### datetimeObj.delta(datetimeObj) -> Duration

Returns a Duration object represents a delta with a given date.

```cs
var datetimeOne = Datetime.strptime("%Y-%m-%d %H:%M:%S", "2020-01-01 00:00:00");
var datetimeTwo = Datetime.strptime("%Y-%m-%d %H:%M:%S", "2020-01-02 00:00:00");
var duration = datetimeOne.delta(datetimeTwo); 

const datetime = Datetime.strptime("%Y-%m-%d %H:%M:%S", "2020-05-01 00:00:00");
datetime.add(duration).strftime(); // Sat May 02 00:00:00 2020
```
