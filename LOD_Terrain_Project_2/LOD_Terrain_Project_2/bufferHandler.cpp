#include "bufferHandler.h"

extern vector<vec3> positions;
extern vector<vec3> normals;
extern vector<vec2> texCoords;
extern vector<unsigned int> indices;

extern vector<BoneInfo> bone_info; 
extern vector<VertexBoneData> vertices_to_bones; 

BufferPair INIT_VEC3_BUFFERS(const vector<vec3>& vertices) {
    BufferPair pair;

    glGenVertexArrays(1, &pair.vao);
    glGenBuffers(1, &pair.vbo);

    glBindVertexArray(pair.vao);
    glBindBuffer(GL_ARRAY_BUFFER, pair.vbo);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    return pair;
}

BufferPair INIT_SIMPLE_VERTEX_BUFFERS(vector<float> vertices) {
	BufferPair pair;

	glGenVertexArrays(1, &pair.vao); //Genera un VAO 
	glGenBuffers(1, &pair.vbo); //Genera un VBO

	glBindVertexArray(pair.vao); //Attiva il VAO appena generato
	glBindBuffer(GL_ARRAY_BUFFER, pair.vbo); //Collega il VBO al target GL_ARRAY_BUFFER

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW); //Copia i dati del piano nella memoria della GPU
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); //Dice a OpenGL come leggere i dati nel VBO per usarli negli shader
	glEnableVertexAttribArray(0); //Attiva l'attributo numero 0 (OpenGL userà i dati collegati a quell'attributo durante il rendering)

    glBindVertexArray(0);

	return pair;
}

BufferPair INIT_DISPLACEMENT_BUFFERS(vector<float> vertices, vector<vec4> edges) {
    BufferPair pair;

    glGenVertexArrays(1, &pair.vao);
    glBindVertexArray(pair.vao);

    // --- VBO per le posizioni ---
    glGenBuffers(1, &pair.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, pair.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // --- VBO per i displacement (vec2) ---
    glGenBuffers(1, &pair.displaceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, pair.displaceVBO);
    glBufferData(GL_ARRAY_BUFFER, edges.size() * sizeof(vec4), edges.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)0);
    glEnableVertexAttribArray(1);

    // Pulizia binding
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return pair;
}

BufferPair INIT_HOUSE_BUFFERS(vector<float> vertices, vector<float> normals) {
	BufferPair pair;

	glGenVertexArrays(1, &pair.vao);
    glBindVertexArray(pair.vao);

    // --- VBO per le posizioni ---
    glGenBuffers(1, &pair.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, pair.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // Posizione: location 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // --- VBO per i centri ---
    glGenBuffers(1, &pair.centerVBO);
    glBindBuffer(GL_ARRAY_BUFFER, pair.centerVBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(1); // Centro: location 1
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0); // Unbind VAO

    return pair;
}

BufferPair INIT_SPHERE_BUFFERS(const vector<vec3>& vertices, const vector<vec3>& centers) {
    BufferPair pair;

    glGenVertexArrays(1, &pair.vao);
    glBindVertexArray(pair.vao);

    // --- VBO per le posizioni ---
    glGenBuffers(1, &pair.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, pair.vbo);
    // Passa dati raw di vec3 (3 float contigui)
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // Posizione: location 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);

    // --- VBO per i centri ---
    glGenBuffers(1, &pair.centerVBO);
    glBindBuffer(GL_ARRAY_BUFFER, pair.centerVBO);
    glBufferData(GL_ARRAY_BUFFER, centers.size() * sizeof(vec3), centers.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(1); // Centro: location 1
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);

    glBindVertexArray(0); // Unbind VAO

    return pair;
}

GLuint INIT_TRANSFORM_FEEDBACK_BUFFERS() {
    GLuint tfBuffer;
    glGenBuffers(1, &tfBuffer);
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, tfBuffer);
    size_t stridePerVertex = sizeof(vec3) * 2;
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, stridePerVertex * 100, NULL, GL_DYNAMIC_READ);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tfBuffer);

    return tfBuffer;
}

ModelBufferPair INIT_MODEL_BUFFERS() {
	ModelBufferPair pair;

    vector<ivec4> boneIDs(vertices_to_bones.size());
    vector<vec4> weights(vertices_to_bones.size());
    for (size_t i = 0; i < vertices_to_bones.size(); i++) {
        boneIDs[i] = ivec4(
            vertices_to_bones[i].boneIDs[0],
            vertices_to_bones[i].boneIDs[1],
            vertices_to_bones[i].boneIDs[2],
            vertices_to_bones[i].boneIDs[3]
        );

        weights[i] = vec4(
            vertices_to_bones[i].weights[0],
            vertices_to_bones[i].weights[1],
            vertices_to_bones[i].weights[2],
            vertices_to_bones[i].weights[3]
        );
    }

    glGenVertexArrays(1, &pair.vao);
    glBindVertexArray(pair.vao);

    // Genera i buffer
    glGenBuffers(1, &pair.vboPositions);
    glGenBuffers(1, &pair.vboNormals);
    glGenBuffers(1, &pair.vboTexCoords);
    glGenBuffers(1, &pair.ebo);

    // POSIZIONI
    glBindBuffer(GL_ARRAY_BUFFER, pair.vboPositions);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(vec3), positions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); // location 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);

    // NORMALI
    glBindBuffer(GL_ARRAY_BUFFER, pair.vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3), normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1); // location 1
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);

    // TEX COORDS
    glBindBuffer(GL_ARRAY_BUFFER, pair.vboTexCoords);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(vec2), texCoords.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2); // location 2
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);

    // BONES: boneIDs (interi)
    glGenBuffers(1, &pair.vboBoneIDs);
    glBindBuffer(GL_ARRAY_BUFFER, pair.vboBoneIDs);
    glBufferData(GL_ARRAY_BUFFER, boneIDs.size() * sizeof(ivec4), boneIDs.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(3); // location 3
    glVertexAttribIPointer(3, 4, GL_INT, sizeof(ivec4), (void*)0); // IPointer per interi

    // BONES: weights (float)
    glGenBuffers(1, &pair.vboBoneWeights);
    glBindBuffer(GL_ARRAY_BUFFER, pair.vboBoneWeights);
    glBufferData(GL_ARRAY_BUFFER, weights.size() * sizeof(vec4), weights.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(4); // location 4
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)0);

    // INDICI
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pair.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    return pair;
}

GLuint INIT_FRAME_BUFFER(GLuint depthCubemap) {
    GLuint depthFBO;

    glGenFramebuffers(1, &depthFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return depthFBO;
}