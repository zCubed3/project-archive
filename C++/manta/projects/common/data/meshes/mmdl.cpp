#include "mmdl.hpp"

#include <fstream>
#include <cstring>

namespace Manta::Data::Meshes {
    const uint32_t MantaMDL::MMDL_IDENT = MAKE_32_IDENT("MMDL");

    uint32_t get_type_size(MantaMDL::ChannelType type) {
        switch (type) {
            case MantaMDL::ChannelType::SCALAR:
                return sizeof(float);

            case MantaMDL::ChannelType::UINT:
                return sizeof(uint32_t);

            case MantaMDL::ChannelType::VEC2:
                return sizeof(float) * 2;

            case MantaMDL::ChannelType::VEC3:
                return sizeof(float) * 3;

            case MantaMDL::ChannelType::VEC4:
                return sizeof(float) * 4;

            default:
                return 0;
        }
    }

    MantaMDL *MantaMDL::LoadFromStream(std::istream &stream) {
        auto mdl = new MantaMDL();

        Header header {};
        stream.read(reinterpret_cast<char*>(&header), sizeof(Header));

        mdl->name.resize(header.name_len);

        for (int c = 0; c < header.descriptor_count; c++) {
            ChannelDescriptor desc {};
            stream.read(reinterpret_cast<char*>(&desc), sizeof(ChannelDescriptor));

            auto channel = new Channel();
            channel->descriptor = desc;

            mdl->channels.emplace_back(channel);
        }

        stream.read(mdl->name.data(), header.name_len);

        for (auto & channel : mdl->channels) {
            for (int d = 0; d < channel->descriptor.read_len; d++) {
                auto size = get_type_size(channel->descriptor.type);
                void* data = malloc(size);
                stream.read(reinterpret_cast<char*>(data), size);

                channel->data.push_back(data);
            }
        }

        return mdl;
    }

    void MantaMDL::WriteToFile(const std::string &path) {
        std::ofstream file(path, std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);

        Header header {};

        header.ident = MMDL_IDENT;
        header.descriptor_count = channels.size();
        header.name_len = name.length();

        file.write(reinterpret_cast<char*>(&header), sizeof(Header));

        for (auto & channel : channels) {
            channel->descriptor.read_len = channel->data.size();
            file.write(reinterpret_cast<char*>(channel), sizeof(ChannelDescriptor));
        }

        file.write(name.data(), header.name_len);

        for (auto & channel : channels) {
            uint32_t len = get_type_size(channel->descriptor.type);
            for (auto raw_element : channel->data) {
                file.write(reinterpret_cast<char*>(raw_element), len);
            }
        }

        file.close();
    }

    // TODO: Prevent duplicate semantics!
    MantaMDL::Channel* MantaMDL::PushChannel(ChannelType type, ChannelHint hint) {
        auto channel = new Channel();
        channel->descriptor.type = type;
        channel->descriptor.hint = hint;

        channels.emplace_back(channel);
        return channel;
    }

    void MantaMDL::SetChannelType(uint8_t idx, ChannelType type) {
        channels[idx]->descriptor.type = type;
    }

    void MantaMDL::SetChannelHint(uint8_t idx, ChannelHint hint) {
        channels[idx]->descriptor.hint = hint;
    }

    void MantaMDL::SetChannelProps(uint8_t idx, ChannelType type, ChannelHint hint) {
        channels[idx]->descriptor.type = type;
        channels[idx]->descriptor.hint = hint;
    }

    void MantaMDL::ClearChannel(uint8_t idx) {
        for (auto element : channels[idx]->data) {
            free(element);
        }

        channels[idx]->data.clear();
    }

    void* MantaMDL::CloneData(void* data, size_t size) {
        void* dupe = malloc(size);
        memcpy(dupe, data, size);
        return dupe;
    }

    void MantaMDL::PushRawData(uint8_t idx, void *data) {
        channels[idx]->data.push_back(data);
    }

    MantaMDL::~MantaMDL() {
        for (int c = 0; c < channels.size(); c++) {
            ClearChannel(c);
        }
    }
}