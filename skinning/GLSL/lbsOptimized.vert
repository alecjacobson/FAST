// Necessary to use mat4x3 syntax for non-square matrices, otherwise compiler
// treats code as version 110 which does not support non-square matrices
#version 120

// 61 goes into software renderer mode
#define MAX_NUM_HANDLES 60
#define MAX_NUM_WEIGHTS_PER_VERTEX 4

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

mat3 inverseTranspose(mat3 M)
{
	mat3 result;
	float determinant =    +M[0][0]*(M[1][1]*M[2][2]-M[2][1]*M[1][2])
	                        -M[0][1]*(M[1][0]*M[2][2]-M[1][2]*M[2][0])
                        +M[0][2]*(M[1][0]*M[2][1]-M[1][1]*M[2][0]);
	float invdet = 1.0 / determinant;
	result[0][0] =  (M[1][1]*M[2][2]-M[2][1]*M[1][2])*invdet;
	result[1][0] = -(M[0][1]*M[2][2]-M[0][2]*M[2][1])*invdet;
	result[2][0] =  (M[0][1]*M[1][2]-M[0][2]*M[1][1])*invdet;
	result[0][1] = -(M[1][0]*M[2][2]-M[1][2]*M[2][0])*invdet;
	result[1][1] =  (M[0][0]*M[2][2]-M[0][2]*M[2][0])*invdet;
	result[2][1] = -(M[0][0]*M[1][2]-M[1][0]*M[0][2])*invdet;
	result[0][2] =  (M[1][0]*M[2][1]-M[2][0]*M[1][1])*invdet;
	result[1][2] = -(M[0][0]*M[2][1]-M[2][0]*M[0][1])*invdet;
	result[2][2] =  (M[0][0]*M[1][1]-M[1][0]*M[0][1])*invdet;

	return result;
}

mat4 inverseTranspose(mat4 M)
{
	mat4 result;
	float determinant =    +M[0][0]*(M[1][1]*M[2][2]-M[2][1]*M[1][2])
	                        -M[0][1]*(M[1][0]*M[2][2]-M[1][2]*M[2][0])
                        +M[0][2]*(M[1][0]*M[2][1]-M[1][1]*M[2][0]);
	float invdet = 1.0 / determinant;
	result[0][0] =  (M[1][1]*M[2][2]-M[2][1]*M[1][2])*invdet;
	result[1][0] = -(M[0][1]*M[2][2]-M[0][2]*M[2][1])*invdet;
	result[2][0] =  (M[0][1]*M[1][2]-M[0][2]*M[1][1])*invdet;
	result[0][1] = -(M[1][0]*M[2][2]-M[1][2]*M[2][0])*invdet;
	result[1][1] =  (M[0][0]*M[2][2]-M[0][2]*M[2][0])*invdet;
	result[2][1] = -(M[0][0]*M[1][2]-M[1][0]*M[0][2])*invdet;
	result[0][2] =  (M[1][0]*M[2][1]-M[2][0]*M[1][1])*invdet;
	result[1][2] = -(M[0][0]*M[2][1]-M[2][0]*M[0][1])*invdet;
	result[2][2] =  (M[0][0]*M[1][1]-M[1][0]*M[0][1])*invdet;

        result[3][0] = 0;
        result[3][1] = 0;
        result[3][2] = 0;
        result[3][3] = 0;
        result[0][3] = 0;
        result[1][3] = 0;
        result[2][3] = 0;
	return result;
}

//mat4 inverseTranspose(mat4 M)
//{
//	mat3 M1inv = inverseTranspose(mat3(M));
//	
//	return mat4(M1inv);
//}

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
  //color = gl_Color;
  color = gl_FrontMaterial.diffuse;

  // Number of influencing handles
  int m = int(min(num_handles,MAX_NUM_WEIGHTS_PER_VERTEX));

  //////////////////////////////////////////////////////////////////////////
  // LBS
  //////////////////////////////////////////////////////////////////////////  

  mat4 blend = mat4(0.0);
  for(int k = 0;k<4;k++)
  {
    // influencing handle index
    int i = int(weight_indices0[k]);    
	blend += handle_transformation[i] * weights0[k];
  }

#if MAX_NUM_WEIGHTS_PER_VERTEX > 4
  // Next 4
  for(int k = 0;k<4;k++)
  {
    // influencing handle index
    int i = int(weight_indices4[k]);
	blend += handle_transformation[i] * weights4[k];
  }
#endif

#if MAX_NUM_WEIGHTS_PER_VERTEX > 8
  // Next 4  
  for(int k = 0;k<4;k++)
  {
    // influencing handle index
    int i = int(weight_indices8[k]);
	blend += handle_transformation[i] * weights8[k];
  }
#endif

#if MAX_NUM_WEIGHTS_PER_VERTEX > 12
  // Next 4
   for(int k = 0;k<4;k++)
  {
    // influencing handle index
    int i = int(weight_indices12[k]);
	blend += handle_transformation[i] * weights12[k];
  }
#endif

  vec4 v = blend * gl_Vertex;
#  ifdef TRANSFORM_NORMALS
  mat4 blendIT = inverseTranspose(blend);
  vec4 n = blendIT * vec4(gl_Normal.xyz,0);
  //vec4 n = blend * vec4(gl_Normal.xyz,0);
  n = vec4(normalize(n.xyz), 0);
#  endif

	v.x += num_handles; v.x -= num_handles; // to prevent error of setting uniform...

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
  //normal = n;
  //lightDir = normalize(vec3(gl_LightSource[0].position));
  //halfVector = normalize(gl_LightSource[0].halfVector.xyz);
  ////diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;
  //diffuse = color * gl_LightSource[0].diffuse;
  //ambient = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;
  ////ambient = gl_Color * gl_LightSource[0].ambient;
  //ambient += gl_LightModel.ambient * gl_FrontMaterial.ambient;
  //// Tell fragment shader what the color is
  //gl_FrontColor = color;

  //n = vec4(normalize(gl_NormalMatrix * gl_Normal), 0.0);

  gl_FrontColor = gl_LightModel.ambient * gl_FrontMaterial.ambient;

/*
	vec3 lightDir1 = vec3(-1.0, 0.0, 0.0);
	vec3 lightDir2 = vec3(1.0, 0.0, 0.0);    
    float NdotL1 = max(dot(n.xyz,lightDir1), 0.0);  
	float NdotL2 = max(dot(n.xyz,lightDir2), 0.0);      
    gl_FrontColor += 
      color*gl_LightSource[0].diffuse*NdotL1 + 
	  color*gl_LightSource[0].diffuse*NdotL2;	  
*/
  
  for (int light=0; light<2; light++)
  {
    lightDir = normalize(vec3(gl_LightSource[light].position));
    halfVector = normalize(gl_LightSource[light].halfVector.xyz);
    float NdotL = max(dot(n.xyz,lightDir), 0.0);  
    float NdotHV = max(dot(n.xyz,halfVector),0.0);
    gl_FrontColor += 
      color*gl_LightSource[light].diffuse*NdotL + 
	  gl_FrontMaterial.specular * gl_LightSource[light].specular * pow(NdotHV, gl_FrontMaterial.shininess);
  }

	//mat4 tst = transpose(blend) * blendIT;
	//gl_FrontColor = vec4(tst[0]);

	//gl_FrontColor = n;
} 
