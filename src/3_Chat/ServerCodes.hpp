#pragma once

#include <map>
#include <ostream>

enum class ServerCodes : int {
    SUCCESS,
    OK = 0,

    // Network connection codes
    IDENTITY_NOT_CONNECTED = 0xFF,
    IDENTITY_ALREADY_CONNECTED,

    // User related codes
    USER_ALREADY_CONNECTED = 0x1FF,
    USER_NOT_CONNECTED,
    USER_ALREADY_EXIST,
    USER_DOES_NOT_EXIST,
    USER_WRONG_PASSWORD,
    USER_INCORRECT_IDENTITY,
    USER_INCORRECT_TOKEN,

    // Group related codes
    GROUP_ALREADY_EXIST = 0x2FF,
    GROUP_DOES_NOT_EXIST,
    GROUP_MEMBER_ALREADY_EXIST,
    GROUP_MEMBER_DOES_NOT_EXIST
};

static std::map<ServerCodes, std::string> ServerCodesString{
    {ServerCodes::SUCCESS, "SUCCESS"},
    {ServerCodes::OK, "SUCCESS"},
    {ServerCodes::IDENTITY_NOT_CONNECTED, "IDENTITY_NOT_CONNECTED"},
    {ServerCodes::IDENTITY_ALREADY_CONNECTED, "IDENTITY_ALREADY_CONNECTED"},
    {ServerCodes::USER_ALREADY_CONNECTED, "USER_ALREADY_CONNECTED"},
    {ServerCodes::USER_NOT_CONNECTED, "USER_NOT_CONNECTED"},
    {ServerCodes::USER_ALREADY_EXIST, "USER_ALREADY_EXIST"},
    {ServerCodes::USER_DOES_NOT_EXIST, "USER_DOES_NOT_EXIST"},
    {ServerCodes::USER_WRONG_PASSWORD, "USER_WRONG_PASSWORD"},
    {ServerCodes::USER_INCORRECT_IDENTITY, "USER_INCORRECT_IDENTITY"},
    {ServerCodes::USER_INCORRECT_TOKEN, "USER_INCORRECT_TOKEN"},
    {ServerCodes::GROUP_ALREADY_EXIST, "GROUP_ALREADY_EXIST"},
    {ServerCodes::GROUP_DOES_NOT_EXIST, "GROUP_DOES_NOT_EXIST"},
    {ServerCodes::GROUP_MEMBER_ALREADY_EXIST, "GROUP_MEMBER_ALREADY_EXIST"},
    {ServerCodes::GROUP_MEMBER_DOES_NOT_EXIST, "GROUP_MEMBER_DOES_NOT_EXIST"}};

std::ostream& operator<<(std::ostream& o, ServerCodes code) {
    return o << ServerCodesString[code];
}
