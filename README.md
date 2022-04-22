# PGII_Rasterizer
DONE:
- weird WASD camera movement
- normal shader
- iradiance map
- weird pefiltered env map


TODO: http://mrl.cs.vsb.cz/people/fabian/pg2/pg2_2021.pdf
- complete BRDF shader
    - fix prefiltered env map
    - integration map
- normal (bumps) map
- hold and use Material structure from .mtl, use its roughness, reflectivity, metalicity...
- fix camera movement (WASD + mouse)
- shadows



WIP:

Materials
1. create Material class
2. use it with MaterialLibrary in Rasterizer
3. get roughness from fragment shader

BRDF shader
1. Irradiance map (done)
2. Prefiltered Environmental map (almost done)
3. Albedo
4. Integration Map
5. Fresnel
6. k_d * Ld + (k_s * sb.x + sb.y) * Lr;
7. PROFIT
