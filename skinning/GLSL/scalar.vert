// Visualize weights on a mesh. Each weight function gets a pseudocolor and
// these colors are belnded according to value of weights.

// Necessary to use mat4x3 syntax for non-square matrices, otherwise compiler
// treats code as version 110 which does not support non-square matrices
#version 120


#define MAT4
#define MAX_NUM_HANDLES 100
#define MAX_NUM_WEIGHTS_PER_VERTEX 16

varying vec4 diffuse,ambient;
varying vec3 normal,lightDir,halfVector;
attribute vec4 weights0;
attribute vec4 weight_indices0;
#if MAX_NUM_WEIGHTS_PER_VERTEX > 4
attribute vec4 weights4;
attribute vec4 weight_indices4;
#endif
#if MAX_NUM_WEIGHTS_PER_VERTEX > 8
attribute vec4 weights8;
attribute vec4 weight_indices8;
#endif
#if MAX_NUM_WEIGHTS_PER_VERTEX > 12
attribute vec4 weights12;
attribute vec4 weight_indices12;
#endif
// Number of handles
uniform int num_handles;
// Selected weight function
uniform int selected_weight;

// Functions
int min(int a, int b) { return a < b ? a : b; }
// Return a color based on scalar value
vec4 heat_color(float s)
{
  vec4 zero = vec4(1.0,1.0,1.0,1.0);
  vec4 one = vec4(1.0,0.0,0.0,1.0);
  vec4 plus = vec4(1.0,1.0,0.0,1.0);
  vec4 minus = vec4(0.0,0.0,1.0,1.0);
  if(s >= 2)
  {
    return plus;
  }else if(s >= 1)
  {
    return (1-(s-1))*one + (s-1)*plus;
  }else if(s>=0)
  {
    return (1-s)*zero+ (s)*one;
  }else if(s>=-1)
  {
    return (1-(s+1))*minus+ (s+1)*zero;
  }
  return minus;
}

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

  // Initialize assuming zero weight
  vec4 color = heat_color(0.0);

  // Number of influencing handles
  int m = int(min(num_handles,MAX_NUM_WEIGHTS_PER_VERTEX));

  // First 4
  int k_max = int(min(m,0+4))-(0);
  for(int k = 0; k<k_max;k++)
  {
    // influencing handle index
    int i = int(weight_indices0[k]);
    if(i == selected_weight)
    {
      color = heat_color(weights0[k]);
    }
  }

#if MAX_NUM_WEIGHTS_PER_VERTEX > 4
  // Next 4
  k_max = int(min(m,4+4))-(4);
  for(int k = 0; k<k_max;k++)
  {
    // influencing handle index
    int i = int(weight_indices4[k]);
    if(i == selected_weight)
    {
      color = heat_color(weights4[k]);
    }
  }
#endif

#if MAX_NUM_WEIGHTS_PER_VERTEX > 8
  // Next 4
  k_max = int(min(m,8+4))-(8);
  for(int k = 0; k<k_max;k++)
  {
    // influencing handle index
    int i = int(weight_indices8[k]);
    if(i == selected_weight)
    {
      color = heat_color(weights8[k]);
    }
  }
#endif

#if MAX_NUM_WEIGHTS_PER_VERTEX > 12
  // Next 4
  k_max = int(min(m,12+4))-(12);
  for(int k = 0; k<k_max;k++)
  {
    // influencing handle index
    int i = int(weight_indices12[k]);
    if(i == selected_weight)
    {
      color = heat_color(weights12[k]);
    }
  }
#endif

  normal = normalize(gl_NormalMatrix * gl_Normal);
  lightDir = normalize(vec3(gl_LightSource[0].position));
  halfVector = normalize(gl_LightSource[0].halfVector.xyz);
  float NdotL = dot(normal,lightDir);
  gl_FrontColor = 
    color*gl_LightSource[0].diffuse*NdotL + 
    gl_FrontMaterial.ambient * gl_LightSource[0].ambient + 
    gl_LightModel.ambient * gl_FrontMaterial.ambient;
} 

