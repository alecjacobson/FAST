// Visualize weights on a mesh. Each weight function gets a pseudocolor and
// these colors are belnded according to value of weights.

// Necessary to use mat4x3 syntax for non-square matrices, otherwise compiler
// treats code as version 110 which does not support non-square matrices
#version 120


#define MAT4
#define MAX_NUM_HANDLES 100
#define MAX_NUM_WEIGHTS_PER_VERTEX 8

varying vec4 diffuse,ambient;
varying vec3 normal,lightDir,halfVector;
attribute vec4 weights0;
attribute vec4 weights4;
attribute vec4 weight_indices0;
attribute vec4 weight_indices4;
// Number of handles
uniform int num_handles;

int min(int a, int b) { return a < b ? a : b; }

void main()
{

  //////////////////////////////////////////////////////////////////////////
  // Vertex position
  //////////////////////////////////////////////////////////////////////////
  vec4 v = gl_Vertex;
  // apply model view and projection matrix
  gl_Position =  gl_ModelViewProjectionMatrix * v;
  ////////////////////////////////////////////////////////////////////////////
  //// Phong shading (per vertex colors)
  ////////////////////////////////////////////////////////////////////////////
  // Color table
  vec4 colors[12];
  colors[0] = vec4(1.0,0.0,0.0,1.0);
  colors[1] = vec4(0.0,1.0,0.0,1.0);
  colors[2] = vec4(0.0,0.0,1.0,1.0);
  colors[3] = vec4(0.0,1.0,1.0,1.0);
  colors[4] = vec4(1.0,0.0,1.0,1.0);
  colors[5] = vec4(1.0,1.0,0.0,1.0);
  colors[6] = vec4(1.0,1.0,1.0,1.0);
  colors[7] = vec4(0.1,0.1,0.1,1.0);
  colors[8] = vec4(0.7,0.7,0.7,1.0);
  colors[9] = vec4(1.0,0.5,0.5,1.0);
  colors[10] = vec4(0.5,1.0,0.5,1.0);
  colors[11] = vec4(0.5,0.5,1.0,1.0);
  vec4 color = vec4(0.0,0.0,0.0,0.0);
  // Number of influencing handles
  int m = int(min(num_handles,MAX_NUM_WEIGHTS_PER_VERTEX));

  // First 4
  for(int k = 0; k<min(m,4);k++)
  {
    // influencing handle index
    int i = int(weight_indices0[k]);
    color +=
      colors[i]*weights0[k];
  }
  // Next 4
  for(int k = 4; k<min(m,8);k++)
  {
    // influencing handle index
    int i = int(weight_indices4[k-4]);
    color +=
      colors[i]*weights4[k-4];
  }
  normal = normalize(gl_NormalMatrix * gl_Normal);
  lightDir = normalize(vec3(gl_LightSource[0].position));
  halfVector = normalize(gl_LightSource[0].halfVector.xyz);
  float NdotL = dot(normal,lightDir);
  gl_FrontColor = 
    color*gl_LightSource[0].diffuse*NdotL + 
    gl_FrontMaterial.ambient * gl_LightSource[0].ambient + 
    gl_LightModel.ambient * gl_FrontMaterial.ambient;
} 
