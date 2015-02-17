// Visualize weights on a mesh. Each weight function gets a pseudocolor and
// these colors are belnded according to value of weights.


// Necessary to use mat4x3 syntax for non-square matrices, otherwise compiler
// treats code as version 110 which does not support non-square matrices
#version 120


#define MAX_NUM_HANDLES 100
#define MAX_NUM_WEIGHTS_PER_VERTEX 8

#define MAT4
// Allow exactly one of MAT4, MAT4x3 or MAT3x4 to be defined, defaulting to
// MAT4
#if defined( MAT4x3 )
#  ifdef MAT3x4
#    error
#  endif
#  ifdef MAT4
#    error
#  endif
#elif defined( MAT3x4 )
#  ifdef MAT4x3
#    error
#  endif
#  ifdef MAT4
#    error
#  endif
#elif defined( MAT4 )
#  ifdef MAT4x3
#    error
#  endif
#  ifdef MAT3x4
#    error
#  endif
#else
#  define MAT4
#endif

varying vec4 diffuse,ambient;
varying vec3 normal,lightDir,halfVector;
//attribute vec4 weight_indices[MAX_NUM_WEIGHTS_PER_VERTEX/4];
//attribute vec4 weights[MAX_NUM_WEIGHTS_PER_VERTEX/4];
attribute vec4 weights0;
attribute vec4 weights4;
attribute vec4 weight_indices0;
attribute vec4 weight_indices4;
// List of transformations, one for each handle
#if defined( MAT4 )
uniform mat4 handle_transformation[MAX_NUM_HANDLES];
#elif defined( MAT4x3 )
uniform mat4x3 handle_transformation[MAX_NUM_HANDLES];
#elif defined( MAT3x4 )
uniform mat3x4 handle_transformation[MAX_NUM_HANDLES];
#endif
// Number of handles
uniform int num_handles;

int min(int a, int b) { return a < b ? a : b; }

// Useful sources:
// http://www.opengl.org/sdk/docs/man/xhtml/glUniform.xml 
//   mat4x3 --> 3 rows, 4 columns
// http://www.opengl.org/sdk/docs/tutorials/ClockworkCoders/attributes.php
// http://www.opengl.org/registry/doc/GLSLangSpec.Full.1.20.8.pdf 
// http://gaim.umbc.edu/2010/06/24/nonlinear-transformations/
void main()
{

  //// Debugger widget
  //if(num_handles == 1000.0)
  //{
  //}else
  //{
  //}

  // Number of influencing handles
  int m = int(min(num_handles,MAX_NUM_WEIGHTS_PER_VERTEX));

  //////////////////////////////////////////////////////////////////////////
  // LBS
  //////////////////////////////////////////////////////////////////////////

  // Alter vertex position
#if defined( MAT4 )
  vec4 v = vec4(0.0,0.0,0.0,0.0);
#elif defined( MAT4x3 )
  vec3 v = vec3(0.0,0.0,0.0);
#elif defined( MAT3x4 )
  vec3 v = vec3(0.0,0.0,0.0);
#endif

  // First 4
  int min_m4 = int(min(m,4));
  for(int k = 0;k<min_m4;k++)
  {
    // influencing handle index
    int i = int(weight_indices0[k]);
#if defined( MAT4 )
    v += weights0[k]*(handle_transformation[i] * gl_Vertex);
#elif defined( MAT4x3 )
    v += weights0[k]*(handle_transformation[i] * gl_Vertex);
#elif defined( MAT3x4 )
    v += weights0[k]*(gl_Vertex * handle_transformation[i]);
#endif
  }

  // Next 4
  int min_m8_4 = int(min(m,8))-4;
  for(int k = 0;k<min_m8_4;k++)
  {
    // influencing handle index
    int i = int(weight_indices4[k]);
#if defined( MAT4 )
    v += weights4[k]*(handle_transformation[i] * gl_Vertex);
#elif defined( MAT4x3 )
    v += weights4[k]*(handle_transformation[i] * gl_Vertex);
#elif defined( MAT3x4 )
    v += weights4[k]*(gl_Vertex * handle_transformation[i]);
#endif
  }

  //v.xyz = gl_Vertex.xyz;

  // Finally, apply model view and projection matrix
#if defined( MAT4 )
  gl_Position =  gl_ModelViewProjectionMatrix * v;
#elif defined( MAT4x3 )
  gl_Position =  gl_ModelViewProjectionMatrix * vec4(v.xyz,1.0);
#elif defined( MAT3x4 )
  gl_Position =  gl_ModelViewProjectionMatrix * vec4(v.xyz,1.0);
#endif

  //color.xyz = handle_transformation[1][2].xyz;
 // color.x = handle_transformation[0][0][3];
 // color.y = handle_transformation[0][1][3];
 // color.z = handle_transformation[0][2][3];

  ////////////////////////////////////////////////////////////////////////////
  //// Phong shading (per vertex colors)
  ////////////////////////////////////////////////////////////////////////////
  //vec4 color;
  //color = gl_Color;
  //normal = normalize(gl_NormalMatrix * gl_Normal);
  //lightDir = normalize(vec3(gl_LightSource[0].position));
  //halfVector = normalize(gl_LightSource[0].halfVector.xyz);
  ////diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;
  //diffuse = color * gl_LightSource[0].diffuse;
  //ambient = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;
  ////ambient = gl_Color * gl_LightSource[0].ambient;
  //ambient += gl_LightModel.ambient * gl_FrontMaterial.ambient;
  //// Tell fragment shader what the color is
  //gl_FrontColor = color;

  //// Color table
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
  //color.xyz = handle_transformation[0][0].xyz;
  //color = vec4(1.0,0.0,0.0,0.0);
    //colors[int(weight_indices[i][0])]*weights[i][0];
  //color.x = weights[asdf][0];
  // First 4
  for(int k = 0; k<min(m,4);k++)
  {
    // influencing handle index
    int i = int(weight_indices0[k]);
    color +=
      colors[i]*weights0[k];
    //color = handle_transformation[k][1];
  }
  // Next 4
  for(int k = 4; k<min(m,8);k++)
  {
    // influencing handle index
    int i = int(weight_indices4[k-4]);
    color +=
      colors[i]*weights4[k-4];
    //color = handle_transformation[k][1];
  }
  normal = normalize(gl_NormalMatrix * gl_Normal);
  lightDir = normalize(vec3(gl_LightSource[0].position));
  halfVector = normalize(gl_LightSource[0].halfVector.xyz);
  float NdotL = dot(normal,lightDir);
  gl_FrontColor = 
    color*gl_LightSource[0].diffuse*NdotL + 
    gl_FrontMaterial.ambient * gl_LightSource[0].ambient + 
    gl_LightModel.ambient * gl_FrontMaterial.ambient;
    //v.xyz = gl_Vertex.xyz;
} 
