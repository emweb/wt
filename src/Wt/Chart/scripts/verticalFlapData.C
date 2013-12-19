/* For the vertical axis-flaps */
#include <iostream>
#include <fstream>
#include <stdio.h>
using namespace std;

#include "flapConstants.h"
#include "v3math.h"
#include "CompositeTexture.h"
using namespace Wt;

const int planes = 4;
const int ptsPerPlane = 4;
const int faces = 4;

v3 backVertexPosV[planes][ptsPerPlane];
v3 leftVertexPosV[planes][ptsPerPlane];
v3 frontVertexPosV[planes][ptsPerPlane];
v3 rightVertexPosV[planes][ptsPerPlane];
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
  //  myfile.open ("flapVertexPos.txt", ios::app);
  myfile.open ("verticalFlapData.cpp", ios::app);
  myfile << "const float axisSlabDataVertical[] = {";
  for (int i=0; i<planes; i++) {
    for (int j=0; j<ptsPerPlane; j++) {
      myfile << backVertexPosV[i][j].x << ", " << backVertexPosV[i][j].y
	     << ", " << backVertexPosV[i][j].z << ", ";
    }
  }
  for (int i=0; i<planes; i++) {
    for (int j=0; j<ptsPerPlane; j++) {
      myfile << leftVertexPosV[i][j].x << ", " << leftVertexPosV[i][j].y
	     << ", " << leftVertexPosV[i][j].z << ", ";
    }
  }
  for (int i=0; i<planes; i++) {
    for (int j=0; j<ptsPerPlane; j++) {
      myfile << frontVertexPosV[i][j].x << ", " << frontVertexPosV[i][j].y
	     << ", " << frontVertexPosV[i][j].z << ", ";
    }
  }
  for (int i=0; i<planes; i++) {
    for (int j=0; j<ptsPerPlane; j++) {
      if (i == planes-1 && j == ptsPerPlane-1)
	break;
      myfile << rightVertexPosV[i][j].x << ", " << rightVertexPosV[i][j].y
	     << ", " << rightVertexPosV[i][j].z << ", ";
    }
  }
  myfile << rightVertexPosV[planes-1][ptsPerPlane-1].x << ", " << rightVertexPosV[planes-1][ptsPerPlane-1].y << ", " << rightVertexPosV[planes-1][ptsPerPlane-1].z << "};\n";
  // myfile.close();

  // myfile.open("flapVertexInPlaneNormals.txt", ios::app);
  myfile << "const float axisPlaneNormalVertical[] = {";
  for (int i=0; i<faces; i++) {
    for (int j=0; j<planes; j++) {
      for (int k=0; k<ptsPerPlane; k++) {
	if (i == faces-1 && j == planes -1 && k == ptsPerPlane -1)
	  break;
	myfile << inPlaneNormals[i][j].x << ", " << inPlaneNormals[i][j].y 
	       << ", " << inPlaneNormals[i][j].z << ", ";
      }
    }
  }
  myfile << inPlaneNormals[faces-1][planes -1].x << ", " << inPlaneNormals[faces-1][planes -1].y << ", " << inPlaneNormals[faces-1][planes -1].z << "};\n";
  // myfile.close();

  // myfile.open("flapVertexOutOfPlaneNormals.txt", ios::app);
  myfile << "const float axisOutOfPlaneNormalVertical[] = {";
  for (int i=0; i<faces; i++) {
    for (int j=0; j<planes; j++) {
      for (int k=0; k<ptsPerPlane; k++) {
	if (i == faces-1 && j == planes -1 && k == ptsPerPlane -1)
	  break;
	myfile << outOfPlaneNormals[i][j].x << ", " <<outOfPlaneNormals[i][j].y 
	       << ", " << outOfPlaneNormals[i][j].z << ", ";
      }
    }
  }
  myfile << outOfPlaneNormals[faces-1][planes -1].x << ", " << outOfPlaneNormals[faces-1][planes -1].y << ", " << outOfPlaneNormals[faces-1][planes -1].z << "};\n";
  // myfile.close();

  // myfile.open("flapVertexInPlaneBools.txt", ios::app);
  myfile << "const float axisInPlaneBoolsVertical[] = {";
  for (int i=0; i<faces; i++) {
    for (int j=0; j<planes; j++) {
      for (int k=0; k<ptsPerPlane; k++) {
	if (i == faces-1 && j == planes -1 && k == ptsPerPlane -1)
	  break;
	myfile << inPlaneBools[i][j] << ", ";
      }
    }
  }
  myfile << inPlaneBools[faces-1][planes -1] << "};\n";

  myfile << "const int axisSlabIndicesVertical[] = {";
  int idx;
  for (int i=0; i<planes*faces; i++) {
    idx = 4*i;
    for (int j=0; j<2; j++){
      if (i == planes*faces-1 && j == 1)
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

int main() {
  remove("verticalFlapData.cpp");

  // define texture
  // CompositeTexture axesTexture("vertexTexCoords.txt");
  CompositeTexture axesTexture;
  for (int i=0; i < 2; i++) {
    axesTexture.defineSubTexture(CompositeTexture::vec2(0.0 + i/2.0, 1.0),
				 CompositeTexture::vec2(0.0 + i/2.0, 0.0),
				 CompositeTexture::vec2(0.5 + i/2.0, 0.0),
				 CompositeTexture::vec2(0.5 + i/2.0, 1.0));
  }

  // Vertices of the BACK PLANE
  // here, the 4 flaps are defined, they will then be rotated for
  // the other faces
  put(backVertexPosV[0][0], -size, 1.0 + extension, 0.0);
  put(backVertexPosV[0][1], -size, -extension, 0.0);
  put(backVertexPosV[0][2], 0.0, -extension, 0.0);
  put(backVertexPosV[0][3], 0.0, 1.0 + extension, 0.0);
  axesTexture.addCoords(0);
  
  put(backVertexPosV[1][0], 0.0, 1.0 + extension, -size);
  put(backVertexPosV[1][1], 0.0, -extension, -size);
  put(backVertexPosV[1][2], 0.0, -extension, 0.0);
  put(backVertexPosV[1][3], 0.0, 1.0 + extension, 0.0);
  axesTexture.addCoords(0);

  put(backVertexPosV[2][0], 1.0, 1.0 + extension, 0.0);
  put(backVertexPosV[2][1], 1.0, -extension, 0.0);
  put(backVertexPosV[2][2], 1.0 + size, -extension, 0.0);
  put(backVertexPosV[2][3], 1.0 + size, 1.0 + extension, 0.0);
  axesTexture.addCoords(1);

  put(backVertexPosV[3][0], 1.0, 1.0 + extension, 0.0);
  put(backVertexPosV[3][1], 1.0, -extension, 0.0);
  put(backVertexPosV[3][2], 1.0, -extension, -size);
  put(backVertexPosV[3][3], 1.0, 1.0 + extension, -size);
  axesTexture.addCoords(1);
  for (int i=0; i<planes; i++) {
    put(inPlaneNormals[0][i], 0.0, 0.0, 1.0);
  }
  put(outOfPlaneNormals[0][0], -1.0, 0.0, 0.0);
  put(outOfPlaneNormals[0][1], -1.0, 0.0, 0.0);
  put(outOfPlaneNormals[0][2], 1.0, 0.0, 0.0);
  put(outOfPlaneNormals[0][3], 1.0, 0.0, 0.0);
  inPlaneBools[0][0] = 1.0; inPlaneBools[0][1] = 0.0;
  inPlaneBools[0][2] = 1.0; inPlaneBools[0][3] = 0.0;


  // vertices of the LEFT PLANE
  for (int i=0; i<planes; i++) {
    for (int j=0; j<ptsPerPlane; j++) {
      leftVertexPosV[i][j] = yTurnAndTranslate(backVertexPosV[i][j],0,0,1);
    }
  }
  axesTexture.addCoords(0);axesTexture.addCoords(0);
  axesTexture.addCoords(1);axesTexture.addCoords(1);
  for (int i=0; i<planes; i++) {
    put(inPlaneNormals[1][i], 1.0, 0.0, 0.0);
  }
  put(outOfPlaneNormals[1][0], 0.0, 0.0, 1.0);
  put(outOfPlaneNormals[1][1], 0.0, 0.0, 1.0);
  put(outOfPlaneNormals[1][2], 0.0, 0.0, -1.0);
  put(outOfPlaneNormals[1][3], 0.0, 0.0, -1.0);
  inPlaneBools[1][0] = 1.0; inPlaneBools[1][1] = 0.0;
  inPlaneBools[1][2] = 1.0; inPlaneBools[1][3] = 0.0;

  // vertices of the FRONT PLANE
  v3 tmp;
  for (int i=0; i<planes; i++) {
    for (int j=0; j<ptsPerPlane; j++) {
      tmp = yTurnAndTranslate(backVertexPosV[i][j],0,0,0);
      frontVertexPosV[i][j] = yTurnAndTranslate(tmp,1,0,1);
    }
  }
  axesTexture.addCoords(0);axesTexture.addCoords(0);
  axesTexture.addCoords(1);axesTexture.addCoords(1);
  for (int i=0; i<planes; i++) {
    put(inPlaneNormals[2][i], 0.0, 0.0, -1.0);
  }
  put(outOfPlaneNormals[2][0], 1.0, 0.0, 0.0);
  put(outOfPlaneNormals[2][1], 1.0, 0.0, 0.0);
  put(outOfPlaneNormals[2][2], -1.0, 0.0, 0.0);
  put(outOfPlaneNormals[2][3], -1.0, 0.0, 0.0);
  inPlaneBools[2][0] = 1.0; inPlaneBools[2][1] = 0.0;
  inPlaneBools[2][2] = 1.0; inPlaneBools[2][3] = 0.0;

  // vertices of the RIGHT PLANE
  for (int i=0; i<planes; i++) {
    for (int j=0; j<ptsPerPlane; j++) {
      rightVertexPosV[i][j] = yCounterTurnAndTranslate(backVertexPosV[i][j],
						       1,0,0);
    }
  }
  axesTexture.addCoords(0);axesTexture.addCoords(0);
  axesTexture.addCoords(1);axesTexture.addCoords(1);
  for (int i=0; i<planes; i++) {
    put(inPlaneNormals[3][i], -1.0, 0.0, 0.0);
  }
  put(outOfPlaneNormals[3][0], 0.0, 0.0, -1.0);
  put(outOfPlaneNormals[3][1], 0.0, 0.0, -1.0);
  put(outOfPlaneNormals[3][2], 0.0, 0.0, 1.0);
  put(outOfPlaneNormals[3][3], 0.0, 0.0, 1.0);
  inPlaneBools[3][0] = 1.0; inPlaneBools[3][1] = 0.0;
  inPlaneBools[3][2] = 1.0; inPlaneBools[3][3] = 0.0;

  writeAll();
  axesTexture.writeToFile("verticalFlapData.cpp", "axisTexCoVertical");

  return 0;
}
