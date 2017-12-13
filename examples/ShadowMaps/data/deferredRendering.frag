uniform sampler2D tDiffuse; 
uniform sampler2D tPosition;
uniform sampler2D tNormals;
uniform sampler2D tShadowMap;
uniform vec3 cameraPosition;
uniform mat4 worldToLightViewMatrix;
uniform mat4 lightViewToProjectionMatrix;
uniform mat4 worldToCameraViewMatrix;

float readShadowMap(vec3 eyeDir)
{
	mat4 cameraViewToWorldMatrix = inverse(worldToCameraViewMatrix);
	mat4 cameraViewToProjectedLightSpace = lightViewToProjectionMatrix * worldToLightViewMatrix * cameraViewToWorldMatrix;
	vec4 projectedEyeDir = cameraViewToProjectedLightSpace * vec4(eyeDir,1);
	projectedEyeDir = projectedEyeDir/projectedEyeDir.w;

	vec2 textureCoordinates = projectedEyeDir.xy * vec2(0.5,0.5) + vec2(0.5,0.5);

	const float bias = 0.0001;
	float depthValue = texture2D( tShadowMap, textureCoordinates ) - bias;
	return projectedEyeDir.z * 0.5 + 0.5 < depthValue;
}

void main( void )
{
	// Read the data from the textures
	vec4 image = texture2D( tDiffuse, gl_TexCoord[0].xy );
	vec4 position = texture2D( tPosition, gl_TexCoord[0].xy );
	vec4 normal = texture2D( tNormals, gl_TexCoord[0].xy );
	
	mat4 lightViewToWolrdMatrix = inverse(worldToLightViewMatrix);
	vec3 light = lightViewToWolrdMatrix[3].xyz;
	vec3 lightDir = light - position.xyz;
	
	normal = normalize(normal);
	lightDir = normalize(lightDir);
	
	vec3 eyeDir = position.xyz - cameraPosition;
	vec3 reflectedEyeVector = normalize(reflect(eyeDir, normal));
	
	float shadow = readShadowMap(eyeDir);
	float diffuseLight = max(dot(normal,lightDir),0) * shadow;
	float ambientLight = 0.1;

	gl_FragColor = (diffuseLight + ambientLight ) * image + pow(max(dot(lightDir,reflectedEyeVector),0.0), 100) * 1.5 * shadow;
}
