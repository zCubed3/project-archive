#include <data/meshes/obj.hpp>
#include <data/meshes/mbsm.hpp>
#include <data/meshes/mmdl.hpp>

using namespace Manta::Data::Meshes;

#include <iostream>
#include <fstream>

#include <math.h>

#include <mikktspace.h>

#define GET_IDX() ((iFace * 3) + iVert)

static std::vector<WavefrontOBJ::OBJVector3> tangents;
static std::vector<float> tangents_w;

int get_num_faces(const SMikkTSpaceContext* context) {
    WavefrontOBJ* mesh = (WavefrontOBJ*)context->m_pUserData;

    auto len = mesh->unweld_indices.size() / 3;
    return len;
}

int get_num_vertices_of_face(const SMikkTSpaceContext* context, const int iFace) {
    return 3;
}

void get_position(const SMikkTSpaceContext *context, float *outdata, const int iFace, const int iVert) {
    WavefrontOBJ* mesh = (WavefrontOBJ*)context->m_pUserData;

    size_t idx = GET_IDX();

    auto position = mesh->unweld_positions[mesh->unweld_indices[idx].v];
    outdata[0] = position.x;
    outdata[1] = position.y;
    outdata[2] = position.z;
}

void get_normal(const SMikkTSpaceContext *context, float *outdata, const int iFace, const int iVert) {
    WavefrontOBJ* mesh = (WavefrontOBJ*)context->m_pUserData;

    size_t idx = GET_IDX();

    auto normal = mesh->unweld_normals[mesh->unweld_indices[idx].n];
    outdata[0] = normal.x;
    outdata[1] = normal.y;
    outdata[2] = normal.z;
}

void get_uv(const SMikkTSpaceContext *context, float *outdata, const int iFace, const int iVert) {
    WavefrontOBJ* mesh = (WavefrontOBJ*)context->m_pUserData;

    size_t idx = GET_IDX();

    auto uv = mesh->unweld_uvs[mesh->unweld_indices[idx].u];
    outdata[0] = uv.x;
    outdata[1] = uv.y;
}

void set_tspace(const SMikkTSpaceContext *context, const float *tangent, const float fSign, const int iFace, const int iVert) {
    WavefrontOBJ* mesh = (WavefrontOBJ*)context->m_pUserData;

    size_t idx = GET_IDX();

    WavefrontOBJ::OBJVector4 v4 = {tangent[0], tangent[1], tangent[2], fSign};
    mesh->unweld_tangents[mesh->unweld_indices[idx].v] = v4;
}

int main(int argc, char** argv) {
    // TODO: Make do more than this
    if (argc < 3)
        throw std::runtime_error("Please provide 2 arguments!");

    auto file = std::ifstream(argv[1]);
    auto obj = WavefrontOBJ::LoadFromStream(file, false);

    obj->unweld_tangents.resize(obj->unweld_positions.size());

    SMikkTSpaceContext context {};
    SMikkTSpaceInterface interface {};

    interface.m_getNumFaces = get_num_faces;
    interface.m_getNumVerticesOfFace = get_num_vertices_of_face;
    interface.m_getPosition = get_position;
    interface.m_getNormal = get_normal;
    interface.m_getTexCoord = get_uv;
    interface.m_setTSpaceBasic = set_tspace;

    context.m_pInterface = &interface;
    context.m_pUserData = obj;

    genTangSpaceDefault(&context);

    obj->WeldVertices();

    auto bsm = new MantaBSM();
    auto mmdl = new MantaMDL();

    mmdl->PushChannel(MantaMDL::ChannelType::VEC3, MantaMDL::ChannelHint::VERTEX);
    mmdl->PushChannel(MantaMDL::ChannelType::VEC3, MantaMDL::ChannelHint::NORMAL);
    mmdl->PushChannel(MantaMDL::ChannelType::VEC2, MantaMDL::ChannelHint::UV0);
    mmdl->PushChannel(MantaMDL::ChannelType::VEC4, MantaMDL::ChannelHint::TANGENT);
    mmdl->PushChannel(MantaMDL::ChannelType::UINT, MantaMDL::ChannelHint::INDEXER);

    mmdl->name = obj->name;

    for (auto vert : obj->weld_vertices) {
        mmdl->PushData<MantaMDL::Vec3>(0, { vert.position[0], vert.position[1], vert.position[2] });
        mmdl->PushData<MantaMDL::Vec3>(1, { vert.normal[0], vert.normal[1], vert.normal[2] });
        mmdl->PushData<MantaMDL::Vec2>(2, { vert.uv[0], vert.uv[1] });
        mmdl->PushData<MantaMDL::Vec4>(3, { vert.tangent[0], vert.tangent[1], vert.tangent[2], vert.tangent[3] });
    }

    for (auto idx : obj->weld_indices) {
        mmdl->PushData<uint32_t>(4, idx);
    }

    mmdl->WriteToFile("test.mmdl");

    for (auto idx : obj->weld_indices)
        bsm->indices.emplace_back(idx);

    for (auto vert : obj->weld_vertices) {
        MantaBSM::BSMVertex bsm_vert {};

        for (int p = 0; p < 3; p++)
            bsm_vert.position[p] = vert.position[p];

        for (int u = 0; u < 2; u++)
            bsm_vert.uv[u] = vert.uv[u];

        float yaw = atan2f(vert.normal[0], vert.normal[2]);
        float pitch = asinf(vert.normal[1]);

        bsm_vert.normal[0] = yaw;
        bsm_vert.normal[1] = pitch;

        yaw = atan2f(vert.tangent[0], vert.tangent[2]);
        pitch = asinf(vert.tangent[1]);

        bsm_vert.tangent[0] = yaw;
        bsm_vert.tangent[1] = pitch;
        bsm_vert.tangent[2] = vert.tangent[3];

        bsm->vertices.emplace_back(bsm_vert);
    }

    bsm->name = obj->name;
    bsm->WriteToFile(argv[2]);
}