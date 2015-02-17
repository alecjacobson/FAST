// Necessary to use mat4x3 syntax for non-square matrices, otherwise compiler
// treats code as version 110 which does not support non-square matrices
#version 120

// 61 goes into software renderer mode
#define MAX_NUM_HANDLES 60
#define MAX_NUM_WEIGHTS_PER_VERTEX 16

// Transform normals by rotation portion of each transformation weighted by
// corresponding skinning weight: a poor man's inverse transpose of the jacobian
// (assumes transformations are locally rigid)
#define TRANSFORM_NORMALS

//#define NO_INDEXING_BY_VARIABLES
#ifdef NO_INDEXING_BY_VARIABLES
#  define IM_MAD_AS_HELL_AND_IM_NOT_GOING_TO_TAKE_IT_ANYMORE
#endif

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
varying vec3 normal;//,lightDir,halfVector;
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

// Functions
int min(int a, int b) { return a < b ? a : b; }

#ifdef NO_INDEXING_BY_VARIABLES
// Hack to return handle_transformation[i]
// Input:
//   i  index into handle_transformation
// Returns handle_transformation[i]
mat4 handle_transformation_at(int i);
#endif

// per vertex color
vec4 color = vec4(0.0,0.0,0.0,0.0);

// Useful sources:
// http://www.opengl.org/sdk/docs/man/xhtml/glUniform.xml 
//   mat4x3 --> 3 rows, 4 columns
// http://www.opengl.org/sdk/docs/tutorials/ClockworkCoders/attributes.php
// http://www.opengl.org/registry/doc/GLSLangSpec.Full.1.20.8.pdf 
// http://gaim.umbc.edu/2010/06/24/nonlinear-transformations/
void main()
{

  color = gl_Color;
  //color = gl_FrontMaterial.diffuse;

  // Number of influencing handles
  int m = int(min(num_handles,MAX_NUM_WEIGHTS_PER_VERTEX));

  //////////////////////////////////////////////////////////////////////////
  // LBS
  //////////////////////////////////////////////////////////////////////////

#ifdef TRANSFORM_NORMALS
  vec4 n = vec4(0.0,0.0,0.0,0.0);
#endif

  // Alter vertex position
#if defined( MAT4 )
  vec4 v = vec4(0.0,0.0,0.0,0.0);
#elif defined( MAT4x3 )
  vec3 v = vec3(0.0,0.0,0.0);
#elif defined( MAT3x4 )
  vec3 v = vec3(0.0,0.0,0.0);
#endif

  int k_max = int(min(m,0+4))-(0);
  for(int k = 0;k<k_max;k++)
  {
    // influencing handle index
    int i = int(weight_indices0[k]);
#if defined (NO_INDEXING_BY_VARIABLES)
    mat4 Ti = handle_transformation_at(i);
#else
    mat4 Ti = handle_transformation[i];
#endif

#if defined( MAT4 )
    v += weights0[k]*(Ti * gl_Vertex);
#  ifdef TRANSFORM_NORMALS
    n += weights0[k]*(Ti * vec4(gl_Normal.xyz,0));
#  endif
#elif defined( MAT4x3 )
    v += weights0[k]*(Ti * gl_Vertex);
#elif defined( MAT3x4 )
    v += weights0[k]*(gl_Vertex * Ti);
#endif
  }


#if MAX_NUM_WEIGHTS_PER_VERTEX > 4
  // Next 4
  k_max = int(min(m,4+4))-(4);
  for(int k = 0;k<k_max;k++)
  {
    // influencing handle index
    int i = int(weight_indices4[k]);
#if defined (NO_INDEXING_BY_VARIABLES)
    mat4 Ti = handle_transformation_at(i);
#else
    mat4 Ti = handle_transformation[i];
#endif
#if defined( MAT4 )
    v += weights4[k]*(Ti * gl_Vertex);
#  ifdef TRANSFORM_NORMALS
    n += weights4[k]*(Ti * vec4(gl_Normal.xyz,0));
#  endif
#elif defined( MAT4x3 )
    v += weights4[k]*(Ti * gl_Vertex);
#elif defined( MAT3x4 )
    v += weights4[k]*(gl_Vertex * handle_transformation[i]);
#endif
  }
#endif


#if MAX_NUM_WEIGHTS_PER_VERTEX > 8
  // Next 4
  k_max = int(min(m,8+4))-(8);
  for(int k = 0;k<k_max;k++)
  {
    // influencing handle index
    int i = int(weight_indices8[k]);
#if defined (NO_INDEXING_BY_VARIABLES)
    mat4 Ti = handle_transformation_at(i);
#else
    mat4 Ti = handle_transformation[i];
#endif
#if defined( MAT4 )
    v += weights8[k]*(Ti * gl_Vertex);
#  ifdef TRANSFORM_NORMALS
    n += weights8[k]*(Ti * vec4(gl_Normal.xyz,0));
#  endif
#elif defined( MAT4x3 )
    v += weights8[k]*(Ti * gl_Vertex);
#elif defined( MAT3x4 )
    v += weights8[k]*(gl_Vertex * handle_transformation[i]);
#endif
  }
#endif

#if MAX_NUM_WEIGHTS_PER_VERTEX > 12
  // Next 4
  k_max = int(min(m,12+4))-(12);
  for(int k = 0;k<k_max;k++)
  {
    // influencing handle index
    int i = int(weight_indices12[k]);
#if defined (NO_INDEXING_BY_VARIABLES)
    mat4 Ti = handle_transformation_at(i);
#else
    mat4 Ti = handle_transformation[i];
#endif
#if defined( MAT4 )
    v += weights12[k]*(Ti * gl_Vertex);
#  ifdef TRANSFORM_NORMALS
    n += weights12[k]*(Ti * vec4(gl_Normal.xyz,0));
#  endif
#elif defined( MAT4x3 )
    v += weights12[k]*(Ti * gl_Vertex);
#elif defined( MAT3x4 )
    v += weights12[k]*(gl_Vertex * handle_transformation[i]);
#endif
  }
#endif

  // Force homogenous w-coordinate to be one (same as ignoring LBS of
  // w-coordinate, which could be garbage if weights don't sum to one for
  // example)
  v.w = 1;

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
  //vec4 colors[12];
  //colors[0] = vec4(1.0,0.0,0.0,1.0);
  //colors[1] = vec4(0.0,1.0,0.0,1.0);
  //colors[2] = vec4(0.0,0.0,1.0,1.0);
  //colors[3] = vec4(0.0,1.0,1.0,1.0);
  //colors[4] = vec4(1.0,0.0,1.0,1.0);
  //colors[5] = vec4(1.0,1.0,0.0,1.0);
  //colors[6] = vec4(1.0,1.0,1.0,1.0);
  //colors[7] = vec4(0.1,0.1,0.1,1.0);
  //colors[8] = vec4(0.7,0.7,0.7,1.0);
  //colors[9] = vec4(1.0,0.5,0.5,1.0);
  //colors[10] = vec4(0.5,1.0,0.5,1.0);
  //colors[11] = vec4(0.5,0.5,1.0,1.0);

  ////color.xyz = handle_transformation[0][0].xyz;
  ////color = vec4(1.0,0.0,0.0,0.0);
  //  //colors[int(weight_indices[i][0])]*weights[i][0];
  ////color.x = weights[asdf][0];
  //// First 4
  //for(int k = 0; k<min(m,4);k++)
  //{
  //  // influencing handle index
  //  int i = int(weight_indices0[k]);
  //  color +=
  //    colors[i]*weights0[k];
  //  //color = handle_transformation[k][1];
  //}
  //// Next 4
  //for(int k = 4; k<min(m,8);k++)
  //{
  //  // influencing handle index
  //  int i = int(weight_indices4[k-4]);
  //  color +=
  //    colors[i]*weights4[k-4];
  //  //color = handle_transformation[k][1];
  //}
#ifdef TRANSFORM_NORMALS
  normal = normalize(gl_NormalMatrix * n.xyz);
#else
  normal = normalize(gl_NormalMatrix * gl_Normal);
#endif

    //lightDir = normalize(vec3(gl_LightSource[0].position));
	
    //halfVector = normalize(gl_LightSource[0].halfVector.xyz);
    
    //diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;
    diffuse = vec4(0,0,0,0);
    ambient = vec4(0,0,0,0);
    const float num_lights = 2.0;
    //int light = 0;
    // AMD SUX0R5
    //for (int light=0; light<int(num_lights); light++)
    //{
      diffuse += (1.0/num_lights)*(color * gl_LightSource[0].diffuse);
      ambient += (1.0/num_lights)*(gl_FrontMaterial.ambient * gl_LightSource[0].ambient);
      ambient += (1.0/num_lights)*(gl_LightModel.ambient * gl_FrontMaterial.ambient);
      diffuse += (1.0/num_lights)*(color * gl_LightSource[1].diffuse);
      ambient += (1.0/num_lights)*(gl_FrontMaterial.ambient * gl_LightSource[1].ambient);
      ambient += (1.0/num_lights)*(gl_LightModel.ambient * gl_FrontMaterial.ambient);
    //}
	
} 


