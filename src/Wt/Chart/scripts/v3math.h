
struct v3 {
  double x;
  double y;
  double z;
};

// Turns 90 degrees around x
v3 xTurnAndTranslate(v3 point,
		     double xOffset,
		     double yOffset,
		     double zOffset)
{
  v3 result;
  result.x =   point.x + xOffset;
  result.y = - point.z + yOffset;
  result.z =   point.y + zOffset;
  return result;
}

// Turns -90 degrees around x
v3 xCounterTurnAndTranslate(v3 point,
			    double xOffset,
			    double yOffset,
			    double zOffset)
{
  v3 result;
  result.x =   point.x + xOffset;
  result.y =   point.z + yOffset;
  result.z = - point.y + zOffset;
  return result;
}

// Turns 90 degrees around Y
v3 yTurnAndTranslate(v3 point,
		     double xOffset,
		     double yOffset,
		     double zOffset)
{
  v3 result;
  result.x =   point.z + xOffset;
  result.y =   point.y + yOffset;
  result.z = - point.x + zOffset;
  return result;
}

// Turns -90 degrees around Y
v3 yCounterTurnAndTranslate(v3 point,
		     double xOffset,
		     double yOffset,
		     double zOffset)
{
  v3 result;
  result.x = - point.z + xOffset;
  result.y =   point.y + yOffset;
  result.z =   point.x + zOffset;
  return result;
}


// Turns 90 degrees around Z
v3 zTurnAndTranslate(v3 point,
		     double xOffset,
		     double yOffset,
		     double zOffset)
{
  v3 result;
  result.x = - point.y + xOffset;
  result.y =   point.x + yOffset;
  result.z =   point.z + zOffset;
  return result;
}
