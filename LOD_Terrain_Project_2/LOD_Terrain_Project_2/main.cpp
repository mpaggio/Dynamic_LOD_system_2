#include "lib.h"
#include "strutture.h"
#include "utilities.h"
#include "guiHandler.h"
#include "modelLoader.h"
#include "noiseHandler.h"
#include "shaderHandler.h"
#include "cameraHandler.h"
#include "bufferHandler.h"
#include "textureHandler.h"
#include "geometryHandler.h"
#include "interactionHandler.h"

int height = 600; //Altezza della finestra
int width = 600; //Larghezza della finestra

float Theta = -90.0f; //Angolo per la rotazione orizzontale
float Phi = 0.0f; //Angolo per la rotazione verticale
float moveSpeed = 0.02;
long long startTimeMillis = 0;

bool mouseLocked = true;
bool lineMode = true;
bool mainCharacter = true;

ViewSetup SetupTelecamera;
PerspectiveSetup SetupProspettiva;

pointLight light;

extern vector<unsigned int> indices;
extern vector<BoneInfo> bone_info_walking;
extern vector<BoneInfo> bone_info_standing;
extern const aiScene* scene_walking;
extern const aiScene* scene_standing;

int main() {
    mat4 model = mat4(1.0f); //(Nessuna trasformazione) --> Qui potrei scalare, ruotare o traslare 
    mat4 view = lookAt(vec3(0.0f, 0.0f, 2.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)); //(Davanti all'origine)
    mat4 proj = perspective(radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f); //(FOV: 45, ASPECT: 4.3, ZNEAR: 0.1, ZFAR: 100)
    
    //GLFW
    glfwInit(); //Inizializzazione di GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); //Specifica a GLFW che verrà utilizzato OpenGL versione 4.x (specifica la versione maggiore)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6); //Specifica a GLFW che verrà utilizzato OpenGL versione 4.6 (specifica la versione minore)
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //Richiede un core profile di OpenGL (che esclude le funzionalità deprecate)

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor(); // Prendi il monitor principale
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor); // Prendi le modalità video del monitor (risoluzione, refresh rate, ecc)
    height = mode->height;
    width = mode->width;
    GLFWwindow* window = glfwCreateWindow(width, height, "Tessellation Shader", primaryMonitor, nullptr); // Crea la finestra fullscreen con le dimensioni del monitor
    //GLFWwindow* window = glfwCreateWindow(width, height, "Tessellation Shader", nullptr, nullptr);

    if (!window) { //Gestione dell'errore
        std::cerr << "Errore nella creazione della finestra GLFW\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window); //Attiva il contesto OpenGL associato alla finestra creata, rendendo il contesto corrente per il thread in cui viene chiamata


    //CALLBACKS
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    

    //Inizializzazione di GLAD (carica i puntatori alle funzioni OpenGL)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Errore nell'inizializzazione di GLAD\n";
        return -1;
    }


    //ILLUMINAZIONE
    light.position = { -30.0, 50.0, 30.0 };
    light.color = { 1.0,1.0,1.0 };
    light.power = 2.0f;


    //MODEL
    //Texture
    string path = "Model/Knight/source/castle_guard_01.fbx";
    //extractEmbeddedTextures(path, "Model/Knight/textures");
    //Walking
    path = "Model/Knight/source/Walking.fbx";
    loadModel(path, WALKING);
    ModelBufferPair walkingModelPair = INIT_MODEL_BUFFERS();
    //Standing
    path = "Model/Knight/source/Standing.fbx";
    loadModel(path, STANDING);
    ModelBufferPair standingModelPair = INIT_MODEL_BUFFERS();


    //SKYBOX
    vector<float> skyboxVertices = generateSkyboxCube();
    BufferPair skyboxPair = INIT_SIMPLE_VERTEX_BUFFERS(skyboxVertices);


    //PLANE
    float map_scale = 2.0;
    int division = 12 * map_scale;
    float size = 5.0f * map_scale;
    auto planeData = roadAndGrass(division, size, 4);
    vector<float> terrainVertices = planeData.first;
    vector<bool> isRoad = planeData.second;
    auto terrainPatches = generatePatches(terrainVertices, isRoad, division);
    vector<float> roadPatches = get<0>(terrainPatches);
    vector<vec4> roadEdges = get<1>(terrainPatches);
    vector<float> grassPatches = get<2>(terrainPatches);
    vector<vec4> grassEdges = get<3>(terrainPatches);
    BufferPair roadPair = INIT_DISPLACEMENT_BUFFERS(roadPatches, roadEdges);
    BufferPair grassPair = INIT_DISPLACEMENT_BUFFERS(grassPatches, grassEdges);


    //BLOCKS
    int numHouses = 6;
    int numHedges = 6;
    int houseSubdivision = 2;
    auto positionData = generateCityPositions(terrainVertices, isRoad, division, numHouses, numHedges, size);
    vector<vec3> housePositions = get<0>(positionData);
    vector<vec3> moldPositions = get<1>(positionData);
    vector<vec3> lampPositions = get<2>(positionData);
    vector<vec3> lampDirections = get<3>(positionData);
    auto blocksData = generateBlocks(housePositions, houseSubdivision, false);
    vector<float> blocksVertices = get<0>(blocksData);
    vector<float> blocksHeights = get<1>(blocksData);
    vec3* blocksOriginalVertices = get<2>(blocksData).data();
    auto patchData = generatePatchesFromBlocks(blocksVertices, false);
    vector<float> blocksPatches = patchData.first;
    vector<float> blocksPatchesA(blocksPatches.begin(), blocksPatches.begin() + blocksPatches.size() / 2);
    vector<float> blocksPatchesB(blocksPatches.begin() + blocksPatches.size() / 2, blocksPatches.end());
    vector<float> blocksNormals = patchData.second;
    vector<float> blocksNormalsA(blocksNormals.begin(), blocksNormals.begin() + blocksNormals.size() / 2);
    vector<float> blocksNormalsB(blocksNormals.begin() + blocksNormals.size() / 2, blocksNormals.end());
    BufferPair housesPairA = INIT_HOUSE_BUFFERS(blocksPatchesA, blocksNormalsA);
    BufferPair housesPairB = INIT_HOUSE_BUFFERS(blocksPatchesB, blocksNormalsB);
    
    vector<pair<vec3, vec3>> bvHouses;
    int floatsPerHouse = blocksVertices.size() / numHouses;
    for (int i = 0; i < numHouses; ++i) {
        int startVertex = i * floatsPerHouse;
        int endVertex = startVertex + floatsPerHouse;

        vector<float> singleHouseVertices(blocksVertices.begin() + startVertex, blocksVertices.begin() + endVertex);
        bvHouses.push_back(getBoundingBox(singleHouseVertices));
    }
    


    //ROOFS
    auto roofsData = generateRoofs(housePositions, blocksHeights, houseSubdivision/2);
    vector<float> roofsVertices = get<0>(roofsData);
    vec3* roofsOriginalVertices = get<1>(roofsData).data();
    auto roofsPatchData = generatePatchesFromRoofs(roofsVertices, houseSubdivision/2);
    vector<float> roofsPatches = roofsPatchData.first;
    vector<float> roofsNormals = roofsPatchData.second;
    BufferPair roofPair = INIT_HOUSE_BUFFERS(roofsPatches, roofsNormals);
    
    vector<pair<vec3, vec3>> bvRoofs;
    int floatsPerRoof = roofsVertices.size() / numHouses;
    for (int i = 0; i < numHouses; ++i) {
        int startVertex = i * floatsPerRoof;
        int endVertex = startVertex + floatsPerRoof;

        vector<float> singleRoofVertices(roofsVertices.begin() + startVertex, roofsVertices.begin() + endVertex);
        bvRoofs.push_back(getBoundingBox(singleRoofVertices));
    }


    //MOLDS
    int moldSubdivision = 1;
    auto moldsData = generateBlocks(moldPositions, moldSubdivision, true);
    vector<float> moldsVertices = get<0>(moldsData);
    vector<float> moldsHeights = get<1>(moldsData);
    vec3* moldsOriginalVertices = get<2>(moldsData).data();
    auto moldPatchData = generatePatchesFromBlocks(moldsVertices, true);
    vector<float> moldsPatches = moldPatchData.first;
    vector<float> moldsNormals = moldPatchData.second;
    BufferPair moldsPair = INIT_HOUSE_BUFFERS(moldsPatches, moldsNormals);

    vector<pair<vec3, vec3>> bvMolds;
    int floatsPerMold = moldsVertices.size() / numHedges;
    for (int i = 0; i < numHedges; ++i) {
        int startVertex = i * floatsPerMold;
        int endVertex = startVertex + floatsPerMold;

        vector<float> singleMoldVertices(moldsVertices.begin() + startVertex, moldsVertices.begin() + endVertex);
        bvMolds.push_back(getBoundingBox(singleMoldVertices));
    }


    //LAMP
    int numLamps = lampPositions.size();
    vector<pair<vec3, vec3>> verticalRods;
    auto lampsData = generateLampLinesFromBases(lampPositions, lampDirections, verticalRods);
    vector<vec3> lampVertices = lampsData.first;
    vector<vec3> lightPositions = lampsData.second;
    BufferPair lampPair = INIT_VEC3_BUFFERS(lampVertices);
    
    vector<pair<vec3, vec3>> bvLamps;
    for (auto& rod : verticalRods) {
        vector<vec3> rodVertices = { rod.first, rod.second };
        bvLamps.push_back(getBoundingBox(rodVertices));
    }


    //LAMP LIGHTS
    auto lampLightsData = generateSphericalBasesFromPositions(lightPositions);
    vector<vec3> lampLightsVertex = lampLightsData.first;
    vector<vec3> lampLightsCenter = lampLightsData.second;
    BufferPair lampLightsPair = INIT_SPHERE_BUFFERS(lampLightsVertex, lampLightsCenter);


    //SHADER PROGRAMS
    unsigned int modelProgram = createSimpleShaderProgram(
        "vertex_model.glsl",
        "fragment_model.glsl"
    );
    unsigned int skyboxProgram = createSimpleShaderProgram(
        "vertex_skybox.glsl",
        "fragment_skybox.glsl"
    );
    unsigned int terrainProgram = createShaderProgram(
        "vertex_terrain.glsl",
        "tess_control_terrain.glsl",
        "tess_eval_terrain.glsl",
        "geometry_terrain.glsl",
        "fragment_terrain.glsl"
    );
    unsigned int housesProgram = createShaderProgram(
        "vertex_houses.glsl",
        "tess_control_houses.glsl",
        "tess_eval_houses.glsl",
        "geometry_houses.glsl",
        "fragment_houses.glsl"
    );
    unsigned int housesProgram2 = createShaderProgram(
        "vertex_houses.glsl",
        "tess_control_houses.glsl",
        "tess_eval_houses.glsl",
        "geometry_houses.glsl",
        "fragment_houses.glsl"
    );
    unsigned int roofProgram = createShaderProgram(
        "vertex_roofs.glsl",
        "tess_control_roofs.glsl",
        "tess_eval_roofs.glsl",
        "geometry_roofs.glsl",
        "fragment_roofs.glsl"
    );
    unsigned int moldsProgram = createShaderProgram(
        "vertex_houses.glsl",
        "tess_control_houses.glsl",
        "tess_eval_houses.glsl",
        "geometry_houses.glsl",
        "fragment_houses.glsl"
    );
    unsigned int lampProgram = createShaderProgram(
        "vertex_lamps.glsl",
        "tess_control_lamps.glsl",
        "tess_eval_lamps.glsl",
        "geometry_lamps.glsl",
        "fragment_lamps.glsl"
    );
    unsigned int lampLightProgram = createShaderProgram(
        "vertex_sphere.glsl",
        "tess_control_sphere.glsl",
        "tess_eval_sphere.glsl",
        "geometry_sphere.glsl",
        "fragment_sphere.glsl"
    );
    

    //TEXTURES
    GLuint modelTexture = loadSingleTexture("Model/Knight/textures/texture_embedded_0.png");
    GLuint skyboxTexture = loadSkybox();
    vector<GLuint> terrainTextures = loadAllTextures({
        "Texture/Terrain/Color_3.png",
        "Texture/Terrain/Displacement_3.png"
    });
    vector<GLuint> grassTextures = loadAllTextures({
        "Texture/Grass/Color_3.png",
        "Texture/Grass/Displacement_3.png"
    });
    vector<GLuint>houseTextures = loadAllTextures({
        "Texture/House/Color.png",
        "Texture/House/Displacement.png"
    });
    vector<GLuint>houseTextures2 = loadAllTextures({
        "Texture/House/Color_2.png",
        "Texture/House/Displacement_2.png"
        });
    vector<GLuint>roofTextures = loadAllTextures({
        "Texture/Roof/Color.png",
        "Texture/Roof/Displacement.png"
    });
    vector<GLuint>moldTextures = loadAllTextures({
        "Texture/Grass/Color.png",
        "Texture/Grass/Displacement.png"
    });


    //UNIFORMS
    //Terrain program
    int modelLoc_terrain = glGetUniformLocation(terrainProgram, "model");
    int viewLoc_terrain = glGetUniformLocation(terrainProgram, "view");
    int projLoc_terrain = glGetUniformLocation(terrainProgram, "proj");
    int cameraPositionLocation = glGetUniformLocation(terrainProgram, "cameraPosition");
    int characterPositionLocation = glGetUniformLocation(terrainProgram, "characterPosition");
    int useCharacterToTessLocation = glGetUniformLocation(terrainProgram, "useCharacterToTess");
    int lightPosLocTerrain = glGetUniformLocation(terrainProgram, "light.position");
    int lightColorLocTerrain = glGetUniformLocation(terrainProgram, "light.color");
    int lightPowerLocTerrain = glGetUniformLocation(terrainProgram, "light.power");
    //Houses program
    int modelLoc_houses = glGetUniformLocation(housesProgram, "model");
    int viewLoc_houses = glGetUniformLocation(housesProgram, "view");
    int projLoc_houses = glGetUniformLocation(housesProgram, "proj");
    int cameraPositionLocation_houses = glGetUniformLocation(housesProgram, "cameraPosition");
    int characterPositionLocation_houses = glGetUniformLocation(housesProgram, "characterPosition");
    int useCharacterToTessLocation_houses = glGetUniformLocation(housesProgram, "useCharacterToTess");
    int lightPosLocTerrain_houses = glGetUniformLocation(housesProgram, "light.position");
    int lightColorLocTerrain_houses = glGetUniformLocation(housesProgram, "light.color");
    int lightPowerLocTerrain_houses = glGetUniformLocation(housesProgram, "light.power");
    int originalVerticesLoc_houses = glGetUniformLocation(housesProgram, "originalPoints");
    int scaleLoc_houses = glGetUniformLocation(housesProgram, "SCALE");
    //Houses program 2
    int modelLoc_houses2 = glGetUniformLocation(housesProgram2, "model");
    int viewLoc_houses2 = glGetUniformLocation(housesProgram2, "view");
    int projLoc_houses2 = glGetUniformLocation(housesProgram2, "proj");
    int cameraPositionLocation_houses2 = glGetUniformLocation(housesProgram2, "cameraPosition");
    int characterPositionLocation_houses2 = glGetUniformLocation(housesProgram2, "characterPosition");
    int useCharacterToTessLocation_houses2 = glGetUniformLocation(housesProgram2, "useCharacterToTess");
    int lightPosLocTerrain_houses2 = glGetUniformLocation(housesProgram2, "light.position");
    int lightColorLocTerrain_houses2 = glGetUniformLocation(housesProgram2, "light.color");
    int lightPowerLocTerrain_houses2 = glGetUniformLocation(housesProgram2, "light.power");
    int originalVerticesLoc_houses2 = glGetUniformLocation(housesProgram2, "originalPoints");
    int scaleLoc_houses2 = glGetUniformLocation(housesProgram2, "SCALE");
    //Roofs program
    int modelLoc_roofs = glGetUniformLocation(roofProgram, "model");
    int viewLoc_roofs = glGetUniformLocation(roofProgram, "view");
    int projLoc_roofs = glGetUniformLocation(roofProgram, "proj");
    int cameraPositionLocation_roofs = glGetUniformLocation(roofProgram, "cameraPosition");
    int characterPositionLocation_roofs = glGetUniformLocation(roofProgram, "characterPosition");
    int useCharacterToTessLocation_roofs = glGetUniformLocation(roofProgram, "useCharacterToTess");
    int lightPosLocTerrain_roofs = glGetUniformLocation(roofProgram, "light.position");
    int lightColorLocTerrain_roofs = glGetUniformLocation(roofProgram, "light.color");
    int lightPowerLocTerrain_roofs = glGetUniformLocation(roofProgram, "light.power");
    int originalVerticesLoc_roofs = glGetUniformLocation(roofProgram, "originalPoints");
    //Molds program
    int modelLoc_molds = glGetUniformLocation(moldsProgram, "model");
    int viewLoc_molds = glGetUniformLocation(moldsProgram, "view");
    int projLoc_molds = glGetUniformLocation(moldsProgram, "proj");
    int cameraPositionLocation_molds = glGetUniformLocation(moldsProgram, "cameraPosition");
    int characterPositionLocation_molds = glGetUniformLocation(moldsProgram, "characterPosition");
    int useCharacterToTessLocation_molds = glGetUniformLocation(moldsProgram, "useCharacterToTess");
    int lightPosLocTerrain_molds = glGetUniformLocation(moldsProgram, "light.position");
    int lightColorLocTerrain_molds = glGetUniformLocation(moldsProgram, "light.color");
    int lightPowerLocTerrain_molds = glGetUniformLocation(moldsProgram, "light.power");
    int originalVerticesLoc_molds = glGetUniformLocation(moldsProgram, "originalPoints");
    int scaleLoc_molds = glGetUniformLocation(moldsProgram, "SCALE");
    //Lamps program
    int model_lamps = glGetUniformLocation(lampProgram, "model");
    int view_lamps = glGetUniformLocation(lampProgram, "view");
    int proj_lamps = glGetUniformLocation(lampProgram, "proj");
    int cameraPosition_lamps = glGetUniformLocation(lampProgram, "cameraPosition");
    int characterPosition_lamps = glGetUniformLocation(lampProgram, "characterPosition");
    int useCharacterToTess_lamps = glGetUniformLocation(lampProgram, "useCharacterToTess");
    int lightPos_lamps = glGetUniformLocation(lampProgram, "light.position");
    int lightColor_lamps = glGetUniformLocation(lampProgram, "light.color");
    int lightPower_lamps = glGetUniformLocation(lampProgram, "light.power");
    int viewPos_lamps = glGetUniformLocation(lampProgram, "ViewPos");
    //Lamps light program
    int model_lampsLight = glGetUniformLocation(lampLightProgram, "model");
    int view_lampsLight = glGetUniformLocation(lampLightProgram, "view");
    int proj_lampsLight = glGetUniformLocation(lampLightProgram, "proj");
    int cameraPosition_lampsLight = glGetUniformLocation(lampLightProgram, "cameraPosition");
    int characterPosition_lampsLight = glGetUniformLocation(lampLightProgram, "characterPosition");
    int useCharacterToTess_lampsLight = glGetUniformLocation(lampLightProgram, "useCharacterToTess");
    int lightPos_lampsLight = glGetUniformLocation(lampLightProgram, "light.position");
    int lightColor_lampsLight = glGetUniformLocation(lampLightProgram, "light.color");
    int lightPower_lampsLight = glGetUniformLocation(lampLightProgram, "light.power");
    int viewPos_lampsLight = glGetUniformLocation(lampLightProgram, "ViewPos");
    //Model program
    int modelLoc = glGetUniformLocation(modelProgram, "model");
    int viewLoc = glGetUniformLocation(modelProgram, "view");
    int projLoc = glGetUniformLocation(modelProgram, "proj");
    int cameraPosLoc = glGetUniformLocation(modelProgram, "ViewPos");
    int lightPosLoc = glGetUniformLocation(modelProgram, "light.position");
    int lightColorLoc = glGetUniformLocation(modelProgram, "light.color");
    int lightPowerLoc = glGetUniformLocation(modelProgram, "light.power");
    int bonesLoc = glGetUniformLocation(modelProgram, "bones");
    //Skybox program
    int viewLocSkybox = glGetUniformLocation(skyboxProgram, "View");
    int projLocSkybox = glGetUniformLocation(skyboxProgram, "Projection");


    //SETTINGS
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Imposta la modalità Wireframe per vedere le suddivisioni fatte dallo shader
    glDisable(GL_CULL_FACE); //Disabilita il culling
    glEnable(GL_DEPTH_TEST); //Abilita il depth test


    //TELECAMERA
    INIT_CAMERA_PROJECTION();


    //GUI
    initializeGui(window); //Inizializza la finestra di interazione


    //TIME
    startTimeMillis = static_cast<long long>(glfwGetTime() * 1000.0);


    //MODEL MOVEMENT
    vec3 modelMovement = vec3(0.0f);
    vec3 previousModelMovement = vec3(0.0f);
    vec3 modelWorldPos = vec3(5.0f, 0.0f, 0.0f); //posizione assoluta del modello in world space
    float rotationAngle = 0.0f;


    //MAIN LOOP
    while (!glfwWindowShouldClose(window)) {
        //TIME
        long long currentTimeMillis = static_cast<long long>(glfwGetTime() * 1000.0);
        float animationTimeSec = ((float)(currentTimeMillis - startTimeMillis)) / 1000.0f;

        //CLEAR
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //LINE MODE
        if (lineMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Imposta la modalità Wireframe per vedere le suddivisioni fatte dallo shader
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //Imposta la modalità Fill per vedere le suddivisioni riempite
        }

        float timeValue = glfwGetTime(); //Restituisce il tempo in secondi dall'avvio
        glPatchParameteri(GL_PATCH_VERTICES, 4); //Dice a OpenGL che ogni patch ha 4 vertici


        //MODEL PROGRAM
        glUseProgram(modelProgram);

        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, modelTexture);
        string uniformName = "modelTexture";
        GLint location = glGetUniformLocation(modelProgram, uniformName.c_str());
        glUniform1i(location, 0);

        ModelState state;
        bool isMoving = length(modelMovement - previousModelMovement) > 0.00001f;
        state = isMoving ? WALKING : STANDING;

        //aggiornamento dell'animazione del personaggio (se presente)
        if (state == WALKING) {
            if (scene_walking && scene_walking->mNumAnimations > 0 && scene_walking->mAnimations[0]) {
                float ticksPerSecond = scene_walking->mAnimations[0]->mTicksPerSecond != 0 ? scene_walking->mAnimations[0]->mTicksPerSecond : 25.0f; //quanti tick al secondo
                float timeInTicks = animationTimeSec * ticksPerSecond; //quanti tick sono passati
                float animationTimeTicks = fmod(timeInTicks, scene_walking->mAnimations[0]->mDuration); //prendo la parte decimale dell'operazione modulo (animazione continua)
                updateBoneTransforms(animationTimeTicks, state);
            }
        }
        else {
            if (scene_standing && scene_standing->mNumAnimations > 0 && scene_standing->mAnimations[0]) {
                float ticksPerSecond = scene_standing->mAnimations[0]->mTicksPerSecond != 0 ? scene_standing->mAnimations[0]->mTicksPerSecond : 25.0f; //quanti tick al secondo
                float timeInTicks = animationTimeSec * ticksPerSecond; //quanti tick sono passati
                float animationTimeTicks = fmod(timeInTicks, scene_standing->mAnimations[0]->mDuration); //prendo la parte decimale dell'operazione modulo (animazione continua)
                updateBoneTransforms(animationTimeTicks, state);
            }
        }

        mat4 objectModel = mat4(1.0f);
        objectModel = translate(objectModel, modelWorldPos);
        objectModel = scale(objectModel, vec3(0.002f));
        objectModel = rotate(objectModel, radians(float(180)), vec3(0.0f, 1.0f, 0.0f));
        objectModel = rotate(objectModel, radians(rotationAngle), vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, value_ptr(objectModel));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, value_ptr(proj));
        glUniform3fv(cameraPosLoc, 1, value_ptr(SetupTelecamera.position));
        glUniform3fv(lightPosLoc, 1, value_ptr(light.position));
        glUniform3fv(lightColorLoc, 1, value_ptr(light.color));
        glUniform1f(lightPowerLoc, light.power);

        mat4 boneTransforms[128];

        if (state == WALKING) {
            for (int i = 0; i < bone_info_walking.size(); i++)
                boneTransforms[i] = bone_info_walking[i].finalTransform;

            glUniformMatrix4fv(bonesLoc, bone_info_walking.size(), GL_FALSE, value_ptr(boneTransforms[0]));
        }
        else {
            for (int i = 0; i < bone_info_standing.size(); i++)
                boneTransforms[i] = bone_info_standing[i].finalTransform;

            glUniformMatrix4fv(bonesLoc, bone_info_standing.size(), GL_FALSE, value_ptr(boneTransforms[0]));
        }


        glBindVertexArray(walkingModelPair.vao);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);


        //SKYBOX
        glDepthFunc(GL_LEQUAL);       // per permettere la skybox in fondo
        glDepthMask(GL_FALSE);        // disattiva scrittura nello z-buffer

        glUseProgram(skyboxProgram);
        
        glUniform1i(glGetUniformLocation(skyboxProgram, "skybox"), 0);
        glUniformMatrix4fv(viewLocSkybox, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(projLocSkybox, 1, GL_FALSE, value_ptr(proj));

        glBindVertexArray(skyboxPair.vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);

        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);         // riattiva scrittura per gli oggetti normali
        glDepthFunc(GL_LESS);         // ripristina depth test standard
 

        //TERRAIN PROGRAM (1)
        glUseProgram(terrainProgram);

        for (int i = 0; i < terrainTextures.size(); i++) { //Attiva le texture
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, terrainTextures[i]);
        }
        //Lega le texture alle relative variabili uniform
        for (int i = 0; i < terrainTextures.size(); i++) {
            string uniformName = "texture" + std::to_string(i);
            GLint location = glGetUniformLocation(terrainProgram, uniformName.c_str());
            glUniform1i(location, i);
        }

        glUniform1i(useCharacterToTessLocation, int(mainCharacter));
        glUniformMatrix4fv(modelLoc_terrain, 1, GL_FALSE, value_ptr(model));
        glUniformMatrix4fv(viewLoc_terrain, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(projLoc_terrain, 1, GL_FALSE, value_ptr(proj));
        glUniform3fv(cameraPositionLocation, 1, value_ptr(SetupTelecamera.position));
        glUniform3fv(characterPositionLocation, 1, value_ptr(modelWorldPos));
        glUniform3fv(lightPosLocTerrain, 1, value_ptr(light.position));
        glUniform3fv(lightColorLocTerrain, 1, value_ptr(light.color));
        glUniform1f(lightPowerLocTerrain, light.power);
        
        glBindVertexArray(roadPair.vao);
        glDrawArrays(GL_PATCHES, 0, division * division * 4);


        //TERRAIN PROGRAM (2)
        glUseProgram(terrainProgram);

        for (int i = 0; i < grassTextures.size(); i++) { //Attiva le texture
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, grassTextures[i]);
        }
        //Lega le texture alle relative variabili uniform
        for (int i = 0; i < grassTextures.size(); i++) {
            string uniformName = "texture" + std::to_string(i);
            GLint location = glGetUniformLocation(terrainProgram, uniformName.c_str());
            glUniform1i(location, i);
        }

        glUniform1i(useCharacterToTessLocation, int(mainCharacter));
        glUniformMatrix4fv(modelLoc_terrain, 1, GL_FALSE, value_ptr(model));
        glUniformMatrix4fv(viewLoc_terrain, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(projLoc_terrain, 1, GL_FALSE, value_ptr(proj));
        glUniform3fv(cameraPositionLocation, 1, value_ptr(SetupTelecamera.position));
        glUniform3fv(characterPositionLocation, 1, value_ptr(modelWorldPos));
        glUniform3fv(lightPosLocTerrain, 1, value_ptr(light.position));
        glUniform3fv(lightColorLocTerrain, 1, value_ptr(light.color));
        glUniform1f(lightPowerLocTerrain, light.power);

        glBindVertexArray(grassPair.vao);
        glDrawArrays(GL_PATCHES, 0, division * division * 4);


        //HOUSES PROGRAM
        glUseProgram(housesProgram);

        for (int i = 0; i < houseTextures.size(); i++) { //Attiva le texture
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, houseTextures[i]);
        }
        //Lega le texture alle relative variabili uniform
        for (int i = 0; i < houseTextures.size(); i++) {
            string uniformName = "texture" + std::to_string(i);
            GLint location = glGetUniformLocation(housesProgram, uniformName.c_str());
            glUniform1i(location, i);
        }

        glUniform1i(useCharacterToTessLocation_houses, int(mainCharacter));
        glUniformMatrix4fv(modelLoc_houses, 1, GL_FALSE, value_ptr(model));
        glUniformMatrix4fv(viewLoc_houses, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(projLoc_houses, 1, GL_FALSE, value_ptr(proj));
        glUniform3fv(cameraPositionLocation_houses, 1, value_ptr(SetupTelecamera.position));
        glUniform3fv(characterPositionLocation_houses, 1, value_ptr(modelWorldPos));
        glUniform3fv(lightPosLocTerrain_houses, 1, value_ptr(light.position));
        glUniform3fv(lightColorLocTerrain_houses, 1, value_ptr(light.color));
        glUniform1f(lightPowerLocTerrain_houses, light.power);
        glUniform3fv(originalVerticesLoc_houses, 8 * numHouses, reinterpret_cast<const float*>(blocksOriginalVertices));
        glUniform1f(scaleLoc_houses, 0.04f);

        glBindVertexArray(housesPairA.vao);
        glDrawArrays(GL_PATCHES, 0, blocksPatchesA.size() / 3);


        //HOUSES PROGRAM
        glUseProgram(housesProgram2);

        for (int i = 0; i < houseTextures2.size(); i++) { //Attiva le texture
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, houseTextures2[i]);
        }
        //Lega le texture alle relative variabili uniform
        for (int i = 0; i < houseTextures2.size(); i++) {
            string uniformName = "texture" + std::to_string(i);
            GLint location = glGetUniformLocation(housesProgram2, uniformName.c_str());
            glUniform1i(location, i);
        }

        glUniform1i(useCharacterToTessLocation_houses2, int(mainCharacter));
        glUniformMatrix4fv(modelLoc_houses2, 1, GL_FALSE, value_ptr(model));
        glUniformMatrix4fv(viewLoc_houses2, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(projLoc_houses2, 1, GL_FALSE, value_ptr(proj));
        glUniform3fv(cameraPositionLocation_houses2, 1, value_ptr(SetupTelecamera.position));
        glUniform3fv(characterPositionLocation_houses2, 1, value_ptr(modelWorldPos));
        glUniform3fv(lightPosLocTerrain_houses2, 1, value_ptr(light.position));
        glUniform3fv(lightColorLocTerrain_houses2, 1, value_ptr(light.color));
        glUniform1f(lightPowerLocTerrain_houses2, light.power);
        glUniform3fv(originalVerticesLoc_houses2, 8 * numHouses, reinterpret_cast<const float*>(blocksOriginalVertices));
        glUniform1f(scaleLoc_houses2, 0.04f);

        glBindVertexArray(housesPairB.vao);
        glDrawArrays(GL_PATCHES, 0, blocksPatchesB.size() / 3);


        //ROOFS PROGRAM
        glUseProgram(roofProgram);

        for (int i = 0; i < roofTextures.size(); i++) { //Attiva le texture
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, roofTextures[i]);
        }
        //Lega le texture alle relative variabili uniform
        for (int i = 0; i < roofTextures.size(); i++) {
            string uniformName = "texture" + std::to_string(i);
            GLint location = glGetUniformLocation(roofProgram, uniformName.c_str());
            glUniform1i(location, i);
        }

        glUniform1i(useCharacterToTessLocation_roofs, int(mainCharacter));
        glUniformMatrix4fv(modelLoc_roofs, 1, GL_FALSE, value_ptr(model));
        glUniformMatrix4fv(viewLoc_roofs, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(projLoc_roofs, 1, GL_FALSE, value_ptr(proj));
        glUniform3fv(cameraPositionLocation_roofs, 1, value_ptr(SetupTelecamera.position));
        glUniform3fv(characterPositionLocation_roofs, 1, value_ptr(modelWorldPos));
        glUniform3fv(lightPosLocTerrain_roofs, 1, value_ptr(light.position));
        glUniform3fv(lightColorLocTerrain_roofs, 1, value_ptr(light.color));
        glUniform1f(lightPowerLocTerrain_roofs, light.power);
        glUniform3fv(originalVerticesLoc_roofs, 8 * numHouses, reinterpret_cast<const float*>(roofsOriginalVertices));

        glBindVertexArray(roofPair.vao);
        glDrawArrays(GL_PATCHES, 0, roofsPatches.size() / 3);


        //MOLDS PROGRAM
        glUseProgram(moldsProgram);

        for (int i = 0; i < moldTextures.size(); i++) { //Attiva le texture
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, moldTextures[i]);
        }
        //Lega le texture alle relative variabili uniform
        for (int i = 0; i < moldTextures.size(); i++) {
            string uniformName = "texture" + std::to_string(i);
            GLint location = glGetUniformLocation(moldsProgram, uniformName.c_str());
            glUniform1i(location, i);
        }

        glUniform1i(useCharacterToTessLocation_molds, int(mainCharacter));
        glUniformMatrix4fv(modelLoc_molds, 1, GL_FALSE, value_ptr(model));
        glUniformMatrix4fv(viewLoc_molds, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(projLoc_molds, 1, GL_FALSE, value_ptr(proj));
        glUniform3fv(cameraPositionLocation_molds, 1, value_ptr(SetupTelecamera.position));
        glUniform3fv(characterPositionLocation_molds, 1, value_ptr(modelWorldPos));
        glUniform3fv(lightPosLocTerrain_molds, 1, value_ptr(light.position));
        glUniform3fv(lightColorLocTerrain_molds, 1, value_ptr(light.color));
        glUniform1f(lightPowerLocTerrain_molds, light.power);
        glUniform3fv(originalVerticesLoc_molds, 8 * numHedges , reinterpret_cast<const float*>(moldsOriginalVertices));
        glUniform1f(scaleLoc_molds, 0.02f);

        glBindVertexArray(moldsPair.vao);
        glDrawArrays(GL_PATCHES, 0, moldsPatches.size() / 3);
        

        //LAMPS PROGRAM
        glUseProgram(lampProgram);

        glUniformMatrix4fv(model_lamps, 1, GL_FALSE, value_ptr(model));
        glUniformMatrix4fv(view_lamps, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(proj_lamps, 1, GL_FALSE, value_ptr(proj));
        glUniform3fv(cameraPosition_lamps, 1, value_ptr(SetupTelecamera.position));
        glUniform3fv(characterPosition_lamps, 1, value_ptr(modelWorldPos));
        glUniform1i(useCharacterToTess_lamps, int(mainCharacter));
        glUniform3fv(lightPos_lamps, 1, value_ptr(light.position));
        glUniform3fv(lightColor_lamps, 1, value_ptr(light.color));
        glUniform1f(lightPower_lamps, light.power);
        glUniform3fv(viewPos_lamps, 1, value_ptr(SetupTelecamera.position));

        glBindVertexArray(lampPair.vao);
        glDrawArrays(GL_PATCHES, 0, lampVertices.size());

        glPatchParameteri(GL_PATCH_VERTICES, 3);

        //LAMPS LIGHT PROGRAM
        glUseProgram(lampLightProgram);

        glUniformMatrix4fv(model_lampsLight, 1, GL_FALSE, value_ptr(model));
        glUniformMatrix4fv(view_lampsLight, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(proj_lampsLight, 1, GL_FALSE, value_ptr(proj));
        glUniform3fv(cameraPosition_lampsLight, 1, value_ptr(SetupTelecamera.position));
        glUniform3fv(characterPosition_lampsLight, 1, value_ptr(modelWorldPos));
        glUniform1i(useCharacterToTess_lampsLight, int(mainCharacter));
        glUniform3fv(lightPos_lampsLight, 1, value_ptr(light.position));
        glUniform3fv(lightColor_lampsLight, 1, value_ptr(light.color));
        glUniform1f(lightPower_lampsLight, light.power);
        glUniform3fv(viewPos_lampsLight, 1, value_ptr(SetupTelecamera.position));

        glBindVertexArray(lampLightsPair.vao);
        glDrawArrays(GL_PATCHES, 0, lampLightsVertex.size());


        renderGui();
        glfwSwapBuffers(window); //Scambia il buffer frontale con quello posteriore
        glfwPollEvents(); //Controlla e gestisce gli eventi della finestra (input tastiera, mouse, ...)


        //MATRICI DI TRASFORMAZIONE
        view = lookAt(SetupTelecamera.position, SetupTelecamera.target, SetupTelecamera.upVector);
        proj = perspective(radians(SetupProspettiva.fovY), SetupProspettiva.aspect, SetupProspettiva.near_plane, SetupProspettiva.far_plane);
    
        auto inputResult = process_input(window);
        previousModelMovement = modelMovement;
        if (length(inputResult.first) > 0.0001f) {
            bool collision = false;
            for (auto bv : bvHouses) {
                if (checkCollision(modelWorldPos + inputResult.first, bv.first, bv.second)) {
                    collision = true;
                }
            }

            for (auto bv : bvRoofs) {
                if (checkCollision(modelWorldPos + inputResult.first, bv.first, bv.second)) {
                    collision = true;
                }
            }

            for (auto bv : bvMolds) {
                if (checkCollision(modelWorldPos + inputResult.first, bv.first, bv.second)) {
                    collision = true;
                }
            }

            for (auto bv : bvLamps) {
                if (checkCollision(modelWorldPos + inputResult.first, bv.first, bv.second)) {
                    collision = true;
                }
            }

            if (!collision) {
                modelMovement += inputResult.first;
                modelWorldPos += inputResult.first;
                rotationAngle = inputResult.second;
            }
        }
    }


    //TERMINAZIONE
    glDeleteProgram(modelProgram);
    glDeleteProgram(skyboxProgram);
    destroyGui();
    glfwDestroyWindow(window); //Elimina la finestra GLFW
    glfwTerminate(); //Termina la libreria GLFW, liberando tutte le risorse rimaste
    return 0;
}