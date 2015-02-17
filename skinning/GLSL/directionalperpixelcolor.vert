varying vec4 diffuse,ambient;
varying vec3 normal;//,lightDir,halfVector;

void main()
{	
    normal = normalize(gl_NormalMatrix * gl_Normal);
    //lightDir = normalize(vec3(gl_LightSource[0].position));
	
    //halfVector = normalize(gl_LightSource[0].halfVector.xyz);
    
    diffuse = vec4(0,0,0,0);
    ambient = vec4(0,0,0,0);
    //diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;
    const float num_lights = 2.0;
    for (int light=0; light<int(num_lights); light++)
    {
      diffuse += (1.0/num_lights)*(gl_Color * gl_LightSource[light].diffuse);
      ambient += (1.0/num_lights)*(gl_FrontMaterial.ambient * gl_LightSource[light].ambient);
      ambient += (1.0/num_lights)*(gl_LightModel.ambient * gl_FrontMaterial.ambient);
    }
	
    gl_Position = ftransform();
} 
