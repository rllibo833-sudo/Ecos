#include "ecos_studio/material.hpp"
#include <cmath>
#include <iostream>
namespace {
int failures=0;
void check(bool v,const char* m){if(!v){std::cerr<<"FAIL: "<<m<<'\n';++failures;}}
bool near(float a,float b){return std::abs(a-b)<1.0e-5F;}
void test_material_properties(){
 using namespace ecos::studio;
 auto parsed=import_mtl_materials(R"MTL(# sample
newmtl painted_metal
Kd 0.2 0.4 0.8
Ns 48
d 0.75
illum 2
newmtl glass
Kd 0.9 0.95 1.0
Tr 0.2
)MTL");
 check(parsed&&parsed.value().size()==2,"parses multiple MTL materials");
 if(parsed){
  const auto& metal=parsed.value()[0];
  check(metal.name=="painted_metal","preserves material name");
  check(near(metal.diffuse_color.y,0.4F)&&near(metal.diffuse_color.z,0.8F),"parses diffuse color");
  check(near(metal.shininess,48)&&near(metal.opacity,0.75F)&&metal.illumination_model==2,"parses shininess, opacity, and illum");
  check(near(parsed.value()[1].opacity,0.8F),"converts Tr transparency to opacity");
 }
}
void test_invalid_materials(){
 using namespace ecos::studio;
 check(!import_mtl_materials("Kd 1 1 1\nnewmtl later\n"),"rejects properties before newmtl");
 check(!import_mtl_materials("newmtl same\nnewmtl same\n"),"rejects duplicate material names");
 check(!import_mtl_materials("newmtl bad\nKd -1 0 0\n"),"rejects negative diffuse values");
 check(!import_mtl_materials("newmtl bad\nd 1.5\n"),"rejects opacity outside [0,1]");
 check(!import_mtl_materials("# no materials\n"),"rejects empty library");
}
}
int main(){test_material_properties();test_invalid_materials();if(failures){std::cerr<<failures<<" material test(s) failed.\n";return 1;}std::cout<<"ECOS Studio MTL import tests passed.\n";return 0;}
