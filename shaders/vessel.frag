# version 330 core
// Do not modify the version directive to anything older than 330.

// Fragment shader inputs are outputs of the same name from vertex shader
in vec3 Color;
in vec3 Normal;

// Uniform variables
uniform vec3 light_direction;
uniform vec4 light_color = vec4(1);
uniform vec4 ambient = vec4(0.1, 0.1, 0.1, 1);
uniform vec4 diffuse = vec4(1, 0, 0, 1);

// Output
out vec4 fragColor;

void main (void) 
{        
	fragColor = ambient + light_color * diffuse * max(dot(light_direction, Normal), 0);
    // fragColor = ambient + light_color;
}