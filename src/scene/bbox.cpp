#include "bbox.h"

#include "GL/glew.h"

#include <algorithm>
#include <iostream>

namespace CGL {

bool BBox::intersect(const Ray& r, double& t0, double& t1) const {

  // TODO (Part 2.2):
  // Implement ray - bounding box intersection test
  // If the ray intersected the bouding box within the range given by
  // t0, t1, update t0 and t1 with the new intersection times.
    float mintx = (min.x - r.o.x)/r.d.x;
    float maxtx = (max.x - r.o.x)/r.d.x;
    if(mintx>maxtx){
        std::swap(mintx,maxtx);
    }
    float minty = (min.y - r.o.y)/r.d.y;
    float maxty = (max.y - r.o.y)/r.d.y;
    if(minty>maxty){
        std::swap(minty,maxty);
    }
    float mintz = (min.z - r.o.z)/r.d.z;
    float maxtz = (max.z - r.o.z)/r.d.z;
    if(mintz>maxtz){
        std::swap(mintz,maxtz);
    }

//    if(t0>t1){
//        std::swap(t0,t1);
//    }

    if( mintx<=maxty && minty <= maxtx){
        float mintxy = (mintx > minty ? mintx : minty);
        float maxtxy = (maxtx < maxty ? maxtx : maxty);
        if( mintxy<=maxtz && mintz <= maxtxy){
            float minbt = (mintxy > mintz ? mintxy : mintz);
            float maxbt = (maxtxy < maxtz ? maxtxy : maxtz);
            if( minbt<=r.max_t && r.min_t <= maxbt){
                return true;
            }
        }
        else{
            return false;
            }
    }
    else{
        return false;
    }
}

void BBox::draw(Color c, float alpha) const {

  glColor4f(c.r, c.g, c.b, alpha);

  // top
  glBegin(GL_LINE_STRIP);
  glVertex3d(max.x, max.y, max.z);
  glVertex3d(max.x, max.y, min.z);
  glVertex3d(min.x, max.y, min.z);
  glVertex3d(min.x, max.y, max.z);
  glVertex3d(max.x, max.y, max.z);
  glEnd();

  // bottom
  glBegin(GL_LINE_STRIP);
  glVertex3d(min.x, min.y, min.z);
  glVertex3d(min.x, min.y, max.z);
  glVertex3d(max.x, min.y, max.z);
  glVertex3d(max.x, min.y, min.z);
  glVertex3d(min.x, min.y, min.z);
  glEnd();

  // side
  glBegin(GL_LINES);
  glVertex3d(max.x, max.y, max.z);
  glVertex3d(max.x, min.y, max.z);
  glVertex3d(max.x, max.y, min.z);
  glVertex3d(max.x, min.y, min.z);
  glVertex3d(min.x, max.y, min.z);
  glVertex3d(min.x, min.y, min.z);
  glVertex3d(min.x, max.y, max.z);
  glVertex3d(min.x, min.y, max.z);
  glEnd();

}

std::ostream& operator<<(std::ostream& os, const BBox& b) {
  return os << "BBOX(" << b.min << ", " << b.max << ")";
}

} // namespace CGL
