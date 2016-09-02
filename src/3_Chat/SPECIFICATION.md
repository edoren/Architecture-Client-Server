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
    - data: arguments sent to the request

### Responses

    Are messages sent from the server depending of the last request received
    from the client.
    
    +------------+--------+======+
    | "response" | status | data |
    +------------+--------+======+
    
    - status: boolean, it defines if the response is successful or not.
    - data: when status is false the data is set to a error message, otherwise
            is are the response arguments
    
    Note: All the request from the clients has its respective response, if is
          not defined bellow is assumed to be only the status with no data.

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
    
    +------------+------+----------+-------+
    | "response" | TRUE | username | token |
    +------------+------+----------+-------+
    
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
    
    contact: the username of the user to add to the contact list.

### Whisper (Chat Unicast)

**Request**

    +-----------+----------+-------+-----------+---------+
    | "whisper" | username | token | recipient | content |
    +-----------+----------+-------+-----------+---------+

    recipient: the username of the destination user

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
