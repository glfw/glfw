void main( void )
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord[0] = gl_MultiTexCoord0;

    gl_FrontColor = vec4(1.0, 1.0, 1.0, 1.0);
}
