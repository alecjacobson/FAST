varying vec4 diffuse,ambient;
varying vec3 normal;//,lightDir,halfVector;

void main()
{
    vec3 n,halfV;
    float NdotL,NdotHV;
    
    vec4 color = ambient;
    
    n = normalize(normal);
    
  
    for (int light=0; light<2; light++)
    {
      vec3 lightDir = normalize(vec3(gl_LightSource[light].position));
      vec3 halfVector = gl_LightSource[light].halfVector.xyz;
      float NdotL = max(dot(n.xyz,lightDir), 0.0);  
      if (NdotL > 0.0) {
          color += diffuse * NdotL;
          halfV = normalize(halfVector);
          NdotHV = max(dot(n,halfV),0.0);
          color += gl_FrontMaterial.specular * gl_LightSource[light].specular * pow(NdotHV, gl_FrontMaterial.shininess);
      }
    }
	
    gl_FragColor = color;
}
