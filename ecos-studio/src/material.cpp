#include "ecos_studio/material.hpp"
#include <charconv>
#include <cmath>
#include <sstream>
#include <string>
#include <utility>
namespace ecos::studio {
namespace {
bool parse_float(const std::string& text,float& value){
 if(text.empty())return false;
 const auto parsed=std::from_chars(text.data(),text.data()+text.size(),value);
 return parsed.ec==std::errc{}&&parsed.ptr==text.data()+text.size()&&std::isfinite(value);
}
bool parse_int(const std::string& text,int& value){
 if(text.empty())return false;
 const auto parsed=std::from_chars(text.data(),text.data()+text.size(),value);
 return parsed.ec==std::errc{}&&parsed.ptr==text.data()+text.size();
}
}
Result<std::vector<MaterialData>> import_mtl_materials(std::string_view source){
 std::vector<MaterialData> materials;
 MaterialData* current=nullptr;
 std::istringstream input{std::string(source)};
 std::string line;
 std::size_t line_number=0;
 while(std::getline(input,line)){
  ++line_number;
  std::istringstream row(line);
  std::string key;
  row>>key;
  if(key.empty()||key[0]=='#')continue;
  if(key=="newmtl"){
   std::string name;
   if(!(row>>name)||name.empty())return Result<std::vector<MaterialData>>::failure({ErrorCode::invalid_argument,"Invalid MTL material name at line "+std::to_string(line_number)});
   for(const auto& material:materials)if(material.name==name)
    return Result<std::vector<MaterialData>>::failure({ErrorCode::invalid_argument,"Duplicate MTL material '"+name+"'"});
   materials.push_back(MaterialData{});
   materials.back().name=name;
   current=&materials.back();
   continue;
  }
  if(key=="Kd"||key=="d"||key=="Tr"||key=="Ns"||key=="illum"){
   if(!current)return Result<std::vector<MaterialData>>::failure({ErrorCode::invalid_argument,"MTL property before newmtl at line "+std::to_string(line_number)});
   if(key=="Kd"){
    std::string sx,sy,sz;float x{},y{},z{};
    if(!(row>>sx>>sy>>sz)||!parse_float(sx,x)||!parse_float(sy,y)||!parse_float(sz,z)||x<0||y<0||z<0)
     return Result<std::vector<MaterialData>>::failure({ErrorCode::invalid_argument,"Invalid MTL diffuse color at line "+std::to_string(line_number)});
    current->diffuse_color={x,y,z};
   }else if(key=="d"||key=="Tr"){
    std::string value;float amount{};
    if(!(row>>value)||!parse_float(value,amount)||amount<0||amount>1)
     return Result<std::vector<MaterialData>>::failure({ErrorCode::invalid_argument,"Invalid MTL opacity/transparency at line "+std::to_string(line_number)});
    current->opacity=key=="Tr"?1.0F-amount:amount;
   }else if(key=="Ns"){
    std::string value;float amount{};
    if(!(row>>value)||!parse_float(value,amount)||amount<0)
     return Result<std::vector<MaterialData>>::failure({ErrorCode::invalid_argument,"Invalid MTL shininess at line "+std::to_string(line_number)});
    current->shininess=amount;
   }else{
    std::string value;int model{};
    if(!(row>>value)||!parse_int(value,model)||model<0)
     return Result<std::vector<MaterialData>>::failure({ErrorCode::invalid_argument,"Invalid MTL illumination model at line "+std::to_string(line_number)});
    current->illumination_model=model;
   }
  }
 }
 if(materials.empty())return Result<std::vector<MaterialData>>::failure({ErrorCode::invalid_argument,"MTL library contains no newmtl declarations"});
 return Result<std::vector<MaterialData>>::success(std::move(materials));
}
} // namespace ecos::studio
