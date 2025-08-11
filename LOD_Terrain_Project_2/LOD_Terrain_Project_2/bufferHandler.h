#pragma once
#include "lib.h"
#include "strutture.h"

BufferPair INIT_VEC3_BUFFERS(const vector<vec3>& vertices);
BufferPair INIT_DISPLACEMENT_BUFFERS(vector<float> vertices, vector<vec4> edges);
BufferPair INIT_SIMPLE_VERTEX_BUFFERS(vector<float> planeVertices);
BufferPair INIT_HOUSE_BUFFERS(vector<float> vertices, vector<float> centers);
BufferPair INIT_SPHERE_BUFFERS(const vector<vec3>& vertices, const vector<vec3>& centers);
GLuint INIT_TRANSFORM_FEEDBACK_BUFFERS();
ModelBufferPair INIT_MODEL_BUFFERS();
GLuint INIT_FRAME_BUFFER(GLuint depthCubemap);