#include <iostream>
#include <fstream>
#include <stdio.h>
using namespace std;

#include "flapConstants.h"
#include "v3math.h"
#include "CompositeTexture.h"
using namespace Wt;

const int planes = 8;
const int ptsPerPlane = 4;
const int faces = 6;

v3 bottomVertexPos[planes][ptsPerPlane];
v3 topVertexPos[planes][ptsPerPlane];
v3 backVertexPos[planes/2][ptsPerPlane];
v3 leftVertexPos[planes/2][ptsPerPlane];
v3 frontVertexPos[planes/2][ptsPerPlane];
v3 rightVertexPos[planes/2][ptsPerPlane];
v3 inPlaneNormals[faces][planes];
v3 outOfPlaneNormals[faces][planes];
double inPlaneBools[faces][planes];

namespace {
  void put(v3 &vertex,
	   double x, double y, double z) {
    vertex.x = x;
    vertex.y = y;
    vertex.z = z;
  }
}

void writeAll() {
  ofstream myfile;
  int planes = ::planes;
  myfile.open ("horizontalFlapData.cpp", ios::app);
  myfile << "const float axisSlabData[] = {";
  for (int i=0; i<planes; i++) {
    for (int j=0; j<ptsPerPlane; j++) {
      myfile << bottomVertexPos[i][j].x << ", " << bottomVertexPos[i][j].y
	     << ", " << bottomVertexPos[i][j].z << ", ";
    }
  }
  for (int i=0; i<planes; i++) {
    for (int j=0; j<ptsPerPlane; j++) {
      myfile << topVertexPos[i][j].x << ", " << topVertexPos[i][j].y
	     << ", " << topVertexPos[i][j].z << ", ";
    }
  }
  planes = ::planes/2;
  for (int i=0; i<planes; i++) {
    for (int j=0; j<ptsPerPlane; j++) {
      myfile << backVertexPos[i][j].x << ", " << backVertexPos[i][j].y
	     << ", " << backVertexPos[i][j].z << ", ";
    }
  }
  for (int i=0; i<planes; i++) {
    for (int j=0; j<ptsPerPlane; j++) {
      myfile << leftVertexPos[i][j].x << ", " << leftVertexPos[i][j].y
	     << ", " << leftVertexPos[i][j].z << ", ";
    }
  }
  for (int i=0; i<planes; i++) {
    for (int j=0; j<ptsPerPlane; j++) {
      myfile << frontVertexPos[i][j].x << ", " << frontVertexPos[i][j].y
	     << ", " << frontVertexPos[i][j].z << ", ";
    }
  }
  for (int i=0; i<planes; i++) {
    for (int j=0; j<ptsPerPlane; j++) {
      if (i == planes-1 && j == ptsPerPlane-1)
	break;
      myfile << rightVertexPos[i][j].x << ", " << rightVertexPos[i][j].y
	     << ", " << rightVertexPos[i][j].z << ", ";
    }
  }
  myfile << rightVertexPos[planes-1][ptsPerPlane-1].x << ", " << rightVertexPos[planes-1][ptsPerPlane-1].y << ", " << rightVertexPos[planes-1][ptsPerPlane-1].z << "};\n";
  // myfile.close();

  // myfile.open("flapVertexInPlaneNormals.txt", ios::app);
  myfile << "const float axisPlaneNormal[] = {";
  planes = ::planes;
  for (int i=0; i<faces; i++) {
    if (i == 2)
      planes = ::planes/2;
    for (int j=0; j<planes; j++) {
      for (int k=0; k<ptsPerPlane; k++) {
	if (i == faces-1 && j == planes-1 && k == ptsPerPlane-1)
	  break;
	myfile << inPlaneNormals[i][j].x << ", " << inPlaneNormals[i][j].y 
	       << ", " << inPlaneNormals[i][j].z << ", ";
      }
    }
  }
  myfile << inPlaneNormals[faces-1][planes-1].x << ", " << inPlaneNormals[faces-1][planes-1].y << ", " << inPlaneNormals[faces-1][planes-1].z << "};\n";
  // myfile.close();

  // myfile.open("flapVertexOutOfPlaneNormals.txt", ios::app);
  myfile << "const float axisOutOfPlaneNormal[] = {";
  planes = ::planes;
  for (int i=0; i<faces; i++) {
    if (i == 2)
      planes = ::planes/2;
    for (int j=0; j<planes; j++) {
      for (int k=0; k<ptsPerPlane; k++) {
	if (i == faces-1 && j == planes-1 && k == ptsPerPlane-1)
	  break;
	myfile << outOfPlaneNormals[i][j].x << ", " <<outOfPlaneNormals[i][j].y 
	       << ", " << outOfPlaneNormals[i][j].z << ", ";
      }
    }
  }
  myfile << outOfPlaneNormals[faces-1][planes-1].x << ", " <<outOfPlaneNormals[faces-1][planes-1].y << ", " << outOfPlaneNormals[faces-1][planes-1].z << "};\n";
  // myfile.close();

  // myfile.open("flapVertexInPlaneBools.txt", ios::app);
  myfile << "const float axisInPlaneBools[] = {";
  planes = ::planes;
  for (int i=0; i<faces; i++) {
    if (i == 2)
      planes = ::planes/2;
    for (int j=0; j<planes; j++) {
      for (int k=0; k<ptsPerPlane; k++) {
	if (i == faces-1 && j == planes-1 && k == ptsPerPlane-1)
	  break;
	myfile << inPlaneBools[i][j] << ", ";
      }
    }
  }
  myfile << inPlaneBools[faces-1][planes-1] << "};\n";

  myfile << "const int axisSlabIndices[] = {";
  int nbPlanes = 2*8 + 4*4;
  int idx;
  for (int i=0; i<nbPlanes; i++) {
    idx = 4*i;
    for (int j=0; j<2; j++){
      if (i == nbPlanes - 1 && j == 1)
	break;
      myfile << idx << ", ";
      myfile << idx+1+j << ", ";
      myfile << idx+2+j << ", ";
    }
  }
  myfile << idx << ", ";
  myfile << idx+2 << ", ";
  myfile << idx+3 << "};\n";

  myfile.close();
}


