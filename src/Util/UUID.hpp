#pragma once

#include <random>
#include <string>

class UUID {
private:
    UUID() = default;

public:
    static UUID UUID4() {
        // Generate UUID version 4 defined by RFC 4122 Section 4.4.
        static std::random_device rd;
        static std::mt19937 rng(rd());
        static std::uniform_int_distribution<uint32_t> dist;

        UUID output;

        uint32_t* uint32_uuid = reinterpret_cast<uint32_t*>(output.uuid_);
        for (size_t i = 0; i < 4; i++) {
            uint32_uuid[i] = dist(rng);
        }

        output.uuid_[6] = (output.uuid_[6] & 0x0f) | 0x40;
        output.uuid_[8] = (output.uuid_[8] & 0x3f) | 0x80;

        return output;
    }

    std::string AsString() {
        static const char hexdigits[16] = {'0', '1', '2', '3', '4', '5',
                                           '6', '7', '8', '9', 'a', 'b',
                                           'c', 'd', 'e', 'f'};

        std::string output;
        output.reserve(36);

        for (int i = 0; i < 16; i++) {
            if ((i == 4) | (i == 6) | (i == 8) | (i == 10))
                output.push_back('-');
            output.push_back(hexdigits[(uuid_[i] & 0xf0) >> 4]);
            output.push_back(hexdigits[uuid_[i] & 0x0f]);
        }

        return output;
    }

private:
    uint8_t uuid_[16];
};
