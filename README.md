# COP3530 Project 3

## Getting Started

### Installing `just`

This project has a `justfile`, which just provides some shorthands for running common commands. You don't _need_ to use `just`, but it's pretty convenient.

There are a bunch of ways to install `just`, depending on whatever package manager you wish to use.

Any of these will work:

```bash
cargo install just
brew install just
npm install -g rust-just
apt install just
```

### Setup commands

There are two commands to setup the project, these only need to be run once.

```bash
just setup-backend
```

```bash
just setup-frontend
```

## Running the Project

To run the backend server, run `just backend`.
To run backend tests, run `just test`.

To run the frontend, run `just frontend`.
The frontend will be served on [http://localhost:3000](http://localhost:3000).

To run both, run `just`.
