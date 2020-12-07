@@ renderv

attribute vec2 attr_pos;
attribute vec2 attr_mapped_vec;
varying vec2 var_mapped_vec;

void main() {
    var_mapped_vec = attr_mapped_vec;
    gl_Position = vec4(attr_pos, 0.0, 1.0);
}

@@ renderf

#ifdef GL_ES
precision mediump float;
#endif
uniform sampler2D u_tex;
varying vec2 var_mapped_vec;

void main() {
  gl_FragColor = texture2D(u_tex, var_mapped_vec);
}

@@ maskedf

#ifdef GL_ES
precision mediump float;
#endif
uniform sampler2D u_tex;
varying vec2 var_mapped_vec;
uniform vec4 u_offset;

void main() {
  vec2 coords = vec2(var_mapped_vec.x, var_mapped_vec.y * u_offset.x);
  vec2 dis_coords = coords * u_offset.y + vec2(u_offset.w, u_offset.z);

  vec4 dis = texture2D(u_tex, dis_coords);
  vec2 dis_shift = vec2(dis.r - 0.5, (dis.g - 0.5) * u_offset.x) / 6.0;

  coords = vec2(
              max(0.0, min(2.991 * u_offset.w,
                      coords.x - dis_shift.x)),
              max(0.0, min(0.999 * u_offset.z,
                      coords.y + dis_shift.y))
            );

  vec2 mask_coords = coords * u_offset.y;
  mask_coords.y += u_offset.z;

  vec4 raw = texture2D(u_tex, coords);
  vec4 mask = texture2D(u_tex, mask_coords);

  float grey_val = (raw.x + raw.y + raw.z) / 2.8;
  float sat = (mask.x + mask.y + mask.z) / 3.0;

  vec4 grey = vec4(grey_val, grey_val, grey_val * 1.1, 1.0);
  vec4 modified = grey + (raw - grey) * sat;

  gl_FragColor = modified * mask;
}

@@ simpleblurf

#ifdef GL_ES
precision mediump float;
#endif
uniform sampler2D u_tex;
varying vec2 var_mapped_vec;
uniform vec2 u_direction;
uniform float u_radius, u_resolution;

void main() {
  vec4 sum = vec4(0.0);
  vec2 tc = var_mapped_vec;

  float blur = u_radius * u_resolution;

  float hstep = u_direction.x * blur;
  float vstep = u_direction.y * blur;

  sum += texture2D(u_tex, vec2(tc.x - 4.0 * hstep, tc.y - 4.0 * vstep)) * 0.0162162162;
  sum += texture2D(u_tex, vec2(tc.x - 3.0 * hstep, tc.y - 3.0 * vstep)) * 0.0540540541;
  sum += texture2D(u_tex, vec2(tc.x - 2.0 * hstep, tc.y - 2.0 * vstep)) * 0.1216216216;
  sum += texture2D(u_tex, vec2(tc.x - hstep, tc.y - vstep)) * 0.1945945946;

  sum += texture2D(u_tex, vec2(tc.x, tc.y)) * 0.2270270270;

  sum += texture2D(u_tex, vec2(tc.x + hstep, tc.y + vstep)) * 0.1945945946;
  sum += texture2D(u_tex, vec2(tc.x + 2.0 * hstep, tc.y + 2.0 * vstep)) * 0.1216216216;
  sum += texture2D(u_tex, vec2(tc.x + 3.0 * hstep, tc.y + 3.0 * vstep)) * 0.0540540541;
  sum += texture2D(u_tex, vec2(tc.x + 4.0 * hstep, tc.y + 4.0 * vstep)) * 0.0162162162;

  gl_FragColor = sum;
}

@@ normv

attribute vec2 attr_pos;
uniform mat4 u_projm, u_viewm, u_modelm; // projection, view, model
attribute vec2 attr_mapped_vec;
varying vec2 var_mapped_vec;

void main() {
    var_mapped_vec = attr_mapped_vec;
    gl_Position = u_projm * u_viewm * u_modelm * vec4(attr_pos, 0.0, 1.0);
}

@@ normf

#ifdef GL_ES
precision mediump float;
#endif
uniform sampler2D u_tex;

uniform vec4 u_color;
varying vec2 var_mapped_vec;

void main() {
  gl_FragColor = texture2D(u_tex, var_mapped_vec) * u_color;
}

@@ gradientv

