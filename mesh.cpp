#include "types.h"
#include "declarations.h"
#include <string>

float3 crossProduct(float3 A, float3 B) {
    return float3(
        A.y * B.z - A.z * B.y,
        A.z * B.x - A.x * B.z,
        A.x * B.y - A.y * B.x
    );
}

float3 normalize(float3 v) {
    float l = sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
    return v / l;
}

//adds CONVEX mesh for flex
void flexAPI::addMeshConvex(GarrysMod::Lua::ILuaBase* LUA, const float* minFloat, const float* maxFloat, size_t tableLen) {
    //prop decleration
    Prop p = Prop();
    p.pos = float4();
    p.ang = float4(0.0f, 0.0f, 0.0f, 1.0f);
    p.lastPos = float4();
    p.lastAng = float4(0.0f, 0.0f, 0.0f, 1.0f);
    p.verts = NvFlexAllocBuffer(flexLibrary, tableLen, sizeof(float4), eNvFlexBufferHost);

    float4* hostVerts = (float4*)(NvFlexMap(p.verts, eNvFlexMapWait));

    //loop through vertices
    for (int i = 0; i < tableLen; i += 3) {
        //the 3 points
        float3 verts[3] = {};
        for (int j = 0; j < 3; j++) {
            //get table data
            LUA->PushNumber((double)(i + j) + 1.0);
            LUA->GetTable(-2);

            Vector thisPos = LUA->GetVector();
            verts[j] = float3(thisPos);

            LUA->Pop(); //pop pos
        }

        //add data to flex planes
        float3 cross = normalize(crossProduct(verts[1] - verts[0], verts[2] - verts[0]));

        //calculate distance for flex plane
        float d = cross.x * verts[0].x + cross.y * verts[0].y + cross.z * verts[0].z;
        hostVerts[i / 3] = float4(-cross.x, -cross.y, -cross.z, d);
    }

    //make sure to unmap the verts
    NvFlexUnmap(p.verts);

    LUA->Pop(); //pop table

    //create the triangle mesh
    p.meshID = NvFlexCreateConvexMesh(flexLibrary);
    NvFlexUpdateConvexMesh(flexLibrary, p.meshID, p.verts, tableLen / 3, minFloat, maxFloat);

    // Add data to BUFFERS
    mapBuffers();

    simBuffers->flags[propCount] = NvFlexMakeShapeFlags(eNvFlexShapeConvexMesh, true);	//always dynamic (props)
    simBuffers->geometry[propCount].convexMesh.mesh = p.meshID;
    simBuffers->geometry[propCount].convexMesh.scale[0] = 1.0f;
    simBuffers->geometry[propCount].convexMesh.scale[1] = 1.0f;
    simBuffers->geometry[propCount].convexMesh.scale[2] = 1.0f;
    simBuffers->positions[propCount] = float4();
    simBuffers->rotations[propCount] = float4(0.0f, 0.0f, 0.0f, 1.0f);	//NEVER SET ROTATION TO 0,0,0,0, FLEX *HATES* IT!
    simBuffers->prevPositions[propCount] = float4();
    simBuffers->prevRotations[propCount] = float4(0.0f, 0.0f, 0.0f, 1.0f);

    unmapBuffers();

    props.push_back(p);
    propCount++;

}

//generate a TRIANGLE mesh for flex
void flexAPI::addMeshConcave(GarrysMod::Lua::ILuaBase* LUA, const float* minFloat, const float* maxFloat, size_t tableLen) {
    //prop decleration
    Prop p = Prop();
    p.pos = float4();
    p.ang = float4(0.0f, 0.0f, 0.0f, 1.0f);
    p.lastPos = float4();
    p.lastAng = float4(0.0f, 0.0f, 0.0f, 1.0f);
    p.verts = NvFlexAllocBuffer(flexLibrary, tableLen, sizeof(float4), eNvFlexBufferHost);
    p.indices = NvFlexAllocBuffer(flexLibrary, tableLen, sizeof(int), eNvFlexBufferHost);

    float4* hostVerts = (float4*)(NvFlexMap(p.verts, eNvFlexMapWait));
    int* hostIndices = (int*)(NvFlexMap(p.indices, eNvFlexMapWait));

    //loop through verticies
    for (int i = 0; i < tableLen; i++) {

        //lua is 1 indexed, C++ is 0 indexed
        LUA->PushNumber((double)i + 1.0);

        //gets data from table at the number ontop of the stack
        LUA->GetTable(-2);

        Vector thisPos = LUA->GetVector();
        float4 vert = float4(thisPos, 0.f);

        hostVerts[i] = vert;
        hostIndices[i] = i;

        //counter clockwise -> clockwise triangle winding
        if (i % 3 == 1) {
            int host = hostIndices[i];
            hostIndices[i] = hostIndices[i - 1];
            hostIndices[i - 1] = host;
        }

        LUA->Pop(); //pop pos
    }

    LUA->Pop(); //pop table

    NvFlexUnmap(p.verts);
    NvFlexUnmap(p.indices);

    // create the triangle mesh
    p.meshID = NvFlexCreateTriangleMesh(flexLibrary);
    NvFlexUpdateTriangleMesh(flexLibrary, p.meshID, p.verts, p.indices, tableLen, tableLen / 3, minFloat, maxFloat);

    //map & unmap buffers
    mapBuffers();

    simBuffers->flags[propCount] = NvFlexMakeShapeFlags(eNvFlexShapeTriangleMesh, propCount != 0);	//index 0 is ALWAYS the world
    simBuffers->geometry[propCount].triMesh.mesh = p.meshID;
    simBuffers->geometry[propCount].triMesh.scale[0] = 1.0f;
    simBuffers->geometry[propCount].triMesh.scale[1] = 1.0f;
    simBuffers->geometry[propCount].triMesh.scale[2] = 1.0f;
    simBuffers->positions[propCount] = float4();
    simBuffers->rotations[propCount] = float4(0.0f, 0.0f, 0.0f, 1.0f);	//NEVER SET ROTATION TO 0,0,0,0, FLEX *HATES* IT!
    simBuffers->prevPositions[propCount] = float4();
    simBuffers->prevRotations[propCount] = float4(0.0f, 0.0f, 0.0f, 1.0f);

    unmapBuffers();

    props.push_back(p);
    propCount++;

}

//generate a TRIANGLE mesh for flex
/*void flexAPI::addCloth(GarrysMod::Lua::ILuaBase* LUA, size_t tableLen) {
    buffer.assign()
    NvFlexExtCreateTearingClothFromMesh((float*)&g_buffers->positions[0], int(g_buffers->positions.size()), int(g_buffers->positions.size()) + g_numExtraParticles, (int*)&mesh->m_indices[0], mesh->GetNumFaces(), 0.8f, 0.8f, 0.0f);

}*/
