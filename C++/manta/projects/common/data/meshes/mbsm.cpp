#include "mbsm.hpp"

#include <fstream>

namespace Manta::Data::Meshes {
    const uint32_t MantaBSM::BSM_IDENT = MAKE_32_IDENT("BSM#");

    MantaBSM* MantaBSM::LoadFromStream(std::istream& stream) {
        auto bsm = new MantaBSM();

        BSMHeader header {};
        stream.read(reinterpret_cast<char*>(&header), sizeof(BSMHeader));

        if (header.ident != BSM_IDENT)
            throw std::runtime_error("The file you're trying to load isn't an BSM file!");

        bsm->name.resize(header.name_len);
        stream.read(bsm->name.data(), header.name_len);

        uint32_t idx = 0;
        for (auto i = 0; i < header.indice_count; i++) {
            stream.read(reinterpret_cast<char*>(&idx), sizeof(uint32_t));
            bsm->indices.emplace_back(idx);
        }

        BSMVertex vert{};
        for (auto v = 0; v < header.vertex_count; v++) {
            stream.read(reinterpret_cast<char*>(&vert), sizeof(BSMVertex));
            bsm->vertices.emplace_back(vert);
        }

        return bsm;
    }

    MantaBSM* MantaBSM::LoadFromFile(const std::string& path) {
        std::ifstream file(path, std::ofstream::binary);
        auto smf = LoadFromStream(file);
        file.close();

        return smf;
    }

    void MantaBSM::WriteToFile(const std::string &path) {
        std::ofstream file(path, std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);

        BSMHeader header {};

        header.ident = BSM_IDENT;
        header.indice_count = indices.size();
        header.vertex_count = vertices.size();
        header.name_len = name.length();

        file.write(reinterpret_cast<char*>(&header), sizeof(BSMHeader));
        file << name;

        for (auto indice : indices) {
            file.write(reinterpret_cast<char*>(&indice), sizeof(uint32_t));
        }

        for (auto vertex : vertices) {
            file.write(reinterpret_cast<char*>(&vertex), sizeof(BSMVertex));
        }

        file.close();
    }
}