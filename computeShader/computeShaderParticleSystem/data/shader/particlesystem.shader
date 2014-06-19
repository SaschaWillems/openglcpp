/*
	This code is licensed under the Mozilla Public License Version 2.0 (http://opensource.org/licenses/MPL-2.0)
	© 2014 by Sascha Willems - www.saschawillems.de

    This compute shader implements a very basic attraction based particle system that changes velocities
    to move the particles towards the target position
*/

#version 430


// Target 0 : Vertex position
layout(std140, binding = 0) buffer Pos {
   vec4 Positions[ ];
};

// Target 1 : Vertex velocity
layout(std140, binding = 1) buffer Vel {
    vec4 Velocities[ ];
};

layout (local_size_x = 16, local_size_y = 16) in;

// Gravity
const vec3 gravity = vec3(0, -9.8f, 0);

// Frame delta for calculations
uniform float deltaT;
uniform vec3 destPos;

// Viewport dimensions for border clamp
uniform vec2 vpDim;
uniform int borderClamp;

void main() {

    // Current SSBO index
    uint index = gl_GlobalInvocationID.x;

    // Read position and velocity

    vec3 vPos = Positions[index].xyz;
    vec3 vVel = Velocities[index].xyz;

    // Calculate new velocity depending on attraction point
    vVel += normalize(destPos - vPos) * 0.001 * deltaT;

    // Move by velocity
    vPos += vVel * deltaT;


    if (borderClamp == 1.0f) {

        if (vPos.x < -vpDim.x) {
            vPos.x = -vpDim.x;
            vVel.x = -vVel.x;
        }

        if (vPos.x > vpDim.x) {
            vPos.x = vpDim.x;
            vVel.x = -vVel.x;
        }

        if (vPos.y < -vpDim.y) {
            vPos.y = -vpDim.y;
            vVel.y = -vVel.y;
        }

        if (vPos.y > vpDim.y) {
            vPos.y = vpDim.y;
            vVel.y = -vVel.y;
        }

    }

    // Write back


    Positions[index].xyz = vPos;
    Velocities[index].xyz = vVel;

}

