#version 130
uniform vec3 uColor; // For control points
varying vec3 vColor;  // For axes

void main() {
    // If vColor is black (its default value if not set by a vertex attribute),
    // it means we are drawing a control point. Use the uniform color instead.
    if (vColor.r == 0.0 && vColor.g == 0.0 && vColor.b == 0.0) {
        gl_FragColor = vec4(uColor, 1.0);
    } else {
        gl_FragColor = vec4(vColor, 1.0);
    }
}