void generateSideFaceData(CompositeTexture& axesTexture) {
  int planes = ::planes/2;
  // vertices of the BACK PLANE
  put(backVertexPos[0][0], -extension, 0.0, 0.0);
  put(backVertexPos[0][1], -extension, -size, 0.0);
  put(backVertexPos[0][2], 1.0 + extension, -size, 0.0);
  put(backVertexPos[0][3], 1.0 + extension, 0.0, 0.0);
  axesTexture.addCoords(0);

  put(backVertexPos[1][0], -extension, 0.0, 0.0);
  put(backVertexPos[1][1], -extension, 0.0, -size);
  put(backVertexPos[1][2], 1.0 + extension, 0.0, -size);
  put(backVertexPos[1][3], 1.0 + extension, 0.0, 0.0);
  axesTexture.addCoords(0);

  put(backVertexPos[2][0], -extension, 1.0 + size, 0.0);
  put(backVertexPos[2][1], -extension, 1.0, 0.0);
  put(backVertexPos[2][2], 1.0 + extension, 1.0, 0.0);
  put(backVertexPos[2][3], 1.0 + extension, 1.0 + size, 0.0);
  axesTexture.addCoords(2);

  put(backVertexPos[3][0], -extension, 1.0, -size);
  put(backVertexPos[3][1], -extension, 1.0, 0.0);
  put(backVertexPos[3][2], 1.0 + extension, 1.0, 0.0);
  put(backVertexPos[3][3], 1.0 + extension, 1.0, -size);
  axesTexture.addCoords(2);
  for (int i=0; i<planes; i++) {
    put(inPlaneNormals[2][i], 0.0, 0.0, 1.0);
  }

  // vertices of the LEFT PLANE
  for (int i=0; i<planes; i++) {
    for (int j=0; j<ptsPerPlane; j++) {
      leftVertexPos[i][j] = yTurnAndTranslate(backVertexPos[i][j],0,0,1);
    }
  }
  axesTexture.addCoords(5);axesTexture.addCoords(5);
  axesTexture.addCoords(7);axesTexture.addCoords(7);
  for (int i=0; i<planes; i++) {
    put(inPlaneNormals[3][i], 1.0, 0.0, 0.0);
  }

  // vertices of the FRONT PLANE
  v3 tmp;
  for (int i=0; i<planes; i++) {
    for (int j=0; j<ptsPerPlane; j++) {
      tmp = yTurnAndTranslate(backVertexPos[i][j],0,0,0);
      frontVertexPos[i][j] = yTurnAndTranslate(tmp,1,0,1);
    }
  }
  axesTexture.addCoords(1);axesTexture.addCoords(1);
  axesTexture.addCoords(3);axesTexture.addCoords(3);
  for (int i=0; i<planes; i++) {
    put(inPlaneNormals[4][i], 0.0, 0.0, -1.0);
  }

  // vertices of the RIGHT PLANE
  for (int i=0; i<planes; i++) {
    for (int j=0; j<ptsPerPlane; j++) {
      rightVertexPos[i][j]=yCounterTurnAndTranslate(backVertexPos[i][j],1,0,0);
    }
  }
  axesTexture.addCoords(4);axesTexture.addCoords(4);
  axesTexture.addCoords(6);axesTexture.addCoords(6);
  for (int i=0; i<planes; i++) {
    put(inPlaneNormals[5][i], -1.0, 0.0, 0.0);
  }


  // same for every side-plane
  for (int i=2; i<faces; i++) {
    put(outOfPlaneNormals[i][0], 0.0, -1.0, 0.0);
    put(outOfPlaneNormals[i][1], 0.0, -1.0, 0.0);
    put(outOfPlaneNormals[i][2], 0.0, 1.0, 0.0);
    put(outOfPlaneNormals[i][3], 0.0, 1.0, 0.0);
    inPlaneBools[i][0] = 1.0;
    inPlaneBools[i][1] = 0.0;
    inPlaneBools[i][2] = 1.0;
    inPlaneBools[i][3] = 0.0;
  }
}

