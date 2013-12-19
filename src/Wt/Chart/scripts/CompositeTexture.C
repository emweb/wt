#include "CompositeTexture.h"

namespace Wt {

void CompositeTexture::defineSubTexture(CompositeTexture::vec2 v0,
					CompositeTexture::vec2 v1,
					CompositeTexture::vec2 v2,
					CompositeTexture::vec2 v3)
{
  subTexture tex;
  tex.push_back(v0);tex.push_back(v1);tex.push_back(v2);tex.push_back(v3);
  texture_.push_back(tex);
}

void CompositeTexture::addCoords(int texIndex,
				 int first,
				 int amount)
{
  if (texIndex >= texture_.size())
    return;

  texCoords texCo;
  texCo.texIndex = texIndex;
  texCo.first = first;
  texCo.amount = amount;

  texCoordBuffer_.push_back(texCo);
}

#if 0
void CompositeTexture::writeToFile(int texIndex,
				   int first,
				   int amount,
				   bool eof)
{
  if (texIndex >= texture_.size())
    return;

  ofstream myfile;
  myfile.open(filename_.c_str(), ios::app);

  subTexture tex = texture_[texIndex];
  for (int i=0; i < tex.size() - 1; i++) {
    myfile << tex[i].x << ", " << tex[i].y << ", ";
  }
  myfile << tex[tex.size() - 1].x << ", " << tex[tex.size() - 1].y;
  if (!eof) {
    myfile << ", ";
  } else {
    myfile << "};\n";
  }

  myfile.close();
}
#endif

void CompositeTexture::writeToFile(std::string filename, std::string varName)
{
  ofstream myfile;
  myfile.open(filename.c_str(), ios::app);
  myfile << "const float " << varName.c_str() << "[] = {";

  int texIndex, amount;
  for (int k=0; k < texCoordBuffer_.size()-1; k++) {
    texIndex = texCoordBuffer_[k].texIndex;
    subTexture tex = texture_[texIndex];
    for (int i=0; i < tex.size(); i++) {
      myfile << tex[i].x << ", " << tex[i].y << ", ";
    }
  }
  subTexture tex = texture_[texCoordBuffer_[texCoordBuffer_.size()-1].texIndex];
  for (int i=0; i < tex.size() - 1; i++) {
    myfile << tex[i].x << ", " << tex[i].y << ", ";
  }
  myfile << tex[tex.size() - 1].x << ", " << tex[tex.size() - 1].y << "};\n";

  myfile.close();
}

}
