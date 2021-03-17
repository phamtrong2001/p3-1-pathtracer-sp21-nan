#include "bvh.h"

#include "CGL/CGL.h"
#include "triangle.h"

#include <iostream>
#include <stack>

using namespace std;

namespace CGL {
namespace SceneObjects {

BVHAccel::BVHAccel(const std::vector<Primitive *> &_primitives,
                   size_t max_leaf_size) {

  primitives = std::vector<Primitive *>(_primitives);
  root = construct_bvh(primitives.begin(), primitives.end(), max_leaf_size);
}

BVHAccel::~BVHAccel() {
  if (root)
    delete root;
  primitives.clear();
}

BBox BVHAccel::get_bbox() const { return root->bb; }

void BVHAccel::draw(BVHNode *node, const Color &c, float alpha) const {
  if (node->isLeaf()) {
    for (auto p = node->start; p != node->end; p++) {
      (*p)->draw(c, alpha);
    }
  } else {
    draw(node->l, c, alpha);
    draw(node->r, c, alpha);
  }
}

void BVHAccel::drawOutline(BVHNode *node, const Color &c, float alpha) const {
  if (node->isLeaf()) {
    for (auto p = node->start; p != node->end; p++) {
      (*p)->drawOutline(c, alpha);
    }
  } else {
    drawOutline(node->l, c, alpha);
    drawOutline(node->r, c, alpha);
  }
}

BVHNode *BVHAccel::construct_bvh(std::vector<Primitive *>::iterator start,
                                 std::vector<Primitive *>::iterator end,
                                 size_t max_leaf_size) {

  // TODO (Part 2.1):
  // Construct a BVH from the given vector of primitives and maximum leaf
  // size configuration. The starter code build a BVH aggregate with a
  // single leaf node (which is also the root) that encloses all the
  // primitives.
    
    struct less_than_x
    {
        inline bool operator() (const Primitive* prim1, const Primitive* prim2)
        {
            return (prim1->get_bbox().centroid().x < prim2->get_bbox().centroid().x );
        }
    };
    struct less_than_y
    {
        inline bool operator() (const Primitive* prim1, const Primitive* prim2)
        {
            return (prim1->get_bbox().centroid().y < prim2->get_bbox().centroid().y );
        }
    };
    struct less_than_z
    {
        inline bool operator() (const Primitive* prim1, const Primitive* prim2)
        {
            return (prim1->get_bbox().centroid().z < prim2->get_bbox().centroid().z );
        }
    };

    
    float minx = 10000000;
    float miny = 10000000;
    float minz = 10000000;
    float maxx = -1000000;
    float maxy = -1000000;
    float maxz = -1000000;
  BBox bbox;
  int objects = 0;
  Vector3D avgCentroid;
  for (auto p = start; p != end; p++) {
    BBox bb = (*p)->get_bbox();
    bbox.expand(bb);
      objects++;
      Vector3D center = bb.centroid();
      avgCentroid = avgCentroid + center;
      minx = ( center.x < minx ? center.x: minx );
      miny = ( center.y < miny ? center.y: miny );
      minz = ( center.z < minz ? center.z: minz );
      maxx = ( center.x > maxx ? center.x: maxx );
      maxy = ( center.y > maxy ? center.y: maxy );
      maxz = ( center.z > maxz ? center.z: maxz );
  }
    int axis;
    if(maxx-minx > maxy-miny && maxx-minx > maxz-minz){
        axis = 0;
        std::sort(start, end, less_than_x());
    }
    if(maxy-miny > maxx-minx && maxy-miny > maxz-minz){
        axis = 1;
        std::sort(start, end, less_than_y());
    }
    if(maxz-minz > maxy-miny && maxz-minz > maxx-minx){
        axis = 2;
        std::sort(start, end, less_than_z());
    }
  avgCentroid = avgCentroid * 1/objects;
  BVHNode *node = new BVHNode(bbox);
    if(objects<max_leaf_size){
        node->l = NULL;
        node->r = NULL;
        node->start = start;
        node->end = end;
    }
    else{
        std::vector<Primitive *>::iterator lstart = start;
        std::vector<Primitive *>::iterator rend = end;
        std::vector<Primitive *>::iterator lend;
        std::vector<Primitive *>::iterator rstart;
        float maxAxis = -111111111.1;
        float minAxis = 111111111.1;
        float centerAxis;
        auto q = start;
        while((*q)->get_bbox().centroid()[axis]<avgCentroid[axis]){
            lend = q;
            q++;
            }
        
        if(lstart!=lend && lend !=rend){
            node->l = construct_bvh(lstart,lend,max_leaf_size);
            node->r = construct_bvh(lend,rend,max_leaf_size);}
        else{
            node->l = NULL;
            node->r = NULL;
            node->start = start;
            node->end = end;
        }
        
    }

  return node;


}

bool BVHAccel::has_intersection(const Ray &ray, BVHNode *node) const {
  // TODO (Part 2.3):
  // Fill in the intersect function.
  // Take note that this function has a short-circuit that the
  // Intersection version cannot, since it returns as soon as it finds
  // a hit, it doesn't actually have to find the closest hit.



  for (auto p : primitives) {
    total_isects++;
    if (p->has_intersection(ray))
      return true;
  }
  return false;


}

bool BVHAccel::intersect(const Ray &ray, Intersection *i, BVHNode *node) const {
  // TODO (Part 2.3):
  // Fill in the intersect function.

    if(node->bb.intersect(ray, ray.min_t, ray.max_t)==false){
        return false;
    }
    bool hit = false;
    if(node->isLeaf()){
        for (auto p=node->start; p!=node->end; p++) {
          total_isects++;
          hit = (*p)->intersect(ray, i) || hit;
        }
    }
    else{
        bool hit1 = intersect(ray, i, node->l);
        bool hit2 = intersect(ray, i, node->r);
        hit = hit1 || hit2;

    }
  return hit;

//  bool hit = false;
//  for (auto p : primitives) {
//    total_isects++;
//    hit = p->intersect(ray, i) || hit;
//  }
//  return hit;


}

} // namespace SceneObjects
} // namespace CGL
