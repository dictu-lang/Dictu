# Web Framework
This is a proof of concept web framework heavily inspired from [https://expressjs.com/](https://expressjs.com/).

**Note:** This is NOT production ready.

## Features
This project is currently setup to have 4 routes. `/`, `/login`, `/logout` and `/home`.
- `/` - GET
    - This route is the login page for the system, it has an incredibly basic login form located in static/views/index.html.
- `/login` - POST
    - This is the route that handles logging a user in. Users are logged in by their session containing a "logged_in" flag, if this is false or not present, they will not be logged in.
- `/logout` - GET
    - This sets the "logged_in" session flag to false.
- `/home` - GET
    - This is a protected route that can only be accessed when the "logged_in" flag is present within the users session.

### Technical details
The users session is stored within HTTP Only a cookie within the users browser and because of this, they must be signed. They are signed with [HMAC-SHA256](https://en.wikipedia.org/wiki/HMAC) and verified on every request to ensure there has been no tampering.

Users themselves are stored within a SQLite database (more in the next section) in a very basic username | password table setup. The users password are hashed using bcrypt.

## Running
To get up and running we must first create the SQLite database and seed a user, to do so run `setup.du`

```
$ dictu setup.du
```

This will ask you where the SQLite database should be stored, for this project enter `users.db` along with a username and password of your choice.

Afterwards start the server

```
$ dictu run.du
```