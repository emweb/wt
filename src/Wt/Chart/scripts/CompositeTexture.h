#include <vector>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

namespace Wt {

class CompositeTexture {
public:
  struct vec2 {
    double x, y;

    vec2(double _x, double _y)
      : x(_x), y(_y)
    {}
  };
  typedef std::vector<vec2> subTexture;

  void defineSubTexture(CompositeTexture::vec2 v0, CompositeTexture::vec2 v1,
			CompositeTexture::vec2 vec2, CompositeTexture::vec2 v3);
  /* void defineSubTexture(std::vector<vec2> corners); */
  
  void addCoords(int texIndex,
		 int first = 0,
		 int amount = 1);

  void writeToFile(std::string filename,
		   std::string varName);
  
private:
  struct texCoords {
    int texIndex;
    int first;
    int amount;
  };

  std::vector<subTexture> texture_;
  std::vector<texCoords> texCoordBuffer_;
};

}