attribute vec2 attr_pos;
uniform mat4 u_projm, u_viewm, u_modelm; // projection, view, model
attribute float attr_gradient;
varying float var_gradient;

void main() {
    var_gradient = attr_gradient;
    gl_Position = u_projm * u_viewm * u_modelm * vec4(attr_pos, 0.0, 1.0);
}

@@ gradientf

#ifdef GL_ES
precision lowp float;
#endif
uniform vec4 u_color, u_color_2;
varying float var_gradient;

void main() {
    gl_FragColor = u_color + (u_color_2 - u_color) * var_gradient;
}

@@ doublesolidv

attribute vec2 attr_pos, attr_dest_pos;
uniform mat4 u_projm, u_viewm, u_modelm; // projection, view, model
uniform float u_inter;  // interpolate value (between 0 and 1)

void main() {
    vec2 pos = attr_pos + (attr_dest_pos - attr_pos) * u_inter;
    gl_Position = u_projm * u_viewm * u_modelm * vec4(pos, 0.0, 1.0);
}

@@ doublenormv

attribute vec2 attr_pos, attr_dest_pos, attr_mapped_vec;
uniform mat4 u_projm, u_viewm, u_modelm; // projection, view, model
uniform float u_inter;  // interpolate value (between 0 and 1)
varying vec2 var_mapped_vec;
uniform vec2 u_map_shift1, u_map_shift2, u_map_mult;

void main() {
    var_mapped_vec = (attr_mapped_vec + u_map_shift1) * u_map_mult + u_map_shift2;
    vec2 pos = attr_pos + (attr_dest_pos - attr_pos) * u_inter;
    gl_Position = u_projm * u_viewm * u_modelm * vec4(pos, 0.0, 1.0);
}

@@ solidv

attribute vec2 attr_pos;
uniform mat4 u_projm, u_viewm, u_modelm;

void main() {
    gl_Position = u_projm * u_viewm * u_modelm * vec4(attr_pos, 0.0, 1.0);
}

@@ solidf

#ifdef GL_ES
precision lowp float;
#endif
uniform vec4 u_color;

void main() {
    gl_FragColor = u_color;
}

@@ noisef  // linked with normv

#ifdef GL_ES
precision highp float;
#endif
uniform vec4 u_color, u_color_2, u_color_3, u_color_4;
varying vec2 var_mapped_vec;
uniform vec2 u_seed;

float snoise(vec2 val, float seed) {
    vec2 a = val * 1.61803398874989484820459;
    float b = distance(a, val);
    float c = tan(b * seed);
    return fract(c * val.y);
}

void main() {
    float dist = min(distance(var_mapped_vec, vec2(0.5, 0.5)) * 1.49, 1.0);
    vec4 noisy_color = u_color_3 + (u_color_4 - u_color_3) * dist;
    vec4 base_color = u_color + (u_color_2 - u_color) * dist;
    float noise_value = snoise(var_mapped_vec * 80.0, u_seed.x);
    gl_FragColor = base_color + (noisy_color - base_color) * noise_value;
}

@@ textv

attribute vec2 attr_pos;
uniform mat4 u_projm, u_viewm, u_modelm; // projection, scale (font-size), model
uniform vec2 u_offset;
attribute vec2 attr_mapped_vec;
varying vec2 var_mapped_vec;

void main() {
    var_mapped_vec = attr_mapped_vec;
    gl_Position = u_projm * u_viewm * u_modelm * vec4(attr_pos + u_offset, 0.0, 1.0);
}

@@ textf

#ifdef GL_ES
precision mediump float;
#endif
uniform sampler2D u_tex;
uniform vec4 u_color;
varying vec2 var_mapped_vec;

void main() {
  float a = texture2D(u_tex, var_mapped_vec).x;
  float c = 0.8 + (a * 0.2);
  gl_FragColor = vec4(c, c, c, a) * u_color; // swizzling won't work on earlier OpenGL
}

@@ fullbgf

#ifdef GL_ES
precision mediump float;
#endif
uniform sampler2D u_tex;
uniform float u_color;
uniform vec2 u_resolution;
uniform float u_speed;

void main() {
    // between -1 & 1
    vec2 uv = gl_FragCoord.xy / u_resolution;
    uv.x *= u_resolution.x / u_resolution.y;
    uv.x -= u_speed;
    uv.y += u_speed;
    gl_FragColor = texture2D(u_tex, uv) * vec4(1, 1, 1, u_color);
}