#ifdef NO_INDEXING_BY_VARIABLES
mat4 handle_transformation_at(int i)
{
  //(1..10).each{|e| puts \"else if(i==#{e}) return handle_transformation[#{e}];\"}
  if(i==0) return handle_transformation[0];
  //if(i<=2)
  //{
  //  if(i==1) return handle_transformation[1];
  //  return handle_transformation[2];
  //}

  //if(i<=6)
  //{
  //  if(i<=4)
  //  {
  //    if(i==3) return handle_transformation[3];
  //    return handle_transformation[4];
  //  }
  //  if(i==5) return handle_transformation[5];
  //  return handle_transformation[6];
  //}

  //if(i<=14)
  //{
  //  if(i<=10)
  //  {
  //    if(i<=8)
  //    {
  //      if(i==7) return handle_transformation[7];
  //      return handle_transformation[8];
  //    }
  //    if(i==9) return handle_transformation[9];
  //    return handle_transformation[10];
  //  }
  //  if(i<=12)
  //  {
  //    if(i==11) return handle_transformation[11];
  //    return handle_transformation[12];
  //  }
  //  if(i==13) return handle_transformation[13];
  //  return handle_transformation[14];
  //}

  //
  //if(i<=30)
  //{
  //  if(i<=22)
  //  {
  //    if(i<=18)
  //    {
  //      if(i<=16)
  //      {
  //        if(i==15) return handle_transformation[15];
  //        return handle_transformation[16];
  //      }
  //      if(i==17) return handle_transformation[17];
  //      return handle_transformation[18];
  //    }
  //    if(i<=20)
  //    {
  //      if(i==19) return handle_transformation[19];
  //      return handle_transformation[20];
  //    }
  //    if(i==21) return handle_transformation[21];
  //    return handle_transformation[22];
  //  }
  //  if(i<=26)
  //  {
  //    if(i<=24)
  //    {
  //      if(i==23) return handle_transformation[23];
  //      return handle_transformation[24];
  //    }
  //    if(i==25) return handle_transformation[25];
  //    return handle_transformation[26];
  //  }
  //  if(i<=28)
  //  {
  //    if(i==27) return handle_transformation[27];
  //    return handle_transformation[28];
  //  }
  //  if(i==29) return handle_transformation[29];
  //  return handle_transformation[30];
  //}
  //if(i==31) return handle_transformation[31];
  //if(i==32) return handle_transformation[32];

  if(i==0) return handle_transformation[0];
  else if(i==1) return handle_transformation[1];
  else if(i==2) return handle_transformation[2];
  else if(i==3) return handle_transformation[3];
  else if(i==4) return handle_transformation[4];
  else if(i==5) return handle_transformation[5];
  else if(i==6) return handle_transformation[6];
  else if(i==7) return handle_transformation[7];
  else if(i==8) return handle_transformation[8];
  else if(i==9) return handle_transformation[9];
  else if(i==10) return handle_transformation[10];
  else if(i==11) return handle_transformation[11];
  else if(i==12) return handle_transformation[12];
  else if(i==13) return handle_transformation[13];
  else if(i==14) return handle_transformation[14];
  else if(i==15) return handle_transformation[15];
  else if(i==16) return handle_transformation[16];
  else if(i==17) return handle_transformation[17];
  else if(i==18) return handle_transformation[18];
  else if(i==19) return handle_transformation[19];
  else if(i==20) return handle_transformation[20];
  else if(i==21) return handle_transformation[21];
  else if(i==22) return handle_transformation[22];
  else if(i==23) return handle_transformation[23];
  else if(i==24) return handle_transformation[24];
// You can always add more but things start to slow down and maybe even switch
// to software renderer if there are too many lines here
#if MAX_NUM_HANDLES > 25
  else if(i==25) return handle_transformation[25];
  else if(i==26) return handle_transformation[26];
  else if(i==27) return handle_transformation[27];
  else if(i==28) return handle_transformation[28];
  else if(i==29) return handle_transformation[29];
  else if(i==30) return handle_transformation[30];
  else if(i==31) return handle_transformation[31];
  else if(i==32) return handle_transformation[32];
  //ERROR: 0:505: Index 100 beyond bounds (size 100)
  //else if(i==100) return handle_transformation[100];
#endif
  color = vec4(1.0,0.0,1.0,1.0);
  // dummy base case, be nice if I could make it crash if it ever got here
  return handle_transformation[0];
}
#endif
