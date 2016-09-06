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

Messages sent from the client to the server, all the request have the same
format-

    +-----------+--------+======+
    | "request" | action | data |
    +-----------+--------+======+

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

Messages sent from the client when something has changed that affect the
client. (E.g. An incomming message from an user)

    +----------+--------+======+
    | "update" | action | data |
    +----------+--------+======+

- action: string defining the type of request the update is linked to
- data: arguments of the update.


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

    +-----------+------------+----------+----------+
    | "request" | "register" | username | password |
    +-----------+------------+----------+----------+


### Login

**Request**

    +-----------+---------+----------+----------+
    | "request" | "login" | username | password |
    +-----------+---------+----------+----------+

**Response**

If sucessfull the response is set to:

    +------------+---------+---+----------+-------+
    | "response" | "login" | 0 | username | token |
    +------------+---------+---+----------+-------+

- token: string defining the UUID of the user, that is used to authenticate
         the other requests


### Logout

**Request**

    +-----------+----------+----------+
    | "request" | "logout" | username |
    +-----------+----------+----------+


### Add Contact

**Request**

    +---------------+----------+-------+---------+
    | "add_contact" | username | token | contact |
    +---------------+----------+-------+---------+

- contact: the username of the user to add to the contact list.


### Whisper (Chat Unicast)

**Request**

    +-----------+-----------+----------+-------+-----------+---------+
    | "request" | "whisper" | username | token | recipient | content |
    +-----------+-----------+----------+-------+-----------+---------+

- recipient: the username of the destination user

**Update**

    +----------+-----------+--------+---------+
    | "update" | "whisper" | sender | content |
    +----------+-----------+--------+---------+


### Create Group

**Request**

    +-----------+----------------+----------+-------+------------+
    | "request" | "create_group" | username | token | group_name |
    +-----------+----------------+----------+-------+------------+


### Join Group

**Request**

    +--------------+----------+-------+------------+
    | "join_group" | username | token | group_name |
    +--------------+----------+-------+------------+


### Message Group (Chat Multicast)

**Request**

    +-----------+-------------+----------+-------+------------+---------+
    | "request" | "msg_group" | username | token | group_name | content |
    +-----------+-------------+----------+-------+------------+---------+

**Update**

    +----------+-------------+------------+--------+---------+
    | "update" | "msg_group" | group_name | sender | content |
    +----------+-------------+------------+--------+---------+


### Voice Message (Chat Multicast)

**Request**

    +-----------+-------------+----------+-------+-----------+----------+-------------+---------+
    | "request" | "voice_msg" | username | token | recipient | channels | sample_rate | samples |
    +-----------+-------------+----------+-------+-----------+----------+-------------+---------+

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


### Join Call

**Request**

Request the server to join the group call

    +-----------+-------------+----------+-------+------------+
    | "request" | "join_call" | username | token | group_name |
    +-----------+-------------+----------+-------+------------+

- group_name: the name of the group to join a call

**Response**

Tell the user that the connexion was established.

    +------------+-------------+---+------------+
    | "response" | "join_call" | 0 | group_name |
    +------------+-------------+---+------------+

- group_name: the name of the group to join a call

**Update**

**Client**

If the conexion was established this is the recorded audio data from the user
to the server

    +----------+-------------+----------+-------+------------+---------+
    | "update" | "call_data" | username | token | group_name | samples |
    +----------+-------------+----------+-------+------------+---------+

- group_name: the name of the group to send the audio samples.
- samples: a list containing the audio samples.

**Server**

If the conexion was established this is the audio data from the server to the
user

    +----------+-------------+--------+---------+
    | "update" | "call_data" | sender | samples |
    +----------+-------------+--------+---------+

- sender: the username who send the call notification
- samples: a list containing the audio samples

### Leave Call

**Request**

Leave the actual call

    +-----------+--------------+----------+-------+
    | "request" | "leave_call" | username | token |
    +-----------+--------------+----------+-------+

- recipient: the username of the destination user
- samples: a list containing the audio samples
