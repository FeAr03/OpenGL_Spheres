#version 330 core

// Positions/Coordinates
layout (location = 0) in vec3 aPos;
// Colors
layout (location = 1) in vec3 aColor;
// Texture Coordinates
layout (location = 2) in vec2 aTex;
// Normals (not necessarily normalized)
layout (location = 3) in vec3 aNormal;
// Instanced sphere position (vec3)
layout (location = 4) in vec3 instanceOffset; // initial position

// Outputs the color for the Fragment Shader
out vec3 color;
out vec2 texCoord;
out vec3 Normal;
out vec3 crntPos;

// Imports the camera matrix from the main function
uniform mat4 camMatrix;
uniform float uTime;
uniform vec3 lightPos;
// Imports the camera position from the main function
uniform vec3 camPos;

void main()
{
    // Animate revolution around the light in the XZ plane
    float angle = uTime * 0.5;
    mat4 rotation = mat4(1.0);
    rotation[0][0] =  cos(angle);
    rotation[0][2] =  sin(angle);
    rotation[2][0] = -sin(angle);
    rotation[2][2] =  cos(angle);
    vec3 relPos = instanceOffset - lightPos;
    vec3 newPos = lightPos + (rotation * vec4(relPos, 1.0)).xyz;

    // --- BILLBOARDING: Make hemisphere's curved side always face the camera, rotating around Y axis ---
    vec3 toCamera = normalize(camPos - newPos);
    float angleY = atan(toCamera.x, toCamera.z); // angle to rotate around Y axis
    mat3 yRotation = mat3(
        cos(angleY), 0, -sin(angleY),
        0,          1,  0,
        sin(angleY), 0,  cos(angleY)
    );
    // Combine animation rotation and billboard rotation
    mat3 animRotation = mat3(rotation);
    mat3 totalRotation = yRotation * animRotation;
    vec3 localPos = aPos;
    vec3 rotated = totalRotation * localPos;
    vec3 finalPos = newPos + rotated * 0.2;

    crntPos = finalPos;
    gl_Position = camMatrix * vec4(finalPos, 1.0);
    color = aColor;
    texCoord = aTex;
    // Rotate the normal as well for correct lighting
    Normal = totalRotation * aNormal;
}