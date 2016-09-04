# Introduction
This document defines the way the data is sent between the server and the
clients.
All the data is encoded using MessagePack which specification is defined in
this document: https://github.com/msgpack/msgpack/blob/master/spec.md

## Format

### Notation

One object

    +--------+  
    |        |  
    +--------+  

Variable number of objects

    +=================+
    |                 |
    +=================+


### Requests

All the request have the same format

    +--------+======+
    | action | data |
    +--------+======+

- action: string defining the type of request
- data: arguments sent to the request, see each message type for reference.


### Responses

Are messages sent from the server depending of the last request received
from the client.

    +------------+--------+------+======+
    | "response" | action | code | data |
    +------------+--------+------+======+

- action: string defining the type of request the request is linked to
- code: integer that defines the return code of the request if is not 0
        the request failed, see ServerCodes.hpp for more reference.
- data: when successful (code equals 0) this is set to the request data, see
        each message type for reference.

Note: All the request from the clients has its respective response.


### Updates

Messages sent from the server when something has changed that affect the
client. (E.g. An incomming message from an user)

    +----------+--------+======+
    | "update" | action | data |
    +----------+--------+======+

- action: string defining the type of request the update is linked to
- data: arguments of the update.


## Message Types


### Register

**Request**

    +------------+----------+----------+
    | "register" | username | password |
    +------------+----------+----------+


### Login

**Request**

    +---------+----------+----------+
    | "login" | username | password |
    +---------+----------+----------+

**Response**

If sucessfull the response is set to:

    +------------+---------+---+----------+-------+
    | "response" | "login" | 0 | username | token |
    +------------+---------+---+----------+-------+

- token: string defining the UUID of the user, that is used to authenticate
         the other requests


### Logout

**Request**

    +----------+----------+
    | "logout" | username |
    +----------+----------+


### Add Contact

**Request**

    +---------------+----------+-------+---------+
    | "add_contact" | username | token | contact |
    +---------------+----------+-------+---------+

- contact: the username of the user to add to the contact list.


### Whisper (Chat Unicast)

**Request**

    +-----------+----------+-------+-----------+---------+
    | "whisper" | username | token | recipient | content |
    +-----------+----------+-------+-----------+---------+

- recipient: the username of the destination user

**Update**

    +----------+-----------+--------+---------+
    | "update" | "whisper" | sender | content |
    +----------+-----------+--------+---------+


### Create Group

**Request**

    +----------------+----------+-------+------------+
    | "create_group" | username | token | group_name |
    +----------------+----------+-------+------------+


### Join Group

**Request**

    +--------------+----------+-------+------------+
    | "join_group" | username | token | group_name |
    +--------------+----------+-------+------------+


### Message Group (Chat Multicast)

**Request**

    +-------------+----------+-------+------------+---------+
    | "msg_group" | username | token | group_name | content |
    +-------------+----------+-------+------------+---------+

**Update**

    +----------+-------------+------------+--------+---------+
    | "update" | "msg_group" | group_name | sender | content |
    +----------+-------------+------------+--------+---------+


### Voice Message (Chat Multicast)

**Request**

    +-------------+----------+-------+-----------+----------+-------------+---------+
    | "voice_msg" | username | token | recipient | channels | sample_rate | samples |
    +-------------+----------+-------+-----------+----------+-------------+---------+

- recipient: the username of the destination user
- channels: the number of channels the audio is recorded
- sample_rate: the audio sample rate
- samples: a list containing the audio samples

**Update**

    +----------+-------------+--------+----------+-------------+---------+
    | "update" | "voice_msg" | sender | channels | sample_rate | samples |
    +----------+-------------+--------+----------+-------------+---------+

- sender: the username who send the voice message
- channels: the number of channels the audio is recorded
- sample_rate: the audio sample rate
- samples: a list containing the audio samples
