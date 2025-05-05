# 42_irc
<p style="text-align:center;">This project is about creating your own IRC server.</p>

You will use an actual IRC client to connect to your server and test it.
The Internet is governed by solid standard protocols that allow connected computers to
interact with each other.
It’s always beneficial to understand these protocols.

## Documentation
## Project Goals and Requirements
The goal is to develop an IRC server using only C++98 standard. The goal is not to develop an IRC client or implementing server-to-server communication.

### Requirements
- The server must handle multiple clients simultaneously;
- No use of forking, and all I/O operations must be non-blocking;
- Only one poll() (or equivalent);
- A client at our choice will be used to connect to the server via TCP/IP (v4 or v6);
- Shall be able to authenticate, set a nickname, a username, join a channel,
send and receive private messages using your reference client;
- All the messages sent from one client to a channel shall be forwarded to
every other client that joined the channel;
- Shall distinguish between regular users and operators;
- Shall implement the following commands:
∗ KICK - Eject a client from the channel
∗ INVITE - Invite a client to a channel
∗ TOPIC - Change or view the channel topic
∗ MODE - Change the channel’s mode:
· i: Set/remove Invite-only channel
· t: Set/remove the restrictions of the TOPIC command to channel
operators
· k: Set/remove the channel key (password)
5
ft_irc Internet Relay Chat
· o: Give/take channel operator privilege
· l: Set/remove the user limit to channel
## Task List
# IRC Project - 42

## 1. Understand the Project Requirements

- Read the subject thoroughly.
- Clarify mandatory, bonus, and forbidden functions/libraries.
- Determine the protocol version (RFC 1459 or 2812, usually).
- Understand the required commands (`PASS`, `NICK`, `USER`, `JOIN`, `PRIVMSG`, etc.).

## 2. Setup the Project Structure

- Create a Git repository.
- Setup the `Makefile`.
- Plan the file structure (`main.cpp`, `Server.hpp/cpp`, `Client.hpp/cpp`, etc.).

## 3. Basic Server Setup

- Use `socket()`, `bind()`, `listen()`, and `accept()` to create the TCP server.
- Use `poll()` or `select()` to handle multiple client connections.


## 4. Handle New Connections

- Accept new client connections.
- Store client sockets and their info in a data structure (e.g. `std::map<int, Client>`).
- Handle client disconnection properly.

## 5. Parse Incoming Data

- Read from client sockets.
- Buffer partial messages.
- Parse complete messages when receiving `\r\n`.

## 6. Implement Command Processing

Implement the following commands:

- `PASS`, `NICK`, `USER`: authentication/login.
- `PING` / `PONG`: keepalive.
- `JOIN`, `PART`, `PRIVMSG`, `NOTICE`, `QUIT`: chat-related commands.
- `MODE`, `TOPIC`, `KICK`, `INVITE`, etc. (if required or as bonus).

## 7. Channel Management

- Implement a `Channel` class.
- Manage users, operators, and channel-specific settings.
- Track which clients are in which channels.
  
## 8. Authentication & Registration

- Ensure clients can only interact after completing the login sequence (`PASS`, `NICK`, and `USER`).


## 9. Test Thoroughly

- Use tools like `nc`, `telnet`, or IRC clients (e.g., HexChat, irssi).
- Test handling of malformed messages and out-of-order commands.
- Simulate multiple clients and edge cases.
  
## Installing and Using
To download, clone the repository into your device and navigate inside using `cd push_swap`, then typing  `make` to compile all the functions and generate the executable push_swap, which you can run as follows:

```
./ircserv <port> <password>
```
where,
- port: The port number on which your IRC server will be listening for incoming
IRC connections.
- password: The connection password. It will be needed by any IRC client that tries
to connect to your server.

## Project Implementation
### Parsing
