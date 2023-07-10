// https://developer.mozilla.org/en-US/docs/Web/API/WebGLShader
function createShader(gl, source, type) {
    var shader = gl.createShader(type);
    gl.shaderSource(shader, source);
    gl.compileShader(shader);


    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
        const info = gl.getShaderInfoLog(shader);
        throw `Could not compile WebGL program. \n\n${info}`;
    }
    
    return shader;
}

//
// Shader sources
//
var vertexShaderSource = `
    attribute vec4 position;
    attribute vec2 uv0;

    varying vec2 uv;

    void main() {
        gl_Position = position;
        uv = uv0;
    }
`;

var fragmentShaderSource = `
    precision highp float;

    uniform vec4 info;
    uniform vec4 precise;

    varying vec2 uv;

    vec3 hsv2rgb(vec3 c) {
        vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    void main() {
        // Basic mandlebrot
        float iter = 0.0;

        float bx = ((uv.x - 0.5) * 2.0) * info.z;
        float by = (((uv.y - 0.5) * 2.0) / info.w) * info.z;

        bx += info.x;
        by += info.y;

        float ix = 0.0;
        float iy = 0.0;

        const float MAX_ITER = 200.0;
        const float BAIL = 256.0;

        for (float t = 0.0; t < MAX_ITER; t++) {
            float f = ix * ix - iy * iy + bx;

            iy = 2.0 * ix * iy + by;
            ix = f;

            iter += 1.0;

            if (!((ix * ix + iy * iy <= BAIL) && (iter < MAX_ITER))) {
                break;
            }
        }

        if (iter < MAX_ITER) {
            float log_zn = log(ix * ix + iy * iy) / 2.0;
            float nu = log(log_zn / log(2.0)) / log(2.0);

            iter += 1.0 - nu;
        }

        float f = iter / MAX_ITER;
        vec3 color = hsv2rgb(vec3(pow(f, 0.7), 0.5, 1.0 - f));

        gl_FragColor = vec4(color, 1.0);
    }
`;

//
// Geo
//
var vertices = new Float32Array([
    -1.0, -1.0, 0.0,
    0.0, 0.0,

    -1.0, 1.0, 0.0,
    0.0, 1.0,

    1.0, -1.0, 0.0,
    1.0, 0.0,

    1.0, 1.0, 0.0,
    1.0, 1.0
]);

var indices = new Uint16Array([
    0, 1, 2,
    2, 1, 3
]);

var u_info = -1;
var u_precise = -1;

var offset_x = 0.0;
var offset_y = 0.0;
var zoom = 1.0;
var aspect = 1.0;

var dragging = false;
var last_x = 0.0;
var last_y = 0.0;

const inset = 30;

var gl;
var canvas;

function draw() {
    gl.viewport(0, 0, canvas.width, canvas.height);

    high_zoom = new Float32Array([zoom]);
    delta = zoom - new Float32Array([high_zoom])[0];
    low_zoom = new Float32Array([delta]);

    gl.uniform4f(u_info, offset_x, offset_y, zoom, aspect);
    gl.uniform4f(u_precise, high_zoom[0], low_zoom[0], 0.0, 0.0);
    
    gl.drawElements(gl.TRIANGLES, indices.length, gl.UNSIGNED_SHORT, 0);

}

function resize() {
    aspect = window.innerWidth / window.innerHeight;

    canvas.width = window.innerWidth - inset;
    canvas.height = window.innerHeight - inset;

    aspect = canvas.width / canvas.height;
}

function main() {
    canvas = document.querySelector("#glCanvas");
    resize();
    gl = canvas.getContext("webgl");

    canvas.addEventListener('wheel', (e) => {
        var rel_z = e.deltaY / 1000.0 * zoom;
        var z_sign = Math.sign(rel_z);

        zoom += rel_z;

        var mid_x = canvas.width / 2.0;
        var mid_y = canvas.height / 2.0;

        var rel_x = (e.offsetX / mid_x) - 1.0;
        var rel_y = (e.offsetY / mid_y) - 1.0;

        rel_y /= aspect;

        offset_x -= rel_x * rel_z;
        offset_y += rel_y * rel_z;
        
        if (zoom < 0.0) {
            zoom = 0.0;
        }

        draw();
    }, false);

    canvas.addEventListener("mousedown", (e) => {
        dragging = true;
        last_x = e.offsetX;
        last_y = e.offsetY;
    }, false);
    

    canvas.addEventListener("mouseup", (e) => {
        dragging = false;
    }, false);

    canvas.addEventListener("mouseleave", (e) => {
        dragging = false;
    }, false);

    canvas.addEventListener('mousemove', (e) => {
        if (dragging) {
            offset_x += (last_x - e.offsetX) / 1000.0 * zoom;
            offset_y -= (last_y - e.offsetY) / 1000.0 * zoom;

            draw();

            last_x = e.offsetX;
            last_y = e.offsetY;
        }
    }, false);

    if (gl === null) {
        alert("Failed to create WebGL context!");
    }

    aspect = canvas.width / canvas.height;

    // Set clear color to black, fully opaque
    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    // Clear the color buffer with specified clear color
    gl.clear(gl.COLOR_BUFFER_BIT);

    // Load the shaders
    var vert = createShader(gl, vertexShaderSource, gl.VERTEX_SHADER);
    var frag = createShader(gl, fragmentShaderSource, gl.FRAGMENT_SHADER);

    var prog = gl.createProgram();
    gl.attachShader(prog, vert);
    gl.attachShader(prog, frag);

    gl.linkProgram(prog);
    gl.useProgram(prog);

    // VBO
    var vbo = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
    gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);

    var a_position = gl.getAttribLocation(prog, 'position');
    if (a_position < 0) {
        console.log('Failed to get the storage location of position');
        return -1;
    }

    var a_uv = gl.getAttribLocation(prog, 'uv0');
    if (a_uv < 0) {
        console.log('Failed to get the storage location of uv0');
        return -1;
    }

    u_info = gl.getUniformLocation(prog, "info");
    if (u_info < 0) {
        console.log('Failed to get the storage location of info');
        return -1;
    }

    u_precise = gl.getUniformLocation(prog, "precise");
    if (u_precise < 0) {
        console.log('Failed to get the storage location of precise');
        return -1;
    }

    var byte = 4; // IEEE Float32

    gl.vertexAttribPointer(a_position, 3, gl.FLOAT, false, byte * 5, 0);
    gl.vertexAttribPointer(a_uv, 2, gl.FLOAT, false, byte * 5, byte * 3);

    gl.enableVertexAttribArray(a_position);
    gl.enableVertexAttribArray(a_uv);

    // IBO
    var ibo = gl.createBuffer();
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, ibo);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, indices, gl.STATIC_DRAW);

    draw();
}

window.onload = main;
window.onresize = (e) => {
    resize();
    draw();
};