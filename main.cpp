#include <iostream>
#include <string>
#include <map>
#include <fstream>

#define MPTS_PACKET_SIZE 188
#define MPTS_SYNC_BYTE 0x47

using namespace std;

map<uint16_t, ofstream *> outputFiles;

void parseHeader(const uint8_t *&packet, uint16_t &id, bool &startIndicator) {
    id = uint16_t((packet[0] & 0x1F) << 8) | uint16_t(packet[1]);
    int shifted = (packet[2] & 0x30) >> 4;
    startIndicator = static_cast<bool> (packet[0] & 0x40);
    packet += 3;
    if ((shifted == 0x2) || (shifted == 0x3)) {
        packet += packet[0] + 1;
    }
}

bool writeChunk(const uint8_t *&packet, const uint8_t *endPointer, uint16_t &id, bool &startIndicator,
                map<uint16_t, ofstream *>::iterator foundOutput) {
    ofstream *output;
    if (startIndicator) {
        if (packet[0] != 0x0 || packet[1] != 0x0 || packet[2] != 0x01) {
            return false;
        }
        if (foundOutput == outputFiles.end()) {
            output = new ofstream("stream_" + to_string(id), ios::out | ios::binary);
            outputFiles[id] = output;
        } else {
            output = foundOutput->second;
        }
        packet += 8;
        packet += packet[0] + 1;
        if (packet >= endPointer) {
            return false;
        }
    } else {
        if (foundOutput == outputFiles.end()) {
            return false;
        }
        output = foundOutput->second;
    }
    output->write(reinterpret_cast<const char *>(packet), endPointer - packet);
    return true;
}

bool demux(const uint8_t *packet) {
    const uint8_t *endPointer = packet + MPTS_PACKET_SIZE;
    uint16_t id;
    bool startIndicator;
    if (packet[0] != MPTS_SYNC_BYTE) {
        return false;
    }
    packet++;
    parseHeader(packet, id, startIndicator);

    auto f = outputFiles.find(id);
    if (f != outputFiles.end() || (startIndicator && packet[0] == 0x0 && packet[1] == 0x0 && packet[2] == 0x01)) {
        if (!writeChunk(packet, endPointer, id, startIndicator, f))
            return false;
    }
    return true;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cout << "Usage: TSDemux [filename]" << endl;
        return 1;
    }
    ifstream tsFile(argv[1], ios::in | ios::binary);
    uint8_t packet[MPTS_PACKET_SIZE];

    while (tsFile) {
        tsFile.read(reinterpret_cast<char *>(packet), MPTS_PACKET_SIZE);
        if (!demux(packet))
            break;
    }
    cout << "Demux finished" << endl;
    for (auto &outputFile : outputFiles) {
        delete outputFile.second;
    }
    return 0;
}
