@@ renderv

attribute vec2 vPos;
attribute vec2 aUV;
varying vec2 vUV;

void main() {
    vUV = aUV;
    gl_Position = vec4(vPos, 0.0, 1.0);
}

@@ renderf

#ifdef GL_ES
precision mediump float;
#endif
uniform sampler2D uT;
varying vec2 vUV;

void main() {
  gl_FragColor = texture2D(uT, vUV);
}

@@ maskedf

#ifdef GL_ES
precision mediump float;
#endif
uniform sampler2D uT;
varying vec2 vUV;
uniform vec3 uO; // the uniform offsets

void main() {
  vec4 raw = texture2D(uT, vec2(vUV.x, vUV.y * uO.x));
  vec4 mask = texture2D(uT, vec2(vUV.x * uO.y, vUV.y * uO.x * uO.y + uO.z));
  gl_FragColor = raw * mask;
}

@@ simpleblurf

#ifdef GL_ES
precision mediump float;
#endif
uniform sampler2D uT;
varying vec2 vUV;
uniform vec2 uDir;
uniform float uRad, uRes;

void main() {
  vec4 sum = vec4(0.0);
  vec2 tc = vUV;

  float blur = uRad * uRes;

  float hstep = uDir.x * blur;
  float vstep = uDir.y * blur;

  sum += texture2D(uT, vec2(tc.x - 4.0 * hstep, tc.y - 4.0 * vstep)) * 0.0162162162;
  sum += texture2D(uT, vec2(tc.x - 3.0 * hstep, tc.y - 3.0 * vstep)) * 0.0540540541;
  sum += texture2D(uT, vec2(tc.x - 2.0 * hstep, tc.y - 2.0 * vstep)) * 0.1216216216;
  sum += texture2D(uT, vec2(tc.x - hstep, tc.y - vstep)) * 0.1945945946;

  sum += texture2D(uT, vec2(tc.x, tc.y)) * 0.2270270270;

  sum += texture2D(uT, vec2(tc.x + hstep, tc.y + vstep)) * 0.1945945946;
  sum += texture2D(uT, vec2(tc.x + 2.0 * hstep, tc.y + 2.0 * vstep)) * 0.1216216216;
  sum += texture2D(uT, vec2(tc.x + 3.0 * hstep, tc.y + 3.0 * vstep)) * 0.0540540541;
  sum += texture2D(uT, vec2(tc.x + 4.0 * hstep, tc.y + 4.0 * vstep)) * 0.0162162162;

  gl_FragColor = sum;
}

@@ normv

attribute vec2 vPos;
uniform mat4 uPM, uVM, uMM; // projection, view, model
attribute vec2 aUV;
varying vec2 vUV;

void main() {
    vUV = aUV;
    gl_Position = uPM * uVM * uMM * vec4(vPos, 0.0, 1.0);
}

@@ normf

#ifdef GL_ES
precision mediump float;
#endif
uniform sampler2D uT;
// uniform bool drawT;
uniform vec4 uCol;
varying vec2 vUV;

void main() {
  // if (drawHl) {
  //     vec4 color = vec4(1, 1, 1, 0);
  //     if (texture2D(uT, vUV).w < 0.95) {
  // 		// ivec2 textureSize2d = textureSize(uT,0);
  // 		// float ts = float((textureSize2d.x + textureSize2d.y) / 2);
  // 		int sum = 0;
  // 		for(int j=-1; j<=1; ++j)
  // 		for(int i=-1; i<=1; ++i)
  // 		if (i != j) {
  // 			vec2 shift = vec2(float(i) * uTSx.x * 4.0, float(j) * uTSx.y
  // * 4.0); 			float value = texture2D(uT, vUV + shift).w; 			color.w += value;
  // 			++sum;
  // 		}
  // 		color.w /= float(sum);
  // 	}
  // 	gl_FragColor = color * vCol;
  // }
  // else
  gl_FragColor = texture2D(uT, vUV) * uCol;
}

@@ gradientv

attribute vec2 vPos;
uniform mat4 uPM, uVM, uMM; // projection, view, model
attribute float aA;
varying float vA;

void main() {
    vA = aA;
    gl_Position = uPM * uVM * uMM * vec4(vPos, 0.0, 1.0);
}

@@ gradientf

#ifdef GL_ES
precision mediump float;
#endif
uniform vec4 uCol, uCo2;
varying float vA;

void main() {
  gl_FragColor = uCol + (uCo2 - uCol) * vA;
}

@@ doublesolidv

attribute vec2 vPos, vDest;
uniform mat4 uPM, uVM, uMM; // projection, view, model
uniform float uIv;  // interpolate value (between 0 and 1)

void main() {
    vec2 pos = vPos + (vDest - vPos) * uIv;
    gl_Position = uPM * uVM * uMM * vec4(pos, 0.0, 1.0);
}

@@ doublenormv

attribute vec2 vPos, vDest, aUV;
uniform mat4 uPM, uVM, uMM; // projection, view, model
uniform float uIv;  // interpolate value (between 0 and 1)
varying vec2 vUV;

void main() {
    vUV = aUV;
    vec2 pos = vPos + (vDest - vPos) * uIv;
    gl_Position = uPM * uVM * uMM * vec4(pos, 0.0, 1.0);
}

@@ solidv

attribute vec2 vPos;
uniform mat4 uPM, uVM, uMM;

void main() {
    gl_Position = uPM * uVM * uMM * vec4(vPos, 0.0, 1.0);
}

@@ solidf

#ifdef GL_ES
precision mediump float;
#endif
uniform vec4 uCol;

void main() {
    gl_FragColor = uCol;
}

@@ noisef  // linked with normv

#ifdef GL_ES
precision mediump float;
#endif
uniform vec4 uCol, uCo2, uCo3;
varying vec2 vUV;
uniform vec2 uSeed;

float rand(vec2 co)  // straight up textbook rand
{
    float a = 12.9898;
    float b = 78.233;
    float c = 43758.5453;
    float dt= dot(co.xy ,vec2(a,b));
    float sn= mod(dt,3.14);
    return fract(sin(sn) * c);
}

void main() {
    float dist = min(distance(vUV, vec2(0.5, 0.5)) * 1.49, 1.0);
    vec4 sum = uCo2 + (uCo3 - uCo2) * dist;
    gl_FragColor = uCol + (sum - uCol) * rand(vUV + uSeed);
}

@@ textv

attribute vec2 vPos;
uniform mat4 uPM, uVM, uMM; // projection, scale (font-size), model
uniform vec2 uOf;
attribute vec2 aUV;
varying vec2 vUV;

void main() {
    vUV = aUV;
    gl_Position = uPM * uVM * uMM * vec4(vPos + uOf, 0.0, 1.0);
}

@@ textf

#ifdef GL_ES
precision mediump float;
#endif
uniform sampler2D uT;
uniform vec4 uCol;
varying vec2 vUV;

void main() {
  float a = texture2D(uT, vUV).x;
  float c = 0.8 + (a * 0.2);
  gl_FragColor = vec4(c, c, c, a) * uCol; // swizzling won't work on earlier OpenGL
}

@@ fullbgf

#ifdef GL_ES
precision mediump float;
#endif
uniform sampler2D uT;
uniform float uCol;
uniform vec2 uR; // resolution
// varying vec2 vUV;
uniform float uI; // iterate

void main() {
    // between -1 & 1
    vec2 uv = gl_FragCoord.xy / uR;
    uv.x *= uR.x / uR.y;
    uv.x -= uI;
    uv.y += uI;
    gl_FragColor = texture2D(uT, uv) * vec4(1, 1, 1, uCol);
}
