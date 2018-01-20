#include "Material.h"

Material::Material() { color = Vector3(0, 0, 0);  type = 1; }
Material::Material(Vector3 color, int type) : color(color), type(type) {}
