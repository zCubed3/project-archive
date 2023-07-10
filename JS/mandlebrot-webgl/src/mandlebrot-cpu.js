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
    precision mediump float;

    uniform sampler2D tex;
    varying vec2 uv;

    vec3 hsv2rgb(vec3 c) {
        vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }
    
    void main() {
        float r = texture2D(tex, uv).r;
        gl_FragColor = vec4(hsv2rgb(vec3(1.0 - r, 0.5, 1.0 - r)), 1.0);
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

var u_tex = -1;

var offset_x = 0.0;
var offset_y = 0.0;
var zoom = 1.0;
var aspect = 1.0;

var texture;

var dragging = false;
var last_x = 0.0;
var last_y = 0.0;

const inset = 30;

var gl;
var canvas;

var drawing = null;
var abort = false;

var pixels;

async function draw() {
    gl.viewport(0, 0, canvas.width, canvas.height);

    gl.activeTexture(gl.TEXTURE0);

    texture = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);

    gl.uniform1i(u_tex, 0);

    let width = canvas.width * 4.0;
    let height = canvas.height * 4.0;

    let log2 = Math.log(2.0);

    for (var x = 0; x < width; x += 4) {
        for (var y = 0; y < canvas.height * 4; y += 4) {
            let y_off = y * canvas.width;

            let u = x / width;
            let v = y / height;

            let bx = ((u - 0.5) * 2.0) * zoom;
            let by = ((v - 0.5) * 2.0) * zoom;

            bx += offset_x;
            by += offset_y;

            let iter = 0.0;

            let ix = 0.0;
            let iy = 0.0;
    
            const MAX_ITER = 400.0;
            const BAIL = 256.0;
    
            while ((ix * ix + iy * iy <= BAIL) && (iter < MAX_ITER)) {
                var f = ix * ix - iy * iy + bx;
    
                iy = 2.0 * ix * iy + by;
                ix = f;
    
                iter += 1.0;
            }
    
            if (iter < MAX_ITER) {
                var log_zn = Math.log(ix * ix + iy * iy) / 2.0;
                var nu = Math.log(log_zn / log2) / log2;
    
                iter += 1.0 - nu;
            }

            var f = iter / MAX_ITER;

            pixels[x + y_off] = f * 255;
            pixels[x + y_off + 1] = 0x00;
            pixels[x + y_off + 2] = 0x00;
            pixels[x + y_off + 3] = 0xFF;
        }
    }

    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, canvas.width, canvas.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, pixels);
    gl.drawElements(gl.TRIANGLES, indices.length, gl.UNSIGNED_SHORT, 0);
}

function resize() {
    aspect = window.innerWidth / window.innerHeight;

    //canvas.width = window.innerWidth - inset;
    //canvas.height = window.innerHeight - inset;

    aspect = canvas.width / canvas.height;

    pixels = new Uint8Array(canvas.width * canvas.height * 4);
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

    u_tex = gl.getUniformLocation(prog, "tex");
    if (u_tex < 0) {
        console.log('Failed to get the storage location of tex');
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

    drawing = draw();
}

window.onload = main;
window.onresize = (e) => {
    resize();

    abort = true;
    drawing = draw();
};