void generateBottomAndTopFaceData(CompositeTexture& axesTexture) {
  // Vertices of the BOTTOM PLANE
  // first in-plane
  /* orientation = the inside of the cube is the front-face of the plane,
     same goes for the in-plane flap, for the out-of-plane flap the side
     pointing away from the cube is the front-face. */
  put(bottomVertexPos[0][0], 1.0 + extension, 0.0, 0.0);
  put(bottomVertexPos[0][1], 1.0 + extension, 0.0, -size);
  put(bottomVertexPos[0][2], -extension, 0.0, -size);
  put(bottomVertexPos[0][3], -extension, 0.0, 0.0);
  axesTexture.addCoords(1);
  v3 tmp;
  for (int j=0; j<ptsPerPlane; j++) { // transform to get the rest
    bottomVertexPos[1][j] = yTurnAndTranslate(bottomVertexPos[0][j],0,0,1);
    tmp = bottomVertexPos[1][j]; tmp.z -= 1;
    bottomVertexPos[2][j] = yTurnAndTranslate(tmp,1,0,1);
    bottomVertexPos[3][j] = yCounterTurnAndTranslate(bottomVertexPos[0][j],
						     1,0,0);
  }
  axesTexture.addCoords(4);
  axesTexture.addCoords(0);
  axesTexture.addCoords(5);
  // first out-of-plane
  put(bottomVertexPos[4][0], 1.0 + extension, 0.0, 0.0);
  put(bottomVertexPos[4][1], 1.0 + extension, -size, 0.0);
  put(bottomVertexPos[4][2], -extension, -size, 0.0);
  put(bottomVertexPos[4][3], -extension, 0.0, 0.0);
  axesTexture.addCoords(1);
  for (int j=0; j<ptsPerPlane; j++) { // transform to get the rest
    bottomVertexPos[5][j] = yTurnAndTranslate(bottomVertexPos[4][j],0,0,1);
    tmp = bottomVertexPos[5][j]; tmp.z -= 1;
    bottomVertexPos[6][j] = yTurnAndTranslate(tmp,1,0,1);
    bottomVertexPos[7][j] = yCounterTurnAndTranslate(bottomVertexPos[4][j],
						     1,0,0);
  }
  axesTexture.addCoords(4);
  axesTexture.addCoords(0);
  axesTexture.addCoords(5);
  for (int i=0; i<8; i++) {
    put(inPlaneNormals[0][i], 0.0, 1.0, 0.0);
  }
  for (int i=0; i<2; i++) {
    put(outOfPlaneNormals[0][i*4 + 0], 0.0, 0.0, -1.0);
    put(outOfPlaneNormals[0][i*4 + 1], -1.0, 0.0, 0.0);
    put(outOfPlaneNormals[0][i*4 + 2], 0.0, 0.0, 1.0);
    put(outOfPlaneNormals[0][i*4 + 3], 1.0, 0.0, 0.0);
  }
  for (int i=0; i<4; i++) {
    inPlaneBools[0][i] = 1.0;
  }
  for (int i=4; i<8; i++) {
    inPlaneBools[0][i] = 0.0;
  }

  
  // Vertices of the TOP PLANE
  // first in-plane
  put(topVertexPos[0][0], 1.0 + extension, 1.0, -size);
  put(topVertexPos[0][1], 1.0 + extension, 1.0, 0.0);
  put(topVertexPos[0][2], -extension, 1.0, 0.0);
  put(topVertexPos[0][3], -extension, 1.0, -size);
  axesTexture.addCoords(3);
  for (int j=0; j<ptsPerPlane; j++) { // transform to get the rest
    topVertexPos[1][j] = yTurnAndTranslate(topVertexPos[0][j],0,0,1);
    tmp = topVertexPos[1][j]; tmp.z -= 1;
    topVertexPos[2][j] = yTurnAndTranslate(tmp,1,0,1);
    topVertexPos[3][j] = yCounterTurnAndTranslate(topVertexPos[0][j],
						  1,0,0);
  }
  axesTexture.addCoords(6);
  axesTexture.addCoords(2);
  axesTexture.addCoords(7);
  // first out-of-plane
  put(topVertexPos[4][0], 1.0 + extension, 1.0+size, 0.0);
  put(topVertexPos[4][1], 1.0 + extension, 1.0, 0.0);
  put(topVertexPos[4][2], -extension, 1.0, 0.0);
  put(topVertexPos[4][3], -extension, 1.0+size, 0.0);
  axesTexture.addCoords(3);
  for (int j=0; j<ptsPerPlane; j++) { // transform to get the rest
    topVertexPos[5][j] = yTurnAndTranslate(topVertexPos[4][j],0,0,1);
    tmp = topVertexPos[5][j]; tmp.z -= 1;
    topVertexPos[6][j] = yTurnAndTranslate(tmp,1,0,1);
    topVertexPos[7][j] = yCounterTurnAndTranslate(topVertexPos[4][j],
						  1,0,0);
  }
  axesTexture.addCoords(6);
  axesTexture.addCoords(2);
  axesTexture.addCoords(7);
  for (int i=0; i<8; i++) {
    put(inPlaneNormals[1][i], 0.0, -1.0, 0.0);
  }
  for (int i=0; i<2; i++) {
    put(outOfPlaneNormals[1][i*4 + 0], 0.0, 0.0, -1.0);
    put(outOfPlaneNormals[1][i*4 + 1], -1.0, 0.0, 0.0);
    put(outOfPlaneNormals[1][i*4 + 2], 0.0, 0.0, 1.0);
    put(outOfPlaneNormals[1][i*4 + 3], 1.0, 0.0, 0.0);
  }
  for (int i=0; i<4; i++) {
    inPlaneBools[1][i] = 1.0;
  }
  for (int i=4; i<8; i++) {
    inPlaneBools[1][i] = 0.0;
  }
}



