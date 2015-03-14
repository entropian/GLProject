#include <stdio.h>
#include <stdlib.h>
#include "vec.h"

int main()
{
    vec2 v2a = {.x = 1, .y = 2};
    vec2 v2b = {.x = 1, .y = 2};
    vec2 *v2c = malloc(sizeof(vec2));
    vec2Sub(v2c, &v2a, &v2b);
    
    vec3 v3a = {.x = 1, .y = 2, .z = 3};
    vec3 v3b = {.x = 1, .y = 2, .z = 3};
    vec3 *v3c = malloc(sizeof(vec3));
    vec3Sub(v3c, &v3a, &v3b);
    
    vec4 v4a = {.x = 1, .y = 2, .z = 3, .a = 4};
    vec4 v4b = {.x = 1, .y = 2, .z = 3, .a = 4};
    vec4 *v4c = malloc(sizeof(vec4));
    vec4Sub(v4c, &v4a, &v4b);

    printf("a: %f %f\n", v2c->x, v2c->y);
    printf("b: %f %f %f\n", v3c->x, v3c->y, v3c->z);
    printf("c: %f %f %f %f\n", v4c->x, v4c->y, v4c->z, v4c->a);
	while (1)
	{
	}
}
