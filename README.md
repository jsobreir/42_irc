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