int main() {
  remove("horizontalFlapData.cpp");

  // define texture (some clear zones are required because mipmapping blends the different subtextures together)
  CompositeTexture axesTexture;
  int i = 0; // buffer zone at bottom
  axesTexture.defineSubTexture(CompositeTexture::vec2(0.0, 1.0-i/8.0),
			       CompositeTexture::vec2(0.0, 0.875-i/8.0 + 0.001),
			       CompositeTexture::vec2(1.0, 0.875-i/8.0 + 0.001),
			       CompositeTexture::vec2(1.0, 1.0-i/8.0));
  for (int i=1; i < 3; i++) {
    axesTexture.defineSubTexture(CompositeTexture::vec2(0.0, 1.0-i/8.0),
				 CompositeTexture::vec2(0.0, 0.875-i/8.0),
				 CompositeTexture::vec2(1.0, 0.875-i/8.0),
				 CompositeTexture::vec2(1.0, 1.0-i/8.0));
  }
  i = 3; // buffer zone at top
  axesTexture.defineSubTexture(CompositeTexture::vec2(0.0, 1.0-i/8.0 - 0.001),
			       CompositeTexture::vec2(0.0, 0.875-i/8.0),
			       CompositeTexture::vec2(1.0, 0.875-i/8.0),
			       CompositeTexture::vec2(1.0, 1.0-i/8.0 - 0.001));
  i = 4; // buffer zone at bottom
  axesTexture.defineSubTexture(CompositeTexture::vec2(0.0, 1.0-i/8.0),
			       CompositeTexture::vec2(0.0, 0.875-i/8.0 + 0.001),
			       CompositeTexture::vec2(1.0, 0.875-i/8.0 + 0.001),
			       CompositeTexture::vec2(1.0, 1.0-i/8.0));
  for (int i=5; i < 7; i++) {
    axesTexture.defineSubTexture(CompositeTexture::vec2(0.0, 1.0-i/8.0),
				 CompositeTexture::vec2(0.0, 0.875-i/8.0),
				 CompositeTexture::vec2(1.0, 0.875-i/8.0),
				 CompositeTexture::vec2(1.0, 1.0-i/8.0));
  }
  i = 7; // buffer zone at top
  axesTexture.defineSubTexture(CompositeTexture::vec2(0.0, 1.0-i/8.0 - 0.001),
			       CompositeTexture::vec2(0.0, 0.875-i/8.0),
			       CompositeTexture::vec2(1.0, 0.875-i/8.0),
			       CompositeTexture::vec2(1.0, 1.0-i/8.0 - 0.001));
  

  generateBottomAndTopFaceData(axesTexture);
  generateSideFaceData(axesTexture);

  writeAll();
  axesTexture.writeToFile("horizontalFlapData.cpp", "axisTexCo");

  return 0;
}
